#include "GameManager.h"

#include <sstream>

void GameManager::loadConsoleCommands(){
    console_commands.insert(std::make_pair("quit", [this](std::vector<std::string> args)->void{
        glfwSetWindowShouldClose(window, true);
    }));

    console_commands.insert(std::make_pair("args", [](std::vector<std::string> args)->void{
        GM_LOG("number of args: " + std::to_string(args.size()));
    }));

    console_commands.insert(std::make_pair("wsa_startup", [](std::vector<std::string> args)->void{

        // Initialize WinSock
        WSADATA wsaData;
        int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
        if(result != 0){
            std::cerr << "WSAStartup failed: " << result << std::endl;
            GM_LOG("WSAStartup failed: " + std::to_string(result) + " <- error code", LOG_ERROR);
            return;
        }
        GM_LOG("WinSock has been initialized! :3");
        // ------------------------------
    }));

    console_commands.insert(std::make_pair("disconnect", [this](std::vector<std::string> args)->void{
        disconnect();
        changeScene("lobby");
    }));

    console_commands.insert(std::make_pair("course", [this](std::vector<std::string> args)->void{
        if(player.onlineStatus != SV_DISCONNECTED){
            GM_LOG("Can't load a map manually while connected to a server! (consider disconnecting)", LOG_ERROR);
            return;
        }
        GM_LOG("TODO: implement system to load a 'hole/map file' from a specific course folder!", LOG_WARNING);
    }));

}
void GameManager::execCommand(std::string command){
    std::stringstream ss(command);
    std::string commName;
    std::getline(ss, commName, ' ');

    for(auto c: console_commands){
        if(c.first == commName){
            std::string arg;
            std::vector<std::string> arg_list;
            while(std::getline(ss, arg, ' ')){
                arg_list.push_back(arg);
            }

            c.second(arg_list);

            return;
        }
    }

    GM_LOG("\"" +commName + "\" is not a defined command!", LOG_ERROR);
}

// CONSOLE =============================
void GameManager::log(std::string msg, int logType)
{
    textLog newLog;
    newLog.msg = msg;

    switch(logType){
        case LOG_NORMAL: newLog.color = {1.0f, 1.0f, 1.0f, 1.0f};
            break;
        case LOG_WARNING: newLog.color = {1.0f, 1.0f, 0.0f, 1.0f};
            break;
        case LOG_ERROR: newLog.color = {1.0f, 0.0f, 0.0f, 1.0f};
            break;
        default: newLog.color = {1.0f, 1.0f, 1.0f, 1.0f};
    }
    console_cache.push_back(newLog);
    if(console_cache.size() > 100){
        console_cache.erase(console_cache.begin());
    }
}
//=====================================