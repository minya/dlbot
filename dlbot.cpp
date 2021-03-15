#include "dlbot.h"
#include "path_util.h"

#include "tgbot/tgbot.h"
#include <fstream>
#include <filesystem>
#include <sstream>

using namespace std;
using namespace dlbot;

const vector<string> SUPPORTED_COMMANDS = 
    { "/progress" };

DLBot::DLBot(Settings settings, TransmissionRpcClient tr_cli)
    : settings_(move(settings))
    , tr_cli_(move(tr_cli)) { }

void save_file(const TgBot::Api& api, string fileId, string filePath);

string format_progress(const vector<TorrentProgress>& progress);

void DLBot::Run() {
    TgBot::Bot bot(settings_.token);

    TgBot::ReplyKeyboardMarkup::Ptr replyMarkup(new TgBot::ReplyKeyboardMarkup);
    TgBot::KeyboardButton::Ptr btn(new TgBot::KeyboardButton);
    btn->text = "/progress";
    replyMarkup->keyboard.push_back({btn});

    bot.getEvents().onAnyMessage([&bot, this, replyMarkup] (const TgBot::Message::Ptr message) {
        const TgBot::Api& api = bot.getApi();
        if (!authorize(bot, message)) {
            return;
        }

        for (const auto& cmd: SUPPORTED_COMMANDS) {
            if (StringTools::startsWith(message->text, cmd)) {
                return;
            }
        }

        if (!message->document) {
            api.sendMessage(
                message->chat->id,
                "Hey! I am torrent bot. Send me a .torrent file.",
                false,
                0,
                replyMarkup);
            return;
        }

        try {
            const auto& doc = *message->document;
            filesystem::path filePath = settings_.dest_path;
            filePath = filePath / doc.fileName;
            save_file(api, doc.fileId, filePath);
            api.sendMessage(message->chat->id, "Received", false, 0, replyMarkup);
        } catch (const exception& e) {
            api.sendMessage(message->chat->id, "error", false, 0, replyMarkup);
        }
    });

    bot.getEvents().onCommand("progress", [&bot, this] (const TgBot::Message::Ptr message) {
        if (!authorize(bot, message)) {
            return;
        }

        bot.getApi().sendMessage(message->chat->id, format_progress(tr_cli_.GetProgressState()));
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

bool DLBot::authorize(const TgBot::Bot& bot, const TgBot::Message::Ptr message) const {
    const TgBot::Api& api = bot.getApi();
    const auto& allowed_users = settings_.allowed_users;
    if (find(begin(allowed_users), end(allowed_users), message->from->id) == end(allowed_users)) {
        api.sendMessage(message->chat->id, "GTFO");
        return false;
    }
    return true;
};

void save_file(const TgBot::Api& api, string fileId, string filePath) {
    TgBot::File::Ptr file = api.getFile(fileId);
    string content = api.downloadFile(file->filePath);
    ofstream fs; 
    fs.open(expand_path(filePath), ios::binary);
    fs.write(content.c_str(), content.size());
}

string format_progress(const vector<TorrentProgress>& progress) {
    ostringstream os;
    os << "Progress:\n";
    for (const TorrentProgress& tp: progress) {
        os << tp.name << ": " << tp.percentage << "\n";
    }
    return os.str();
}
