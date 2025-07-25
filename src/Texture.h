#pragma once

#include "Renderer.h"

class Texture{
private:
    unsigned int m_RendererID;
    std::string m_FilePath;
    unsigned char* m_LocalBuffer;
    int m_Width, m_Height, m_BytesPerPixel;

public:
    /**
     * Generates a texture from a image. 
     * 
     * @param path the path of the image file
     */
    Texture(const std::string& path);
    Texture(int width, int height, int bytesPerPixel, unsigned char* data);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    inline int GetWidth() const {return m_Width;}
    inline int GetHeight() const {return m_Height;}
};