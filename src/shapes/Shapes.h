#pragma once

#include "Shader.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "VertexArray.h"

namespace shapes{

    class Shape{
        public:
        VertexBufferLayout shapeLayout;

        VertexBuffer* vb;
        VertexArray* va;
        unsigned int count;

        Shape(){}
        ~Shape(){
            delete vb;
            delete va;

            std::cout << "shapeDeleted \n";
        }
    };

    class Circle: public Shape{
    public:
        Circle();
    };

    class Square: public Shape{
        public:
            Square();
    };

    class Line: public Shape{
        public:
            Line();
    };
}