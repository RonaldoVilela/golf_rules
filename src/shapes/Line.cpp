#include "Shapes.h"

namespace shapes{

    Line::Line(){
        vb = new VertexBuffer(nullptr, sizeof(float)*2* 100, GL_DYNAMIC_DRAW);
        va = new VertexArray();
        shapeLayout.Push<float>(2);
        va->AddBuffer(*vb, shapeLayout);
    }
};