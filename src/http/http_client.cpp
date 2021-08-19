#include "http_client.h"
#include "../3rdparty/Base64.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <algorithm>
#include <iostream>

using namespace std;

namespace me::brel::http {

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

http::verb str_to_verb(const string& s_verb);


string make_basic_auth_header_value(const string& username, const string& password) {
    ostringstream os;
    os << username << ':' << password;
    ostringstream os_res;
    os_res << "Basic " << macaron::Base64::Encode(os.str());
    return os_res.str();
}

HttpResponse HttpClient::MakeRequest(const HttpRequest& request) {

    // The io_context is required for all I/O
    // net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc_);
    beast::tcp_stream stream(ioc_);

    // Look up the domain name
    auto const results = resolver.resolve(request.Uri().host, std::to_string(request.Uri().port));

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    string dest = request.Uri().PathAndQuery();
    http::request<http::string_body> req(str_to_verb(request.Verb()), dest, 11);
    req.set(http::field::host, request.Uri().host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // req.set(http::field::content_length, to_string(req_body.size()));
    req.set(http::field::connection, "Close");
    req.set(http::field::content_length, to_string(request.Body().size()));
    if (request.Uri().username.size() > 0) {
        string auth_header_val = make_basic_auth_header_value(request.Uri().username, request.Uri().password);
        req.set(http::field::authorization, auth_header_val);
    }
    for (const auto& [k, v]: request.Headers()) {
        req.set(k, v);
    }
    req.body() = request.Body();

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to ostringstream
    ostringstream os;
    os << res;

    // Gracefully close the socket
    beast::error_code ec;
    stream.socket().shutdown(tcp::socket::shutdown_both, ec);

    // not_connected happens sometimes
    // so don't bother reporting it.
    //
    if(ec && ec != beast::errc::not_connected)
        throw beast::system_error{ec};

    // If we get here then the connection is closed gracefully
    // return os.str();
    unordered_map<string, string> headers;
    for (const auto& field: res) {
        headers.insert({
                field.name_string().to_string(),
                field.value().to_string()
        });
    }
    return HttpResponse(static_cast<int>(res.result()), headers, os.str());
}

    HttpClient::HttpClient() {

    }

    http::verb str_to_verb(const string& s_verb) {
        if (s_verb == "POST")
            return http::verb::post;
        if (s_verb == "PUT")
            return http::verb::put;
        if (s_verb == "DELETE")
            return http::verb::delete_;
        if (s_verb =="HEAD")
            return http::verb::head;
        if (s_verb =="OPTIONS")
            return http::verb::options;
        if (s_verb == "PATCH")
            return http::verb::patch;
        if (s_verb == "GET")
            return http::verb::get;
        throw runtime_error("unsupported verb");
    }
};