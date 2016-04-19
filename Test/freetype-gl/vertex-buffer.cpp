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

		if (end == NULL)
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
		case GL_BOOL:           attribute_size = sizeof(GLboolean);		break;
		case GL_BYTE:           attribute_size = sizeof(GLbyte);		break;
		case GL_UNSIGNED_BYTE:  attribute_size = sizeof(GLubyte);		break;
		case GL_SHORT:          attribute_size = sizeof(GLshort);		break;
		case GL_UNSIGNED_SHORT: attribute_size = sizeof(GLushort);		break;
		case GL_INT:            attribute_size = sizeof(GLint);			break;
		case GL_UNSIGNED_INT:   attribute_size = sizeof(GLuint);		break;
		case GL_FLOAT:          attribute_size = sizeof(GLfloat);		break;
		default:                attribute_size = 0;
		}
		stride  += attribute->size * attribute_size;
		pointer += attribute->size * attribute_size;
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

	this->vertices = new vector_t( stride );
	this->vertices_id  = 0;
	this->GPU_vsize = 0;

	this->indices = new vector_t( sizeof(GLuint) );
	this->indices_id  = 0;
	this->GPU_isize = 0;

	this->items = new vector_t( sizeof( item_t ) );
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
			this->attributes[i] = nullptr;
		}
	}

#ifdef FREETYPE_GL_USE_VAO
	if( this->VAO_id )
	{
		glDeleteVertexArrays( 1, &this->VAO_id );
	}
	this->VAO_id = 0;
#endif

	delete this->vertices;
	this->vertices = 0;
	if( this->vertices_id )
	{
		glDeleteBuffers( 1, &this->vertices_id );
	}
	this->vertices_id = 0;

	delete this->indices;
	this->indices = 0;
	if( this->indices_id )
	{
		glDeleteBuffers( 1, &this->indices_id );
	}
	this->indices_id = 0;

	delete this->items;

	if( this->format )
	{
		free( this->format );
	}
	this->format = 0;
	this->state = 0;
	free( this );
}

size_t ftgl::vertex_buffer::getSize()
{
	return this->items->size();
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

	fprintf( stderr, "%ld vertices, %ld indices\n",this->vertices->size(), this->indices->size() );
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

void ftgl::vertex_buffer::render(GLenum mode)
{
	size_t vcount = this->vertices->size();
	size_t icount = this->indices->size();

	this->renderSetup(  mode );

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

void ftgl::vertex_buffer::clear()
{
	this->state = FROZEN;
	this->indices->clear();
	this->vertices->clear();
	this->items->clear();
	this->state = DIRTY;
}

void ftgl::vertex_buffer::pushBackIndices(const GLuint * indices, const size_t icount)
{
	this->state |= DIRTY;
	this->indices->push_back_data( indices, icount );
}

void ftgl::vertex_buffer::pushBackVertices(const void * vertices, const size_t vcount)
{
	this->state |= DIRTY;
	this->vertices->push_back_data( vertices, vcount );
}

void ftgl::vertex_buffer::insertIndices(const size_t index, const GLuint *indices, const size_t count)
{
	assert( this->indices );
	assert( index < this->indices->size() + 1 );

	this->state |= DIRTY;
	this->indices->insert_data( index, indices, count );
}

void ftgl::vertex_buffer::insertVertices(const size_t index, const void *vertices, const size_t vcount)
{
	size_t i;
	assert( this );
	assert( this->vertices );
	assert( index < this->vertices->size() + 1 );

	this->state |= DIRTY;

	for( i=0; i < this->indices->size(); ++i )
	{
		if( *(GLuint *)(  this->indices->at(i) ) > index )
		{
			*(GLuint *)( this->indices->at(i) ) += index;
		}
	}
	this->vertices->insert_data( index, vertices, vcount );
}

void ftgl::vertex_buffer::eraseIndices(const size_t first, const size_t last)
{
	assert( this->indices );
	assert( first < this->indices->size() );
	assert( (last) <= this->indices->size() );

	this->indices->erase_range( first, last );
}

void ftgl::vertex_buffer::eraseVertices(const size_t first, const size_t last)
{
	size_t i;
	assert( this->vertices );
	assert( first < this->vertices->size() );
	assert( last <= this->vertices->size() );
	assert( last > first );

	this->state |= DIRTY;
	for( i=0; i < this->indices->size(); ++i )
	{
		if( *(GLuint *)( this->indices->at(i) ) > first )
		{
			*(GLuint *)( this->indices->at(i) ) -= (last-first);
		}
	}
	this->vertices->erase_range( first, last );
}

size_t ftgl::vertex_buffer::pushBack(const void * vertices, const size_t vcount, const GLuint * indices, const size_t icount)
{
	return this->insert( this->items->size(), vertices, vcount, indices, icount );
}

size_t ftgl::vertex_buffer::insert(const size_t index, const void * vertices, const size_t vcount, const GLuint * indices, const size_t icount)
{
	size_t vstart, istart, i;
	item_t item;
	assert( vertices );
	assert( indices );

	this->state = FROZEN;

	// Push back vertices
	vstart = this->vertices->size();
	this->pushBackVertices( vertices, vcount );

	// Push back indices
	istart = this->indices->size();
	this->pushBackIndices( indices, icount );

	// Update indices within the vertex buffer
	for( i=0; i<icount; ++i )
	{
		*(GLuint *)( this->indices->at( istart+i ) ) += vstart;
	}

	// Insert item
	item.vstart = vstart;
	item.vcount = vcount;
	item.istart = istart;
	item.icount = icount;
	this->items->insert( index, &item );

	this->state = DIRTY;
	return index;
}

void ftgl::vertex_buffer::erase(const size_t index)
{
	item_t * item;
	int vstart;
	size_t vcount, istart, icount, i;

	assert( index <  this->items->size() );

	item = (item_t *) this->items->at( index );
	vstart = item->vstart;
	vcount = item->vcount;
	istart = item->istart;
	icount = item->icount;

	// Update items
	for( i=0; i< this->items->size(); ++i )
	{
		item_t * item = ( item_t * ) ( this->items->at( i ) );
		if( item->vstart > vstart)
		{
			item->vstart -= vcount;
			item->istart -= icount;
		}
	}

	this->state = FROZEN;

	this->eraseIndices( istart, istart+icount );
	this->eraseVertices( vstart, vstart+vcount );
	this->items->erase( index );

	this->state = DIRTY;
}

void ftgl::vertex_buffer::renderItem(size_t index)
{
	assert( index < this->items->size() );

	item_t * item = ( item_t *) this->items->at( index );

	if( this->indices->size() )
	{
		size_t start = item->istart;
		size_t count = item->icount;
		glDrawElements( this->mode, count, GL_UNSIGNED_INT, ( void * )( start * sizeof( GLuint ) ) );
	}
	else if( this->vertices->size() )
	{
		size_t start = item->vstart;
		size_t count = item->vcount;
		glDrawArrays( this->mode, start * this->vertices->item_size(), count );
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
			vertex_attribute_t *attribute = this->attributes[i];
			if( attribute == 0 )
			{
				continue;
			}
			else
			{
				vertex_attribute_enable( attribute );
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

	if( this->indices->size() )
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
		}
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
#endif
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

	vsize = this->vertices->size() *this->vertices->item_size();
	isize = this->indices->size() *this->indices->item_size();


	// Always upload vertices first such that indices do not point to non
	// existing data (if we get interrupted in between for example).

	// Upload vertices
	glBindBuffer( GL_ARRAY_BUFFER, this->vertices_id );
	if( vsize != this->GPU_vsize )
	{
		glBufferData( GL_ARRAY_BUFFER,
			vsize, this->vertices->front(), GL_DYNAMIC_DRAW );
		this->GPU_vsize = vsize;
	}
	else
	{
		glBufferSubData( GL_ARRAY_BUFFER,
			0, vsize, this->vertices->front() );
	}
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	// Upload indices
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, this->indices_id );
	if( isize != this->GPU_isize )
	{
		glBufferData( GL_ELEMENT_ARRAY_BUFFER,
			isize, this->indices->front(), GL_DYNAMIC_DRAW );
		this->GPU_isize = isize;
	}
	else
	{
		glBufferSubData( GL_ELEMENT_ARRAY_BUFFER,
			0, isize, this->indices->front() );
	}
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}
