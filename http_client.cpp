#include "http_client.h"

#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>

namespace beast = boost::beast;     // from <boost/beast.hpp>
namespace http = beast::http;       // from <boost/beast/http.hpp>
namespace net = boost::asio;        // from <boost/asio.hpp>
using tcp = net::ip::tcp;           // from <boost/asio/ip/tcp.hpp>

using namespace std;

http::verb str_to_verb(const string& s_verb);

string HttpClient::MakeRequest(string verb, string host, string port, string target, string body) {

    // The io_context is required for all I/O
    // net::io_context ioc;

    // These objects perform our I/O
    tcp::resolver resolver(ioc_);
    beast::tcp_stream stream(ioc_);

    // Look up the domain name
    auto const results = resolver.resolve(host, port);

    // Make the connection on the IP address we get from a lookup
    stream.connect(results);

    // Set up an HTTP GET request message
    http::request<http::string_body> req{str_to_verb(verb), target, 11};
    req.set(http::field::host, host);
    req.set(http::field::user_agent, BOOST_BEAST_VERSION_STRING);
    // req.set(http::field::content_length, to_string(req_body.size()));
    req.set(http::field::connection, "Close");
    req.body() = body;

    // Send the HTTP request to the remote host
    http::write(stream, req);

    // This buffer is used for reading and must be persisted
    beast::flat_buffer buffer;

    // Declare a container to hold the response
    http::response<http::dynamic_body> res;

    // Receive the HTTP response
    http::read(stream, buffer, res);

    // Write the message to standard out
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
    return os.str();
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