#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <assert.h>
#include "opengl.h"
#include "text-buffer.h"
#include "utf8-utils.h"
#include "math.h"

#define SET_GLYPH_VERTEX( value, x0, y0, z0, s0, t0, r, g, b, a, sh, gm ) { \
	glyph_vertex *gv = &value;                                 \
	gv->x=x0; gv->y=y0; gv->z=z0;                              \
	gv->u=s0; gv->v=t0;                                        \
	gv->r=r; gv->g=g; gv->b=b; gv->a=a;                        \
	gv->shift=sh; gv->gamma=gm;}

// ----------------------------------------------------------------------------

ftgl::text_buffer::text_buffer(size_t depth, const char * vert_filename, const char * frag_filename)
{
	this->shader.LoadFromFile( GL_VERTEX_SHADER, vert_filename );
	this->shader.LoadFromFile( GL_FRAGMENT_SHADER, frag_filename );
	this->shader.CreateAndLinkProgram();

	this->buffer = new vertex_buffer( "vertex:3f,tex_coord:2f,color:4f,ashift:1f,agamma:1f" );
	this->manager = new font_manager( 512, 512, depth );
	this->shader = program;
	this->shader_texture = glGetUniformLocation(this->shader, "tex");
	this->shader_pixel = glGetUniformLocation(this->shader, "pixel");
	this->line_start = 0;
	this->line_ascender = 0;
	this->base_color.r = 0.0;
	this->base_color.g = 0.0;
	this->base_color.b = 0.0;
	this->base_color.a = 1.0;
	this->line_descender = 0;
	this->lines = vector_new( sizeof(line_info_t) );
	this->bounds.left   = 0.0;
	this->bounds.top    = 0.0;
	this->bounds.width  = 0.0;
	this->bounds.height = 0.0;
}

ftgl::text_buffer::~text_buffer()
{
	vector_delete( this->lines );
	delete this->manager;
	delete this->buffer;
	
	this->shader.DeleteShaderProgram();

}


void ftgl::text_buffer::render()
{
	glEnable( GL_BLEND );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, this->manager->atlas->id );

	if( this->manager->atlas->depth == 1 )
	{
		//glDisable( GL_COLOR_MATERIAL );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBlendColor( 1, 1, 1, 1 );
	}
	else
	{
		//glEnable( GL_COLOR_MATERIAL );
		//glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		//glBlendFunc( GL_ONE, GL_ONE_MINUS_SRC_ALPHA );
		//glBlendColor( 1.0, 1.0, 1.0, 1.0 );
		//glBlendFunc( GL_CONSTANT_COLOR_EXT,  GL_ONE_MINUS_SRC_COLOR );
		//glBlendColor( this->base_color.r,
		//this->base_color.g,
		//this->base_color.b,
		//this->base_color.a );
		glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
		glBlendColor( 1, 1, 1, 1 );
	}

	this->shader.Use()
	{
		glUniform1i( this->shader_texture, 0 );
		glUniform3f( this->shader_pixel, 1.0f/this->manager->atlas->width, 1.0f/this->manager->atlas->height, (float)this->manager->atlas->depth );
		this->buffer->render( GL_TRIANGLES );
		glBindTexture( GL_TEXTURE_2D, 0 );
		glBlendColor( 0, 0, 0, 0 );
	}
	this->shader.UnUse();
}


void ftgl::text_buffer::print(glm::vec2 * pen, ...)
{
	markup_t * markup;
	char *text;
	va_list args;

	if( this->buffer->getSize() == 0 )
	{
		this->origin = *pen;
	}

	va_start ( args, pen );
	do {
		markup = va_arg( args, markup_t * );
		if( markup == NULL )
		{
			return;
		}
		text = va_arg( args, char * );
		this->add_text( pen, markup, text, 0 );

	} while( markup != 0 );
	va_end ( args );
}


void ftgl::text_buffer::add_text(glm::vec2 * pen, markup_t * markup, const char * text, size_t length)
{
	font_manager * manager = this->manager;
	size_t i;
	const char * prev_character = NULL;

	if( markup == NULL )
	{
		return;
	}

	if( !markup->font )
	{
		markup->font = manager->getFontFromMarkup( markup );
		if( ! markup->font )
		{
			fprintf( stderr, "Houston, we've got a problem !\n" );
			exit( EXIT_FAILURE );
		}
	}

	if( length == 0 )
	{
		length = utf8_strlen(text);
	}
	if( vertex_buffer_size( this->buffer ) == 0 )
	{
		this->origin = *pen;
		this->line_left = pen->x;
		this->bounds.left = pen->x;
		this->bounds.top = pen->y;
	}
	else
	{
		if (pen->x < this->origin.x)
		{
			this->origin.x = pen->x;
		}
		if (pen->y != this->last_pen_y)
		{
			text_buffer_finish_line(this, pen, false);
		}
	}

	for( i = 0; utf8_strlen( text + i ) && length; i += utf8_surrogate_len( text + i ) )
	{
		this->add_char( pen, markup, text + i, prev_character );
		prev_character = text + i;
		length--;
	}

	this->last_pen_y = pen->y;
}


void ftgl::text_buffer::add_char(glm::vec2 * pen, markup_t * markup, const char * current, const char * previous)
{
	size_t vcount = 0;
	size_t icount = 0;
	vertex_buffer * buffer = this->buffer;
	texture_font * font = markup->font;
	float gamma = markup->gamma;

	// Maximum number of vertices is 20 (= 5x2 triangles) per glyph:
	//  - 2 triangles for background
	//  - 2 triangles for overline
	//  - 2 triangles for underline
	//  - 2 triangles for strikethrough
	//  - 2 triangles for glyph
	glyph_vertex vertices[4*5];
	GLuint indices[6*5];
	texture_glyph *glyph;
	texture_glyph *black;
	float kerning = 0.0f;

	if( markup->font->ascender > this->line_ascender )
	{
		float y = pen->y;
		pen->y -= (markup->font->ascender - this->line_ascender);
		this->move_last_line( (float)(int)(y-pen->y) );
		this->line_ascender = markup->font->ascender;
	}
	if( markup->font->descender < this->line_descender )
	{
		this->line_descender = markup->font->descender;
	}

	if( *current == '\n' )
	{
		this->finish_line(this, pen, true);
		return;
	}

	glyph = font->getGlyph( font, current );
	black = font->getGlyph( font, NULL );

	if( glyph == NULL )
	{
		return;
	}

	if( previous && markup->font->kerning )
	{
		kerning = glyph->getKerning( previous );
	}
	pen->x += kerning;

	// Background
	if( markup->background_color.alpha > 0 )
	{
		float r = markup->background_color.r;
		float g = markup->background_color.g;
		float b = markup->background_color.b;
		float a = markup->background_color.a;
		float x0 = ( pen->x -kerning );
		float y0 = (float)(int)( pen->y + font->descender );
		float x1 = ( x0 + glyph->advance_x );
		float y1 = (float)(int)( y0 + font->height + font->linegap );
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		SET_GLYPH_VERTEX(vertices[vcount+0],
			(float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+1],
			(float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+2],
			(float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+3],
			(float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
		indices[icount + 0] = vcount+0;
		indices[icount + 1] = vcount+1;
		indices[icount + 2] = vcount+2;
		indices[icount + 3] = vcount+0;
		indices[icount + 4] = vcount+2;
		indices[icount + 5] = vcount+3;
		vcount += 4;
		icount += 6;
	}

	// Underline
	if( markup->underline )
	{
		float r = markup->underline_color.r;
		float g = markup->underline_color.g;
		float b = markup->underline_color.b;
		float a = markup->underline_color.a;
		float x0 = ( pen->x - kerning );
		float y0 = (float)(int)( pen->y + font->underline_position );
		float x1 = ( x0 + glyph->advance_x );
		float y1 = (float)(int)( y0 + font->underline_thickness );
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;

		SET_GLYPH_VERTEX(vertices[vcount+0],
			(float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+1],
			(float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+2],
			(float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+3],
			(float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
		indices[icount + 0] = vcount+0;
		indices[icount + 1] = vcount+1;
		indices[icount + 2] = vcount+2;
		indices[icount + 3] = vcount+0;
		indices[icount + 4] = vcount+2;
		indices[icount + 5] = vcount+3;
		vcount += 4;
		icount += 6;
	}

	// Overline
	if( markup->overline )
	{
		float r = markup->overline_color.r;
		float g = markup->overline_color.g;
		float b = markup->overline_color.b;
		float a = markup->overline_color.a;
		float x0 = ( pen->x -kerning );
		float y0 = (float)(int)( pen->y + (int)font->ascender );
		float x1 = ( x0 + glyph->advance_x );
		float y1 = (float)(int)( y0 + (int)font->underline_thickness );
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;
		SET_GLYPH_VERTEX(vertices[vcount+0],
			(float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+1],
			(float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+2],
			(float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+3],
			(float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
		indices[icount + 0] = vcount+0;
		indices[icount + 1] = vcount+1;
		indices[icount + 2] = vcount+2;
		indices[icount + 3] = vcount+0;
		indices[icount + 4] = vcount+2;
		indices[icount + 5] = vcount+3;
		vcount += 4;
		icount += 6;
	}

	/* Strikethrough */
	if( markup->strikethrough )
	{
		float r = markup->strikethrough_color.r;
		float g = markup->strikethrough_color.g;
		float b = markup->strikethrough_color.b;
		float a = markup->strikethrough_color.a;
		float x0  = ( pen->x -kerning );
		float y0  = (float)(int)( pen->y + (int)font->ascender*.33f);
		float x1  = ( x0 + glyph->advance_x );
		float y1  = (float)(int)( y0 + (int)font->underline_thickness );
		float s0 = black->s0;
		float t0 = black->t0;
		float s1 = black->s1;
		float t1 = black->t1;
		SET_GLYPH_VERTEX(vertices[vcount+0],
			(float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+1],
			(float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+2],
			(float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+3],
			(float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
		indices[icount + 0] = vcount+0;
		indices[icount + 1] = vcount+1;
		indices[icount + 2] = vcount+2;
		indices[icount + 3] = vcount+0;
		indices[icount + 4] = vcount+2;
		indices[icount + 5] = vcount+3;
		vcount += 4;
		icount += 6;
	}
	{
		// Actual glyph
		float r = markup->foreground_color.red;
		float g = markup->foreground_color.green;
		float b = markup->foreground_color.blue;
		float a = markup->foreground_color.alpha;
		float x0 = ( pen->x + glyph->offset_x );
		float y0 = (float)(int)( pen->y + glyph->offset_y );
		float x1 = ( x0 + glyph->width );
		float y1 = (float)(int)( y0 - glyph->height );
		float s0 = glyph->s0;
		float t0 = glyph->t0;
		float s1 = glyph->s1;
		float t1 = glyph->t1;

		SET_GLYPH_VERTEX(vertices[vcount+0],
			(float)(int)x0,y0,0,  s0,t0,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+1],
			(float)(int)x0,y1,0,  s0,t1,  r,g,b,a,  x0-((int)x0), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+2],
			(float)(int)x1,y1,0,  s1,t1,  r,g,b,a,  x1-((int)x1), gamma );
		SET_GLYPH_VERTEX(vertices[vcount+3],
			(float)(int)x1,y0,0,  s1,t0,  r,g,b,a,  x1-((int)x1), gamma );
		indices[icount + 0] = vcount+0;
		indices[icount + 1] = vcount+1;
		indices[icount + 2] = vcount+2;
		indices[icount + 3] = vcount+0;
		indices[icount + 4] = vcount+2;
		indices[icount + 5] = vcount+3;
		vcount += 4;
		icount += 6;

		buffer->pushBack( vertices, vcount, indices, icount );
		pen->x += glyph->advance_x * (1.0f + markup->spacing);
	}
}

void ftgl::text_buffer::align(glm::vec2 * pen, Align alignment)
{
	if ( ALIGN_LEFT == alignment )
	{
		return;
	}

	size_t total_items = vector_size( this->buffer->items );
	if ( this->line_start != total_items )
	{
		this->finish_line( pen, false );
	}


	size_t i, j;
	int k;
	float this_left, this_right, this_center;
	float line_left, line_right, line_center;
	float dx;

	this_left = this->bounds.left;
	this_right = this->bounds.left + this->bounds.width;
	this_center = (this_left + this_right) / 2;

	line_info_t* line_info;
	size_t lines_count, line_end;

	lines_count = vector_size( this->lines );
	for ( i = 0; i < lines_count; ++i )
	{
		line_info = (line_info_t *)vector_get( this->lines, i );

		if ( i + 1 < lines_count )
		{
			line_end = ((line_info_t*)vector_get( this->lines, i + 1 ))->line_start;
		}
		else
		{
			line_end = vector_size( this->buffer->items );
		}

		line_right = line_info->bounds.left + line_info->bounds.width;

		if ( ALIGN_RIGHT == alignment )
		{
			dx = this_right - line_right;
		}
		else // ALIGN_CENTER
		{
			line_left = line_info->bounds.left;
			line_center = (line_left + line_right) / 2;
			dx = this_center - line_center;
		}

		dx = (float)round( dx );

		for( j=line_info->line_start; j < line_end; ++j )
		{
			ivec4 *item = (ivec4 *) vector_get( this->buffer->items, j);
			for( k=item->vstart; k<item->vstart+item->vcount; ++k)
			{
				glyph_vertex * vertex =
					(glyph_vertex *)vector_get( this->buffer->vertices, k );
				vertex->x += dx;
			}
		}

	}
}


glm::vec4 ftgl::text_buffer::get_bounds(glm::vec2* pen)
{
	size_t total_items = vector_size( this->buffer->items );
	if ( this->line_start != total_items )
	{
		this->finish_line( pen, false );
	}

	return this->bounds;
}


void ftgl::text_buffer::clear()
{
	vector_clear( this->lines );

	this->line_start = 0;
	this->line_ascender = 0;
	this->line_descender = 0;
	vector_clear( this->lines );
	this->bounds.left   = 0.0;
	this->bounds.top    = 0.0;
	this->bounds.width  = 0.0;
	this->bounds.height = 0.0;
}

void ftgl::text_buffer::move_last_line(float dy)
{
	size_t i;
	int j;
	for( i = this->line_start; i < vector_size( this->buffer->items ); ++i )
	{
		ivec4 *item = (ivec4 *) vector_get( this->buffer->items, i);
		for( j=item->vstart; j<item->vstart+item->vcount; ++j)
		{
			glyph_vertex * vertex =
				( glyph_vertex *)  vector_get( this->buffer->vertices, j );
			vertex->y -= dy;
		}
	}
}

void ftgl::text_buffer::finish_line(vec2* pen, bool advancePen)
{
	float line_left = this->line_left;
	float line_right = pen->x;
	float line_width  = line_right - line_left;
	float line_top = pen->y + this->line_ascender;
	float line_height = this->line_ascender - this->line_descender;
	float line_bottom = line_top - line_height;

	line_info_t line_info;
	line_info.line_start = this->line_start;
	line_info.bounds.left = line_left;
	line_info.bounds.top = line_top;
	line_info.bounds.width = line_width;
	line_info.bounds.height = line_height;

	vector_push_back( this->lines,  &line_info);


	if (line_left < this->bounds.left)
	{
		this->bounds.left = line_left;
	}
	if (line_top > this->bounds.top)
	{
		this->bounds.top = line_top;
	}

	float this_right = this->bounds.left + this->bounds.width;
	float this_bottom = this->bounds.top - this->bounds.height;

	if (line_right > this_right)
	{
		this->bounds.width = line_right - this->bounds.left;
	}
	if (line_bottom < this_bottom)
	{
		this->bounds.height = this->bounds.top - line_bottom;
	}

	if ( advancePen )
	{
		pen->x = this->origin.x;
		pen->y += (int)(this->line_descender);
	}

	this->line_descender = 0;
	this->line_ascender = 0;
	this->line_start = vector_size( this->buffer->items );
	this->line_left = pen->x;
}

