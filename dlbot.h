#pragma once 

#include <string>
#include <vector>

namespace dlbot {

struct Settings {
    std::string token;
    std::string dest_path;
    std::vector<int> allowed_users;
    };
    
class DLBot {

public:
    DLBot(Settings settings);

    void Run();

private:
    Settings settings_;
};


} // namespace dlbot