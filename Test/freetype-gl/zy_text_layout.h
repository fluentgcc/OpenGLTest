#ifndef __ZY_TEXT_LAYOUT__H
#define __ZY_TEXT_LAYOUT__H

#include <freetype-gl.h>
#include <glm/glm.hpp>
#include <GLSLShader.h>

enum Align
{	
	ALIGN_LEFT,
	ALIGN_CENTER,
	ALIGN_RIGHT
};


struct vertex
{
	//最简单一个点
	glm::vec3 vertice;		//顶点位置
	glm::vec2 texture;		//纹理坐标
};

struct character 
{
	//一个字符四个点;
	vertex v0;
	vertex v1;
	vertex v2;
	vertex v3;

};

struct line_info 
{
    /**
     * Index (in the vertex buffer) where this line starts
     */
    size_t line_start;

    /**
     * bounds of this line
     * 1-left
	 * 2-top
	 * 3-width
	 * 4-height
	 */
    glm::vec4 bounds;

};


class zy_text_layout
{
public:
	zy_text_layout(  GLSLShader* shader, texture_font* ft );
	~zy_text_layout();

	void render();

	//添加一段文字;
	void addText( glm::vec2* pen , const char* text, size_t length = 0 );
	void clear();

	void setAlign();
	Align getAlign(){ return this->align_; }

	glm::vec4 getBounds();
	
private:
	void addChar( glm::vec2* pen, const char* current, const char* previous );

	void finishLine( glm::vec2* pen, bool advancePen );
	void moveLastLine( float dy );

private:
	GLSLShader* shader_;
	ftgl::texture_font* font_;

	Align align_;

	glm::vec4 bounds_;

	glm::vec2 pen_orign_;		//Pen origin;
	float	  pen_last_y_;		//Last pen y location;
	size_t	  line_start_;		//Index of the current line start;
	float	  line_left_;		//Location of the start of the line;
	float	  line_ascender_;	//Current line ascender;
	float	  line_descender_;	//Current line decender;

// 	glm::vec4 base_color_;
// 	glm::vec4 color_;

	std::vector< character > buffer_;	//相当于一个vertex_buffer;

	std::vector< line_info > lines_;


};

#endif