#version 330

uniform mat4 MVP;

layout (location = 0) in vec4 vVertex;
layout (location = 1) in vec2 vTextCoord;

out vec2 textCoord;

void main(void)
{
	textCoord = vTextCoord;
    gl_Position = MVP * vVertex;
}