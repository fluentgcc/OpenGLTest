#ifndef __ZY_FONT_MANAGER__H
#define __ZY_FONT_MANAGER__H

#include "freetype-gl.h"


// class font_info
// {
// 	const char* file_name;
// 	size_t font_size;
// 
// 	bool operator==( const font_info & lhs );
// };
// 
// class zy_font_manager 
// {
// public:
// 	static zy_font_manager* instance();
// 	~zy_font_manager();
// 
// 	//使用之前必须执行;参数为创建atles的大小;
// 	void init(  size_t width, size_t height, size_t depth );
// 
// 	void createFont( );
// 
// 	void getFont( size_t font_size );
// 			   
// private:	   
// 	zy_font_manager();
// 
// private:
//     /**
//      * Texture atlas to hold font glyphs.
//      */
// 	ftgl::texture_atlas * atlas;
// 
// 	std::vector< ftgl::texture_font* > fonts;
// 

#endif