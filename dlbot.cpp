#include "dlbot.h"
#include "path_util.h"

#include <tgbot/tgbot.h>
#include <fstream>

using namespace std;
using namespace dlbot;

DLBot::DLBot(Settings settings): settings_(move(settings)) { }

void save_file(const TgBot::Api& api, string fileId, string filePath);

void DLBot::Run() {
    TgBot::Bot bot(settings_.token);
    bot.getEvents().onAnyMessage([&bot, this] (const TgBot::Message::Ptr message) {
        const TgBot::Api& api = bot.getApi();
        if (find(begin(settings_.allowed_users), end(settings_.allowed_users), message->from->id) == end(settings_.allowed_users)) {
            api.sendMessage(message->chat->id, "GTFO");
            return;
        }
        if (!message->document) {
            api.sendMessage(message->chat->id, "Hey! I am torrent bot. Send me a .torrent file.");
            return;
        }
        try {
            const auto& doc = *message->document;
            filesystem::path filePath = settings_.dest_path;
            filePath = filePath / doc.fileName;
            save_file(api, doc.fileId, filePath);
            api.sendMessage(message->chat->id, "Received");
        } catch (const exception& e) {
            api.sendMessage(message->chat->id, "error");
        }
    });
    try {
        printf("Bot username: %s\n", bot.getApi().getMe()->username.c_str());
        TgBot::TgLongPoll longPoll(bot);
        while (true) {
            printf("Long poll started\n");
            longPoll.start();
        }
    } catch (TgBot::TgException& e) {
        printf("error: %s\n", e.what());
    }
}

void save_file(const TgBot::Api& api, string fileId, string filePath) {
    TgBot::File::Ptr file = api.getFile(fileId);
    string content = api.downloadFile(file->filePath);
    ofstream fs; 
    fs.open(expand_path(filePath), ios::binary);
    fs.write(content.c_str(), content.size());
}