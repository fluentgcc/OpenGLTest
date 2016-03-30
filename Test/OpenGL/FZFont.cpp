#include "FZFont.h"
#include <iostream>

#include <glm/gtc/type_ptr.hpp>

FZFont::FZFont(const char* file_path )
{
	if (FT_Init_FreeType( &this->ft_library_ ) )
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;

	}

	if ( FT_New_Face( this->ft_library_, "C:/Windows/Fonts/msyh.ttf", 0, &this->ft_face_ ) )
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
	}

	this->setFaceSize( 48 );

	assert( glGetError()== GL_NO_ERROR );

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

	this->shader_.LoadFromFile( GL_VERTEX_SHADER,   "shaders/text.vert" );
	this->shader_.LoadFromFile( GL_FRAGMENT_SHADER, "shaders/text.frag" );
	this->shader_.CreateAndLinkProgram();
	this->shader_.Use();
	{
		this->shader_.AddAttribute("vVertex" ); 
		this->shader_.AddAttribute("vTexCoord" );
		this->shader_.AddUniform( "MVP" );
		this->shader_.AddUniform( "vColor" );
	}
	this->shader_.UnUse();

	glGenVertexArrays( 1, &VAO );
	glGenBuffers( 1, &VBO );
	glBindVertexArray( VAO );
	glBindBuffer( GL_ARRAY_BUFFER, VBO );
	glBufferData( GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 5, NULL, GL_DYNAMIC_DRAW );
		
	assert( glGetError()== GL_NO_ERROR );
	
	glEnableVertexAttribArray(  this->shader_[ "vVertex" ] );
	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 0 );

	glEnableVertexAttribArray( this->shader_[ "vTexCoord" ] );
	glVertexAttribPointer( 0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	assert( glGetError()== GL_NO_ERROR );

	this->initFrequency();

	assert( glGetError()== GL_NO_ERROR );

}

FZFont::~FZFont()
{
	FT_Done_Face( this->ft_face_ );
	FT_Done_FreeType( this->ft_library_ );
}

void FZFont::renderText(std::wstring& text, glm::vec3 pos, glm::mat4 MVP,  GLfloat scale, glm::vec3 color )
{
	glEnable( GL_TEXTURE_2D );
	this->shader_.Use();
	{
		glUniform3fv( shader_("vColor"), 1, glm::value_ptr( color ) );
		glUniformMatrix4fv( shader_( "MVP" ), 1, GL_FALSE, glm::value_ptr( MVP ) );

		glActiveTexture( GL_TEXTURE0 );
		glBindVertexArray( VAO );

		std::wstring::const_iterator c;
		for ( c = text.begin(); c != text.end(); c++ ) 
		{

			if ( !this->checkGlyph( *c ) )
			{
				continue;
			}
			

			Character ch = characters_[*c];

			GLfloat xpos = pos.x + ch.Bearing.x * scale;
			GLfloat ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale;

			GLfloat w = ch.Size.x * scale;
			GLfloat h = ch.Size.y * scale;

			// Update VBO for each character
			GLfloat vertices[6][5] = {
				{ xpos,     ypos + h, pos.z,  0.0, 0.0 },            
				{ xpos,     ypos,     pos.z,  0.0, 1.0 },
				{ xpos + w, ypos,     pos.z,  1.0, 1.0 },

				{ xpos,     ypos + h, pos.z,  0.0, 0.0 },
				{ xpos + w, ypos,     pos.z,  1.0, 1.0 },
				{ xpos + w, ypos + h, pos.z,  1.0, 0.0 }           
			};
			// Render glyph texture over quad
			glBindTexture( GL_TEXTURE_2D, ch.TextureID );
			// Update content of VBO memory
			glBindBuffer( GL_ARRAY_BUFFER, VBO );

			glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( vertices ), vertices ); // Be sure to use glBufferSubData and not glBufferData

			glBindBuffer( GL_ARRAY_BUFFER, 0 );
			// Render quad
			glDrawArrays( GL_TRIANGLES, 0, 6 );
			// Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
		
			pos.x += ( ch.Advance >> 6 ) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64 (divide amount of 1/64th pixels by 64 to get amount of pixels))
		}
		glBindVertexArray( 0 );
		glBindTexture( GL_TEXTURE_2D, 0 );
	}

	this->shader_.UnUse();

}

void FZFont::setFaceSize( unsigned int sz )
{
	this->face_size_ = sz;
	FT_Set_Pixel_Sizes( this->ft_face_, 0, sz );
	
}

void FZFont::initFrequency()
{
	for (GLubyte c = 0; c < 128; c++)
	{
		this->checkGlyph( unsigned int( c ) );
	}
}

bool FZFont::checkGlyph(unsigned int code )
{
	if ( this->characters_.find( code ) != this->characters_.end() )
	{
		return true;
	}

	if ( FT_Load_Char( this->ft_face_, code, FT_LOAD_RENDER ) )
	{
		std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
		return false;
	}

	unsigned int glyth_w = ft_face_->glyph->bitmap.width;
	unsigned int glyth_h = ft_face_->glyph->bitmap.rows;
	// Generate texture
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, glyth_w, glyth_h, 0, GL_RED, GL_UNSIGNED_BYTE, ft_face_->glyph->bitmap.buffer );
	//glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, glyth_w, glyth_h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, ft_face_->glyph->bitmap.buffer );
	
	// Set texture options
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	// Now store character for later use
	Character character = {
		texture,
		glm::ivec2( glyth_w, glyth_h ),
		glm::ivec2( ft_face_->glyph->bitmap_left, ft_face_->glyph->bitmap_top ),
		ft_face_->glyph->advance.x
	};
	this->characters_.insert( std::pair<unsigned int, Character>( code, character ) );

	glBindTexture(GL_TEXTURE_2D, 0);

	return true;
}
