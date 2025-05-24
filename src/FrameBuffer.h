#pragma once

class FrameBuffer{
private:
    unsigned int m_RendererId;
    unsigned int m_RenderBufferId;
    unsigned int m_TextureId;
public:
    
    FrameBuffer(int width, int height);
    ~FrameBuffer();

    void resize(int width, int height);

    void Bind();
    void Unbind();

    void BindTexture();
    void UnbindTexture();
};