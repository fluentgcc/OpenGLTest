#include "TextPainter.h"

#include <gl/glew.h>
#include <gl/gl.h>
#include <gl/glu.h>

#include <Windows.h>
#include <iostream>
#include <QtCore/QString>

//-------------------------------------------------------------------

FontManager* FontManager::Instance()
{
	{
		static FontManager tm;
		return& tm;
	}
}


bool FontManager::init()
{
	if ( !this->error_code_ )
	{
		return true;
	}
	
	delete this->font_;
	this->font_ = nullptr;
	return false;
}

FontManager::~FontManager()
{
	if ( nullptr != this->font_ )
	{
		delete this->font_;
		this->font_ = nullptr;
	}
}

FontManager::FontManager()
	:error_code_( 0 )
{
	this->font_ = new FTTextureFont( "fonts/msyh.ttf"  );

	if( this->font_->Error() )
	{	
		std::cout <<  " 字体创建失败 " << std::endl;
		
		this->error_code_ = 1;
		return;
	}

	font_->FaceSize( 32 );

	if( this->font_->Error() )
	{	
		std::cout << " 字体大小设置失败 " <<std::endl;

		this->error_code_ = 2;
		return;
	}

	this->error_code_ = 0;
}

//--------------------------------------------------------------

TextPainter::TextPainter()
	:begin_point_( glm::dvec2( 0, 0 ) ), max_advance_( 0 ), line_count_( 0 ), fixed_line_lenth_( false )

{
	this->ft_layout_ = new FTSimpleLayout();
	this->ft_layout_->SetFont( FontManager::Instance()->getFTFont() );
	this->ft_layout_->SetLineSpacing( 1.0 );
	this->ft_layout_->SetAlignment( FTGL::ALIGN_LEFT );

	this->base_size_ =  1024;

	this->scale_factor_ = 1.0;
}

void TextPainter::addTextLine(const std::string& s)
{
	this->addTextLine( this->string2wstring( s ) );
}

void TextPainter::addTextLine(const std::wstring& ws)
{
	if ( this->wstring_.empty()  )
	{
		this->wstring_ = ws + L" ";
		
	}
	else
	{
		this->wstring_  =  this->wstring_ + L"\n" + ws + L" ";
	}

	float advance = FontManager::Instance()->getFTFont()->Advance( ws.c_str() );
	
	if ( !this->fixed_line_lenth_ )
	{
		if ( this->max_advance_ < advance )
		{
			max_advance_ = advance;
		}

		ft_layout_->SetLineLength( max_advance_ );
	}
	
}

void TextPainter::setText(const std::string& s)
{
	this->setText( this->string2wstring( s ) );
}

void TextPainter::setText(const std::wstring& ws)
{
	this->wstring_ = ws;

	if (!this->fixed_line_lenth_ )
	{
		float adv = FontManager::Instance()->getFTFont()->Advance( ws.c_str()  );
		ft_layout_->SetLineLength( adv );
	}

}

std::string TextPainter::getString()
{
	return  this->wstring2string( this->wstring_); 
}

void TextPainter::setTotalSize( const glm::dvec2& vec )
{
	//假定512*512时facesize = 32 ;

	this->total_size_ = vec;

	double min;
	double delta = 0;
	

	if ( vec[0] < vec[1] )
	{
		min = vec[0];
		
	}
	else
	{
		min = vec[1];
		delta = vec[0] - vec[1];
	}
	
	this->scale_factor_ = min / base_size_;
}

void TextPainter::setAlignment(  Alignment alignment )
{
	this->ft_layout_->SetAlignment( ( FTGL::TextAlignment )alignment );
}

void TextPainter::setLineLenth(const float len)
{
	this->fixed_line_lenth_ = true;
	this->ft_layout_->SetLineLength( len * this->scale_factor_ );
}


void TextPainter::setFontSize( float size )
{
	//default size = 65
	if ( size < 1.0 || size > 128.0 )
	{
		return;
	}
	this->font_size_ = size;
	this->base_size_ = 2064.0 - size * 16.0;
}

float TextPainter::Advance( const std::string& s )
{
	return this->Advance( this->string2wstring(s) );
}

float TextPainter::Advance(const std::wstring& ws )
{
	float adv = FontManager::Instance()->getFTFont()->Advance( ws.c_str() );

	adv *= this->scale_factor_;

	return adv;
}

TextBoundingBox TextPainter::getBoundingBox()
{
	TextBoundingBox tmp;
 
	FTBBox box = ft_layout_->BBox( this->wstring_.c_str() );

	float asc = FontManager::Instance()->getFTFont()->Ascender();
	
	tmp.lower = glm::dvec2( box.Lower().X(), box.Lower().Y() );
	tmp.upper = glm::dvec2( box.Upper().X(), box.Upper().Y() );

	//缩放：
	tmp.lower *= this->scale_factor_;
	tmp.upper *= this->scale_factor_; 
	//平移:
	glm::dvec2 trans = begin_point_; 
	tmp.lower += trans;
	tmp.upper += trans;

	return tmp;

}

float TextPainter::getLineSpacing()
{
	float linesp = this->ft_layout_->GetLineSpacing();

	return linesp;
}

float TextPainter::getLineHeight()
{
	float lineHeight = FontManager::Instance()->getFTFont()->LineHeight();

	lineHeight *= this->scale_factor_;
	return lineHeight;
}

float TextPainter::getDescender()
{
	return this->ft_layout_->GetFont()->Descender() * this->scale_factor_;
}

float TextPainter::getAscender()
{
	return this->ft_layout_->GetFont()->Ascender() * this->scale_factor_;
}

void TextPainter::Render()
{
	glPushMatrix();

	glTranslated( this->begin_point_.x , ( this->begin_point_.y ), 0.0 );
	glScaled( this->scale_factor_, this->scale_factor_, 1.0 );
	
	glColor4d( 0.0, 0.0, 0.0, 1.0 );
	ft_layout_->Render( this->wstring_.c_str(), -1, FTPoint(), FTGL::RENDER_ALL );

	glTranslated( -1, -1, 0.0 );
	glColor4d( 1.0, 1.0, 1.0, 1.0 );
	ft_layout_->Render( this->wstring_.c_str(), -1, FTPoint(), FTGL::RENDER_ALL );

	glPopMatrix();

	glBindTexture( GL_TEXTURE_2D, 0 );
}


std::wstring TextPainter::string2wstring( const std::string &str )
{
	setlocale( LC_ALL, "chs" ); 

	const char* _Source = str.c_str();
	size_t _Dsize = str.size() + 1;
	wchar_t* _Dest = new wchar_t[_Dsize];
	wmemset( _Dest, 0, _Dsize );
	mbstowcs( _Dest,_Source,_Dsize );
	std::wstring result = _Dest;
	delete[] _Dest;

	setlocale( LC_ALL, "C" );

	return result;
}

std::string TextPainter::wstring2string(const std::wstring &wstr  )
{
// 	std::string str(wstr.length(), ' ');
// 	std::copy(wstr.begin(), wstr.end(), str.begin());
// 	return str;

	std::string str;

	int nLen = (int)wstr.length();    
	str.resize(nLen,' ');

	int nResult = WideCharToMultiByte( CP_ACP, 0, ( LPCWSTR )wstr.c_str(), nLen, ( LPSTR )str.c_str(), nLen, NULL, NULL );

	if (nResult == 0)
	{
		return "";
	}

	return str;
}



