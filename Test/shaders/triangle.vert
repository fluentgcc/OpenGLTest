#version 330

uniform mat4 MVP;

layout (location = 0) in vec4 vVertex;
layout (location = 1) in vec4 vColor;

out vec4 vs_fs_color;

void main(void)
{
    vs_fs_color = vColor;
    gl_Position = MVP * vVertex;
}