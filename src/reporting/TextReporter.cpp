#include "upgrade_guard/reporting/ReporterFactories.hpp"
#include "upgrade_guard/reporting/Redaction.hpp"

#include <algorithm>
#include <sstream>

namespace upgrade_guard::reporting {
namespace {

std::string status(domain::CheckStatus value) {
  switch (value) {
  case domain::CheckStatus::passed:
    return "PASS";
  case domain::CheckStatus::warning:
    return "WARNING";
  case domain::CheckStatus::blocked:
    return "BLOCKER";
  case domain::CheckStatus::unknown:
    return "UNKNOWN";
  case domain::CheckStatus::error:
    return "ERROR";
  }
  return "UNKNOWN";
}

std::string colored(const std::string &text, domain::CheckStatus value, bool color) {
  if (!color) {
    return text;
  }
  if (value == domain::CheckStatus::blocked || value == domain::CheckStatus::error) {
    return "\033[31m" + text + "\033[0m";
  }
  if (value == domain::CheckStatus::warning || value == domain::CheckStatus::unknown) {
    return "\033[33m" + text + "\033[0m";
  }
  return "\033[32m" + text + "\033[0m";
}

std::string overall(domain::ReadinessStatus value) {
  switch (value) {
  case domain::ReadinessStatus::ready:
    return "READY";
  case domain::ReadinessStatus::ready_with_warnings:
    return "READY WITH WARNINGS";
  case domain::ReadinessStatus::blocked:
    return "BLOCKED";
  case domain::ReadinessStatus::incomplete:
    return "INCOMPLETE";
  }
  return "INCOMPLETE";
}

std::size_t count_status(const domain::ScanReport &report, domain::CheckStatus value) {
  return static_cast<std::size_t>(std::count_if(report.findings.begin(), report.findings.end(), [value](const auto &f) {
    return f.status == value;
  }));
}

class TextReporter final : public ports::IReporter {
public:
  explicit TextReporter(bool color) : color_(color) {}
  [[nodiscard]] std::string format(const domain::ScanReport &report) const override {
    std::ostringstream out;
    out << "Upgrade Guard " << domain::ToolVersion << "\n";
    out << "Current system: " << report.platform.distribution.name << "\n";
    out << "Target release: Ubuntu " << report.request.target_release << " LTS\n\n";
    out << "Scan ID: " << report.scan_id << "\n";
    out << "Started: " << report.started_at << "\n";
    out << "Completed: " << report.completed_at << "\n\n";
    out << "Overall status: " << overall(report.overall_status) << "\n\n";
    out << "Blockers: " << count_status(report, domain::CheckStatus::blocked) << "\n";
    out << "Warnings: " << count_status(report, domain::CheckStatus::warning) << "\n";
    out << "Unknown: " << count_status(report, domain::CheckStatus::unknown) << "\n\n";
    for (const auto &finding : report.findings) {
      if (finding.status == domain::CheckStatus::passed && !report.request.verbose) {
        continue;
      }
      out << "[" << colored(status(finding.status), finding.status, color_) << "] " << finding.id << "\n";
      out << finding.title << "\n";
      out << finding.explanation << "\n";
      out << "Evidence:";
      if (finding.evidence.empty()) {
        out << " none";
      }
      out << "\n";
      for (const auto &evidence : finding.evidence) {
        out << "  - " << redact_sensitive(evidence.label) << ": " << redact_sensitive(evidence.value) << "\n";
      }
      out << "Recommended action: " << finding.recommendation << "\n\n";
    }
    for (const auto &limitation : report.limitations) {
      if (limitation != "No system changes were made.") {
        out << "Limitation: " << limitation << "\n";
      }
    }
    out << "Privacy: sensitive values are redacted by default.\n";
    out << "No system changes were made.\n";
    return out.str();
  }

private:
  bool color_;
};

} // namespace

std::unique_ptr<ports::IReporter> make_text_reporter(bool color) { return std::make_unique<TextReporter>(color); }

} // namespace upgrade_guard::reporting
