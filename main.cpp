#include "path_util.h"

#include <stdio.h>
#include <tgbot/tgbot.h>
#include <fstream>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <optional>

using namespace std;

struct Settings {
    string token;
    string dest_path;
    vector<int> allowed_users;
};

optional<Settings> read_settings();

void save_file(const TgBot::Api& api, string fileId, string filePath);

int main() {
    auto settings_maybe = read_settings();
    if (!settings_maybe.has_value()) {
        return -1;
    }

    const Settings& settings = *settings_maybe;

    TgBot::Bot bot(settings.token);
    bot.getEvents().onAnyMessage([&bot, &settings] (const TgBot::Message::Ptr message) {
        const TgBot::Api& api = bot.getApi();
        if (find(begin(settings.allowed_users), end(settings.allowed_users), message->from->id) == end(settings.allowed_users)) {
            api.sendMessage(message->chat->id, "GTFO");
            return;
        }
        if (!message->document) {
            api.sendMessage(message->chat->id, "Hey! I am torrent bot. Send me a .torrent file.");
            return;
        }
        try {
            const auto& doc = *message->document;
            filesystem::path filePath = settings.dest_path;
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
    return 0;
}

void save_file(const TgBot::Api& api, string fileId, string filePath) {
    TgBot::File::Ptr file = api.getFile(fileId);
    string content = api.downloadFile(file->filePath);
    ofstream fs; 
    fs.open(expand_path(filePath), ios::binary);
    fs.write(content.c_str(), content.size());
}

optional<Settings> read_settings() {
    char* token = getenv("TOKEN");
    if (!token) {
        cerr << "No TOKEN in envioronment" << endl;
        return nullopt;
    }
    char* dest_path = getenv("DEST_PATH");
    if (!dest_path) {
        cerr << "No DEST_PATH in envioronment" << endl;
        return nullopt;
    }
    char* c_allowed_users = getenv("ALLOWED_USERS");
    if (!c_allowed_users) {
        cerr << "No ALLOWED_USERS in envioronment" << endl;
        return nullopt;
    }
    vector<int> allowed_users;
    string_view auv = c_allowed_users;
    while (auv.size()) {
        auto pos = auv.find(',');
        string elem;
        if (pos == string_view::npos) {
            elem = auv;
            auv.remove_prefix(auv.size());
        } else {
            elem = auv.substr(0, pos);
            auv.remove_prefix(pos + 1);
        }
        allowed_users.push_back(atoi(elem.c_str()));
    }
    return Settings{.token = token, .dest_path = dest_path, .allowed_users = allowed_users};
}