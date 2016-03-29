#ifndef __FZ_FONT__H
#define __FZ_FONT__H

#include <ft2build.h>
#include FT_FREETYPE_H

#include "GLSLShader.h"
#include <map>

#include <glm/glm.hpp>
#include <glm/fwd.hpp>

struct Character
{
	GLuint TextureID; 
	glm::ivec2 Size;    // Size of glyph
	glm::ivec2 Bearing;  // Offset from baseline to left/top of glyph
	GLuint Advance;    // Horizontal offset to advance to next glyph
};

class FZFont
{
public:

	FZFont( const char* file_path );
	~FZFont();

	void renderText( std::wstring& text,glm::vec3 pos, glm::mat4 MVP,  GLfloat scale, glm::vec3 color );

	void setFaceSize( unsigned int sz );
	unsigned int  getFaceSize(){ return this->face_size_; }

	float advance(  std::wstring& text );
	float ascender();
	float descender();

	void initFrequency();



protected:
	bool checkGlyph( unsigned int );

private:
	GLSLShader shader_;

	unsigned int face_size_;

	std::map< unsigned int, Character > characters_;

	FT_Library ft_library_;
	FT_Face ft_face_;

	GLuint VAO, VBO;

};



#endif