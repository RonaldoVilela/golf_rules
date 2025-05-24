#include "FrameBuffer.h"
#include "Renderer.h"

FrameBuffer::FrameBuffer(int width, int height)
{
    //generateFrameBuffer
    GLCall(glGenFramebuffers(1,&m_RendererId));
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));

    //generate texture
    GLCall(glGenTextures(1, &m_TextureId));
    GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureId));

    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
    GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));

    GLCall(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_TextureId, 0));

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

FrameBuffer::~FrameBuffer()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    GLCall(glDeleteFramebuffers(1, &m_RendererId));
    GLCall(glDeleteTextures(1, &m_TextureId));
}

void FrameBuffer::resize(int width, int height)
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));

    GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureId));
    GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void FrameBuffer::Bind()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, m_RendererId));
}

void FrameBuffer::Unbind()
{
    GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
}

void FrameBuffer::BindTexture()
{
    GLCall(glBindTexture(GL_TEXTURE_2D, m_TextureId));
}

void FrameBuffer::UnbindTexture()
{
    GLCall(glBindTexture(GL_TEXTURE_2D, 0));
}
