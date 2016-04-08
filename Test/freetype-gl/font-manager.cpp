#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "font-manager.h"

int file_exists( const char * filename )
{
	FILE * file = fopen( filename, "r" );
	if ( file )
	{
		fclose(file);
		return 1;
	}
	return 0;
}

ftgl::font_manager* ftgl::font_manager::instance()
{
	static font_manager tm;
	return& tm;
}

ftgl::font_manager::font_manager()
{
	this->atlas = nullptr;
}

ftgl::font_manager::~font_manager()
{
	size_t i;
	texture_font *font;

	for( i=0; i< this->fonts.size(); ++i)
	{
		delete this->fonts[i];
	}
	this->fonts.clear();

	delete this->atlas;

	if( this->cache )
	{
		free( this->cache );
	}
	free( this );
}

void ftgl::font_manager::init(size_t width, size_t height, size_t depth)
{
	texture_atlas *atlas = new texture_atlas( width, height, depth );

	this->atlas = atlas;
	this->cache = strdup( " " );
}

void ftgl::font_manager::deleteFont( texture_font* font )
{

	auto it = std::find( this->fonts.begin(), this->fonts.end(), font );

	if ( it != this->fonts.end() )
	{
		delete *it;
		this->fonts.erase( it );
	}
}

ftgl::texture_font* ftgl::font_manager::getFontFromFile(const char* filename, const float size)
{
	size_t i;
	texture_font *font;

	for( i=0; i < this->fonts.size(); ++i )
	{
		font = fonts[i];
		if( ( strcmp( font->getFileName(), filename ) == 0 ) && ( font->size == size ) )
		{
			return font;
		}
	}

	font = new texture_font( this->atlas, size, filename );

	if( font )
	{
		this->fonts.push_back( font );
		font->loadGlyphs( this->cache );
		return font;
	}
	fprintf( stderr, "Unable to load \"%s\" (size=%.1f)\n", filename, size );
	return 0;
}

ftgl::texture_font* ftgl::font_manager::getFontFromDescription(const char * family, const float size, const int bold, const int italic)
{

}

ftgl::texture_font* ftgl::font_manager::getFontFromMarkup(const markup* markup_a)
{

}

char* ftgl::font_manager::matchDescription(const char * family, const float size, const int bold, const int italic)
{

}

