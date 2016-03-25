#ifndef GLTEXTURE_H
#define GLTEXTURE_H

#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/gl.h>


class GLTexture
{

public:
	GLTexture();
	~GLTexture();

	bool create1d( GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid* data );
	bool create2d( GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data );
	bool create3d( GLenum target, GLint internalFormat, GLsizei width, GLsizei height,  GLsizei depth, GLenum format,  GLenum type,  const GLvoid * data);

	void bind();
	void unbind();

	void eidt2d( GLenum target, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data );

	void destroy(); 

	GLuint id_;

	GLenum target_;
	GLint  internal_format_;
	
	GLsizei width_;
	GLsizei height_;
	GLsizei depth_;
};

#endif
