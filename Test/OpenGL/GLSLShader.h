#pragma once
// #ifndef __GLSL_SHADER__H
// #define __GLSL_SHADER__H

#include <GL/glew.h>
#include <map>
#include <string>

using namespace std;

class GLSLShader
{
public:
	GLSLShader();
	~GLSLShader();	
	
	void LoadFromString( GLenum whichShader, const string& source );
	void LoadFromFile(   GLenum whichShader, const string& filename );
	void CreateAndLinkProgram();
	
	void Use();
	void UnUse();
	
	void AddAttribute( const string& attribute );
	void AddUniform(   const string& uniform );

	//An indexer that returns the location of the attribute/uniform
	GLuint operator[]( const string& attribute );
	GLuint operator()( const string& uniform );

	void DeleteShaderProgram();

private:
	enum ShaderType { VERTEX_SHADER, FRAGMENT_SHADER, GEOMETRY_SHADER};
	GLuint	program_;
	int totalShaders_;
	GLuint shaders_[3];			//0->vertexshader, 1->fragmentshader, 2->geometryshader

	map<string,GLuint> attributeList_;
	map<string,GLuint> uniformLocationList_;
};	

//#endif