#version 330

in vec2 textCoord;
out vec4 color;

uniform sampler2D text;

void main(void)
{
    color = vec4( 1.0, 1.0, 0.0, texture( text, textCoord ).r );
	//color  = texture( text, textCoord );
}