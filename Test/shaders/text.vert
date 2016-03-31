#version 330
layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec2 vTexCoord;
out vec2 TexCoords;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vVertex.xyz, 1.0);
    TexCoords = vTexCoord;
}