#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include "opengl.h"
#include "texture-atlas.h"


ftgl::texture_atlas::texture_atlas(const size_t width, const size_t height, const size_t depth)
{
	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	glm::ivec3 node = glm::ivec3( 1, 1, width-2 );

	assert( (depth == 1) || (depth == 3) || (depth == 4) );

	this->nodes = vector_new( sizeof( glm::ivec3 ) );
	this->used = 0;
	this->width = width;
	this->height = height;
	this->depth = depth;
	this->id = 0;

	vector_push_back( this->nodes, &node );

	//·ÖÅä¿Õ¼ä;
	this->data = ( unsigned char * )calloc( width * height * depth, sizeof( unsigned char ) );

	if( this->data == NULL)
	{
		fprintf( stderr,
			"line %d: No more memory for allocating data\n", __LINE__ );
		return;
	}
}

ftgl::texture_atlas::~texture_atlas()
{
	vector_delete( this->nodes );
	if( this->data )
	{
		free( this->data );
	}
	if( this->id )
	{
		glDeleteTextures( 1, &this->id );
	}
}

void ftgl::texture_atlas::upload()
{
	assert( this->data );

	if( !this->id )
	{
		glGenTextures( 1, &this->id );
	}

	glBindTexture( GL_TEXTURE_2D, this->id );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );

	if( this->depth == 4 )
	{
#ifdef GL_UNSIGNED_INT_8_8_8_8_REV
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height,
			0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, this->data );
#else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, self->width, self->height,
			0, GL_RGBA, GL_UNSIGNED_BYTE, self->data );
#endif
	}
	else if( this->depth == 3 )
	{
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height,
			0, GL_RGB, GL_UNSIGNED_BYTE, this->data );
	}
	else
	{
#if defined( GL_ES_VERSION_2_0) || defined(GL_ES_VERSION_3_0 )
		glTexImage2D( GL_TEXTURE_2D, 0, GL_LUMINANCE, self->width, self->height,
			0, GL_LUMINANCE, GL_UNSIGNED_BYTE, self->data );
#else
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, this->width, this->height,
			0, GL_RED, GL_UNSIGNED_BYTE, this->data );
#endif
	}
}

glm::ivec4 ftgl::texture_atlas::getRegion(const size_t width, const size_t height)
{
	int y, best_index;
	size_t best_height, best_width;
	glm::ivec3 *node, *prev;
	glm::ivec4 region( 0, 0, width, height );
	size_t i;

	best_height = UINT_MAX;
	best_index  = -1;
	best_width = UINT_MAX;

	for( i = 0; i < this->nodes->size; ++i )
	{
		y = this->fit( i, width, height );
		if( y >= 0 )
		{
			node = ( glm::ivec3 * ) vector_get( this->nodes, i );
			if( ( ( y + height ) < best_height ) ||
				( ( ( y + height ) == best_height ) && ( node->z > 0 && ( size_t )node->z < best_width ) ) )
			{
				best_height = y + height;
				best_index = i;
				best_width = node->z;
				region.x = node->x;
				region.y = y;
			}
		}
	}

	if( best_index == -1 )
	{
		region.x = -1;				//x
		region.y = -1;				//y
		region.z = 0;				//width
		region.w = 0;				//height
		return region;
	}

	node = new glm::ivec3();
	if( node == NULL)
	{
		fprintf( stderr,
			"line %d: No more memory for allocating data\n", __LINE__ );
		exit( EXIT_FAILURE );
	}

	node->x = region.x;
	node->y = region.y + height;
	node->z = width;
	vector_insert( this->nodes, best_index, node );
	
	delete node ;

	for(i = best_index + 1; i < this->nodes->size; ++i)
	{
		node = ( glm::ivec3 * ) vector_get( this->nodes, i );
		prev = ( glm::ivec3 * ) vector_get( this->nodes, i-1 );

		if (node->x < (prev->x + prev->z) )
		{
			int shrink = prev->x + prev->z - node->x;
			node->x += shrink;
			node->z -= shrink;
			if (node->z <= 0)
			{
				vector_erase( this->nodes, i );
				--i;
			}
			else
			{
				break;
			}
		}
		else
		{
			break;
		}
	}
	
	this->merge( );
	this->used += width * height;
	
	return region;
}

void ftgl::texture_atlas::setRegion(const size_t x, const size_t y, const size_t width, const size_t height, const unsigned char *data, const size_t stride)
{
	size_t i;
	size_t depth;
	size_t charsize;

	assert( x > 0 );
	assert( y > 0 );
	assert( x < ( this->width-1 ) );
	assert( ( x + width ) <= ( this->width-1 ) );
	assert( y < ( this->height-1 ) );
	assert( ( y + height ) <= ( this->height-1 ) );

	depth = this->depth;
	charsize = sizeof( char );
	for( i=0; i<height; ++i )
	{
		memcpy( this->data+( ( y+i )* this->width + x ) * charsize * depth,
			data + ( i*stride ) * charsize, width * charsize * depth  );
	}
}

void ftgl::texture_atlas::clear()
{
	glm::ivec3 node( 1, 1, 1 );

	assert( this->data );

	vector_clear( this->nodes );
	this->used = 0;
	// We want a one pixel border around the whole atlas to avoid any artefact when
	// sampling texture
	node.z = this->width-2;

	vector_push_back( this->nodes, &node );
	memset( this->data, 0, this->width * this->height * this->depth );
}

int ftgl::texture_atlas::fit(const size_t index, const size_t width, const size_t height)
{
	glm::ivec3 *node;
	int x, y, width_left;
	size_t i;

	node = ( glm::ivec3 * ) ( vector_get( this->nodes, index ) );
	x = node->x;
	y = node->y;
	width_left = width;
	i = index;

	if ( ( x + width ) > ( this->width-1 ) )
	{
		return -1;
	}

	y = node->y;
	while( width_left > 0 )
	{
		node = ( glm::ivec3 *) (vector_get( this->nodes, i ));
		if( node->y > y )
		{
			y = node->y;
		}
		if( ( y + height ) > (this->height - 1 ) )
		{
			return -1;
		}
		width_left -= node->z;
		++i;
	}
	return y;
}

void ftgl::texture_atlas::merge()
{
	glm::ivec3 *node, *next;
	size_t i;

	for( i=0; i< this->nodes->size-1; ++i )
	{
		node = ( glm::ivec3 * ) (vector_get( this->nodes, i ) );
		next = ( glm::ivec3 * ) (vector_get( this->nodes, i+1 ) );
		if( node->y == next->y )
		{
			node->z += next->z;
			vector_erase( this->nodes, i+1 );
			--i;
		}
	}
}
