#include "dlbot.h"
#include "skeleton_daemon.h"

#include <stdio.h>
#include <tgbot/tgbot.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <optional>

using namespace std;

optional<dlbot::Settings> read_settings();

void run_as_daemon(dlbot::DLBot& bot);
void run_as_user(dlbot::DLBot& bot);

int main(int argc, char** argv) {
    auto settings_maybe = read_settings();
    if (!settings_maybe.has_value()) {
        return EXIT_FAILURE;
    }

    dlbot::DLBot bot(*settings_maybe);

    if (argc == 2 && strncmp(argv[1], "-d", 2) == 0) {
        run_as_daemon(bot);
    } else {
        run_as_user(bot);
    }

    return EXIT_SUCCESS;
}

void run_as_daemon(dlbot::DLBot& bot) {
        skeleton_daemon(strdup("dlbot"));
        syslog (LOG_NOTICE, "dlbot started.");
        bot.Run();
        syslog (LOG_NOTICE, "dlbot terminated.");
        closelog();
}

void run_as_user(dlbot::DLBot& bot) {
    bot.Run();
}

optional<dlbot::Settings> read_settings() {
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
    return dlbot::Settings{.token = token, .dest_path = dest_path, .allowed_users = allowed_users};
}