#pragma once
#include "RenderableObject.h"


class CRectangle 
/*	: public RenderableObject*/
{
public: 
	CRectangle(void);
	virtual ~CRectangle(void);

	void render( const float* MVP );

// 	int getTotalVertices();
// 	int getTotalIndices();
// 	GLenum getPrimitiveType();
// 	void setCustomUniforms(){};

private:

	GLSLShader shader_;

	GLuint vaoID_;						//vertex array;
	GLuint vboVerticesID_;				//vertex array buffer;
	GLuint vboIndicesID_;				//element array buffer;

	GLenum primType_;
	int totalVertices_, totalIndices_;

};
