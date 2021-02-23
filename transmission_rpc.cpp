#include "transmission_rpc.h"

#include <boost/json.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>

#include "http_client.h"

using namespace std;
namespace json = boost::json;

TransmissionRpcClient::TransmissionRpcClient(string uri)
    : uri_(move(uri)) {}

optional<json::value> request(string tag, string method, boost::json::object args);

std::vector<TorrentProgress> TransmissionRpcClient::GetProgressState() {
    request("1", "progress", {});
    return {
        { "Movie 1" , 0.54 },
        { "Series 2" , 1.0 },
    };
}

optional<json::value> str_to_json(const string& str);

optional<json::value> request(string tag, string method, boost::json::object args) {
    boost::json::object req_data;
    req_data["tag"] = tag; 
    req_data["method"] = method;
    req_data["arguments"] = args;
    string req_body = boost::json::serialize(req_data);

    try
    {
        HttpClient cl;
        string resp = cl.MakeRequest("POST", "ubuntu.local", "9091", "/transmission/rpc", req_body);
        return str_to_json(resp);
    }
    catch(std::exception const& e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return nullopt;
    }
}

optional<json::value> str_to_json(const string& str) {
    boost::json::stream_parser sp;
    json::error_code ec;
    sp.write(str, ec);
    if (ec)
        return nullopt;

    return sp.release();
}