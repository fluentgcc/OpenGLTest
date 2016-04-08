#include <assert.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "platform.h"
#include "vertex-buffer.h"

#include <glm/glm.hpp>
#include <glm/fwd.hpp>


/**
 * Buffer status
 */
#define CLEAN  (0)
#define DIRTY  (1)
#define FROZEN (2)

// ----------------------------------------------------------------------------


ftgl::vertex_buffer::vertex_buffer(const char* format)
{
	size_t i, index = 0, stride = 0;
	const char *start = 0, *end = 0;
	GLchar *pointer = 0;

	this->format = strdup( format );

	for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
	{
		this->attributes[i] = 0;
	}

	start = format;
	do
	{
		char *desc = 0;
		vertex_attribute *attribute;
		GLuint attribute_size = 0;
		end = (char *) (strchr(start+1, ','));

		if ( end == NULL )
		{
			desc = strdup( start );
		}
		else
		{
			desc = strndup( start, end-start );
		}
		attribute = vertex_attribute_parse( desc );
		start = end+1;
		free(desc);
		attribute->pointer = pointer;

		switch( attribute->type )
		{
		case GL_BOOL:           attribute_size = sizeof(GLboolean); break;
		case GL_BYTE:           attribute_size = sizeof(GLbyte); break;
		case GL_UNSIGNED_BYTE:  attribute_size = sizeof(GLubyte); break;
		case GL_SHORT:          attribute_size = sizeof(GLshort); break;
		case GL_UNSIGNED_SHORT: attribute_size = sizeof(GLushort); break;
		case GL_INT:            attribute_size = sizeof(GLint); break;
		case GL_UNSIGNED_INT:   attribute_size = sizeof(GLuint); break;
		case GL_FLOAT:          attribute_size = sizeof(GLfloat); break;
		default:                attribute_size = 0;
		}
		stride  += attribute->size*attribute_size;
		pointer += attribute->size*attribute_size;
		this->attributes[index] = attribute;
		index++;
	} while ( end && (index < MAX_VERTEX_ATTRIBUTE) );

	for( i=0; i<index; ++i )
	{
		this->attributes[i]->stride = stride;
	}

#ifdef FREETYPE_GL_USE_VAO
	this->VAO_id = 0;
#endif

	this->vertices = vector_new( stride );
	this->vertices_id  = 0;
	this->GPU_vsize = 0;

	this->indices = vector_new( sizeof(GLuint) );
	this->indices_id  = 0;
	this->GPU_isize = 0;

	this->items = vector_new( sizeof( glm::ivec4 ) );
	this->state = DIRTY;
	this->mode = GL_TRIANGLES;
}

ftgl::vertex_buffer::~vertex_buffer()
{
	size_t i;

	for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
	{
		if( this->attributes[i] )
		{
			delete this->attributes[i];
		}
	}

#ifdef FREETYPE_GL_USE_VAO
	if( this->VAO_id )
	{
		glDeleteVertexArrays( 1, &this->VAO_id );
	}
	this->VAO_id = 0;
#endif

	vector_delete( this->vertices );
	this->vertices = 0;
	if( this->vertices_id )
	{
		glDeleteBuffers( 1, &this->vertices_id );
	}
	this->vertices_id = 0;

	vector_delete( this->indices );
	this->indices = 0;
	if( this->indices_id )
	{
		glDeleteBuffers( 1, &this->indices_id );
	}
	this->indices_id = 0;

	vector_delete( this->items );

	if( this->format )
	{
		free( this->format );
	}
	this->format = 0;
	this->state = 0;

}

size_t ftgl::vertex_buffer::getSize()
{
	return vector_size( this->items );
}

const char* ftgl::vertex_buffer::getFormat()
{
	return this->format;
}

void ftgl::vertex_buffer::print()
{
	int i = 0;
	static char *gltypes[9] = {
		"GL_BOOL",
		"GL_BYTE",
		"GL_UNSIGNED_BYTE",
		"GL_SHORT",
		"GL_UNSIGNED_SHORT",
		"GL_INT",
		"GL_UNSIGNED_INT",
		"GL_FLOAT",
		"GL_VOID"
	};

	fprintf( stderr, "%ld vertices, %ld indices\n",
		vector_size( this->vertices ), vector_size( this->indices ) );
	while( this->attributes[i] )
	{
		int j = 8;
		switch( this->attributes[i]->type )
		{
		case GL_BOOL:           j=0; break;
		case GL_BYTE:           j=1; break;
		case GL_UNSIGNED_BYTE:  j=2; break;
		case GL_SHORT:          j=3; break;
		case GL_UNSIGNED_SHORT: j=4; break;
		case GL_INT:            j=5; break;
		case GL_UNSIGNED_INT:   j=6; break;
		case GL_FLOAT:          j=7; break;
		default:                j=8; break;
		}
		fprintf(stderr, "%s : %dx%s (+%p)\n",
			this->attributes[i]->name,
			this->attributes[i]->size,
			gltypes[j],
			this->attributes[i]->pointer);

		i += 1;
	}
}

void ftgl::vertex_buffer::renderSetup(GLenum mode)
{
	size_t i;

#ifdef FREETYPE_GL_USE_VAO
	// Unbind so no existing VAO-state is overwritten,
	// (e.g. the GL_ELEMENT_ARRAY_BUFFER-binding).
	glBindVertexArray( 0 );
#endif

	if( this->state != CLEAN )
	{
		this->upload();
		this->state = CLEAN;
	}

#ifdef FREETYPE_GL_USE_VAO
	if( this->VAO_id == 0 )
	{
		// Generate and set up VAO

		glGenVertexArrays( 1, &this->VAO_id );
		glBindVertexArray( this->VAO_id );

		glBindBuffer( GL_ARRAY_BUFFER, this->vertices_id );

		for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
		{
			vertex_attribute *attribute = this->attributes[i];
			if( attribute == 0 )
			{
				continue;
			}
			else
			{
				attribute->enable();
			}
		}

		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		if( this->indices->size )
		{
			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->indices_id );
		}
	}

	// Bind VAO for drawing
	glBindVertexArray( this->VAO_id );
#else

	glBindBuffer( GL_ARRAY_BUFFER, this->vertices_id );

	for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
	{
		vertex_attribute *attribute = this->attributes[i];
		if ( attribute == 0 )
		{
			continue;
		}
		else
		{
			attribute->enable();
		}
	}

	if( this->indices->size )
	{
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->indices_id );
	}
#endif

	this->mode = mode;
}

void ftgl::vertex_buffer::renderFinish()
{
#ifdef FREETYPE_GL_USE_VAO
	glBindVertexArray( 0 );
#else
	int i;

	unsigned int e = 0;

	for( i=0; i<MAX_VERTEX_ATTRIBUTE; ++i )
	{
		vertex_attribute *attribute = this->attributes[i];
		if( attribute == 0 )
		{
			continue;
		}
		else
		{
			glDisableVertexAttribArray( attribute->index );
			e =  glGetError();

		}
	}
	e =  glGetError();
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
#endif
}

void ftgl::vertex_buffer::render(GLenum mode)
{
	size_t vcount = this->vertices->size;
	size_t icount = this->indices->size;
	
	this->renderSetup( mode );

	if( icount )
	{
		glDrawElements( mode, icount, GL_UNSIGNED_INT, 0 );
	}
	else
	{
		glDrawArrays( mode, 0, vcount );
	}

	this->renderFinish();
}

void ftgl::vertex_buffer::renderItem(size_t index)
{
	glm::ivec4 * item = ( glm::ivec4 *) vector_get( this->items, index );

	assert( index < vector_size( this->items ) );


	if( this->indices->size )
	{
		size_t start = item->z;
		size_t count = item->w;
		glDrawElements( this->mode, count, GL_UNSIGNED_INT, (void *)(start*sizeof(GLuint)) );
	}
	else if( this->vertices->size )
	{
		size_t start = item->x;
		size_t count = item->y;
		glDrawArrays( this->mode, start*this->vertices->item_size, count);
	}
}

void ftgl::vertex_buffer::upload()
{
	size_t vsize, isize;

	if( this->state == FROZEN )
	{
		return;
	}

	if( !this->vertices_id )
	{
		glGenBuffers( 1, &this->vertices_id );
	}
	if( !this->indices_id )
	{
		glGenBuffers( 1, &this->indices_id );
	}

	vsize = this->vertices->size*this->vertices->item_size;
	isize = this->indices->size*this->indices->item_size;


	// Always upload vertices first such that indices do not point to non
	// existing data (if we get interrupted in between for example).

	// Upload vertices
	glBindBuffer( GL_ARRAY_BUFFER, this->vertices_id );
	if( vsize != this->GPU_vsize )
	{
		glBufferData( GL_ARRAY_BUFFER,
			vsize, this->vertices->items, GL_DYNAMIC_DRAW );
		this->GPU_vsize = vsize;
	}
	else
	{
		glBufferSubData( GL_ARRAY_BUFFER,
			0, vsize, this->vertices->items );
	}
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Upload indices
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->indices_id );
	if( isize != this->GPU_isize )
	{
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
			isize, this->indices->items, GL_DYNAMIC_DRAW );
		this->GPU_isize = isize;
	}
	else
	{
		glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
			0, isize, this->indices->items );
	}
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void ftgl::vertex_buffer::clear()
{
	this->state = FROZEN;
	vector_clear( this->indices );
	vector_clear( this->vertices );
	vector_clear( this->items );
	this->state = DIRTY;
}

void ftgl::vertex_buffer::pushBackIndices(const GLuint * indices, const size_t icount)
{
	this->state |= DIRTY;
	vector_push_back_data( this->indices, indices, icount );
}

void ftgl::vertex_buffer::pushBackVertices(const void * vertices, const size_t vcount)
{
	this->state |= DIRTY;
	vector_push_back_data( this->vertices, vertices, vcount );
}

void ftgl::vertex_buffer::insertIndices(const size_t index, const GLuint *indices, const size_t icount)
{
	assert( this->indices );
	assert( index < this->indices->size+1 );

	this->state |= DIRTY;
	vector_insert_data( this->indices, index, indices, icount );
}

void ftgl::vertex_buffer::insertVertices(const size_t index, const void *vertices, const size_t vcount)
{
	size_t i;

	assert( this->vertices );
	assert( index < this->vertices->size+1 );

	this->state |= DIRTY;

	for( i=0; i < this->indices->size; ++i )
	{
		if( *(GLuint *)(vector_get( this->indices, i )) > index )
		{
			*(GLuint *)(vector_get( this->indices, i )) += index;
		}
	}

	vector_insert_data( this->vertices, index, vertices, vcount );

}

void ftgl::vertex_buffer::eraseIndices(const size_t first, const size_t last)
{
	assert( this->indices );
	assert( first < this->indices->size );
	assert( (last) <= this->indices->size );

	this->state |= DIRTY;
	vector_erase_range( this->indices, first, last );
}

void ftgl::vertex_buffer::eraseVertices(const size_t first, const size_t last)
{
	size_t i;

	assert( this->vertices );
	assert( first < this->vertices->size );
	assert( last <= this->vertices->size );
	assert( last > first );

	this->state |= DIRTY;
	for( i=0; i<this->indices->size; ++i )
	{
		if( *(GLuint *)(vector_get( this->indices, i )) > first )
		{
			*(GLuint *)(vector_get( this->indices, i )) -= (last-first);
		}
	}
	vector_erase_range( this->vertices, first, last );
}

size_t ftgl::vertex_buffer::pushBack(const void * vertices, const size_t vcount, const GLuint * indices, const size_t icount)
{
	return this->insert( vector_size( this->items ),
		vertices, vcount, indices, icount );
}

size_t ftgl::vertex_buffer::insert(const size_t index, const void * vertices, const size_t vcount, const GLuint * indices, const size_t icount)
{
	size_t vstart, istart, i;
	glm::ivec4 item;

	assert( vertices );
	assert( indices );

	this->state = FROZEN;

	// Push back vertices
	vstart = vector_size( this->vertices );
	this->pushBackVertices( vertices, vcount );

	// Push back indices
	istart = vector_size( this->indices );
	this->pushBackIndices( indices, icount );

	// Update indices within the vertex buffer
	for( i=0; i<icount; ++i )
	{
		*(GLuint *)(vector_get( this->indices, istart+i )) += vstart;
	}

	// Insert item
	item.x = vstart;
	item.y = vcount;
	item.z = istart;
	item.w = icount;
	vector_insert( this->items, index, &item );

	this->state = DIRTY;
	return index;
}

void ftgl::vertex_buffer::erase(const size_t index)
{
	glm::ivec4 * item;
	int vstart;
	size_t vcount, istart, icount, i;

	assert( this );
	assert( index < vector_size( this->items ) );

	item = (glm::ivec4 *) vector_get( this->items, index );
	vstart = item->x;
	vcount = item->y;
	istart = item->z;
	icount = item->w;

	// Update items
	for( i=0; i<vector_size(this->items); ++i )
	{
		glm::ivec4 * item = ( glm::ivec4 *) vector_get( this->items, i );
		if( item->x > vstart)
		{
			item->x -= vcount;
			item->z -= icount;
		}
	}

	this->state = FROZEN;
	this->eraseIndices( istart, istart+icount );
	this->eraseVertices( vstart, vstart+vcount );
	vector_erase( this->items, index );
	this->state = DIRTY;
}
