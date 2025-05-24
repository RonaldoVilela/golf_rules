#pragma once
#include "VertexBuffer.h"

class VertexBufferLayout;

class VertexArray{
private:
    unsigned int m_RendererID;
public:
    /**
     * Generate a new Vertex Array.
     * A Vertex Array holds a VertexBuffer
     * and a VertexBufferLayoult.
     * 
     * use AddBuffer() to add the buffers
     */
    VertexArray();
    ~VertexArray();

    /**
     * Adds a VertexBuffer and it's VertexBufferLayout to the VertexArray
     * so it knows how which stride/vertex in the vertex buffer is organized.
     * 
     * @param vb A VertexBuffer object witch contains the needed data.
     * 
     * @param layout A VertexBufferLayout object witch difines a
     * layout to the data contained in the VertexBuffer(vb).
     */
    void AddBuffer(const VertexBuffer& vb, const VertexBufferLayout layout);

    void Bind() const;
    void Unbind() const;
};