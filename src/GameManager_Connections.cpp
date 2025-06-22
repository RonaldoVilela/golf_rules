#include "GameManager.h"
#include <thread>

// CONNECTION COMUNICATION FUNCTIONS ================= //
void GameManager::sendEvent(void* eventBuffer)
{
    if(player.onlineStatus == SV_DISCONNECTED){return;}

    if(player.onlineStatus == SV_HOSTING){
        for(auto p: connected_players){
            if(p.first == 0){continue;}
            send(p.second.m_socket, (char*)eventBuffer, 256, 0);
        }

        return;
    }

    send(player.m_socket, (char*)eventBuffer, 256, 0);
}

void GameManager::sendEventTo(void* eventBuffer, SOCKET dest)
{
    if(player.onlineStatus == SV_DISCONNECTED){return;}

    send(dest, (char*)eventBuffer, 256, 0);
}

void GameManager::manageConnections()
{
    while(player.onlineStatus != SV_DISCONNECTED){

        if(player.onlineStatus == SV_HOSTING){

            SOCKET clientSocket = accept(player.m_socket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET){
                
                Player* newPlayer = (Player *)malloc(sizeof(Player));
                if(std::strcmp(newPlayer->m_name, "DefaultName")){

                    
                    lastConnectionId++;
                    recv(clientSocket, (char*)newPlayer, sizeof(Player),0);
                    newPlayer->m_socket = clientSocket;
                    newPlayer->connection_id = lastConnectionId;
                    
                    std::cout << newPlayer->m_name << " joined the server! \n";
                    GM_LOG(std::string(newPlayer->m_name) + " joined the crew!");

                    for(auto p: connected_players){
                        AbstractEvent newEvent{PLAYER_CONNECTION_EVENT, p.second.connection_id};
                        strcpy(newEvent.m_string, p.second.m_name);
                        sendEventTo(&newEvent, clientSocket);
                    }

                    connected_players.insert(std::make_pair(newPlayer->connection_id,*newPlayer));
                    

                    AbstractEvent newEvent{PLAYER_CONNECTION_EVENT, newPlayer->connection_id};
                    strcpy(newEvent.m_string, newPlayer->m_name);
                    sendEvent(&newEvent);
                    

                    delete newPlayer;
                    std::cout << connected_players.at(lastConnectionId).m_name << "\n";
                }
            }
            

            
            for(auto i = connected_players.begin(); i != connected_players.end();){
                if((*i).first == 0){
                    ++i;
                    continue;
                }
                char buffer[256];

                int infoLenght = recv((*i).second.m_socket, buffer, sizeof(buffer), 0);

                if(infoLenght == 0){
                    std::cout << (*i).second.m_name<< " left the server \n";
                    int playerConnId = (*i).second.connection_id;
                    i = connected_players.erase(i);
                    AbstractEvent event{PLAYER_DISCONNECTION_EVENT, playerConnId};
                    sendEvent(&event);
                    continue;
                }else if(infoLenght < 0){
                    int err = WSAGetLastError();
                    if(err != WSAEWOULDBLOCK){
                        int playerConnId = (*i).second.connection_id;
                        std::cerr << "A unexpected error ocurred on one of the sockets: " << err << "| player: ["<< connected_players.at((*i).first).m_name <<"]\n";
                        i = connected_players.erase(i);
                        AbstractEvent event{PLAYER_DISCONNECTION_EVENT, playerConnId};
                        sendEvent(&event);
                        continue;
                    }
                }
                ++i;
                
            }
        }
        else if(player.onlineStatus == SV_CLIENT){
            char buffer[256];

            int infoLenght = recv(player.m_socket, buffer, sizeof(buffer), 0);

            if(infoLenght > 0){

                AbstractEvent* event = (AbstractEvent*)buffer;
                int eventType = event->eventType;
                std::cout << "event type: " << eventType << ", string: "<< event->m_string <<"\n";

                std::array<char, 256> temp{};
                std::memcpy(temp.data(), buffer, 256);
                eventList.push(temp);

            }else if(infoLenght == 0){
                std::cout << "Disconnected from server \n";
                disconnect();
                continue;
            }else {
                int err = WSAGetLastError();
                if(err != WSAEWOULDBLOCK){
                    disconnect();
                    std::cout << "A unexpected error ocurred on one of the sockets: " << err << "\n";
                    continue;
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(20));

    }
    std::cout << "Disconnected! \n";
    disconnect();
}
// -------------------------------//



// CONNECTION FUNCTIONS ====================== //

int GameManager::hostServer()
{   
    // If Winsock is not initialized, block the function. 
    if(!isWsaInitialized()){
        GM_LOG("Couldn't host a server: WinSock is not initialized.", LOG_ERROR);
        return 0;
    }

    if(player.onlineStatus != SV_DISCONNECTED){return false;}

    player.m_socket = socket(AF_INET, SOCK_STREAM,0);
    sockaddr_in socketAddr;

    socketAddr.sin_family = AF_INET;
    socketAddr.sin_addr.s_addr = INADDR_ANY;
    socketAddr.sin_port = htons(0); // let winsock choose a available port

    u_long mode = 1;
    ioctlsocket(player.m_socket, FIONBIO, &mode);

    if(bind(player.m_socket, (sockaddr*)&socketAddr, sizeof(socketAddr))){
        std::cerr << "Error:["<< WSAGetLastError() << "] couldn't bind local address to the user's SOCKET. \n";
        disconnect();
        return 0;
    }

    // get the chosen port
    sockaddr_in boundAddr;
    int len = sizeof(boundAddr);
    getsockname(player.m_socket, (sockaddr*)&boundAddr, &len);

    GM_LOG("Server port:" + std::to_string(ntohs(boundAddr.sin_port)));
    int port = ntohs(boundAddr.sin_port);
    
    if(listen(player.m_socket, 5)){
        std::cerr << "Error:["<< WSAGetLastError() << "] couldn't set SOCKET to listening state. \n";
        disconnect();
        return 0;
    }

    player.onlineStatus = SV_HOSTING;

    connected_players.insert(std::make_pair(lastConnectionId, player));

    std::thread connectionManagerThread(GameManager::manageConnections);
    connectionManagerThread.detach();

    return port;
}


bool GameManager::isWsaInitialized(){
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET){
        return false;
    }

    closesocket(s);
    return true;
}

bool GameManager::joinServer(const char *ipAddress ,int port)
{
    // If Winsock is not initialized, block the function. 
    if(!isWsaInitialized()){
        GM_LOG("Couldn't join the server: WinSock is not initialized.", LOG_ERROR);
        return false;
    }

    // if the player is already connected to a server, don't join.
    if(player.onlineStatus != SV_DISCONNECTED){return false;}

    // Initialize the user's socket:
    player.m_socket = socket(AF_INET, SOCK_STREAM,0);
    sockaddr_in socketAddr;

    // Set the ip and port:
    socketAddr.sin_family = AF_INET;
    socketAddr.sin_port = htons(port);
    socketAddr.sin_addr.s_addr = inet_addr(ipAddress);

    // connect to the informed ip [ipAddress]:
    if(connect(player.m_socket, (sockaddr*)&socketAddr, sizeof(socketAddr))){
        std::cerr << "Error: ["<< WSAGetLastError() << "] couldn't connect to server. \n";
        GM_LOG("Error: [" + std::to_string(WSAGetLastError()) + "] couldn't connect to server.", LOG_ERROR);
        
        disconnect();
        return false;
    }

    // send to the server the user's "Player" struct (GameManager.h) data, witch contains it's name, and online status:
    if(send(player.m_socket, (char *)&player, sizeof(Player), 0) < 0){
        std::cerr << "Error: ["<< WSAGetLastError() << "] couldn't send the user's information to the server. \n";
        GM_LOG("Error: [" + std::to_string(WSAGetLastError()) + "] couldn't send the user's information to the server.", LOG_ERROR);
        
        disconnect();
        return false;
    }

    // initialize a thread responsible for manage the connection (receive or send data):
    std::thread connectionManagerThread(GameManager::manageConnections);
    connectionManagerThread.detach();

    player.onlineStatus = SV_CLIENT; // set the user's online status as: CLIENT, since it's joining a server.
    return true;
}


void GameManager::disconnect()
{
    if(player.onlineStatus == SV_DISCONNECTED){return;} // if the user is already disconnected, block the function.

    if(player.onlineStatus == SV_HOSTING){
        /*
        If the user is the host, send a event, telling every connection(connected users)
        that the host is disconnected and force them to disconnect as well.
        */
        AbstractEvent newEvent{SERVER_CLOSED_EVENT};
        sendEvent(&newEvent);
    }


    player.onlineStatus = SV_DISCONNECTED;  // set connection status to: DISCONNECTED.

    shutdown(player.m_socket, SD_BOTH);     // shutdown socket connection.
    closesocket(player.m_socket);           // close socket.
    player.m_socket = INVALID_SOCKET;       // define the socket as a invalid socket (just in case).

    connected_players.clear();              // clear all the stored past players information.
    lastConnectionId = 0;                   // set the last connection id to 0.
    
}
// -------------------------- //