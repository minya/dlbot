#include "transmission_rpc.h"

#include <boost/json.hpp>

#include <cstdlib>
#include <iostream>
#include <string>
#include <optional>

#include "http_client.h"

using namespace std;
namespace json = boost::json;

using namespace me::brel::http;

TransmissionRpcClient::TransmissionRpcClient(string uri)
    : uri_(move(uri)) {}

optional<json::value> request(string uri, string tag, string method, boost::json::object args);

vector<TorrentProgress> TransmissionRpcClient::GetProgressState() {
    json::object args;
    json::array fields;
    fields.push_back(json::string("name"));
    fields.push_back(json::string("percentDone"));
    args["fields"] = fields;
    auto response_maybe = request(uri_,"1", "torrent-get", args);
    auto obj = *response_maybe;
    vector<TorrentProgress> result;
    for (const auto& item: obj.as_object()["response"].as_array()) {
        const auto& name_jstr = item.as_object().at("name").as_string();
        result.push_back(TorrentProgress{
            .name = {name_jstr.begin(), name_jstr.end()},
            .percentage = item.as_object().at("name").as_double(),
        });
    }
    return result;
}

optional<json::value> str_to_json(const string& str);

optional<json::value> request(string uri, string tag, string method, boost::json::object args) {
    boost::json::object req_data;
    req_data["tag"] = tag; 
    req_data["method"] = method;
    req_data["arguments"] = args;
    string req_body = boost::json::serialize(req_data);

    try
    {
        HttpClient cl;
        HttpRequest req("POST", parse_uri(move(uri)), req_body);
        auto resp = cl.MakeRequest(req);
        auto sessionId = resp.FindHeader("X-Transmission-Session-Id");
        HttpRequest req2("POST", parse_uri(move(uri)), req_body);
        req2.AddHeader("X-Transmission-Session-Id", *sessionId);
        auto resp2 = cl.MakeRequest(req2);
        return str_to_json(resp2.Body());
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