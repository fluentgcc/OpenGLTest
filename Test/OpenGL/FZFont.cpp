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
//--------------------------------
	glEnable( GL_TEXTURE_2D );
	this->initFrequency();
//--------------------------------
	shader_.LoadFromFile( GL_VERTEX_SHADER,   "shaders/text.vert" );
	shader_.LoadFromFile( GL_FRAGMENT_SHADER, "shaders/text.frag" );
	shader_.CreateAndLinkProgram();
	shader_.Use();
	{
		shader_.AddAttribute("vVertex"); 
		shader_.AddUniform("MVP");
		shader_.AddAttribute("vTextCoord");
		//glUniform3fv( shader_("vColor"),1, glm::value_ptr( color ) );
	}
	shader_.UnUse();
//--------------------------------
	glGenVertexArrays( 1, &this->vaoID_ );
	glGenBuffers( 1, &this->vboVerticesID_ );
	glGenBuffers( 1, &this->vboIndicesID_ );

	glBindVertexArray( this->vaoID_ );
	{
		glBindBuffer( GL_ARRAY_BUFFER, vboVerticesID_ );
		glBufferData( GL_ARRAY_BUFFER, 6 * 4 * sizeof( GLfloat ), nullptr, GL_STATIC_DRAW );

		assert( glGetError()== GL_NO_ERROR );

		glEnableVertexAttribArray( this->shader_[ "vVertex" ] );
		glVertexAttribPointer( shader_[ "vVertex" ], 4, GL_FLOAT, GL_FALSE, 0, NULL );
		assert( glGetError()== GL_NO_ERROR );

		glEnableVertexAttribArray( this->shader_[ "vTextCoord" ] );
		glVertexAttribPointer( shader_[ "vTextCoord" ], 2, GL_FLOAT, GL_FALSE, 0, ( const GLvoid * ) ( 16 * sizeof( GLfloat ) ) );

		assert( glGetError()== GL_NO_ERROR );

		const GLfloat text_pos[] =
		{
			0.0f, 0.0f,
			1.0f, 0.0f,
			1.0f, 1.0f,
			0.0f, 1.0f
		};

		glBufferSubData(GL_ARRAY_BUFFER, 16 * sizeof( GLfloat ), sizeof( text_pos ), text_pos );


		static const GLushort vertex_indeces[] = 
		{
			0, 1, 2, 0, 2, 3
		};

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID_ );
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof( GLushort ), &vertex_indeces[0], GL_STATIC_DRAW );

		assert( glGetError()== GL_NO_ERROR );
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
	}

	glBindVertexArray( 0 );

//---------------------------------------------------------------------------------

}

FZFont::~FZFont()
{
	FT_Done_Face( this->ft_face_ );
	FT_Done_FreeType( this->ft_library_ );
}

void FZFont::renderText(std::wstring& text, glm::vec3 pos, glm::mat4 MVP,  GLfloat scale, glm::vec3 color )
{
	glActiveTexture( GL_TEXTURE0 );
	glBindVertexArray( this->vaoID_ );
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID_ );

	this->shader_.Use();
	glUniformMatrix4fv( shader_( "MVP" ) , 1, GL_FALSE, glm::value_ptr( MVP )  );

	std::wstring::const_iterator c;
	for ( c = text.begin(); c != text.end(); c++ ) 
	{
		Character ch = characters_[*c];
		glBindTexture( GL_TEXTURE_2D, ch.TextureID );

		GLfloat xpos = pos.x + ch.Bearing.x * scale / 500.0 ;
		GLfloat ypos = pos.y - (ch.Size.y - ch.Bearing.y) * scale / 500.0;

		GLfloat w = ch.Size.x * scale / 500.0;
		GLfloat h = ch.Size.y * scale / 500.0;
		// Update VBO for each character
		const GLfloat vertex_pos[] = 
		{
			xpos,	  ypos + h, pos.z, 1.0f,
			xpos + w, ypos + h, pos.z, 1.0f,
			xpos + w, ypos, 	pos.z, 1.0f,
			xpos,     ypos,	    pos.z, 1.0f
		};

		glBindBuffer( GL_ARRAY_BUFFER, vboVerticesID_ );
		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof( vertex_pos ), vertex_pos );
		glBindBuffer( GL_ARRAY_BUFFER, 0 );

		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL );

		pos.x += (ch.Advance >> 6) * scale /500.0;
	}
	this->shader_.UnUse();

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	
//------------------------------------------------------------------


// 	GLuint tex_id =  this->characters_[ 66 ].TextureID;
// 
// 	glBindTexture( GL_TEXTURE_2D, tex_id );
// 
// 	this->shader_.Use();
// 	{
// 		glUniformMatrix4fv( shader_( "MVP" ) , 1, GL_FALSE, glm::value_ptr( MVP )  );
// 		
// 		const GLfloat vertex_pos[] = 
// 		{
// 			0.0f, 0.0f, -2.0f, 1.0f,
// 			1.0f, 0.0f, -2.0f, 1.0f,
// 			1.0f, 1.0f, -2.0f, 1.0f,
// 			0.0f, 1.0f, -2.0f, 1.0f,
// 		};
// 
// 
// 
// 		glBindBuffer( GL_ARRAY_BUFFER, vboVerticesID_ );
// 		glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof( vertex_pos ), vertex_pos );
// 		glBindBuffer( GL_ARRAY_BUFFER, 0 );
// 		
// 		glBindVertexArray( this->vaoID_ );
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vboIndicesID_ );
// 		glDrawElements( GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, NULL );
// 
// 		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0 );
// 		glBindVertexArray( 0 );
// 	}
// 
// 	this->shader_.UnUse();

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

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); 

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
