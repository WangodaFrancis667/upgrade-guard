#include "upgrade_guard/reporting/Redaction.hpp"

#include <regex>

namespace upgrade_guard::reporting {

std::string redact_sensitive(std::string value) {
  static const std::regex home_pattern(R"(/home/[^/\s]+)");
  static const std::regex url_credentials(R"((https?://)[^/@\s]+@)", std::regex::icase);
  static const std::regex url_hostname(R"((https?://)(?:\[redacted\]@)?[^/\s]+)", std::regex::icase);
  static const std::regex proxy_credentials(R"((proxy[^=\s]*=)[^\s]+)", std::regex::icase);
  static const std::regex token_pattern(R"(((token|password|passwd|secret|apikey|api_key)[=:])[A-Za-z0-9._~+/-]+)",
                                        std::regex::icase);
  static const std::regex ipv4_pattern(R"(\b(?:[0-9]{1,3}\.){3}[0-9]{1,3}\b)");
  value = std::regex_replace(value, home_pattern, "/home/[redacted]");
  value = std::regex_replace(value, url_credentials, "$1[redacted]@");
  value = std::regex_replace(value, url_hostname, "$1[redacted-host]");
  value = std::regex_replace(value, proxy_credentials, "$1[redacted]");
  value = std::regex_replace(value, token_pattern, "$1[redacted]");
  value = std::regex_replace(value, ipv4_pattern, "[redacted-ip]");
  return value;
}

} // namespace upgrade_guard::reporting
