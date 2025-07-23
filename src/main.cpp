#define _WIN32_WINNT 0x0501

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>

#include "box2d/box2d.h"

#include <fstream>
#include <iostream>
#include <sstream>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Renderer.h"
#include "GameManager.h"

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"
#include "Shader.h"

#include "Texture.h"

#include "scenes/Scene.h"
#include "scenes/ClearColorTest.h"
#include "scenes/TextureTransformTest.h"
#include "scenes/QuadBashTest.h"
#include "scenes/DynamicTest.h"
#include "scenes/Match.h"

#include <thread>  // Para std::this_thread::sleep_for
#include <chrono>  // Para std::chrono::duration

void processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void error_callback(int error, const char* description);

int main(){

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()){
        return -1;
    }
   
    GLFWwindow* window;
    
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1024, 768, "Hello World", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create GLFW window: \n";
        glfwTerminate();
        return -1;
    }
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    //glfwSwapInterval(1);

    if(!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)){
        std::cout << "Failed to initialize GLAD: \n";
    }

    std::cout << glGetString(GL_VERSION) << "\n";

    { // extra scope to make sure the classes destructors are called before OpenGL closes.
        
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 130");

        GLCall(glEnable(GL_BLEND));
        GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));

        GameManager gameManager(window);
        Renderer::Init();
        
        gameManager.updateScreenSize(window,1024, 768);
        gameManager.changeScene("server_list");

        glfwSetScrollCallback(window, GameManager::scroll_callback);
        
        
        /* Loop until the user closes the window */

        double lastTime = glfwGetTime();
        float deltaTime = 0;
        float frameTime = 1.0f/60.0f;
        
        while (!glfwWindowShouldClose(window))
        {
            
            glfwPollEvents();
            
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            if (width <= 0 || height <= 0) {continue;}

            gameManager.handleEvents(window);
            processInput(window);
            gameManager.actualScene->HandleEvents(window);
            
            double currentTime = glfwGetTime();
            deltaTime = currentTime - lastTime;
            lastTime = currentTime;

            Renderer::Clear();
            
            gameManager.ui_FrameBuffer.Bind();
            glViewport(0,0, gameManager.screenSize.width, gameManager.screenSize.height);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            GLCall(glClear(GL_COLOR_BUFFER_BIT));
            gameManager.ui_FrameBuffer.Unbind();

            gameManager.actualScene->OnUpdate(deltaTime);
            gameManager.actualScene->OnRender();

            gameManager.drawUiFrameBuffer();

            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
            
            gameManager.actualScene->OnImGuiRender();
            glClearColor(0.3f, 0.2f, 0.1f, 1.0f);

            //Console
            {
                if(gameManager.consoleOpen && ImGui::Begin("Console", &gameManager.consoleOpen, ImGuiWindowFlags_NoCollapse)){

                    ImGui::BeginChild("Console_cache", ImVec2(0, ImGui::GetWindowSize().y - 60), true, ImGuiWindowFlags_NoTitleBar);
                    for(auto l: GameManager::console_cache){
                        //ImGui::Text();
                        ImGui::PushStyleColor(ImGuiCol_Text, l.color);
                        ImGui::TextWrapped(l.msg.c_str());
                        ImGui::PopStyleColor();
                    }
                    ImGui::EndChild();
                    
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    if(ImGui::InputText("",gameManager.actualCommand, 126, ImGuiInputTextFlags_EnterReturnsTrue)){
                        GM_LOG("> "+ (std::string)gameManager.actualCommand);
                        gameManager.execCommand(gameManager.actualCommand);
                        strcpy(gameManager.actualCommand, "");
                    }

                    ImGui::End();
                }
            }

            //std::cout << deltaTime << '\n';

            ImGui::Render();
            int display_w, display_h;
            glfwGetFramebufferSize(window, &display_w, &display_h);
            glViewport(0, 0, display_w, display_h);
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            glfwSwapBuffers(window);

            double frameEndTime = glfwGetTime();
            double frameDuration = frameEndTime - currentTime;

            if (frameDuration < frameTime) {
                double sleepTime = (frameTime - frameDuration) * 1000.0; // ms
                std::this_thread::sleep_for(std::chrono::milliseconds((int)sleepTime));
            }
        }

        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

    }
    

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height){
    if(width > 0 && height > 0){
        glViewport(0, 0, width, height);
    }
}

void processInput(GLFWwindow* window){
    
}

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}
