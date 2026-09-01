#include "upgrade_guard/reporting/ReporterFactories.hpp"

#include <algorithm>
#include <cctype>
#include <sstream>

namespace upgrade_guard::reporting {
namespace {

std::string redact(std::string value) {
  const auto at = value.find('@');
  const auto scheme = value.find("://");
  if (scheme != std::string::npos && at != std::string::npos && at > scheme) {
    value.replace(scheme + 3, at - scheme - 3, "[redacted]");
  }
  const auto home = value.find("/home/");
  if (home != std::string::npos) {
    const auto next = value.find('/', home + 6);
    value.replace(home, next == std::string::npos ? std::string::npos : next - home, "/home/[redacted]");
  }
  return value;
}

std::string esc(const std::string &text) {
  std::ostringstream out;
  for (const char ch : text) {
    switch (ch) {
    case '\\':
      out << "\\\\";
      break;
    case '"':
      out << "\\\"";
      break;
    case '\n':
      out << "\\n";
      break;
    case '\r':
      break;
    default:
      out << ch;
    }
  }
  return out.str();
}

std::string check(domain::CheckStatus value) {
  switch (value) {
  case domain::CheckStatus::passed:
    return "passed";
  case domain::CheckStatus::warning:
    return "warning";
  case domain::CheckStatus::blocked:
    return "blocked";
  case domain::CheckStatus::unknown:
    return "unknown";
  case domain::CheckStatus::error:
    return "error";
  }
  return "unknown";
}

std::string severity(domain::Severity value) {
  switch (value) {
  case domain::Severity::info:
    return "info";
  case domain::Severity::warning:
    return "warning";
  case domain::Severity::blocker:
    return "blocker";
  }
  return "info";
}

std::string overall(domain::ReadinessStatus value) {
  switch (value) {
  case domain::ReadinessStatus::ready:
    return "ready";
  case domain::ReadinessStatus::ready_with_warnings:
    return "ready_with_warnings";
  case domain::ReadinessStatus::blocked:
    return "blocked";
  case domain::ReadinessStatus::incomplete:
    return "incomplete";
  }
  return "incomplete";
}

std::size_t count_status(const domain::ScanReport &report, domain::CheckStatus value) {
  return static_cast<std::size_t>(std::count_if(report.findings.begin(), report.findings.end(), [value](const auto &f) {
    return f.status == value;
  }));
}

class JsonReporter final : public ports::IReporter {
public:
  [[nodiscard]] std::string format(const domain::ScanReport &report) const override {
    std::ostringstream out;
    out << "{\n";
    out << "\"schema_version\":\"1.0\",\"tool_version\":\"" << domain::ToolVersion << "\",";
    out << "\"scan_id\":\"" << esc(report.scan_id) << "\",\"started_at\":\"" << report.started_at << "\",";
    out << "\"completed_at\":\"" << report.completed_at << "\",";
    out << "\"current_system\":{\"id\":\"" << esc(report.platform.distribution.id) << "\",";
    out << "\"name\":\"" << esc(report.platform.distribution.name) << "\",";
    out << "\"version_id\":\"" << esc(report.platform.distribution.version_id) << "\",";
    out << "\"architecture\":\"" << esc(report.platform.architecture) << "\"},";
    out << "\"target_release\":\"" << esc(report.request.target_release) << "\",";
    out << "\"overall_status\":\"" << overall(report.overall_status) << "\",";
    out << "\"summary\":{\"blockers\":" << count_status(report, domain::CheckStatus::blocked);
    out << ",\"warnings\":" << count_status(report, domain::CheckStatus::warning);
    out << ",\"unknown\":" << count_status(report, domain::CheckStatus::unknown) << "},";
    out << "\"findings\":[";
    for (std::size_t i = 0; i < report.findings.size(); ++i) {
      const auto &f = report.findings[i];
      if (i != 0) {
        out << ",";
      }
      out << "{\"id\":\"" << f.id << "\",\"status\":\"" << check(f.status) << "\",";
      out << "\"severity\":\"" << severity(f.severity) << "\",\"title\":\"" << esc(f.title) << "\",";
      out << "\"explanation\":\"" << esc(f.explanation) << "\",\"evidence\":[";
      for (std::size_t j = 0; j < f.evidence.size(); ++j) {
        if (j != 0) {
          out << ",";
        }
        out << "{\"label\":\"" << esc(f.evidence[j].label) << "\",\"value\":\"" << esc(redact(f.evidence[j].value)) << "\"}";
      }
      out << "],\"recommendation\":\"" << esc(f.recommendation) << "\",";
      out << "\"collector_complete\":" << (f.collector_complete ? "true" : "false") << "}";
    }
    out << "],\"limitations\":[";
    for (std::size_t i = 0; i < report.limitations.size(); ++i) {
      if (i != 0) {
        out << ",";
      }
      out << "\"" << esc(report.limitations[i]) << "\"";
    }
    out << "],\"privacy\":{\"redaction\":\"default\"}}\n";
    return out.str();
  }
};

} // namespace

std::unique_ptr<ports::IReporter> make_json_reporter() { return std::make_unique<JsonReporter>(); }

} // namespace upgrade_guard::reporting
