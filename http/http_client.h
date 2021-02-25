#pragma once

#include "http_response.h"
#include "http_request.h"

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>

namespace me::brel::http {

class HttpClient {
public:
    HttpClient();

    HttpResponse MakeRequest(const HttpRequest& request);

private:
    // The io_context is required for all I/O
    boost::beast::net::io_context ioc_;
};

} // namespace me::brel::http