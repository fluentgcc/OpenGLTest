#ifndef __TEXT_BUFFER_H__
#define __TEXT_BUFFER_H__

#include "vertex-buffer.h"
#include "font-manager.h"
#include "markup.h"
#include "GLSLShader.h"

#include <glm/glm.hpp>


namespace ftgl {

/**
 * Use LCD filtering
 */
#define LCD_FILTERING_ON    3

/**
 * Do not use LCD filtering
 */
#define LCD_FILTERING_OFF 1


/**
 * Align enumeration
 */
typedef enum Align
{
    //Align text to the left hand side
    ALIGN_LEFT,

    //Align text to the center
    ALIGN_CENTER,

	//Align text to the right hand side
    ALIGN_RIGHT
} Align;

class text_buffer
{

public:
	
	/*
	* @param depth          Underlying atlas bit depth (1 or 3)
	* @param vert_filename  Path to vertex shader
	* @param frag_filename  Path to fragment shader
	*/
	text_buffer( size_t depth, 
				 const char * vert_filename,
				 const char * frag_filename);

	/*
	* @param depth          Underlying atlas bit depth (1 or 3)
	* @param program        Shader program
	*/
	text_buffer( size_t depth,
				 GLuint program );

	~text_buffer();

	void render();

	/*Print some text to the text buffer
	@param pen  position of text start
	@param ...  a series of markup_t *, char * ended by NULL
	*/
	void print( glm::vec2 * pen, ... );

	/**
	* Add some text to the text buffer
	*
	* @param self   a text buffer
	* @param pen    position of text start
	* @param markup Markup to be used to add text
	* @param text   Text to be added
	* @param length Length of text to be added
	*/
	void add_text( glm::vec2 * pen, markup_t * markup, const char * text, size_t length  );


	/**
	* Add a char to the text buffer
	* @param pen      position of text start
	* @param markup   markup to be used to add text
	* @param current  charactr to be added
	* @param previous previous character (if any)
	*/
	void add_char( glm::vec2 * pen, markup_t * markup, const char * current, const char * previous );

	/**
	* Align all the lines of text already added to the buffer
	* This alignment will be relative to the overall bounds of the
	* text which can be queried by text_buffer_get_bounds
	*
	* @param pen       pen used in last call (must be unmodified)
	* @param alignment desired alignment of text
	*/
	void align( glm::vec2 * pen, Align alignment );

	/*
	Get the rectangle surrounding the text
	* @param pen       pen used in last call (must be unmodified)
	*/
	glm::vec4 get_bounds( glm::vec2* pen );

	//Clear text buffer;
	void clear();


private:
	vertex_buffer *buffer;

	font_manager  *manager;

	glm::vec4	  base_color;

	glm::vec2	origin;
	float		last_pen_y;
	glm::vec4	bounds;

	size_t		line_start;
	float		line_left;
	vector_t*	lines;

	float line_ascender;
	float line_decender;

	GLSLShader shader;

	GLuint shader_texture;
	GLuint shader_pixel;
};



/**
 * Glyph vertex structure
 */
typedef struct glyph_vertex 
{
    float x;		//Vertex x coordinates
    float y;		//Vertex y coordinates
    float z;		//Vertex z coordinates

    float u;		//Texture first coordinate
    float v;		//Texture second coordinate

    float r;		//Color red component
    float g;		//Color green component
    float b;		//Color blue component
    float a;		//Color alpha component

// 	glm::vec3 xyz;	//Vertex x y z coordinates
// 	glm::vec2 uv;
// 	glm::vec4 rgba;

    float shift;	//Shift along x

    float gamma;	//Color gamma correction
}glyph_vertex;


/**
 * Line structure
 */

typedef struct line_info 
{
    size_t line_start;		//Index (in the vertex buffer) where this line starts
    vec4 bounds;			//bounds of this line

}line_info;

}


#endif /* #define __TEXT_BUFFER_H__ */
