#pragma once

#include <string>
#include <sstream>
#include <optional>
#include <unordered_map>
#include <string_view>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

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

struct Uri {
    std::string scheme;
    std::string host;
    int port;
    std::string path;
    std::string query;
    std::string fragment;

    [[nodiscard]] std::string PathAndQuery() const;
};

Uri parse_uri(std::string uri);

class HttpRequest {
public:
    HttpRequest(std::string verb, Uri uri, std::string body);
    HttpRequest& AddHeader(const std::string& name, std::string value);

    const std::string& Verb() const;
    const Uri& Uri() const;
    const std::string& Body() const;
    const std::unordered_map<std::string, std::string>& Headers() const;
private:
    std::string verb_;
    struct Uri uri_;
    std::string body_;
    std::unordered_map<std::string, std::string> headers_;
};

class HttpClient {

public:
    HttpClient();

    HttpResponse MakeRequest(const HttpRequest& request);

private:
    // The io_context is required for all I/O
    boost::beast::net::io_context ioc_;
};

} // namespace me::brel