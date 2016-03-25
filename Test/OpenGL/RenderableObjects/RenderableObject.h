#pragma once
#include "GLSLShader.h"

class RenderableObject
{
public:
	RenderableObject(void);
	virtual ~RenderableObject( void );
	void render(const float* MVP);
	
	virtual int    getTotalVertices() =0;
	virtual int    getTotalIndices()  =0;
	virtual GLenum getPrimitiveType() =0;

	virtual void fillVertexBuffer( GLfloat* pBuffer ) = 0;
	virtual void fillIndexBuffer( GLuint* pBuffer )   = 0;
	virtual void setCustomUniforms() = 0;

	void init();
	void destroy();

protected:
	GLuint vaoID_;						//vertex array;
	GLuint vboVerticesID_;				//vertex array buffer;
	GLuint vboIndicesID_;				//element array buffer;
	
	GLSLShader shader_;

	GLenum primType_;
	int totalVertices_, totalIndices_;
};

