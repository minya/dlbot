#pragma once

#include <string>

namespace me::brel::http {

struct Uri {
    std::string scheme;
    std::string host;
    int port;
    std::string path;
    std::string query;
    std::string fragment;

    std::string PathAndQuery() const;
};

Uri parse_uri(std::string uri);

} //namespace me::brel::http