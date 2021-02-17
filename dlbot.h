#pragma once 

#include "transmission_rpc.h"

#include <string>
#include <vector>
#include <tgbot/tgbot.h>

namespace dlbot {

struct Settings {
    std::string token;
    std::string dest_path;
    std::vector<int> allowed_users;
    };
    
class DLBot {

public:
    DLBot(Settings settings, TransmissionRpcClient tr_cli);

    void Run();

private:
    bool authorize(const TgBot::Bot& bot, const TgBot::Message::Ptr message) const;
    Settings settings_;
    TransmissionRpcClient tr_cli_;
};


} // namespace dlbot
