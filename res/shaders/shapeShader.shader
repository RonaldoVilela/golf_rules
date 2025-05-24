#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 a_Color;
layout (location = 2) in vec2 texCoord;

out vec2 v_TexCoord;
out vec4 v_Color;
uniform mat4 u_MVP;

void main(){
    gl_Position = u_MVP * position;
    v_Color = a_Color;
    v_TexCoord = texCoord;
}

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec2 v_TexCoord;
in vec4 v_Color;

void main(){
    color = vec4(v_Color.x, v_Color.y, v_Color.z, v_TexCoord.y);
}