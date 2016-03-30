#version 330 core
layout (location = 0) in vec3 vVertex;
layout (location = 1) in vec2 vTexCoord;
out vec2 TexCoords;

uniform mat4 projection;

void main()
{
    gl_Position = projection * vec4(vVertex.xy, 0.0 , 1.0);
    TexCoords = vTexCoord;
}