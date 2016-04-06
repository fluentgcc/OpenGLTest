#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "platform.h"
#include "vertex-attribute.h"

ftgl::vertex_attribute* ftgl::vertex_attribute_parse(char* format)
{
	GLenum type = 0;
	int size;
	int normalized = 0;
	char ctype;
	char *name;
	vertex_attribute *attr;
	char *p = strchr(format, ':');
	if( p != NULL)
	{
		name = strndup(format, p-format);
		if( *(++p) == '\0' )
		{
			fprintf( stderr, "No size specified for '%s' attribute\n", name );
			free( name );
			return 0;
		}
		size = *p - '0';

		if( *(++p) == '\0' )
		{
			fprintf( stderr, "No format specified for '%s' attribute\n", name );
			free( name );
			return 0;
		}
		ctype = *p;

		if( *(++p) != '\0' )
		{
			if( *p == 'n' )
			{
				normalized = 1;
			}
		}

	}
	else
	{
		fprintf(stderr, "Vertex attribute format not understood ('%s')\n", format );
		return 0;
	}

	switch( ctype )
	{
	case 'b': type = GL_BYTE;           break;
	case 'B': type = GL_UNSIGNED_BYTE;  break;
	case 's': type = GL_SHORT;          break;
	case 'S': type = GL_UNSIGNED_SHORT; break;
	case 'i': type = GL_INT;            break;
	case 'I': type = GL_UNSIGNED_INT;   break;
	case 'f': type = GL_FLOAT;          break;
	default:  type = 0;                 break;
	}
	attr = new vertex_attribute( name, size, type, normalized, 0, 0 );
	free( name );
	return attr;
}


void ftgl::vertex_attribute_enable(vertex_attribute *attr)
{
	if( attr->index == -1 )
	{
		GLint program;
		glGetIntegerv( GL_CURRENT_PROGRAM, &program );
		if( program == 0)
		{
			return;
		}
		attr->index = glGetAttribLocation( program, attr->name );
		if( attr->index == -1 )
		{
			return;
		}
	}
	glEnableVertexAttribArray( attr->index );
	glVertexAttribPointer( attr->index, attr->size, attr->type,
		attr->normalized, attr->stride, attr->pointer );
}

ftgl::vertex_attribute::vertex_attribute(GLchar * name, GLint size, GLenum type, GLboolean normalized, GLsizei stride, GLvoid *pointer)
{
	this->name       = (GLchar *) strdup( name );
	this->index      = -1;
	this->size       = size;
	this->type       = type;
	this->normalized = normalized;
	this->stride     = stride;
	this->pointer    = pointer;
}

ftgl::vertex_attribute::~vertex_attribute()
{
	free( this->name );
}
