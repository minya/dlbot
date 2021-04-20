#include "dlbot.h"
#include "skeleton_daemon.h"
#include "boost/program_options.hpp"

#include <stdio.h>
#include <iostream>
#include <string>
#include <filesystem>
#include <vector>
#include <optional>
#include <syslog.h>

using namespace std;

struct ProgramOptions {
    std::string token;
    std::string dest_path;
    std::vector<int> allowed_users;
    bool daemon;
    std::string transmission_rpc_uri;
};

optional<ProgramOptions> read_options(int argc, char** argv);

void run_as_daemon(dlbot::DLBot& bot);
void run_as_user(dlbot::DLBot& bot);

int main(int argc, char** argv) {
    auto settings_maybe = read_options(argc, argv);
    if (!settings_maybe.has_value()) {
        return EXIT_FAILURE;
    }

    const ProgramOptions& po = *settings_maybe;
    dlbot::DLBot bot(
        { .token = po.token, .dest_path = po.dest_path, .allowed_users = po.allowed_users },
        { po.transmission_rpc_uri });

    if (po.daemon) {
        run_as_daemon(bot);
    } else {
        run_as_user(bot);
    }

    return EXIT_SUCCESS;
}

void run_as_daemon(dlbot::DLBot& bot) {
    skeleton_daemon(strdup("dlbot"));
    syslog(LOG_NOTICE, "dlbot started.");
    bot.Run();
    syslog(LOG_NOTICE, "dlbot terminated.");
    closelog();
}

void run_as_user(dlbot::DLBot& bot) {
    openlog(NULL, LOG_PID, LOG_USER | LOG_CONS);
    bot.Run();
}

optional<ProgramOptions> read_options(int argc, char** argv) {
    namespace po = boost::program_options;

    po::options_description desc("Usage");
    desc.add_options()
        ("help", "Produce help message")
        ("token,t", po::value<string>()->required(), "Bot api token")
        ("dest-path,d", po::value<string>()->required(), "Path for saving torrent files")
        ("allowed-users,u", po::value<vector<int>>()->multitoken(), "Allowed users")
        ("transmission-rpc-uri,R", po::value<string>()->required(), "Transmission RPC base uri")
        ("daemon,D", "Run as daemon")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);

    if (vm.count("help")) {
        cout << desc << endl;
        return nullopt;
    }

    if (!vm.count("token") || !vm.count("dest-path") || !vm.count("allowed-users") || !vm.count("transmission-rpc-uri"))  {
        cerr << desc << endl;
        return nullopt;
    }
    string token = vm["token"].as<string>();
    string dest_path = vm["dest-path"].as<string>();
    vector<int> allowed_users = vm["allowed-users"].as<vector<int>>();
    bool daemon = vm.count("daemon");
    string transmission_rpc_uri = vm["transmission-rpc-uri"].as<string>();

    return ProgramOptions{
        .token = token,
        .dest_path = dest_path,
        .allowed_users = allowed_users,
        .daemon = daemon,
        .transmission_rpc_uri = transmission_rpc_uri
    };
}