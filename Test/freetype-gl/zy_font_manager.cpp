#include "zy_font_manager.h"

zy_font_manager* zy_font_manager::instance()
{
	static zy_font_manager tm;
	return& tm;
}

zy_font_manager::~zy_font_manager()
{
	delete this->font_;
	delete this->atlas_;
	
}

void zy_font_manager::init(size_t width, size_t height, size_t depth )
{
	this->atlas_ = new ftgl::texture_atlas( width, height, depth );

	const char * filename = "C:/Windows/Fonts/msyh.ttf";
	this->font_ = new ftgl::texture_font( this->atlas_, 32, filename );
// 	font_->setOutlineType( 1 );
// 	font_->setOutlineThickness( 0.5 );

	char * text = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";

	this->font_->loadGlyphs( text );
	this->atlas_->upload();

}

zy_font_manager::zy_font_manager()
{
}

