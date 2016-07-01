#include "zyTextLayout.h"
#include "utf8-utils.h"
#include "distance-field.h"

#include <glm/gtc/type_ptr.hpp>

#include <math.h>


zyTextLayout::zyTextLayout( texture_font* ft )
	: align_( ALIGN_LEFT ), fixed_line_length_( -1 )
{
	this->font_   = ft;
	this->line_start_ = 0;
	this->line_length_current_ = 0;

	this->line_descender_ = this->font_->getDescender();
	this->line_ascender_ = this->font_->getAscender();
	this->clear();

	this->setBeginPos( glm::vec2( 0, 0 ) );	
	
}

zyTextLayout::~zyTextLayout()
{

}


void zyTextLayout::render()
{

	glActiveTexture( GL_TEXTURE0 );
	glEnable( GL_TEXTURE_2D );
	glBindTexture( GL_TEXTURE_2D, this->font_->getAtlas()->getTexID() );

// 	glEnable( GL_BLEND );
// 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
// 	glBlendColor( 1, 1, 1, 1 );

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

// 	glBegin( GL_QUADS );
// 	for (int i = 0; i < this->lines_.size(); ++ i )
// 	{
// 		bounds_t bt = lines_[i].bounds;
// 
// 		glVertex2f(  bt.left, bt.top );
// 		glVertex2f(  bt.left, bt.top - bt.height  );
// 		glVertex2f(  bt.left + bt.width, bt.top - bt.height );
// 		glVertex2f(  bt.left + bt.width, bt.top );
// 		
// 	}
// 	glEnd( );

}

void zyTextLayout::setBeginPos(glm::vec2& pen)
{
	this->pen_current_ = pen;
	this->pen_orign_ = pen;
	this->line_left_ = pen.x;
	this->bounds_.left = pen.x;
	this->bounds_.top = pen.y;
}

void zyTextLayout::addText( const std::string text, size_t length /*= 0 */)
{
 	const char* text_u8 = text.c_str();
	if ( !ftgl::is_uft8( text_u8 ) )
	{
		//如果不是utf8编码，就认为是GBK编码的;
		//略有不严谨，但是在中文环境下管用;
		text_u8 = ftgl::GBK_to_utf8( text.c_str() );
	}

	if( length == 0 )
	{
		length = utf8_strlen( text_u8 );
	}


	const char* prev_character = NULL;

	for( size_t i = 0; utf8_strlen( text_u8 + i ) && length; i += utf8_surrogate_len( text_u8 + i ) )
	{
		this->addChar( text_u8 + i, prev_character );
		prev_character = text_u8 + i;
		length--;
	}

	if ( this->font_->getUpdateFlag() )
	{
		this->font_->setUpdateFlag( false );

		//this->font_->getAtlas()->makeDistanceField();

		this->font_->getAtlas()->upload();
	}
	
	if ( pen_current_.x != pen_orign_.x )
	{
		this->finishLine( true );
	}

	//delete []text_u8;
}

void zyTextLayout::clear()
{
	this->lines_.clear();
	this->buffer_.clear();

	this->line_start_ = 0;
	this->pen_current_	= pen_orign_;
	this->line_left_	= pen_orign_.x;
	this->bounds_.left	= pen_orign_.x;
	this->bounds_.top	= pen_orign_.y;
	this->bounds_.width = 0;
	this->bounds_.height = 0;
}

void zyTextLayout::addChar( const char* current, const char* previous )
{

	texture_glyph *glyph;
	//texture_glyph *black;
	float kerning = 0.0f;

	
	if( *current == '\n' )
	{
		this->finishLine( true );
		return;
	}

	glyph = this->font_->getGlyph(  current );
	//black = this->font_->getGlyph( NULL );

	if( glyph == NULL )
	{
		return;
	}

	if( previous && this->font_->getKerningStatus() )
	{
		kerning = glyph->getKerning( previous );
	}
	pen_current_.x += kerning;

	//--大于最大行长度,进行换行;
	float current_line_length = pen_current_.x + glyph->advance_x;
	if ( current_line_length > this->fixed_line_length_ && this->fixed_line_length_ > 0 )
	{
		this->finishLine(  true );
	}

	// Actual glyph

	float x0 = ( pen_current_.x + glyph->offset_x );
	float y0 = (float)( pen_current_.y + glyph->offset_y );
	float x1 = ( x0 + glyph->width );
	float y1 = (float)( y0 - glyph->height );
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

	pen_current_.x += glyph->advance_x;

}

void zyTextLayout::finishLine( bool advancePen )
{
	float line_left		= this->line_left_;
	float line_right	= pen_current_.x;
	float line_width	= line_right - line_left;
	float line_top		= pen_current_.y + this->line_ascender_;
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
		this->line_length_current_ = bounds_.width;
	}
	if (line_bottom < this_bottom)
	{
		this->bounds_.height = this->bounds_.top - line_bottom;
	}

	if ( advancePen )
	{
		pen_current_.x = this->pen_orign_.x;
		pen_current_.y -= ( this->line_ascender_ - this->line_descender_ );
	}

	this->line_start_ = this->buffer_.size();
	this->line_left_ = pen_current_.x;
}




void zyTextLayout::setAlign(  Align align)
{
	if ( this->align_ == align )
	{
		return;
	}

	this->align_ = align;

	size_t total_items = this->buffer_.size();
	if ( this->line_start_ != total_items )
	{
		this->finishLine( false );
	}

	size_t i, j;
	int k;
	float this_left, this_right, this_center;
	float line_left, line_right, line_center;
	float dx;

	this_left = this->bounds_.left;
	this_right = this->bounds_.left + this->bounds_.width;
	if ( this->fixed_line_length_ > 0  )
	{
		this_right = this->bounds_.left + this->fixed_line_length_;
		this->bounds_.width = this->fixed_line_length_;
	}
	
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

		if ( ALIGN_RIGHT == align ) // ALIGN_RIGHT
		{
			dx = this_right - line_right;
		}
		else if ( ALIGN_LEFT == align  )
		{
			line_left = line_info->bounds.left;
			dx = this_left - line_left;
			this->bounds_.width = this->line_length_current_;
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

		line_info->bounds.left += dx;
	}

}



void zyTextLayout::moveLastLine( float dy )
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

void zyTextLayout::moveTo(glm::vec2& pos)
{
	glm::vec3 delta( pos - this->pen_orign_, 0 );

	for ( int i = 0; i < this->buffer_.size(); ++i )
	{
		buffer_[i].v0.vertice += delta;
		buffer_[i].v1.vertice += delta;
		buffer_[i].v2.vertice += delta;
		buffer_[i].v3.vertice += delta;
	}

	for ( int j = 0; j < this->lines_.size(); ++j )
	{
		lines_[j].bounds.left += delta.x;
		lines_[j].bounds.top += delta.y;
	}

	this->bounds_.left += delta.x;
	this->bounds_.top += delta.y;

	this->pen_current_ += glm::vec2( delta );
	this->pen_orign_ += glm::vec2( delta );
	this->line_left_ += delta.x;

}

