#include "VertexBuffer.h"
#include "Renderer.h"

VertexBuffer::VertexBuffer(const void *data, unsigned int size, unsigned int usage)
{
    GLCall(glGenBuffers(1, &m_RendererID));
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID)); // liga e define o objetivo do Buffer
    GLCall(glBufferData(GL_ARRAY_BUFFER, size, data, usage)); // insere os dados do buffer
}

VertexBuffer::~VertexBuffer()
{
    GLCall(glDeleteBuffers(1 , &m_RendererID));
}

void VertexBuffer::Bind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_RendererID));
}

void VertexBuffer::Unbind() const
{
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}
