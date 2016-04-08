#ifndef __FONT_MANAGER_H__
#define __FONT_MANAGER_H__

#include "markup.h"
#include "texture-font.h"
#include "texture-atlas.h"

#include <vector>

namespace ftgl 
{
/**
 * Structure in charge of caching fonts.
 *
 * <b>Example Usage</b>:
 * @code
 * #include "font-manager.h"
 *
 * int main( int arrgc, char *argv[] )
 * {
 *     font_manager_t * manager = manager_new( 512, 512, 1 );
 *     texture_font_t * font = font_manager_get( manager, "Mono", 12, 0, 0 );
 *
 *     return 0;
 * }
 * @endcode

/**
 * Structure in charge of caching fonts.
 */
class font_manager 
{
public:
	static font_manager* instance();
	~font_manager();

	//使用之前必须执行;参数为创建atles的大小;
	void init(  size_t width, size_t height, size_t depth );


	//使用时 先get;
	
	void deleteFont( texture_font* font );


	//Request for a font;
	texture_font* getFontFromFile( const char* filename, const float size );
	
	//
	texture_font* getFontFromDescription( const char * family, const float size, const int bold, const int italic );
	texture_font* getFontFromMarkup( const markup* markup_a );

	//Search for a font filename that match description.
	char* matchDescription( const char * family, const float size, const int bold, const int italic );
			   
private:	   
	font_manager();

private:
    /**
     * Texture atlas to hold font glyphs.
     */
	texture_atlas * atlas;

    /**
     * Cached textures.
     */
	std::vector< texture_font* > fonts;

    /**
     * Default glyphs to be loaded when loading a new font.
     */
    char * cache;
};

}


#endif /* __FONT_MANAGER_H__ */
