#include "Triangle.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#define GL_CHECK_ERRORS assert( glGetError()== GL_NO_ERROR );

CTriangle::CTriangle(void)
{

	shader_.LoadFromFile( GL_VERTEX_SHADER,   "shaders/triangle.vert" );
	shader_.LoadFromFile( GL_FRAGMENT_SHADER, "shaders/triangle.frag" );
	shader_.CreateAndLinkProgram();
	shader_.Use();
	{
		shader_.AddAttribute("vVertex"); 
		shader_.AddUniform("MVP");
		shader_.AddAttribute("vColor");
		//glUniform3fv( shader_("vColor"),1, glm::value_ptr( color ) );
	}
	shader_.UnUse();

	GL_CHECK_ERRORS

	static const GLfloat vertex_pos[] = 
	{
		-0.5f, -0.5f, -2.0f, 1.0f,
		 0.5f, -0.5f, -2.0f, 1.0f,
		 0.5f,  0.5f, -2.0f, 1.0f,
		-0.5f,  0.5f, -2.0f, 1.0f,
	};

	static const GLfloat vertex_colors[] =
	{
		1.0f, 1.0f, 1.0f, 1.0f,
		1.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 1.0f
	};

	static const GLushort vertex_indeces[] = 
	{
		0, 1, 2, 0, 2, 3
	};

	glGenVertexArrays( 1, &this->vaoID_ );
	glGenBuffers( 1, &this->vboVerticesID_ );
	glGenBuffers( 1, &this->vboIndicesID_ );

	glBindVertexArray( this->vaoID_ );
	{
		glBindBuffer( GL_ARRAY_BUFFER, vboVerticesID_ );
		glBufferData( GL_ARRAY_BUFFER, sizeof( vertex_pos ) + sizeof( vertex_colors ), nullptr, GL_STATIC_DRAW );
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof( vertex_pos ), vertex_pos );
		glBufferSubData(GL_ARRAY_BUFFER, sizeof( vertex_pos ), sizeof( vertex_colors ), vertex_colors );

		GL_CHECK_ERRORS

		glEnableVertexAttribArray( this->shader_[ "vVertex" ] );
		glVertexAttribPointer( shader_[ "vVertex" ], 4, GL_FLOAT, GL_FALSE, 0, NULL );
		GL_CHECK_ERRORS

		glEnableVertexAttribArray( this->shader_[ "vColor" ] );
		glVertexAttribPointer( shader_[ "vColor" ], 4, GL_FLOAT, GL_FALSE, 0, ( const GLvoid * )sizeof( vertex_pos ) );
		//上述两个函数的解释:
		//glEnableVertexAttribArray, glDisableVertexAttribArray ： 启用/禁用 对应的顶点属性数组;
		//glVertexAttribPointer; 指定访问值;
		GL_CHECK_ERRORS

		//element array buffer;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID_ );
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof( vertex_indeces ), &vertex_indeces[0], GL_STATIC_DRAW );
		GL_CHECK_ERRORS
	}
}

CTriangle::~CTriangle(void)
{

}

void CTriangle::render(const float* MVP)
{
	shader_.Use();
	{
		glUniformMatrix4fv( shader_( "MVP" ) , 1, GL_FALSE, MVP  );
		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL );
		//glDrawArrays( GL_LINE_LOOP, 0, 4 );
		//glDrawElementsBaseVertex( GL_TRIANGLES, 3, GL_UNSIGNED_SHORT, NULL, 1 );
	}
	shader_.UnUse();
}


