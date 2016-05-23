#include "zy_text_layout.h"
#include "utf8-utils.h"
#include "distance-field.h"

#include <glm/gtc/type_ptr.hpp>

#include <math.h>


zy_text_layout::zy_text_layout( GLSLShader* shader, texture_font* ft )
	: align_( ALIGN_LEFT ), fixed_line_length_( -1 )
{

	this->shader_ = shader;
	this->font_   = ft;
	this->line_start_ = 0;
	
}

zy_text_layout::~zy_text_layout()
{
	

}

void zy_text_layout::render()
{


	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, this->font_->getAtlas()->getTexID() );

// 	glEnable( GL_BLEND );
// 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
// 	glBlendColor( 1, 1, 1, 1 );

	this->shader_->Use();
	glBegin( GL_QUADS );
	for ( size_t i = 0; i < this->buffer_.size(); ++ i )
	{
		character_t ch = this->buffer_[i];

		glTexCoord2fv( glm::value_ptr( ch.v0.texture ) );glVertex3fv( glm::value_ptr( ch.v0.vertice ) );
		glTexCoord2fv( glm::value_ptr( ch.v1.texture ) );glVertex3fv( glm::value_ptr( ch.v1.vertice ) );
		glTexCoord2fv( glm::value_ptr( ch.v2.texture ) );glVertex3fv( glm::value_ptr( ch.v2.vertice ) );
		glTexCoord2fv( glm::value_ptr( ch.v3.texture ) );glVertex3fv( glm::value_ptr( ch.v3.vertice ) );
	}
	glEnd();
	this->shader_->UnUse();
	glBindTexture( GL_TEXTURE_2D, 0 );
	//this->shader.Use()
// 	{
// 		glUniform1i( this->shader_texture, 0 );
// 		glUniform3f( this->shader_pixel, 1.0f/this->manager->atlas->width, 1.0f/this->manager->atlas->height, (float)this->manager->atlas->depth );
// 		this->buffer->render( GL_TRIANGLES );
// 		glBindTexture( GL_TEXTURE_2D, 0 );
// 		glBlendColor( 0, 0, 0, 0 );
// 	}
	//this->shader.UnUse();

	

}

void zy_text_layout::addText(glm::vec2* pen , const char* text, size_t length /*= 0 */)
{
	const char* text_u8 = GBK_to_utf8( text );

	if( length == 0 )
	{
		length = utf8_strlen(text);
	}

	const char* prev_character = NULL;

	if( this->buffer_.empty() )
	{
		this->pen_orign_ = *pen;
		this->line_left_ = pen->x;
		this->bounds_.left = pen->x;
		this->bounds_.top = pen->y;
	}
	else
	{
		if ( pen->x < this->pen_orign_.x )
		{
			this->pen_orign_.x = pen->x;
		}
		if ( pen->y != this->pen_last_y_ )
		{
			this->finishLine( pen, false);
		}
	}

	for( size_t i = 0; utf8_strlen( text_u8 + i ) && length; i += utf8_surrogate_len( text_u8 + i ) )
	{
		this->addChar( pen, text_u8 + i, prev_character );
		prev_character = text + i;
		length--;
	}

	this->pen_last_y_ = pen->y;

	if ( this->font_->getUpdateFlag() )
	{
		this->font_->setUpdateFlag( false );

// 		unsigned char* map;
// 		
// 		ftgl::texture_atlas* atlas = this->font_->getAtlas();
// 
// 		map = ftgl::make_distance_mapb( atlas->getData(), atlas->getWidth(), atlas->getHeight()  );
// 		
// 		memcpy( this->font_->getAtlas()->getData(), map, atlas->getWidth()*atlas->getHeight()*sizeof( unsigned char ) );
// 		
// 		free(map);

		this->font_->getAtlas()->upload();
	}
	

	delete []text_u8;
}

void zy_text_layout::addChar(glm::vec2* pen, const char* current, const char* previous)
{

	texture_glyph *glyph;
	texture_glyph *black;
	float kerning = 0.0f;

	this->line_descender_ = this->font_->getDescender();
	this->line_ascender_ = this->font_->getAscender();
	//

	if( *current == '\n' )
	{
		this->finishLine( pen, true );
		return;
	}

	glyph = this->font_->getGlyph(  current );
	black = this->font_->getGlyph( NULL );

	if( glyph == NULL )
	{
		return;
	}

	if( previous && this->font_->getKerningStatus() )
	{
		kerning = glyph->getKerning( previous );
	}
	pen->x += kerning;

	//--大于最大行长度,进行换行;
	float current_line_length = pen->x + glyph->advance_x;
	if ( current_line_length > this->fixed_line_length_ )
	{
		this->finishLine( pen, true );
	}


	// Actual glyph

	float x0 = ( pen->x + glyph->offset_x );
	float y0 = (float)(int)( pen->y + glyph->offset_y );
	float x1 = ( x0 + glyph->width );
	float y1 = (float)(int)( y0 - glyph->height );
	float s0 = glyph->s0;
	float t0 = glyph->t0;
	float s1 = glyph->s1;
	float t1 = glyph->t1;

	character_t c;
	c.v0.vertice = glm::vec3( x0, y0, 0.0 ); c.v0.texture = glm::vec2( s0, t0 );
	c.v1.vertice = glm::vec3( x0, y1, 0.0 ); c.v1.texture = glm::vec2( s0, t1 );
	c.v2.vertice = glm::vec3( x1, y1, 0.0 ); c.v2.texture = glm::vec2( s1, t1 );
	c.v3.vertice = glm::vec3( x1, y0, 0.0 ); c.v3.texture = glm::vec2( s1, t0 );

	this->buffer_.push_back( c );

	pen->x += glyph->advance_x;

}

void zy_text_layout::finishLine(glm::vec2* pen, bool advancePen)
{
	float line_left		= this->line_left_;
	float line_right	= pen->x;
	float line_width	= line_right - line_left;
	float line_top		= pen->y + this->line_ascender_;
	float line_height	= this->line_ascender_ - this->line_descender_;
	float line_bottom	= line_top - line_height;

	line_info_t line_in;
	line_in.line_start		= this->line_start_;
	line_in.bounds.left		= line_left;
	line_in.bounds.top		= line_top;
	line_in.bounds.width	= line_width;
	line_in.bounds.height	= line_height;

	this->lines_.push_back( line_in );

	if (line_left < this->bounds_.left )
	{
		this->bounds_.left = line_left;
	}
	if (line_top > this->bounds_.top )
	{
		this->bounds_.top = line_top;
	}

	float this_right = this->bounds_.left + this->bounds_.width;
	float this_bottom = this->bounds_.top - this->bounds_.height;

	if (line_right > this_right)
	{
		this->bounds_.width = line_right - this->bounds_.left;
	}
	if (line_bottom < this_bottom)
	{
		this->bounds_.height = this->bounds_.top - line_bottom;
	}

	if ( advancePen )
	{
		pen->x = this->pen_orign_.x;
		pen->y -= ( this->line_ascender_ - this->line_descender_ );
	}

	this->line_descender_ = 0;
	this->line_ascender_ = 0;
	this->line_start_ = this->buffer_.size();
	this->line_left_ = pen->x;
}


void zy_text_layout::setAlign(  Align align)
{
	if ( ALIGN_LEFT == align )
	{
		return;
	}

	size_t total_items = this->buffer_.size();
	if ( this->line_start_ != total_items )
	{
		glm::vec2 pen( this->pen_orign_.x, this->pen_last_y_ );
		this->finishLine( &pen, false );
	}

	size_t i, j;
	int k;
	float this_left, this_right, this_center;
	float line_left, line_right, line_center;
	float dx;

	this_left = this->bounds_.left;
	this_right = this->bounds_.left + this->bounds_.width;
	this_center = (this_left + this_right) / 2;

	line_info_t* line_info;
	size_t lines_count, line_end;

	lines_count = this->lines_.size();
	for ( i = 0; i < lines_count; ++i )
	{
		line_info = &this->lines_[i];

		if ( i + 1 < lines_count )
		{
			line_end = this->lines_[ i + 1 ].line_start;
		}
		else
		{
			line_end = this->buffer_.size();
		}

		line_right = line_info->bounds.left + line_info->bounds.width;

		if ( ALIGN_RIGHT == align )
		{
			dx = this_right - line_right;
		}
		else // ALIGN_CENTER
		{
			line_left = line_info->bounds.left;
			line_center = (line_left + line_right) / 2;
			dx = this_center - line_center;
		}

		dx = glm::round( dx );

		for( j=line_info->line_start; j <  line_end; ++j )
		{
			character_t*  ch = &this->buffer_[j];
			ch->v0.vertice.x += dx;
			ch->v1.vertice.x += dx;
			ch->v2.vertice.x += dx;
			ch->v3.vertice.x += dx;
		}
	}
}



void zy_text_layout::moveLastLine( float dy )
{
	size_t i;
	int j;
	for( i = this->line_start_; i <  this->buffer_.size(); ++i )
	{
		character_t* ch = &this->buffer_[i];

		ch->v0.vertice.y -= dy;
		ch->v1.vertice.y -= dy;
		ch->v2.vertice.y -= dy;
		ch->v3.vertice.y -= dy;
	}
}
