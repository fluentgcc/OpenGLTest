#include "GLTexture.h"
#include <assert.h>
#include <iostream>


GLTexture::GLTexture()
	:id_(0), target_(0), width_(0), height_(0), depth_(0)
{
}

GLTexture::~GLTexture()
{
}

bool GLTexture::create1d(GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, const GLvoid* data)
{
	return false;
}

bool GLTexture::create2d(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* data)
{
	if ( 0 != this->id_ )
	{
		this->destroy();
	}

	glEnable( GL_TEXTURE_2D );

	glGenTextures( 1, &this->id_ );

	assert( 0 != this->id_ );

	glBindTexture( GL_TEXTURE_2D, this->id_ );

	glPixelStorei( GL_PACK_ALIGNMENT, 1 );
	glPixelStorei( GL_UNPACK_ALIGNMENT, 1 );

	glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data );

	GLenum e = glGetError();

	if ( GL_NO_ERROR != e )
	{
		std::cerr << "Create Texture2D failure. Error Type = " << e;

		return false;
	}

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );


	this->width_ = width;
	this->height_ = height;
	this->target_ = target;
	this->internal_format_ = internalFormat;

	return true;
}

bool GLTexture::create3d(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * data)
{
	  return false;
}

void GLTexture::bind()
{
	glBindTexture( target_, this->id_ );
}

void GLTexture::unbind()
{
	glBindTexture( target_, 0 );
}

void GLTexture::eidt2d(GLenum target, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * data)
{
	this->bind();
	
	glTexSubImage2D( target, 0, xoffset, yoffset, width, height, format, type, data );
	
	this->unbind();
}

void GLTexture::destroy()
{
	glDeleteTextures( 1, &this->id_ );
}

