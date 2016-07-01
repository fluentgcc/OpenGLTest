#include "zyFontManager.h"

zyFontManager* zyFontManager::instance()
{
	static zyFontManager tm;
	return& tm;
}

zyFontManager::~zyFontManager()
{
	delete this->font_;
	delete this->atlas_;
	
}

void zyFontManager::init(size_t width, size_t height, size_t depth )
{

	const char * filename = "C:/Windows/Fonts/msyh.ttf";
	this->atlas_ = new ftgl::texture_atlas( width, height, depth );
	this->font_ = new ftgl::texture_font( this->atlas_, 32, filename );
	this->atlas_->enableDistanceField( true );

// 	font_->setOutlineType( 1 );
// 	font_->setOutlineThickness( 0.5 );

	char * text = "A!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~ ";

	this->font_->loadGlyphs( text );
	this->atlas_->upload();

}

zyFontManager::zyFontManager()
{
}

