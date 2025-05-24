#pragma once

class VertexBuffer{
private:
    unsigned int m_RendererID;

public:
    /**
     * Generates a vertex array buffer from a array of values
     * 
     * @param data the data array
     * 
     * @param size the "data" size in bytes
     */
    VertexBuffer(const void* data, unsigned int size, unsigned int usage);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
};