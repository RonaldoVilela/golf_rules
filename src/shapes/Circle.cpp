#include "Shapes.h"
#include <math.h>

namespace shapes{

    Circle::Circle(){
        const float vertices[24]={
            0.0f, 0.5f,
            cos(3.1415f/3)/2, sin(3.1415f/3)/2,
            cos(3.1415f/6)/2, sin(3.1415f/6)/2,
            0.5f, 0.0f,
            cos(11 * 3.1415f/6)/2, sin(11 * 3.1415f/6)/2,
            cos(5 * 3.1415f/3)/2, sin(5 * 3.1415f/3)/2,
            0.0f, -0.5f,
            cos(4 * 3.1415f/3)/2, sin(4 * 3.1415f/3)/2,
            cos(7 * 3.1415f/6)/2, sin(7 * 3.1415f/6)/2,
            -0.5f, 0.0f,
            cos(5 * 3.1415f/6)/2, sin(5 * 3.1415f/6)/2,
            cos(2 * 3.1415f/3)/2, sin(2 * 3.1415f/3)/2
        };

        count = 12;

        vb = new VertexBuffer(vertices, sizeof(vertices), GL_STATIC_DRAW);
        va = new VertexArray();
        shapeLayout.Push<float>(2);
        va->AddBuffer(*vb, shapeLayout);
    }
};