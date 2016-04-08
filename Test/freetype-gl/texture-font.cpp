#include "texture-font.h"
#include FT_STROKER_H
// #include FT_ADVANCES_H
#include FT_LCD_FILTER_H

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "platform.h"
#include "utf8-utils.h"

#define HRES  64
#define HRESf 64.f
#define DPI   72

#undef __FTERRORS_H__
#define FT_ERRORDEF( e, v, s )  { e, s },
#define FT_ERROR_START_LIST     {
#define FT_ERROR_END_LIST       { 0, 0 } };
const struct {
	int          code;
	const char*  message;
} FT_Errors[] =
#include FT_ERRORS_H

//------------------------------------------------------------
ftgl::texture_glyph::texture_glyph()
{
	this->codepoint  = -1;
	this->width     = 0;
	this->height    = 0;
	this->outline_type = 0;
	this->outline_thickness = 0.0;
	this->offset_x  = 0;
	this->offset_y  = 0;
	this->advance_x = 0.0;
	this->advance_y = 0.0;
	this->s0        = 0.0;
	this->t0        = 0.0;
	this->s1        = 0.0;
	this->t1        = 0.0;
}

ftgl::texture_glyph::~texture_glyph()
{
	this->kerning.clear();
}

float ftgl::texture_glyph::getKerning(const char * codepoint)
{
	size_t i;
	uint32_t ucodepoint = utf8_to_utf32( codepoint );

	for( i=0; i < this->kerning.size(); ++i )
	{
		kerning_t kerning_c = this->kerning[i];
		if( kerning_c.codepoint == ucodepoint )
		{
			return kerning_c.kerning;
		}
	}
	return 0;
}
//--------------------------------------------------------------

ftgl::texture_font::texture_font(texture_atlas* atlas, const float pt_size, const char * filename)
{
	assert(filename);

	this->atlas = atlas;
	this->size  = pt_size;

	this->location = TEXTURE_FONT_FILE;
	this->filename = strdup( filename );

	if ( 0 != this->init() ) 
	{
		fprintf( stderr, "text_font_init_error"  );
		return;
	}
	
}

ftgl::texture_font::texture_font(texture_atlas* atlas, float pt_size, const void *memory_base, size_t memory_size)
{
	assert(memory_base);
	assert(memory_size);

	this->atlas = atlas;
	this->size  = pt_size;

	this->location = TEXTURE_FONT_MEMORY;
	this->memory.base = memory_base;
	this->memory.size = memory_size;

	if ( 0 != this->init() ) 
	{
		fprintf( stderr, "text_font_init_error" );
		return;
	}
	

}

ftgl::texture_font::~texture_font()
{
	size_t i;
	texture_glyph *glyph;

	if( this->location == TEXTURE_FONT_FILE && this->filename)
		free( this->filename );

	for( i = 0; i < this->glyphs.size(); ++i )
	{
		glyph = this->glyphs[i];
		delete glyph;
	}

	this->glyphs.clear();
}


ftgl::texture_glyph* ftgl::texture_font::getGlyph(const char* codepoint)
{
	texture_glyph *glyph;

    assert( this->filename );
    assert( this->atlas );

    /* Check if codepoint has been already loaded */
    if( (glyph = this->findGlyph( codepoint ) ) )
	{
        return glyph;
	}


    /* codepoint NULL is special : it is used for line drawing (overline,
     * underline, strikethrough) and background.
     */
    if( !codepoint )
    {
        size_t width  = this->atlas->getWidth();
        size_t height = this->atlas->getHeight();
        glm::ivec4 region = this->atlas->getRegion( 5, 5 );
        texture_glyph * glyph = new texture_glyph();
        static unsigned char data[4*4*3] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
                                            -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
        if ( region.x < 0 )
        {
            fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
            return NULL;
        }
        this->atlas->setRegion( region.x, region.y, 4, 4, data, 0 );
        glyph->codepoint = -1;
        glyph->s0 = (region.x+2)/(float)width;
        glyph->t0 = (region.y+2)/(float)height;
        glyph->s1 = (region.x+3)/(float)width;
        glyph->t1 = (region.y+3)/(float)height;

        this->glyphs.push_back( glyph );
        return glyph;
    }

    /* Glyph has not been already loaded */
    if( this->loadGlyphs( codepoint ) == 0 )
    {
        return this->findGlyph( codepoint );
    }
    return NULL;

}


size_t ftgl::texture_font::loadGlyphs( const char* codepoints )
{
	size_t i, x, y, width, height, depth, w, h;

	FT_Library library;
	FT_Error error;
	FT_Face face;
	FT_Glyph ft_glyph;
	FT_GlyphSlot slot;
	FT_Bitmap ft_bitmap;

	FT_UInt glyph_index;
	texture_glyph* glyph;
	FT_Int32 flags = 0;
	int ft_glyph_top = 0;
	int ft_glyph_left = 0;

	glm::ivec4 region;
	size_t missed = 0;

	assert( codepoints );

	width  = this->atlas->getWidth();
	height = this->atlas->getHeight();
	depth  = this->atlas->getDepth();

	if (! this->loadFace( this->size, &library, &face ) )
	{
		return utf8_strlen( codepoints );
	}


	/* Load each glyph */
	for( i = 0; i < utf8_strlen(codepoints); i += utf8_surrogate_len( codepoints + i ) ) {
		/* Check if codepoint has been already loaded */
		if( this->findGlyph( codepoints + i ) )
			continue;

		flags = 0;
		ft_glyph_top = 0;
		ft_glyph_left = 0;
		glyph_index = FT_Get_Char_Index( face, ( FT_ULong )utf8_to_utf32( codepoints + i ) );
		// WARNING: We use texture-atlas depth to guess if user wants
		//          LCD subpixel rendering

		if( this->outline_type > 0 )
		{
			flags |= FT_LOAD_NO_BITMAP;
		}
		else
		{
			flags |= FT_LOAD_RENDER;
		}

		if( !this->hinting )
		{
			flags |= FT_LOAD_NO_HINTING | FT_LOAD_NO_AUTOHINT;
		}
		else
		{
			flags |= FT_LOAD_FORCE_AUTOHINT;
		}

		if( depth == 3 )
		{
			FT_Library_SetLcdFilter( library, FT_LCD_FILTER_LIGHT );
			flags |= FT_LOAD_TARGET_LCD;

			if( this->filtering )
			{
				FT_Library_SetLcdFilterWeights( library, this->lcd_weights );
			}
		}

		error = FT_Load_Glyph( face, glyph_index, flags );
		if( error )
		{
			fprintf( stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
				__LINE__, FT_Errors[error].code, FT_Errors[error].message );
			FT_Done_Face( face );
			FT_Done_FreeType( library );
			return utf8_strlen(codepoints) - utf8_strlen(codepoints + i);
		}


		if( this->outline_type == 0 )
		{
			slot            = face->glyph;
			ft_bitmap       = slot->bitmap;
			ft_glyph_top    = slot->bitmap_top;
			ft_glyph_left   = slot->bitmap_left;
		}
		else
		{
			FT_Stroker stroker;
			FT_BitmapGlyph ft_bitmap_glyph;
			error = FT_Stroker_New( library, &stroker );
			if( error )
			{
				fprintf( stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message );
				FT_Done_Face( face );
				FT_Stroker_Done( stroker );
				FT_Done_FreeType( library );
				return 0;
			}
			FT_Stroker_Set(stroker,
				(int)(this->outline_thickness * HRES),
				FT_STROKER_LINECAP_ROUND,
				FT_STROKER_LINEJOIN_ROUND,
				0);
			error = FT_Get_Glyph( face->glyph, &ft_glyph);
			if( error )
			{
				fprintf(stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message);
				FT_Done_Face( face );
				FT_Stroker_Done( stroker );
				FT_Done_FreeType( library );
				return 0;
			}

			if( this->outline_type == 1 )
			{
				error = FT_Glyph_Stroke( &ft_glyph, stroker, 1 );
			}
			else if ( this->outline_type == 2 )
			{
				error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 0, 1 );
			}
			else if ( this->outline_type == 3 )
			{
				error = FT_Glyph_StrokeBorder( &ft_glyph, stroker, 1, 1 );
			}
			if( error )
			{
				fprintf(stderr, "FT_Error (0x%02x) : %s\n",
					FT_Errors[error].code, FT_Errors[error].message);
				FT_Done_Face( face );
				FT_Stroker_Done( stroker );
				FT_Done_FreeType( library );
				return 0;
			}

			if( depth == 1 )
			{
				error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_NORMAL, 0, 1);
				if( error )
				{
					fprintf(stderr, "FT_Error (0x%02x) : %s\n",
						FT_Errors[error].code, FT_Errors[error].message);
					FT_Done_Face( face );
					FT_Stroker_Done( stroker );
					FT_Done_FreeType( library );
					return 0;
				}
			}
			else
			{
				error = FT_Glyph_To_Bitmap( &ft_glyph, FT_RENDER_MODE_LCD, 0, 1);
				if( error )
				{
					fprintf(stderr, "FT_Error (0x%02x) : %s\n",
						FT_Errors[error].code, FT_Errors[error].message);
					FT_Done_Face( face );
					FT_Stroker_Done( stroker );
					FT_Done_FreeType( library );
					return 0;
				}
			}

			ft_bitmap_glyph = (FT_BitmapGlyph) ft_glyph;
			ft_bitmap       = ft_bitmap_glyph->bitmap;
			ft_glyph_top    = ft_bitmap_glyph->top;
			ft_glyph_left   = ft_bitmap_glyph->left;
			FT_Stroker_Done(stroker);
		}

		// We want each glyph to be separated by at least one black pixel
		w = ft_bitmap.width/depth;
		h = ft_bitmap.rows;
		region = this->atlas->getRegion( w+1, h+1 );
		if ( region.x < 0 )
		{
			missed++;
			fprintf( stderr, "Texture atlas is full (line %d)\n",  __LINE__ );
			continue;
		}
		x = region.x;
		y = region.y;
		this->atlas->setRegion( x, y, w, h,
			ft_bitmap.buffer, ft_bitmap.pitch );

		glyph = new texture_glyph();
		glyph->codepoint = utf8_to_utf32( codepoints + i );
		glyph->width    = w;
		glyph->height   = h;
		glyph->outline_type = this->outline_type;
		glyph->outline_thickness = this->outline_thickness;
		glyph->offset_x = ft_glyph_left;
		glyph->offset_y = ft_glyph_top;
		glyph->s0       = x/(float)width;
		glyph->t0       = y/(float)height;
		glyph->s1       = (x + glyph->width)/(float)width;
		glyph->t1       = (y + glyph->height)/(float)height;

		// Discard hinting to get advance
		FT_Load_Glyph( face, glyph_index, FT_LOAD_RENDER | FT_LOAD_NO_HINTING);
		slot = face->glyph;
		glyph->advance_x = slot->advance.x / HRESf;
		glyph->advance_y = slot->advance.y / HRESf;

		this->glyphs.push_back( glyph );

		if( this->outline_type > 0 )
		{
			FT_Done_Glyph( ft_glyph );
		}
	}

	FT_Done_Face( face );
	FT_Done_FreeType( library );
	this->generateKerning();

	return missed;
}


int ftgl::texture_font::loadFace(float size, FT_Library *library, FT_Face *face)
{
	FT_Error error;
	FT_Matrix matrix = {
		( int )( ( 1.0/HRES)  * 0x10000L ),
		( int )( ( 0.0 )      * 0x10000L ),
		( int )( ( 0.0 )      * 0x10000L ),
		( int )( ( 1.0 )      * 0x10000L ) };

		assert(library);
		assert(size);

		/* Initialize library */
		error = FT_Init_FreeType(library);
		if(error) {
			fprintf( stderr, "FT_Error (0x%02x) : %s\n",
				FT_Errors[error].code, FT_Errors[error].message);
			return 0;
		}

		/* Load face */
		switch ( this->location ) {
		case TEXTURE_FONT_FILE:
			error = FT_New_Face(*library, this->filename, 0, face);
			break;

		case TEXTURE_FONT_MEMORY:
			error = FT_New_Memory_Face( *library,
				( const FT_Byte* ) this->memory.base, this->memory.size, 0, face);
			break;
		}

		if(error) {
			fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
				__LINE__, FT_Errors[error].code, FT_Errors[error].message);
			FT_Done_FreeType(*library);
			return 0;
		}

		/* Select charmap */
		error = FT_Select_Charmap(*face, FT_ENCODING_UNICODE);
		if(error) {
			fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
				__LINE__, FT_Errors[error].code, FT_Errors[error].message);
			FT_Done_Face(*face);
			FT_Done_FreeType(*library);
			return 0;
		}

		/* Set char size */
		error = FT_Set_Char_Size(*face, (int)(size * HRES), 0, DPI * HRES, DPI);

		if(error) {
			fprintf(stderr, "FT_Error (line %d, code 0x%02x) : %s\n",
				__LINE__, FT_Errors[error].code, FT_Errors[error].message);
			FT_Done_Face(*face);
			FT_Done_FreeType(*library);
			return 0;
		}

		/* Set transform matrix */
		FT_Set_Transform(*face, &matrix, NULL);

		return 1;
}

void ftgl::texture_font::generateKerning()
{
	size_t i, j;
	FT_Library library;
	FT_Face face;
	FT_UInt glyph_index, prev_index;
	texture_glyph *glyph, *prev_glyph;
	FT_Vector kerning;

	/* Load font */
	if( !this->loadFace( this->size, &library, &face ) )
	{
		return;
	}
	
	/* For each glyph couple combination, check if kerning is necessary */
	/* Starts at index 1 since 0 is for the special backgroudn glyph */
	for( i = 1; i < this->glyphs.size(); ++i )
	{
		glyph = this->glyphs[i];
		glyph_index = FT_Get_Char_Index( face, glyph->codepoint );
		glyph->kerning.clear();

		for( j=1; j < this->glyphs.size(); ++j )
		{
			prev_glyph = this->glyphs[j];
			prev_index = FT_Get_Char_Index( face, prev_glyph->codepoint );
			FT_Get_Kerning( face, prev_index, glyph_index, FT_KERNING_UNFITTED, &kerning );
			// printf("%c(%d)-%c(%d): %ld\n",
			//       prev_glyph->codepoint, prev_glyph->codepoint,
			//       glyph_index, glyph_index, kerning.x);
			if( kerning.x )
			{
				kerning_t k = {prev_glyph->codepoint, kerning.x / (float)( HRESf*HRESf ) };
				glyph->kerning.push_back( k );
			}
		}
	}

	FT_Done_Face( face );
	FT_Done_FreeType( library );
}

int ftgl::texture_font::init()
{
	FT_Library library;
	FT_Face face;
	FT_Size_Metrics metrics;

	assert(this->atlas);
	assert(this->size > 0);
	assert((this->location == TEXTURE_FONT_FILE && this->filename)
		|| (this->location == TEXTURE_FONT_MEMORY
		&& this->memory.base && this->memory.size));

	this->height = 0;
	this->ascender = 0;
	this->descender = 0;
	this->outline_type = 0;
	this->outline_thickness = 0.0;
	this->hinting = 1;
	this->kerning = 1;
	this->filtering = 1;

	// FT_LCD_FILTER_LIGHT   is (0x00, 0x55, 0x56, 0x55, 0x00)
	// FT_LCD_FILTER_DEFAULT is (0x10, 0x40, 0x70, 0x40, 0x10)
	this->lcd_weights[0] = 0x10;
	this->lcd_weights[1] = 0x40;
	this->lcd_weights[2] = 0x70;
	this->lcd_weights[3] = 0x40;
	this->lcd_weights[4] = 0x10;

	if (!this->loadFace( this->size * 100.f, &library, &face))
		return -1;

	this->underline_position = face->underline_position / (float)(HRESf*HRESf) * this->size;
	this->underline_position = round( this->underline_position );
	if( this->underline_position > -2 )
	{
		this->underline_position = -2.0;
	}

	this->underline_thickness = face->underline_thickness / (float)(HRESf*HRESf) * this->size;
	this->underline_thickness = round( this->underline_thickness );
	if( this->underline_thickness < 1 )
	{
		this->underline_thickness = 1.0;
	}

	metrics = face->size->metrics;
	this->ascender = (metrics.ascender >> 6) / 100.0;
	this->descender = (metrics.descender >> 6) / 100.0;
	this->height = (metrics.height >> 6) / 100.0;
	this->linegap = this->height - this->ascender + this->descender;
	FT_Done_Face( face );
	FT_Done_FreeType( library );

	/* NULL is a special glyph */
	this->getGlyph( NULL );

	return 0;
}

ftgl::texture_glyph* ftgl::texture_font::findGlyph( const char* codepoint)
{
	size_t i;
	texture_glyph *glyph;
	uint32_t ucodepoint = utf8_to_utf32( codepoint );

	for( i = 0; i < this->glyphs.size(); ++i )
	{
		glyph = this->glyphs[i];

		// If codepoint is -1, we don't care about outline type or thickness
		if( ( glyph->codepoint == ucodepoint ) &&
			( ( ucodepoint == -1 ) ||
			( ( glyph->outline_type == this->outline_type ) &&
			( glyph->outline_thickness == this->outline_thickness ) ) ) )
		{
			return glyph;
		}
	}

	return NULL;
}
