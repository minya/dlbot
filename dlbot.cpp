#include "dlbot.h"
#include "path_util.h"

#include "tgbot/tgbot.h"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <syslog.h>

using namespace std;
using namespace dlbot;

const vector<string> SUPPORTED_COMMANDS = 
    { "/progress" };

DLBot::DLBot(Settings settings, TransmissionRpcClient tr_cli)
    : settings_(move(settings))
    , tr_cli_(move(tr_cli)) { }

void save_file(const TgBot::Api& api, string fileId, string filePath);

string format_progress(const vector<TorrentProgress>& progress);

TgBot::ReplyKeyboardMarkup::Ptr make_reply_markup();

void DLBot::Run() {
    TgBot::Bot bot(settings_.token);

    bot.getEvents().onAnyMessage([&bot, this] (const TgBot::Message::Ptr message) {
        const TgBot::Api& api = bot.getApi();
        if (!authorize(bot, message)) {
            return;
        }

        for (const auto& cmd: SUPPORTED_COMMANDS) {
            if (StringTools::startsWith(message->text, cmd)) {
                return;
            }
        }

        auto reply = [&](const string& msg) {
            api.sendMessage(message->chat->id, msg, false, 0, make_reply_markup());
        };

        if (!message->document) {
            reply("Hey! I am torrent bot. Send me a .torrent file.");
            return;
        }

        try {
            const auto& doc = *message->document;
            filesystem::path filePath = settings_.dest_path;
            filePath = filePath / doc.fileName;
            save_file(api, doc.fileId, filePath);
            reply("Received");
        } catch (const exception& e) {
            reply("Error");
        }
    });

    bot.getEvents().onCommand("progress", [&bot, this] (const TgBot::Message::Ptr message) {
        if (!authorize(bot, message)) {
            return;
        }
        auto progress_state = tr_cli_.GetProgressState();
        if (!progress_state.has_value()) {
            bot.getApi().sendMessage(message->chat->id, "Не удалось получить прогресс((");
            return;
        }
        bot.getApi().sendMessage(message->chat->id, format_progress(*progress_state));
    });

    syslog(LOG_INFO, "Bot username: %s", bot.getApi().getMe()->username.c_str());
    TgBot::TgLongPoll longPoll(bot);
    while (true) {
        try {
            syslog(LOG_INFO, "Long poll started");
            longPoll.start();
        } catch (TgBot::TgException& e) {
            syslog(LOG_ERR, "error: %s, cooldown", e.what());
            sleep(60000);
        }
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

TgBot::ReplyKeyboardMarkup::Ptr make_reply_markup() {
    TgBot::ReplyKeyboardMarkup::Ptr replyMarkup(new TgBot::ReplyKeyboardMarkup);
    TgBot::KeyboardButton::Ptr btn(new TgBot::KeyboardButton);
    btn->text = "/progress";
    vector<TgBot::KeyboardButton::Ptr> row;
    row.push_back(btn);
    replyMarkup->keyboard.push_back(row);
    return replyMarkup;
}
