#pragma once

#include "uri.h"

#include <unordered_map>
#include <string>

namespace me::brel::http {

class HttpRequest {
public:
    HttpRequest(std::string verb, Uri uri, std::string body);
    HttpRequest& AddHeader(const std::string& name, std::string value);

    const std::string& Verb() const;
    const struct Uri& Uri() const;
    const std::string& Body() const;
    const std::unordered_map<std::string, std::string>& Headers() const;
private:
    std::string verb_;
    struct Uri uri_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
};

} //namespace me::brel::http
