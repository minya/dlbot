#include "http_response.h"

using namespace std;

namespace me::brel::http {

HttpResponse::HttpResponse(int status_code, std::unordered_map<std::string, std::string> headers,
                            std::string response)
                            : status_code_(status_code)
                            , headers_(move(headers))
                            , response_(move(response)) {

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

std::string_view HttpResponse::Body() const {
    string_view result(response_);
    auto body_pos = result.find("\r\n\r\n");
    if (body_pos != string_view::npos) {
        result.remove_prefix(body_pos + 4);
    }
    return result;
}

}