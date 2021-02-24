#pragma once

#include <vector>
#include <string>

struct TorrentProgress {
    std::string name;
    double percentage;
};


class TransmissionRpcClient {
public:
    TransmissionRpcClient(std::string uri);
    std::vector<TorrentProgress> GetProgressState();

private:
    std::string uri_;
};