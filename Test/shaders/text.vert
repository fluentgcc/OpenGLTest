#version 330
layout (location = 0) in vec4 vVertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 MVP;

void main()
{
    gl_Position = MVP * vec4(vVertex.xy, 0.0, 1.0);
    TexCoords = vVertex.zw;
}