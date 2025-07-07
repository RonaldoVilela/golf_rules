#include "GameManager.h"
#include "scenes/Match.h"

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
        if(isWsaInitialized()){
            GM_LOG("WinSock is already initialized.");
            return;}
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
        changeScene("server_list");
    }));
    
    console_commands.insert(std::make_pair("course", [this](std::vector<std::string> args)->void{
        if(player.onlineStatus != SV_DISCONNECTED){
            GM_LOG("Can't load a map manually while connected to a server! (consider disconnecting)", LOG_ERROR);
            return;
        }

        if(args.size() < 2){
            GM_LOG("Too few arguments. This is the \"course\" command structure  >> \"course [couse_name_here] [map_name_here]\" ", LOG_ERROR);
            return;
        }

        if(actual_scene_name == "match"){
            scenes["match"]->unload();
        }

        if(((scene::Match*)scenes["match"])->loadCourse(args[0]) != 0){return;}
        if(((scene::Match*)scenes["match"])->loadMap(args[1]) != 0){return;}

        changeScene("match");

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