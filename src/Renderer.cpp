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

void Renderer::Init()
{
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

void Renderer::drawString(std::string text,int x, int y, glm::vec4 color, int h_Align, int v_Align)
{
    if(text.length() == 0){return;}

    float lastLetterAdvance = 0;
    for(int i = 0; i < text.length(); i++){
        float vertices[16] = {
            0.0f + lastLetterAdvance + x, GameManager::font.glyphs[text[i]].height + y + GameManager::font.glyphs[text[i]].yoff,                                         GameManager::font.glyphs[text[i]].x0, GameManager::font.glyphs[text[i]].y1,
            0.0f + lastLetterAdvance + x, 0.0f + y + GameManager::font.glyphs[text[i]].yoff,                                                                               GameManager::font.glyphs[text[i]].x0, GameManager::font.glyphs[text[i]].y0,
            GameManager::font.glyphs[text[i]].width + lastLetterAdvance + x, GameManager::font.glyphs[text[i]].height + y + GameManager::font.glyphs[text[i]].yoff,    GameManager::font.glyphs[text[i]].x1, GameManager::font.glyphs[text[i]].y1,
            
            GameManager::font.glyphs[text[i]].width + lastLetterAdvance + x, 0.0f + y + GameManager::font.glyphs[text[i]].yoff,                                           GameManager::font.glyphs[text[i]].x1, GameManager::font.glyphs[text[i]].y0
        };
        lastLetterAdvance += GameManager::font.glyphs[text[i]].xadvance;

        unsigned int vertexIndexOffset = i*4;

        unsigned int indices[6]{
            0 + vertexIndexOffset, 1 + vertexIndexOffset, 2 + vertexIndexOffset,
            1 + vertexIndexOffset, 2 + vertexIndexOffset, 3 + vertexIndexOffset
        };

        GameManager::textVertexBuffer->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, i*sizeof(float)* 16, sizeof(float)* 16, vertices);

        GameManager::textIndexBuffer->Bind();
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, i*6*sizeof(unsigned int), sizeof(unsigned int)* 6, indices);
    }

    GameManager::useShader(GM_FONT_SHADER);
    GameManager::activeShader->SetUniform4f("u_Color", color.x, color.y, color.z, color.w);
    Draw(*GameManager::textVertexArray, *GameManager::textIndexBuffer, text.length()*6, *GameManager::activeShader);
}
