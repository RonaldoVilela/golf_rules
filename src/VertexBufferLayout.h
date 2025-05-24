#pragma once
#include <vector>
#include <glad/glad.h>
#include "Renderer.h"

struct VertexBufferElement{

    unsigned int type;
    unsigned int count;
    unsigned char normalized;

    /**
     * returns the size in bytes of the specified type. ex: (float , unsigned int, unsigned byte)
     * 
     * @return the size in bytes (unsigned int) of the specified type;
     */
    static unsigned int getSizeOfType(unsigned int type){
        switch(type){
            case GL_FLOAT:          return sizeof(GLfloat);
            case GL_UNSIGNED_INT:   return sizeof(GLuint);
            case GL_UNSIGNED_BYTE:  return sizeof(GLubyte);
        }
        ASSERT(false);

        return 0;
    }
};

class VertexBufferLayout{
private:
    std::vector<VertexBufferElement> m_Elements; // elements of each stride
    unsigned int m_Stride; // size of each stride in bytes
public:
    /**
     * Creates a VertexBufferLayout.
     * 
     * With it you can define a layout of a stride. 
     * It stores the elements of the strides, and it's size in bytes
     * acoording to the layout and it's element's values.
     */
    VertexBufferLayout() : m_Stride(0){}

    /**
     * Inserts a new element to the strides of this layoult.
     * Ex:
     * position{x, y, z} -> push<float>(3).
     * color{r, g, b, a} -> push<float>(4).
     * texPos{s, t} -> push<float>(2).
     * 
     * @param count the number of values of the element to be added.
     */
    template<typename T>
    void Push(unsigned int count);

    inline const std::vector<VertexBufferElement> GetElements() const {return m_Elements;}

    /**
     * returns the size in bytes of each stride in this layout.
     * 
     * @return the stride size in bytes.
     */
    inline unsigned int GetStride() const {return m_Stride;}
};

    /**
     * Inserts a new element to the strides of this layoult.
     * Ex: position{x, y, z} -> push<float>(3).
     *     color{r, g, b, a} -> push<float>(4).
     *     texPos{s, t} -> push<float>(2).
     * 
     * @param count the number of values of the element to be added.
     */
template<>
inline void VertexBufferLayout::Push<float>(unsigned int count){
    m_Elements.push_back({GL_FLOAT,count, GL_FALSE});
    m_Stride += count * VertexBufferElement::getSizeOfType(GL_FLOAT);
};

template<>
inline void VertexBufferLayout::Push<unsigned int>(unsigned int count){
    m_Elements.push_back({GL_UNSIGNED_INT,count, GL_FALSE});
    m_Stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_INT);
};

template<>
inline void VertexBufferLayout::Push<unsigned char>(unsigned int count){
    m_Elements.push_back({GL_UNSIGNED_BYTE,count, GL_TRUE});
    m_Stride += count * VertexBufferElement::getSizeOfType(GL_UNSIGNED_BYTE);
};