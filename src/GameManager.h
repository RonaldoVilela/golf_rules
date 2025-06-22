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
     * Updates the GL_Viewport, the stored window dimention, and
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

    static Player player;
    static std::map<int , Player> connected_players;
    static std::queue<std::array<char, 256>> eventList;

    // The [lastConnectionId] is only used when the user is hosting a server.
    // It is used for setting a connected player's [connection_id], used
    // to identify and differentiate players more efficiently, each player who
    // connects will receive the [lastConnectionId] as it's [connection_id], then
    // the variable value will increase by one, wich will be used to set the id of
    // the next player who might connect.
    // .
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
     * @returns The port of the server's (user's) socket. If it returns 0
     * it means it failed to host the server.
     */
    static int hostServer();
    bool joinServer(const char* ipAddress, int port);

    static void disconnect();
};