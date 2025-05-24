#pragma once

class IndexBuffer{
private:
    unsigned int m_RendererID;
    unsigned int m_Count;

public:
    /**
     * Generates a index buffer/element array from a array of unsigned ints.
     * 
     * @param data the array of unsigned ints, who represents the indices of the vertices.
     * @param count the number of elements of the array.
     */
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    /** 
     * returns the total number of elements of the Index Buffer
     * 
     * @return the total number of elements of the Index Buffer
    */
    inline unsigned int getCount() const {return m_Count;}
};