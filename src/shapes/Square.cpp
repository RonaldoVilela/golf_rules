#include "Shapes.h"

namespace shapes{
    Square::Square(){
        const float vertices[8]={
            -0.5f, -0.5f,
            -0.5f, 0.5f,
            0.5f, 0.5f,
            0.5f, -0.5f
        };

        count = 4;

        vb = new VertexBuffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
        va = new VertexArray();
        shapeLayout.Push<float>(2);
        va->AddBuffer(*vb, shapeLayout);
    }
};