#include "Renderer.h"
#include "shapes/Shapes.h"
#include "GameManager.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

void GLClearError(){
    while(glGetError());
}

bool GLLogCall(const char* function, const char* file, int line){
    while(GLenum error = glGetError()){
        std::cout << "[OpenGL error: " << error << "]: " << function << " - " << file << " : " << line << '\n';
        return false;
    }
    return true;
}

VertexBuffer* Renderer::textVertexBuffer;
IndexBuffer* Renderer::textIndexBuffer;
VertexArray* Renderer::textVertexArray;

VertexBuffer* Renderer::defaultVertexBuffer;
IndexBuffer* Renderer::defaultIndexBuffer;
VertexArray* Renderer::defaultVertexArray;

void Renderer::Init()
{
    /*
        Load a dynamic vertexBuffer, wich can and will be used to store texts letter's vertices, for
        later text rendering operations.
    */
    textVertexBuffer = new VertexBuffer(nullptr ,sizeof(float)*16*1000 ,GL_DYNAMIC_DRAW);
    textIndexBuffer = new IndexBuffer(nullptr,6*1000);
    textVertexArray = new VertexArray();

    VertexBufferLayout layout;
    layout.Push<float>(2);
    layout.Push<float>(2);
    
    textVertexArray->AddBuffer(*textVertexBuffer, layout);

    // default buffers for rendering basic stuff, like rects ans texture quads.
    defaultVertexBuffer = new VertexBuffer(nullptr ,sizeof(float)*16*1000 ,GL_DYNAMIC_DRAW);
    defaultIndexBuffer = new IndexBuffer(nullptr,6*1000);;
    defaultVertexArray = new VertexArray();

    defaultVertexArray->AddBuffer(*defaultVertexBuffer, layout);
}

void Renderer::Close()
{
    delete textVertexBuffer;
    delete textIndexBuffer;
    delete textVertexArray;
}

void Renderer::Clear()
{
    GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void Renderer::Draw(const VertexArray &va, const IndexBuffer &ib, const Shader &shader)
{
    shader.Bind();

    va.Bind();
    ib.Bind();

    GLCall(glDrawElements(GL_TRIANGLES, ib.getCount(), GL_UNSIGNED_INT, nullptr));
}

void Renderer::Draw(const VertexArray &va, const IndexBuffer &ib, unsigned int indiciesCount, const Shader &shader)
{
    shader.Bind();

    va.Bind();
    ib.Bind();

    GLCall(glDrawElements(GL_TRIANGLES, indiciesCount, GL_UNSIGNED_INT, nullptr));
}

void Renderer::drawShape(shapes::Shape* shape, float x, float y, float w_scale, float h_scale, float rotation, glm::vec4 color)
{
    GameManager::useShader(GM_BASIC_SHADER);

    GameManager::activeShader->SetUniformMat4f("u_Model", glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f))*
        glm::scale(glm::mat4(1.0f), glm::vec3(w_scale, h_scale, 1.0f)) * 
        glm::rotate(glm::mat4(1.0f),rotation, glm::vec3(0.0f, 0.0f, 1.0f)));

    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);

    shape->va->Bind();
    glDrawArrays(GL_LINE_LOOP, 0, shape->count);

}

void Renderer::drawShape(shapes::Shape* shape, float x, float y, float scale, glm::vec4 color)
{
    GameManager::useShader(GM_BASIC_SHADER);

    GameManager::activeShader->SetUniformMat4f("u_Model", glm::translate(glm::mat4(1.0f), glm::vec3(x, y, 0.0f))*
        glm::scale(glm::mat4(1.0f), glm::vec3(scale, scale, 1.0f)));

    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);

    shape->va->Bind();
    glDrawArrays(GL_LINE_LOOP, 0, shape->count);

}

void Renderer::drawLine(shapes::Line* line, float pos1[2], float pos2[2], glm::vec4 color)
{
    GameManager::useShader(GM_BASIC_SHADER);

    GameManager::activeShader->SetUniformMat4f("u_Model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))*
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);

    float vertices[4] = {
        pos1[0], pos1[1],
        pos2[0], pos2[1]
    };
    line->va->Bind();
    line->vb->Bind();
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices));

    line->va->Bind();
    GLCall(glDrawArrays(GL_LINE_STRIP, 0, 2));

}

void Renderer::drawSegment(shapes::Line *line, float *vertices, unsigned int count, glm::vec4 color)
{
    GameManager::useShader(GM_BASIC_SHADER);
    GameManager::activeShader->SetUniformMat4f("u_Model", glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f))*
        glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)));

    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);

    line->va->Bind();
    line->vb->Bind();
    GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float) * count * 2, vertices));

    line->va->Bind();
    GLCall(glDrawArrays(GL_LINE_STRIP, 0, count));
}


void Renderer::drawString(std::string text,int x, int y, float scale, glm::vec4 color, int h_Align, int v_Align)
{
    if(text.length() == 0){return;}

    float lastLetterAdvance = 0;
    for(int i = 0; i < text.length(); i++){
        
        Glyph actual_glyph = GameManager::font.glyphs[text[i]];

        float vertices[16] = {
            0.0f + (lastLetterAdvance + x) * scale, (actual_glyph.height + y + actual_glyph.yoff) * scale,                      actual_glyph.x0, actual_glyph.y1,
            0.0f + (lastLetterAdvance + x) * scale, 0.0f + (y + actual_glyph.yoff)*scale,                                       actual_glyph.x0, actual_glyph.y0,
            (actual_glyph.width + lastLetterAdvance + x) * scale, (actual_glyph.height + y + actual_glyph.yoff) * scale,        actual_glyph.x1, actual_glyph.y1,
            
            (actual_glyph.width + lastLetterAdvance + x) * scale, 0.0f + (y + actual_glyph.yoff) * scale,                       actual_glyph.x1, actual_glyph.y0
        };
        lastLetterAdvance += actual_glyph.xadvance;

        unsigned int vertexIndexOffset = i*4;

        unsigned int indices[6]{
            0 + vertexIndexOffset, 1 + vertexIndexOffset, 2 + vertexIndexOffset,
            1 + vertexIndexOffset, 2 + vertexIndexOffset, 3 + vertexIndexOffset
        };

        textVertexBuffer->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, i*sizeof(float)* 16, sizeof(float)* 16, vertices);

        textIndexBuffer->Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i*6*sizeof(unsigned int), sizeof(unsigned int)* 6, indices);
    }

    GameManager::useShader(GM_FONT_SHADER);
    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);
    Draw(*textVertexArray, *textIndexBuffer, text.length()*6, *GameManager::activeShader);
}

void Renderer::drawTextures(float x, float y, float w, float h)
{
    float vertices[16] = {
        x, y,       0.0f, 1.0f,
        x+w, y,     1.0f, 1.0f,
        x, y+h,     0.0f, 0.0f,
        
        x+w, y+h,   1.0f, 0.0f,
        
    };
    unsigned int indices[6]{
        0,1,2,
        1,2,3
    };
    defaultVertexBuffer->Bind();
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(float)* 16, vertices);

    defaultIndexBuffer->Bind();
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(unsigned int)* 6, indices);

    Draw(*defaultVertexArray, *defaultIndexBuffer, 6, *GameManager::activeShader);
}
