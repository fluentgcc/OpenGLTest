#include "RenderableObject.h"
#include <glm/glm.hpp>

RenderableObject::RenderableObject(void)
{
	
}


RenderableObject::~RenderableObject(void)
{
	destroy();
}

void RenderableObject::init() 
{
	//setup vao and vbo stuff
	glGenVertexArrays( 1, &vaoID_ );
	glGenBuffers( 1, &vboVerticesID_ );
	glGenBuffers( 1, &vboIndicesID_ );

	//get total vertices and indices
	totalVertices_ = getTotalVertices();
	totalIndices_  = getTotalIndices();
	primType_      = getPrimitiveType();

	//now allocate buffers
	glBindVertexArray(vaoID_);	
	//---------------------------------------------------
		glBindBuffer ( GL_ARRAY_BUFFER, vboVerticesID_ );
		glBufferData ( GL_ARRAY_BUFFER, totalVertices_ * sizeof( glm::vec3 ), 0, GL_STATIC_DRAW );
		
		//映射缓冲区到客户端地址空间中，访问修改;
		GLfloat* pBuffer = static_cast<GLfloat*>( glMapBuffer( GL_ARRAY_BUFFER, GL_WRITE_ONLY ) );
		{
			fillVertexBuffer( pBuffer );
		}
		glUnmapBuffer( GL_ARRAY_BUFFER );

		glEnableVertexAttribArray( shader_["vVertex"] );
		glVertexAttribPointer( shader_["vVertex"], 3, GL_FLOAT, GL_FALSE, 0, 0 );
	//----------------------------------------------------
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, vboIndicesID_ );
		glBufferData( GL_ELEMENT_ARRAY_BUFFER, totalIndices_ * sizeof(GLuint), 0, GL_STATIC_DRAW );
		
		GLuint* pIBuffer = static_cast<GLuint*>( glMapBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY ) );
			fillIndexBuffer( pIBuffer );
		glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
	//---------------------------------------------------
	glBindVertexArray(0);
}

void RenderableObject::destroy() {
	//Destroy shader
	shader_.DeleteShaderProgram();

	//Destroy vao and vbo
	glDeleteBuffers(1, &vboVerticesID_);
	glDeleteBuffers(1, &vboIndicesID_);
	glDeleteVertexArrays(1, &vaoID_);
}


void RenderableObject::render(const GLfloat* MVP) 
{
	shader_.Use();
	{
		if(MVP!=0)
		{
			glUniformMatrix4fv(shader_("MVP"), 1, GL_FALSE, MVP);
		}
		setCustomUniforms();
		glBindVertexArray(vaoID_);
			glDrawElements(primType_, totalIndices_, GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);
	}
	shader_.UnUse();
}