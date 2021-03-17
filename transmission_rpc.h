#pragma once

#include <vector>
#include <string>
#include <optional>

struct TorrentProgress {
    std::string name;
    double percentage;
};


class TransmissionRpcClient {
public:
    TransmissionRpcClient(std::string uri);
    std::optional<std::vector<TorrentProgress>> GetProgressState();

private:
    std::string uri_;
};