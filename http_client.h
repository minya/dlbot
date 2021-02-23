#pragma once

#include <string>
#include <sstream>

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

class HttpClient {

public:
    HttpClient() { }

    std::string MakeRequest(
            std::string verb, std::string host, std::string port, std::string target, std::string body);

private:
    // The io_context is required for all I/O
    boost::beast::net::io_context ioc_;
};