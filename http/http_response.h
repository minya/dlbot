#pragma once

#include <unordered_map>
#include <string_view>
#include <string>

namespace me::brel::http {

class HttpResponse {
public:
    HttpResponse(int status_code, std::unordered_map<std::string, std::string> headers, std::string body);

    int StatusCode() const;
    const std::string* FindHeader(const std::string& header_name) const;
    std::string_view Body() const;

private:
    int status_code_;
    std::unordered_map<std::string, std::string> headers_;
    std::string response_;
};

} //namespace me::brel::http
