#ifndef __TEXTURE_FONT_H__
#define __TEXTURE_FONT_H__

#include <stdlib.h>
#include <stdint.h>

#include "vector_t.h"
#include "texture-atlas.h"

#include <ft2build.h>
#include FT_FREETYPE_H

namespace ftgl 
{

	/**
	* Texture font.
	*
	* Example Usage:
	* @code
	* #include "texture-font.h"
	*
	* int main( int arrgc, char *argv[] )
	* {
	*   return 0;
	* }
	* @endcode
	*
	*/

//-----------------------------------------------------------------------
	/**
	* A structure that hold a kerning value relatively to a Unicode
	* codepoint.
	*
	* This structure cannot be used alone since the (necessary) right
	* Unicode codepoint is implicitely held by the owner of this structure.
	*/
	typedef struct kerning_t
	{
		/**
		* Left Unicode codepoint in the kern pair in UTF-32 LE encoding.
		*/
		uint32_t codepoint;

		/**
		* Kerning value (in fractional pixels).
		*/
		float kerning;

	} kerning_t;
//--------------------------------------------------------------------------------



	/*
	* Glyph metrics:
	* --------------
	*
	*                       xmin                     xmax
	*                        |                         |
	*                        |<-------- width -------->|
	*                        |                         |
	*              |         +-------------------------+----------------- ymax
	*              |         |    ggggggggg   ggggg    |     ^        ^
	*              |         |   g:::::::::ggg::::g    |     |        |
	*              |         |  g:::::::::::::::::g    |     |        |
	*              |         | g::::::ggggg::::::gg    |     |        |
	*              |         | g:::::g     g:::::g     |     |        |
	*    offset_x -|-------->| g:::::g     g:::::g     |  offset_y    |
	*              |         | g:::::g     g:::::g     |     |        |
	*              |         | g::::::g    g:::::g     |     |        |
	*              |         | g:::::::ggggg:::::g     |     |        |
	*              |         |  g::::::::::::::::g     |     |      height
	*              |         |   gg::::::::::::::g     |     |        |
	*  baseline ---*---------|---- gggggggg::::::g-----*--------      |
	*            / |         |             g:::::g     |              |
	*     origin   |         | gggggg      g:::::g     |              |
	*              |         | g:::::gg   gg:::::g     |              |
	*              |         |  g::::::ggg:::::::g     |              |
	*              |         |   gg:::::::::::::g      |              |
	*              |         |     ggg::::::ggg        |              |
	*              |         |         gggggg          |              v
	*              |         +-------------------------+----------------- ymin
	*              |                                   |
	*              |------------- advance_x ---------->|
	*/
//-----------------------------------------------------------------------------------
	/**
	* A structure that describe a glyph.
	*/
	class texture_glyph
	{

	public:
		texture_glyph();
		~texture_glyph();

		float getKerning( const char * codepoint );

	public:
		uint32_t codepoint;						//Unicode codepoint this glyph represents in UTF-32 LE encoding.

		size_t width;							//Glyph's width in pixels.
		size_t height;							//Glyph's height in pixels.

		int offset_x;							//Glyph's left bearing expressed in integer pixels.
		int offset_y;							//Glyphs's top bearing expressed in integer pixels.
		
		float advance_x;
		float advance_y;

		float s0;								//First normalized texture coordinate (x) of top-left corner
		float t0;								//Second normalized texture coordinate (y) of top-left corner
		
		float s1;								//First normalized texture coordinate (x) of bottom-right corner
		float t1;								//Second normalized texture coordinate (y) of bottom-right corner

		vector_t * kerning;						//A vector of kerning pairs relative to this glyph.

		int outline_type;						// Glyph outline type (0 = None, 1 = line, 2 = inner, 3 = outer)

		float outline_thickness;				// Glyph outline thickness

	};
//------------------------------------------------------------------------

	/**
	*  Texture font structure.
	*/
	class texture_font
	{

	public:
		texture_font( texture_atlas* atlas, const float pt_size, const char * filename );
		texture_font( texture_atlas* atlas, float pt_size, const void *memory_base, size_t memory_size );

		~texture_font();

		texture_glyph* getGlyph( const char* codepoint );

		/*
		Request the loading of several glyphs at once.
			*
			* @param codepoints Character codepoints to be loaded in UTF-8 encoding. May
			*                   contain duplicates.
			*
			* @return Number of missed glyph if the texture is not big enough to hold
			*         every glyphs.
		*/
		size_t loadGlyphs( const char* codepoints );


		inline float getHeight() { return this->height; }
		inline texture_atlas* getAtlas(){ return this->atlas; }


	private:
		int loadFace( float size, FT_Library *library, FT_Face *face );
		void generateKerning();

		int init();

		texture_glyph* findGlyph( const char* codepoint ); 

	private:
		/**
		* Vector of glyphs contained in this font.
		*/
		vector_t * glyphs;

		/**
		* Atlas structure to store glyphs data.
		*/
		texture_atlas * atlas;

		/**
		* font location
		*/
		enum {
			TEXTURE_FONT_FILE = 0,
			TEXTURE_FONT_MEMORY,
		} location;

		union {
			/**
			* Font filename, for when location == TEXTURE_FONT_FILE
			*/
			char *filename;

			/**
			* Font memory address, for when location == TEXTURE_FONT_MEMORY
			*/
			struct {
				const void *base;
				size_t size;
			} memory;
		};

		/**
		* Font size
		*/
		float size;

		/**
		* Whether to use autohint when rendering font
		*/
		int hinting;

		/**
		* Outline type (0 = None, 1 = line, 2 = inner, 3 = outer)
		*/
		int outline_type;

		/**
		* Outline thickness
		*/
		float outline_thickness;

		/**
		* Whether to use our own lcd filter.
		*/
		int filtering;

		/**
		* LCD filter weights
		*/
		unsigned char lcd_weights[5];

		/**
		* Whether to use kerning if available
		*/
		int kerning;


		/**
		* This field is simply used to compute a default line spacing (i.e., the
		* baseline-to-baseline distance) when writing text with this font. Note
		* that it usually is larger than the sum of the ascender and descender
		* taken as absolute values. There is also no guarantee that no glyphs
		* extend above or below subsequent baselines when using this distance.
		*/
		float height;

		/**
		* This field is the distance that must be placed between two lines of
		* text. The baseline-to-baseline distance should be computed as:
		* ascender - descender + linegap
		*/
		float linegap;

		/**
		* The ascender is the vertical distance from the horizontal baseline to
		* the highest 'character' coordinate in a font face. Unfortunately, font
		* formats define the ascender differently. For some, it represents the
		* ascent of all capital latin characters (without accents), for others it
		* is the ascent of the highest accented character, and finally, other
		* formats define it as being equal to bbox.yMax.
		*/
		float ascender;

		/**
		* The descender is the vertical distance from the horizontal baseline to
		* the lowest 'character' coordinate in a font face. Unfortunately, font
		* formats define the descender differently. For some, it represents the
		* descent of all capital latin characters (without accents), for others it
		* is the ascent of the lowest accented character, and finally, other
		* formats define it as being equal to bbox.yMin. This field is negative
		* for values below the baseline.
		*/
		float descender;

		/**
		* The position of the underline line for this face. It is the center of
		* the underlining stem. Only relevant for scalable formats.
		*/
		float underline_position;

		/**
		* The thickness of the underline for this face. Only relevant for scalable
		* formats.
		*/
		float underline_thickness;

	};

}

#endif /* __TEXTURE_FONT_H__ */
