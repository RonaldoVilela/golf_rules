#pragma once

#include <glad/glad.h>
//#include <glm/glm.hpp>

#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"

#define ASSERT(x) if(!(x))__builtin_trap()
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__));

namespace shapes{
    class Shape;
    class Line;
}

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);

class Renderer{
private:
    // TEXT RENDERING ================== //
    static VertexBuffer* textVertexBuffer;
    static IndexBuffer* textIndexBuffer;
    static VertexArray* textVertexArray;
    // ================================

    // default buffers for rendering conventional elements such as Rects and Textures 
    //
    static VertexBuffer* defaultVertexBuffer;
    static IndexBuffer* defaultIndexBuffer;
    static VertexArray* defaultVertexArray;
public:
    static void Init();
    static void Close();

    static void Clear();
    static void Draw(const VertexArray& va, const IndexBuffer& ib, const Shader& shader);
    static void Draw(const VertexArray& va, const IndexBuffer& ib, unsigned int indiciesCount, const Shader& shader);
    static void drawShape(shapes::Shape* shape, float x, float y, float w_scale, float h_scale, float rotation, glm::vec4 color);
    static void drawShape(shapes::Shape* shape, float x, float y, float scale, glm::vec4 color);

    static void drawLine(shapes::Line* line, float pos1[2], float pos2[2], glm::vec4 color);
    static void drawSegment(shapes::Line* line, float* vertices, unsigned int count, glm::vec4 color);

    static void drawString(std::string text,int x, int y, float scale ,glm::vec4 color, int h_Align = 0, int v_Align = 0);

    static void drawTextures(float x, float y, float w, float h);
};