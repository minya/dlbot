#include "dlbot.h"
#include "path_util.h"

#include "tgbot/tgbot.h"
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <syslog.h>
#include <unistd.h>

using namespace std;
using namespace dlbot;

const vector<string> SUPPORTED_COMMANDS = 
    { "/progress" };

const unsigned int POLL_ERROR_COOLDOWN_SEC = 60;

DLBot::DLBot(Settings settings, TransmissionRpcClient tr_cli, SentryLogger& sentry_log)
    : settings_(std::move(settings))
    , tr_cli_(std::move(tr_cli))
    , sentry_log(sentry_log) { }

void save_file(const TgBot::Api& api, string fileId, string filePath);

string safe_file_name(const optional<string>& name);

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
            if (message->text.value_or("").starts_with(cmd)) {
                return;
            }
        }

        auto reply = [&](const string& msg) {
            api.sendMessage(message->chat->id, msg, nullptr, nullptr, make_reply_markup());
        };

        if (!message->document) {
            reply("Hey! I am torrent bot. Send me a .torrent file.");
            return;
        }

        try {
            const auto& doc = *message->document;
            filesystem::path filePath = settings_.dest_path;
            filePath = filePath / safe_file_name(doc.fileName);
            save_file(api, doc.fileId, filePath);
            reply("Received");
        } catch (const exception& e) {
            syslog(LOG_ERR, "Error while saving file: %s", e.what());
            reply("Error");
        }
    });

    bot.getEvents().onCommand("progress", [&bot, this] (const TgBot::Message::Ptr message) {
        try {
            if (!authorize(bot, message)) {
                return;
            }
            auto progress_state = tr_cli_.GetProgressState();
            if (!progress_state.has_value()) {
                bot.getApi().sendMessage(message->chat->id, "Не удалось получить прогресс((");
                return;
            }
            bot.getApi().sendMessage(message->chat->id, format_progress(*progress_state));
        } catch (const exception& ex) {
            sentry_log.Error(ex.what());
            syslog(LOG_ERR, "%s", ex.what());
        } catch (...) {
            sentry_log.Error("error");
            syslog(LOG_ERR, "error");
        }

    });

    std::string bot_username = bot.getApi().getMe()->username.value_or("");
    syslog(LOG_INFO, "Bot username: %s", bot_username.c_str());
    TgBot::TgLongPoll longPoll(bot);
    while (true) {
        try {
            longPoll.start();
        } catch (TgBot::TgException& e) {
            syslog(LOG_ERR, "[poll] tgerror: %s, cooldown", e.what());
            sleep(POLL_ERROR_COOLDOWN_SEC);
        } catch (exception& e) {
            syslog(LOG_ERR, "[poll] error: %s, cooldown", e.what());
            sleep(POLL_ERROR_COOLDOWN_SEC);
        } catch (...) {
            string msg = "[poll] ...something, not an exception, dunno what, cooldown";
            syslog(LOG_ERR, "%s", msg.c_str());
            sentry_log.Error(msg);
            sleep(POLL_ERROR_COOLDOWN_SEC);
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
    string content = api.downloadFile(file->filePath.value());
    ofstream fs; 
    fs.open(expand_path(filePath), ios::binary);
    fs.write(content.c_str(), content.size());
    fs.close();
    if (!fs) {
        throw runtime_error("failed to write " + filePath);
    }
}

// Keep only the last path component so a crafted name can't escape dest_path.
string safe_file_name(const optional<string>& name) {
    string result = filesystem::path(name.value_or("")).filename().string();
    if (result.empty() || result == "." || result == "..") {
        return "file.torrent";
    }
    return result;
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
    replyMarkup->resizeKeyboard = true;
    return replyMarkup;
}
