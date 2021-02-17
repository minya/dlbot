#include "transmission_rpc.h"

using namespace std;

TransmissionRpcClient::TransmissionRpcClient(string uri)
    : uri_(move(uri)) {}

std::vector<TorrentProgress> TransmissionRpcClient::GetProgressState() {
    return {
        { "Movie 1" , 0.54 },
        { "Series 2" , 1.0 },
    };
}
