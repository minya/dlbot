#include "http_request.h"

using namespace std;

namespace me::brel::http {

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


const std::unordered_map<std::string, std::string>& HttpRequest::Headers() const {
    return headers_;
}

}
