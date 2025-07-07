#pragma once

#define GM_LOG GameManager::log
#include <glad/glad.h>
#include "imgui.h"

#include "box2d/box2d.h"

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Shader.h"

#include "Texture.h"
#include "FontLoader.h"

#include "FrameBuffer.h"

#include "scenes/Scene.h"
#include "shapes/Shapes.h"

#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <queue>
#include <cstring>
#include <functional>
#include <winsock2.h>

#include "EventTypes.h"

enum shaderType{
    GM_BASIC_SHADER = 0,
    GM_FONT_SHADER = 1,
    GM_TEXTURE_SHADER = 2
};

enum logType{
    LOG_NORMAL = 0,
    LOG_WARNING = 1,
    LOG_ERROR = 2
};

enum online_status{
    SV_DISCONNECTED = 0,
    SV_HOSTING = 1,
    SV_CLIENT = 2,
};

struct textLog {
    ImVec4 color;
    std::string msg;
};

struct AbstractEvent{
    int eventType;
    int m_flag;
    char m_string[30];
};

enum ScreenRatios{
    RATIO_4X3 = 0,
    RATIO_16X10 = 1,
    RATIO_16X9 = 2
};

struct Dimension{
    int width;
    int height;
};

struct ImageData{
    int width, height;
    int m_bytesPerPixel, m_byteSize;
    unsigned char* m_data;
};

struct Bounds{
    float left;
    float right;

    float bottom;
    float top;
};

struct Player{
    char m_name[24];
    SOCKET m_socket = INVALID_SOCKET;
    int onlineStatus;
    int connection_id;

    Player(){
        strncpy(m_name, "DefaultName", sizeof(m_name));
    }
};

/**
 * This struct is used to store essencial informations about the server.
 * 
 * If the player is not connect to a server, the actual ServerInfo should just get
 * invalidated (ServerInfo.invalidate()).
 */
struct ServerInfo{

    bool valid = false; // Defines if the server is valid or not (Duhh)

    std::string m_name = "LAN server";
    std::string server_address;
    int m_port = 0;
    std::string actual_course = "";
    int online_players = 0;
    int playerId_1 = -1, playerId_2 = -1; // connection id of the competiting players

    /**
     * This socket is only initialized if you are hosting a server.
     * 
     * It will be used to detect other user's search requests on the local network
     * and respond with the actual ServerInfo data [ getServerInfo() ].
     * 
     * OBS: Notice that this is a UDP socket, so it can send and read data
     * from broadcast, differently from TCP sockets (used by the players).
     */
    SOCKET udp_socket;

    /**
     * Set the server port, initializes it's UDP socket and validates the server.
     * This function will be usually used just when hosting a new server.
     */
    void start(int server_port);

    void invalidate();

    /**
     * Set the actual ServerInfo values acoording to the binary data passed
     * as paramether.
     * 
     * @param data The data wich will be read.
     * @attention This data paramether should be created by another ServerInfo, using the [ getServerInfo() ] function.
     */
    bool setServerInfo(char* data);

    /**
     * Transform the actual ServerInfo values into a char array, wich can be used
     * in the [ setServerInfo(char* data) ] function of another ServerInfo.
     * 
     * @returns The actual ServerInfo data as a character array
     */
    char* getServerInfo(int players_online);
};

//TODO: find a better way of declaring those structs

class GameManager{
private:

    const unsigned int screenVexIndicies[6]{
        0, 1, 2,
        1, 2, 3
    };

    VertexBuffer screenVertexBuffer;
    IndexBuffer screenIndexBuffer;
    VertexArray screenVertexArray;

    //commands//
    std::unordered_map< std::string, std::function<void(std::vector<std::string>)> > console_commands;
    void loadConsoleCommands();

    //Shaders
    static std::vector<Shader*> shaders;

    std::vector<std::vector<Dimension>>screenResolutions;

    GLFWwindow* window;

public:
    
    GameManager(GLFWwindow* window);
    ~GameManager();

    // CONSOLE ================ //
    /*these variables will be defined and used (in most cased) in: GameManager_Commands.cpp*/
    char actualCommand [126];
    bool consoleOpen = false;
    static std::vector<textLog> console_cache;

    void execCommand(std::string command);
    static void log(std::string msg, int logType = 0);
    // ================================

    static FontAtlas font;

    // RENDERING ====================== //

    // frameBuffer that will be used to draw Ui elements (uses window pixel position).

    FrameBuffer ui_FrameBuffer;
    void drawUiFrameBuffer();

    // default abstract shapes for debug rendering.

    shapes::Circle* circle;
    shapes::Square* square;
    shapes::Line* line;

    // scenes
    std::unordered_map<std::string, scene::Scene*> scenes;
    scene::Scene* actualScene = nullptr;
    std::string actual_scene_name = "";

    void changeScene(std::string name);

    // shaders
    static Shader* activeShader;

    void addShader(Shader* shader);
    static void useShader(int index);

    static int getBinaryImages(std::string dat_file, std::map<std::string, ImageData>* image_data_buffer);

    // ==================================


    // Window and Viewport ======== //

    glm::mat4 projection;
    glm::mat4 windowProjection;
    
    Dimension screenSize;
    Bounds screenBounds;

    /**
     * Updates the GL_Viewport, the stored window dimension, and
     * automatically defines a screen ratio acoording to the new 
     * window size.
     * 
     * @param window The actual GLFW Window being used.
     * 
     * @param width the new desired window width.
     * @param height the new desired window height.
     */
    void updateScreenSize(GLFWwindow* window, int width, int height);

    // ===========================


    // INPUT ======================= //
    static float scrollFactor;
    glm::vec2 mousePos;
    glm::vec4 mouseNormPos;

    void updateMousePos(GLFWwindow* window);
    void handleEvents(GLFWwindow* window);
    static void scroll_callback(GLFWwindow *window, double offsetX, double offsetY);
    // =============================



    // CONNECTION ==================== //
    /* functions in here will be defined in: GameManager_Connections.cpp */

    static ServerInfo actual_server;            // actual server data
    static Player player;                       // user/player's data

    static const int DISCOVERY_PORT = 8080;     // fixed discovery port (might run into some problems)

    static std::map<int , Player> connected_players;
    static std::queue<std::array<char, 256>> eventList;


    // The [lastConnectionId] is only used when the user is hosting a server.
    // It is used for setting a connected player's [connection_id], used
    // to identify and differentiate players more efficiently, each player who
    // connects will receive the [lastConnectionId] as it's [connection_id], then
    // the variable value will increase by one, wich will be used to set the id of
    // the next player who might connect.
    // 
    // Obs: The [lastConnectionId] must be set back to zero [0] when stop hosting a server!!
    static int lastConnectionId;

    static bool isWsaInitialized();

    /**
     * Sends a buffer of a event to the host, and
     * if you ARE THE HOST send to all the connected players.
     * 
     * every event must: 
     *  - start with a event_type (int);
     *  - not be bigger than 256 bytes;
     */
    static void sendEvent(void* eventBuffer);

    static void sendEventTo(void* eventBuffer, SOCKET dest);
    static void manageConnections();

    /**
     * Set the user's socket as a host, set it available for
     * connections, and set the user's online status as SV_HOSTING.
     * 
     * @param server_name the name of the server (Leave empty to give a default name)
     * @returns The port of the server's (user's) socket. If it returns 0
     * it means it failed to host the server.
     */
    static int hostServer(std::string server_name = std::string(player.m_name) + "'s Server");

    /**
     * Obs: This function automatically calls disconnect() when it fails.
     */
    bool joinServer(const char* ipAddress, int port);

    ServerInfo searchServer();

    static void disconnect();
    // =============================
};