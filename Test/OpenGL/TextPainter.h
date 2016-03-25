#ifndef __FONT_PAINTER__H
#define __FONT_PAINTER__H

#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include <FTGL/ftgl.h>

struct TextBoundingBox 
{
	//表示文字边框;
	glm::dvec2 upper; //右上;
	glm::dvec2 lower; //左下;
};
//----------------------------------------------------------------
class FontManager
{
	//管理并初始化字体，不单独对外使用;
	//要绘制文字请使用TextPainter类;

public:
	static FontManager* Instance();
	
	//为了在程序开始初始化时检查状态;
	bool init();

	~FontManager();
	FTFont* getFTFont(){ return this->font_; }

	

private:
	FontManager();
	// 	FontManager( const FontManager& ){};
	// 	FontManager& operator = ( const FontManager& ){ return *this; };

private:
	FTFont* font_;
	
	int error_code_;


};
//----------------------------------------------------------------

class TextPainter
{
public:
	enum Alignment
	{

		Align_Left    = FTGL::ALIGN_LEFT,
		Align_Center  = FTGL::ALIGN_CENTER, 	
		Align_Right   = FTGL::ALIGN_RIGHT,	
		Align_Justify = FTGL::ALIGN_JUSTIFY
	};
public:
	TextPainter();
	~TextPainter(){};


	//！！！注意：如果一行中有"\n"的话，务必在"\n"之后加一个空格;
	//添加中文请使用wstring, ( L"" );
	void addTextLine( const std::string& s );
	void addTextLine( const std::wstring& ws );
	//直接全部修改文字内容;
	//同样要加空格;
	void setText( const std::string& s );
	void setText( const std::wstring& ws );

	std::wstring getWString(){ return this->wstring_; }
	std::string	 getString();

	inline void clearText(){ this->wstring_.clear(); }

	inline void setBeginPoint( const glm::dvec2&  pos ){ this->begin_point_ = pos; }
	void setTotalSize( const glm::dvec2& vec );
	void setAlignment( TextPainter::Alignment alignment );

	//只要调用过该函数，就不会自动调节行长度;
	void setLineLenth( const float len );

	void setFontSize( float size );		//大小范围（1 - 128 ）以60为宜;
	float getFontSize(){ return this->font_size_; };

	float Advance( const std::string& s );
	float Advance( const std::wstring& ws );

	//返回文字周围边框位置;
	TextBoundingBox getBoundingBox();

	// 行间距
	float getLineSpacing();
	void  setLineSpacing( float spacing );

	//字体的高度;
	float getLineHeight();

	float getDescender();
	float getAscender();
	
	void Render();

private:
	
	std::wstring string2wstring( const std::string &str );
	std::string  wstring2string( const std::wstring &ws );				//

private:
	FTSimpleLayout* ft_layout_;

	std::wstring  wstring_;

	glm::dvec2 begin_point_;			//起始绘制位置;

	glm::dvec2 total_size_;			//cell的大小，用于计算缩放因子;
	double base_size_;				//用于计算缩放因子;
	float font_size_;				//字体在cell中的大小设置; 不改变face_size; 只是改变了缩放因子;
	double scale_factor_;			//缩放因子

	float max_advance_;				//自动调节lineLenth时最大的LineLenth;

	bool fixed_line_lenth_;			//固定lineLenth; true时不自动调节行长度;

	unsigned int line_count_;

};


#endif
