#include "GameManager.h"
#include "scenes/Match.h"
#include <thread>
#include <ws2tcpip.h>

#define DISCOVERY_MESSAGE "SERVER_INFO_REQUEST"
#define DISCOVERY_RESPONSE_MSG "SERVER_INFO_RESPONSE"

// SERVER STRUCT'S FUNCTION DEFINITION =============== //

void ServerInfo::start(int server_port)
{
    valid = true;
    m_port = server_port;

    //initialize UDP socket
    udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    int yes = 1;
    setsockopt(udp_socket, SOL_SOCKET, SO_REUSEADDR, (char*)&yes, sizeof(yes));

    // Enable broadcast
    BOOL bOpt = TRUE;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

    sockaddr_in udpAddr{};
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(GameManager::DISCOVERY_PORT);
    udpAddr.sin_addr.s_addr = INADDR_ANY;

    u_long mode = 1;
    ioctlsocket(udp_socket, FIONBIO, &mode);

    bind(udp_socket, (sockaddr*)&udpAddr, sizeof(udpAddr));

    // get the chosen port
    sockaddr_in boundAddr;
    int len = sizeof(boundAddr);
    getsockname(udp_socket, (sockaddr*)&boundAddr, &len);

    GM_LOG("[Server] Waiting broadcast UDP on port: "+std::to_string(ntohs(boundAddr.sin_port))+"...");
}

void ServerInfo::invalidate()
{
    valid = false;
    m_name = "LAN server";
    m_port = 0;
    online_players = 0;
    playerId_1 = -1;
    playerId_2 = -1;
    actual_course = "";
    
    shutdown(udp_socket, SD_BOTH);
    closesocket(udp_socket);
    GM_LOG("UDP socket closed.");
}

// Server info >>
bool ServerInfo::setServerInfo(const char *data)
{
    std::cout << "[setting the server info from data]" << '\n';
    int buffer_position = 0;

    int size = 0;
    memcpy(&size, &data[buffer_position], sizeof(int)); // 1 - server name lenght
    buffer_position += sizeof(int);
    std::cout << "Read server name size: " << size << " | position: " << buffer_position << '\n';

    char* name_buffer = (char*)malloc(size + 1);
    name_buffer[size] = '\0';
    memcpy(name_buffer, &data[buffer_position], size); // 2 - server name
    m_name = name_buffer;

    free(name_buffer);
    buffer_position += size;
    std::cout << "Read server name: " << m_name << " | position: " << buffer_position << '\n';

    memcpy(&m_port, &data[buffer_position], sizeof(int)); // 3 - server port
    buffer_position += sizeof(int);

    std::cout << "Read server port: " << m_port << " | position: " << buffer_position << '\n';


    memcpy(&size, &data[buffer_position], sizeof(int)); // 4 - server address lenght
    buffer_position += sizeof(int);
    std::cout << "Read server address lenght: " << size << " | position: " << buffer_position << '\n';

    name_buffer = (char*)malloc(size + 1);
    name_buffer[size] = '\0';
    memcpy(name_buffer, &data[buffer_position], size); // 5 - server address
    server_address = name_buffer;

    free(name_buffer);
    buffer_position += size;
    std::cout << "Read server address: " << server_address << " | position: " << buffer_position << '\n';

    memcpy(&online_players, &data[buffer_position], sizeof(int)); // 6 - number of online players
    buffer_position += sizeof(int);

    std::cout << "Read number of online players: " << online_players << " | position: " << buffer_position << '\n';

    memcpy(&size, &data[buffer_position], sizeof(int)); // 7 - course name lenght
    buffer_position += sizeof(int);

    std::cout << "Read server course name lenght: " << size << " | position: " << buffer_position << '\n';

    char* course_name_buffer = (char*)malloc(size + 1);
    course_name_buffer[size] = '\0';
    memcpy(course_name_buffer, &data[buffer_position], size); // 8 - actual course name
    actual_course = course_name_buffer;

    free(course_name_buffer);
    buffer_position += size;

    std::cout << "Read server course name: " << actual_course << " | position: " << buffer_position << '\n';

    memcpy(&playerId_1, &data[buffer_position], sizeof(int)); // 9 - player 1
    buffer_position += sizeof(int);

    memcpy(&playerId_2, &data[buffer_position], sizeof(int)); // 10 - player 2
    buffer_position += sizeof(int);

    std::cout << "Read server players of the round: " << playerId_1 << ":"<< playerId_2 << " | position: " << buffer_position << '\n';

    valid = true;
    std::cout << "[setted server info] Final buffer position: "<< buffer_position << '\n';
    return true;
}

std::string ServerInfo::getServerInfo(int players_online)
{
    std::cout << "[getting server info size]" << '\n';
    std::ostringstream server_info_buffer(std::ios::binary);
    int size = m_name.size();

    server_info_buffer.write((char*)&size, sizeof(int)); // 1 - server name lenght
    std::cout << "Written name size (int): " << server_info_buffer.str().size() << '\n';

    server_info_buffer.write(m_name.c_str(), size); // 2 - server name
    std::cout << "Written name with size: " << m_name.size() << " | Total Size: " << server_info_buffer.str().size() << '\n';

    server_info_buffer.write((char*)&m_port, sizeof(int)); // 3 - server port
    std::cout << "Written server port(int): " << 4 << " | Total Size: " << server_info_buffer.str().size() << '\n';

    size = server_address.size();
    server_info_buffer.write((char*)&size, sizeof(int)); // 4 - server address lenght
    std::cout << "Written address lenght (int): " << 4 << " | Total Size: " << server_info_buffer.str().size() << '\n';
    server_info_buffer.write(server_address.c_str(), size); // 5 - server address
    std::cout << "Written address with size: " << server_address.size() << " | Total Size: " << server_info_buffer.str().size() << '\n';

    server_info_buffer.write((char*)&players_online, sizeof(int)); // 6 - number of online players
    std::cout << "Written number of online players(int): " << 4 << " | Total Size: " << server_info_buffer.str().size() << '\n';

    size = actual_course.length();
    server_info_buffer.write((char*)&size, sizeof(int)); // 7 - course name lenght
    std::cout << "Written course name lenght(int): " << 4 << " | Total Size: " << server_info_buffer.str().size() << '\n';

    server_info_buffer.write(actual_course.c_str(), size); // 8 - actual course name
    std::cout << "Written course name with size: " << actual_course.size() << " | Total Size: " << server_info_buffer.str().size() << '\n';

    server_info_buffer.write((char*)&playerId_1, sizeof(int)); // 9 - player 1
    server_info_buffer.write((char*)&playerId_2, sizeof(int)); // 10 - player 2
    std::cout << "Written p1 and p2 id (2x int): " << 8 << " | Total Size: " << server_info_buffer.str().size() << '\n';

    std::cout << "[server info buffer completed] Total server Info size: " << server_info_buffer.str().size() << '\n';

    return server_info_buffer.str();
}

// ------------------------------- //

// CONNECTION COMUNICATION FUNCTIONS ================= //
void GameManager::sendEvent(const void* eventBuffer)
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

void GameManager::sendEventTo(const void* eventBuffer, SOCKET dest)
{
    if(player.onlineStatus == SV_DISCONNECTED){return;}

    send(dest, (char*)eventBuffer, 256, 0);
}

void GameManager::manageConnections()
{
    while(player.onlineStatus != SV_DISCONNECTED){

        // The host is responsible for hearing all connected players and updating them about the server and game state.
        if(player.onlineStatus == SV_HOSTING){

            // whatch for discovery requests with the actual server UDP socket
            {
                char buffer[256];
                sockaddr_in clientAddr{};
                int len = sizeof(clientAddr);

                int broad_data_lenght = recvfrom(actual_server.udp_socket, buffer, sizeof(buffer), 0, (sockaddr*)&clientAddr, &len);
                
                // discover and set apropriate ip to give to client
                sockaddr_in tempAddr{};
                tempAddr.sin_family = AF_INET;
                tempAddr.sin_port = htons(0); // any unused port
                tempAddr.sin_addr = clientAddr.sin_addr; // same ip as the client

                SOCKET temp = socket(AF_INET, SOCK_DGRAM, 0);
                connect(temp, (sockaddr*)&tempAddr, sizeof(tempAddr));

                sockaddr_in localAddr{};
                len = sizeof(localAddr);
                getsockname(temp, (sockaddr*)&localAddr, &len);

                actual_server.server_address = inet_ntoa(localAddr.sin_addr); // defines the server info port as the new one

                closesocket(temp);
                //--------------------------------------
                
                if(broad_data_lenght > 0){
                    std::cout << "received something" << std::endl;
                }

                buffer[broad_data_lenght] = '\0';
                if (strcmp(buffer, DISCOVERY_MESSAGE) == 0) {
                    std::ostringstream server_response(std::ios::binary);
                    server_response.write(DISCOVERY_RESPONSE_MSG, 20);
                    //GameManager::actual_server.setServerInfo(GameManager::actual_server.getServerInfo(connected_players.size()));
                    server_response.write(GameManager::actual_server.getServerInfo(connected_players.size()).data(), 256 - 20);

                    sendto(actual_server.udp_socket, server_response.str().data(), server_response.str().size(), 0, (sockaddr*)&clientAddr, sizeof(clientAddr));
                    std::cout << "[Server] Client discovered server!" << std::endl;
                }
            }

            //ckech new connections
            SOCKET clientSocket = accept(player.m_socket, nullptr, nullptr);
            if (clientSocket != INVALID_SOCKET){
                
                
                lastConnectionId++;
                char received_data[256];
                recv(clientSocket, received_data, 256,0);

                GameEvent event(received_data);

                // Add new connected player
                Player* newPlayer = (Player *)malloc(sizeof(Player));

                std::memcpy((char*)newPlayer, &received_data[sizeof(int)], sizeof(Player));

                newPlayer->m_socket = clientSocket;
                newPlayer->connection_id = lastConnectionId;

                // send the player's connection id
                std::ostringstream connection_event_buffer(std::ios::binary);
                connection_event_buffer.write((char*)&newPlayer->connection_id, sizeof(int));
                connection_event_buffer.write(GameManager::actual_server.getServerInfo(connected_players.size()).data(), 256-sizeof(int));

                send(clientSocket,connection_event_buffer.str().data(),256,0);
                
                std::cout << newPlayer->m_name << " joined the server! \n";
                GM_LOG(std::string(newPlayer->m_name) + " joined the crew!");

                for(auto p: connected_players){

                    GameEvent newEvent(game_event::PLAYER_CONNECTION);
                    newEvent.pushData((int)strlen(p.second.m_name));
                    newEvent.pushData(p.second.m_name, (int)strlen(p.second.m_name));
                    newEvent.pushData(p.first);

                    sendEventTo(newEvent.getData(), clientSocket);
                }

                connected_players.insert(std::make_pair(newPlayer->connection_id,*newPlayer));
                

                //AbstractEvent newEvent{game_event::PLAYER_CONNECTION, newPlayer->connection_id};
                //strcpy(newEvent.m_string, newPlayer->m_name);
                GameEvent newEvent(game_event::PLAYER_CONNECTION);
                    newEvent.pushData((int)strlen(newPlayer->m_name));
                    newEvent.pushData(newPlayer->m_name, (int)strlen(newPlayer->m_name));
                    newEvent.pushData(newPlayer->connection_id);
                sendEvent(newEvent.getData());
                

                delete newPlayer;
                std::cout << connected_players.at(lastConnectionId).m_name << "\n";
                
            }
            

            
            for(auto i = connected_players.begin(); i != connected_players.end();){
                if((*i).first == 0){
                    ++i;
                    continue;
                }
                char buffer[256];

                int infoLenght = recv((*i).second.m_socket, buffer, sizeof(buffer), 0);

                if(infoLenght > 0){

                    AbstractEvent* event = (AbstractEvent*)buffer;
                    int eventType = event->eventType;
                    std::cout << "event type: " << eventType << ", string: "<< event->m_string <<"\n";

                    std::array<char, 256> temp{};
                    std::memcpy(temp.data(), buffer, 256);
                    eventList.push(temp);

                }
                else if(infoLenght == 0){
                    std::cout << (*i).second.m_name<< " left the server \n";
                    int playerConnId = (*i).second.connection_id;
                    i = connected_players.erase(i);

                    GameEvent event(game_event::PLAYER_DISCONNECTION);
                    event.pushData(playerConnId);
                    sendEvent(event.getData());

                    continue;
                }else if(infoLenght < 0){
                    int err = WSAGetLastError();
                    if(err != WSAEWOULDBLOCK){
                        int playerConnId = (*i).second.connection_id;
                        std::cerr << "A unexpected error ocurred on one of the sockets: " << err << "| player: ["<< connected_players.at((*i).first).m_name <<"]\n";
                        i = connected_players.erase(i);
                        AbstractEvent event{game_event::PLAYER_DISCONNECTION, playerConnId};
                        sendEvent(&event);
                        continue;
                    }
                }
                ++i;
                
            }
        }
        // A client only sends its personal events and receive update events from the server.
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

int GameManager::hostServer(std::string server_name)
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
    GameManager::actual_server.m_name = server_name;
    GameManager::actual_server.start(port); // make the actual server valid;

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

    std::ostringstream event_buffer(std::ios::binary);
    int eventType = game_event::PLAYER_CONNECTION;
    event_buffer.write((char*)&eventType, sizeof(int));
    event_buffer.write((char*)&player, sizeof(Player));

    if(send(player.m_socket, event_buffer.str().data(), 256, 0) < 0){
        std::cerr << "Error: ["<< WSAGetLastError() << "] couldn't send the user's information to the server. \n";
        GM_LOG("Error: [" + std::to_string(WSAGetLastError()) + "] couldn't send the user's information to the server.", LOG_ERROR);
        
        disconnect();
        return false;
    }

    event_buffer.str("");

    std::cout << "ganna receive server info and id \n";
    // get it's connection id and updated server info:
    char data_buffer[256];
    if(recv(player.m_socket, data_buffer,256, 0) > 0){ // receive the connection id and the updated server information
        memcpy(&player.connection_id, data_buffer, sizeof(int)); // connection id
        GameManager::actual_server.setServerInfo(&data_buffer[sizeof(int)]); // server info
    }else{
        GM_LOG("Error: [" + std::to_string(WSAGetLastError()) + "] couldn't receive the socket conection id or information from server.", LOG_ERROR);
        std::cout << "Error: [" << std::to_string(WSAGetLastError()) << "] couldn't receive the socket conection id or information from server.\n";
        disconnect();
        return false;
    }

    std::cout << "ok until now.. \n";

    player.onlineStatus = SV_CLIENT; // set the user's online status as: CLIENT, since it's joining a server.



    // if the server is in mid match, send a request for the course folder name and hole file name:
    if(GameManager::actual_server.actual_course.size() > 0){ // (course name empty = not in game)
        
        GameEvent event(game_event::MATCH_MAP_REQUEST);
        event.pushData(player.connection_id);
        

        sendEvent(event.getData());
        std::cout << "Sended the request \n";

    }

    // initialize a thread responsible for manage the connection (receive or send data):
    std::thread connectionManagerThread(GameManager::manageConnections);
    connectionManagerThread.detach();

    changeScene("lobby");
    return true;
}

ServerInfo GameManager::searchServer()
{
    // this function assumes WinSock is initialized.
     ServerInfo server_found;

    // initialize UDP socket
    SOCKET udp_socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    // Enable broadcast
    BOOL bOpt = TRUE;
    setsockopt(udp_socket, SOL_SOCKET, SO_BROADCAST, (char*)&bOpt, sizeof(bOpt));

    // Define a timeOut
    DWORD timeout = 3000; // 3 seconds
    setsockopt(udp_socket, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));

    sockaddr_in udpAddr{};
    udpAddr.sin_family = AF_INET;
    udpAddr.sin_port = htons(DISCOVERY_PORT);
    udpAddr.sin_addr.s_addr = inet_addr("255.255.255.255");
    bind(udp_socket, (sockaddr*)&udpAddr, sizeof(udpAddr));

    // resquest the servers information via broadcast.

    sendto(udp_socket, DISCOVERY_MESSAGE, strlen(DISCOVERY_MESSAGE), 0, (sockaddr*)&udpAddr, sizeof(udpAddr));
    GM_LOG("Sent discovery request on broadcast.", LOG_WARNING);

    // wait for response.
    char buffer[256];
    sockaddr_in serverAddr{};
    int len = sizeof(serverAddr);
    int recvLen = recvfrom(udp_socket, buffer, sizeof(buffer), 0, (sockaddr*)&serverAddr, &len);

    std::cout << "[received response] lenght of the data received: " << recvLen << '\n';
    //GM_LOG("Received request response.", LOG_WARNING);

    if(recvLen <= 0){
        int err = WSAGetLastError();
        if(err == WSAETIMEDOUT){
            GM_LOG("No local servers were found :(");
        }else{
            GM_LOG("Error receiving the discovery request ["+ std::to_string(err) +"]", LOG_ERROR);
        }
        closesocket(udp_socket); //close udp socket
        return server_found;
    }

    closesocket(udp_socket); //close udp socket
    

    char received_message[21];
    memcpy(received_message, buffer, 20);
    received_message[20] = '\0'; // just in case
    if(std::strcmp(received_message, DISCOVERY_RESPONSE_MSG)){
        GM_LOG("Read a non-response data from broadcast!", LOG_WARNING);
        return server_found;
    }

    std::cout << "Now the data received will be read>> \n";
    server_found.setServerInfo(&buffer[20]);

    return server_found;
}

void GameManager::disconnect()
{
    if(player.onlineStatus == SV_DISCONNECTED){return;} // if the user is already disconnected, block the function.
    GameManager::actual_server.invalidate();
    if(player.onlineStatus == SV_HOSTING){
        /*
        If the user is the host, send a event, telling every connection(connected users)
        that the host is disconnected and force them to disconnect as well.
        */
        GameEvent event(game_event::SERVER_CLOSED);
        sendEvent(event.getData());
    }


    player.onlineStatus = SV_DISCONNECTED;  // set connection status to: DISCONNECTED.

    shutdown(player.m_socket, SD_BOTH);     // shutdown socket connection.
    closesocket(player.m_socket);           // close socket.
    player.m_socket = INVALID_SOCKET;       // define the socket as a invalid socket (just in case).

    connected_players.clear();              // clear all the stored past players information.
    lastConnectionId = 0;                   // set the last connection id to 0.
    
}
// -------------------------- //