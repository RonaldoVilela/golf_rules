#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 a_TexCoor;
layout (location = 3) in float a_TexId;

out vec4 v_Color;
out vec2 v_TexCoor;
out float v_TexId;
uniform mat4 u_MVP;

void main(){
    v_Color = a_Color;
    v_TexCoor = a_TexCoor;
    v_TexId = a_TexId;
    gl_Position = u_MVP * position;
}

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;
in vec4 v_Color;
in vec2 v_TexCoor;
in float v_TexId;

uniform sampler2D u_Textures[2];

void main(){
    int index = int(v_TexId);
    color = texture(u_Textures[index], v_TexCoor);
}