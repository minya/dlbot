#include "http_client.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

#include <regex>
#include <algorithm>

using namespace std;



namespace me::brel::http {

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

http::verb str_to_verb(const string& s_verb);

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
    http::request<http::string_body> req(str_to_verb(request.Verb()), request.Uri().PathAndQuery(), 11);
    req.set(http::field::host, request.Uri().host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // req.set(http::field::content_length, to_string(req_body.size()));
    req.set(http::field::connection, "Close");
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

    string Uri::PathAndQuery() const {
        ostringstream os;
        os << this->path << this->query;
        return os.str();
    }

    HttpResponse::HttpResponse(int status_code, std::unordered_map<std::string, std::string> headers,
                               std::string body)
                               : status_code_(status_code)
                               , headers_(move(headers))
                               , body_(move(body)) {

    }

    int HttpResponse::StatusCode() const {
        return status_code_;
    }

    const std::string *HttpResponse::FindHeader(const std::string& header_name) const {
        auto it = headers_.find(header_name);
        if (it == end(headers_))
            return nullptr;
        return &(it->second);
    }

    std::string HttpResponse::Body() const {
        return body_;
    }

    HttpRequest::HttpRequest(std::string verb, struct Uri uri, std::string body)
        : verb_(move(verb))
        , uri_(move(uri))
        , body_(move(body)){

    }

    HttpRequest &HttpRequest::AddHeader(const std::string& name, std::string value) {
        headers_[name] = move(value);
        return *this;
    }

    const std::string &HttpRequest::Verb() const {
        return verb_;
    }

    const Uri& HttpRequest::Uri() const {
        return uri_;
    }

    const std::string& HttpRequest::Body() const {
        return body_;
    }

    Uri parse_uri(std::string url) {
        regex ex("(http|https)://([^/ :]+):?([^/ ]*)(/?[^ #?]*)\\x3f?([^ #]*)#?([^ ]*)");
        cmatch what;
        if(regex_match(url.c_str(), what, ex))
        {
            return Uri {
                .scheme = string(what[1].first, what[1].second),
                .host = string(what[2].first, what[2].second),
                .port = atoi(string(what[3].first, what[3].second).c_str()),
                .path = string(what[4].first, what[4].second),
                .query = string(what[5].first, what[5].second),
                .fragment = string(what[6].first, what[6].second),
            };
        }
        throw runtime_error("Cant parse uri");
    }
};