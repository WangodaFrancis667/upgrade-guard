#include "upgrade_guard/ports/IProcessRunner.hpp"

#include <algorithm>
#include <array>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

namespace upgrade_guard::infrastructure {
namespace {

bool allowed(const std::string &name) {
  static const std::array<const char *, 8> names{"dpkg",   "dpkg-query", "apt-mark", "apt-get",
                                                 "uname",  "dkms",       "mokutil",  "true"};
  return std::find(names.begin(), names.end(), name) != names.end();
}

std::string resolve(const std::string &name) {
  if (name.find('/') != std::string::npos) {
    return name;
  }
  for (const auto *dir : {"/usr/bin/", "/bin/", "/usr/sbin/", "/sbin/"}) {
    std::string path = std::string(dir) + name;
    if (access(path.c_str(), X_OK) == 0) {
      return path;
    }
  }
  return {};
}

bool append_limited(std::string &target, const char *data, ssize_t count, std::size_t limit, bool &truncated) {
  if (count <= 0) {
    return false;
  }
  const auto current = target.size();
  const auto incoming = static_cast<std::size_t>(count);
  if (current + incoming <= limit) {
    target.append(data, incoming);
    return true;
  }
  if (current < limit) {
    target.append(data, limit - current);
  }
  truncated = true;
  return true;
}

std::vector<char *> make_argv(const std::string &path, const std::vector<std::string> &args) {
  std::vector<char *> argv;
  argv.reserve(args.size() + 2);
  argv.push_back(const_cast<char *>(path.c_str()));
  for (const auto &arg : args) {
    argv.push_back(const_cast<char *>(arg.c_str()));
  }
  argv.push_back(nullptr);
  return argv;
}

domain::Result<ports::ProcessResult> spawn_child(const ports::ProcessRequest &request, int out_pipe[2],
                                                 int err_pipe[2]) {
  const std::string path = resolve(request.executable);
  if (path.empty() || !allowed(request.executable)) {
    ports::ProcessResult result;
    result.spawn_failed = true;
    result.failure_message = "executable is unavailable or not allowed";
    return result;
  }

  const pid_t pid = fork();
  if (pid < 0) {
    return domain::Error{std::string("fork failed: ") + std::strerror(errno)};
  }
  if (pid == 0) {
    dup2(out_pipe[1], STDOUT_FILENO);
    dup2(err_pipe[1], STDERR_FILENO);
    close(out_pipe[0]);
    close(out_pipe[1]);
    close(err_pipe[0]);
    close(err_pipe[1]);

    auto argv = make_argv(path, request.arguments);
    std::vector<std::string> env_store{"LC_ALL=C", "PATH=/usr/sbin:/usr/bin:/sbin:/bin"};
    for (const auto &[key, value] : request.environment) {
      env_store.push_back(key + "=" + value);
    }
    std::vector<char *> envp;
    envp.reserve(env_store.size() + 1);
    for (auto &entry : env_store) {
      envp.push_back(entry.data());
    }
    envp.push_back(nullptr);
    execve(path.c_str(), argv.data(), envp.data());
    _exit(127);
  }
  ports::ProcessResult result;
  result.exit_code = static_cast<int>(pid);
  return result;
}

} // namespace

class PosixProcessRunner final : public ports::IProcessRunner {
public:
  [[nodiscard]] domain::Result<ports::ProcessResult> run(const ports::ProcessRequest &request) const override {
    int out_pipe[2]{};
    int err_pipe[2]{};
    if (pipe(out_pipe) != 0 || pipe(err_pipe) != 0) {
      return domain::Error{std::string("pipe failed: ") + std::strerror(errno)};
    }
    fcntl(out_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(err_pipe[0], F_SETFL, O_NONBLOCK);

    auto child = spawn_child(request, out_pipe, err_pipe);
    close(out_pipe[1]);
    close(err_pipe[1]);
    if (!child.ok()) {
      close(out_pipe[0]);
      close(err_pipe[0]);
      return child;
    }

    ports::ProcessResult result;
    if (child.value().spawn_failed) {
      close(out_pipe[0]);
      close(err_pipe[0]);
      return child.value();
    }
    const pid_t pid = static_cast<pid_t>(child.value().exit_code);
    auto deadline = std::chrono::steady_clock::now() + request.timeout;
    bool running = true;
    while (running) {
      std::array<pollfd, 2> fds{{{out_pipe[0], POLLIN, 0}, {err_pipe[0], POLLIN, 0}}};
      const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(deadline - std::chrono::steady_clock::now());
      const int wait_ms = remaining.count() > 0 ? static_cast<int>(remaining.count()) : 0;
      poll(fds.data(), fds.size(), wait_ms);

      std::array<char, 4096> buffer{};
      for (auto fd : {out_pipe[0], err_pipe[0]}) {
        for (;;) {
          const auto read_count = read(fd, buffer.data(), buffer.size());
          if (read_count <= 0) {
            break;
          }
          auto &target = fd == out_pipe[0] ? result.stdout_text : result.stderr_text;
          append_limited(target, buffer.data(), read_count, request.max_output_bytes, result.truncated);
        }
      }

      int status = 0;
      const pid_t done = waitpid(pid, &status, WNOHANG);
      if (done == pid) {
        result.exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : 128 + WTERMSIG(status);
        running = false;
      } else if (std::chrono::steady_clock::now() >= deadline) {
        kill(pid, SIGKILL);
        waitpid(pid, &status, 0);
        result.timed_out = true;
        result.exit_code = 124;
        running = false;
      }
    }
    close(out_pipe[0]);
    close(err_pipe[0]);
    return result;
  }
};

} // namespace upgrade_guard::infrastructure

std::unique_ptr<upgrade_guard::ports::IProcessRunner> make_posix_process_runner() {
  return std::make_unique<upgrade_guard::infrastructure::PosixProcessRunner>();
}
