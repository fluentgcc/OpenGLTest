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


struct vertex_t
{
	//���һ����
	glm::vec3 vertice;		//����λ��
	glm::vec2 texture;		//��������
};

struct character_t 
{
	//һ���ַ��ĸ���;
	vertex_t v0;
	vertex_t v1;
	vertex_t v2;
	vertex_t v3;

};

struct bounds_t
{
	float left;
	float top;
	float width;
	float height;

};


struct line_info_t 
{
    /**
     * Index (in the vertex buffer) where this line starts
     */
    size_t line_start;

    /**
     * bounds of this line
	 */
    bounds_t bounds;

};


class zy_text_layout
{
public:
	zy_text_layout(  GLSLShader* shader, texture_font* ft );
	~zy_text_layout();
	
	//�����������֮ǰ����;
	void setLineLength( float length ){ this->fixed_line_length_ = length; };

	//���һ������;
	void addText( glm::vec2* pen , const char* text, size_t length = 0 );
	void clear();
	
	//�������֮������;
	void setAlign(  Align align );
	Align getAlign(){ return this->align_; }

	void render();

	bounds_t getBounds(){ return this->bounds_; }



	
private:
	void addChar( glm::vec2* pen, const char* current, const char* previous );

	void finishLine( glm::vec2* pen, bool advancePen );
	void moveLastLine( float dy );

private:
	GLSLShader* shader_;
	ftgl::texture_font* font_;

	Align align_;

	//total bounds;
	bounds_t bounds_;

	glm::vec2 pen_orign_;		//Pen origin;
	float	  pen_last_y_;		//Last pen y location;
	size_t	  line_start_;		//Index of the current line start;
	float	  line_left_;		//Location of the start of the line;
	float	  line_ascender_;	//Current line ascender;
	float	  line_descender_;	//Current line decender;

// 	glm::vec4 base_color_;
// 	glm::vec4 color_;

	std::vector< character_t > buffer_;	//�൱��һ��vertex_buffer;

	std::vector< line_info_t > lines_;


	//---------���������г���;--------
	//С�ڵ���0 ʱ���ַ��Զ�����;
	float fixed_line_length_;
};

#endif