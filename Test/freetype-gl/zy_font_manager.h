#ifndef __ZY_FONT_MANAGER__H
#define __ZY_FONT_MANAGER__H

#include "freetype-gl.h"


class font_info
{
	const char* file_name;
	size_t font_size;

	bool operator==( const font_info & lhs );
};
//单实例;
//目前只管理一种字体;
//todo: 可加多种字体，字体描述等等;

class zy_font_manager 
{
public:
	static zy_font_manager* instance();
	~zy_font_manager();

	//使用之前必须执行;参数为创建atles的大小;
	void init(  size_t width, size_t height, size_t depth = 1 );

	//void getFont( size_t font_size );

	ftgl::texture_font* getFont(){ return this->font_; }
			   
private:	   
	zy_font_manager();

private:

	ftgl::texture_atlas * atlas_;

	//std::vector< ftgl::texture_font* > fonts;

	ftgl::texture_font* font_; 

};

#endif