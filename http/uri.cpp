#include "uri.h"


#include <sstream>
#include <regex>
#include <iostream>

using namespace std;

namespace me::brel::http {

string Uri::PathAndQuery() const {
    ostringstream os;
    os << this->path << this->query;
    return os.str();
}

Uri parse_uri(std::string url) {
    regex ex("(http|https)://((.+):(.+)@)?([^/ :]+):?([^/ ]*)(/?[^ #?]*)\x3f?([^ #]*)#?([^ ]*)");

    cmatch what;
    if(!regex_match(url.c_str(), what, ex))
        throw runtime_error("Cant parse uri");

    return Uri {
        .scheme = string(what[1].first, what[1].second),
        .username = string(what[3].first, what[3].second),
        .password = string(what[4].first, what[4].second),
        .host = string(what[5].first, what[5].second),
        .port = atoi(string(what[6].first, what[6].second).c_str()),
        .path = string(what[7].first, what[7].second),
        .query = string(what[8].first, what[8].second),
        .fragment = string(what[9].first, what[9].second),
    };
}

}