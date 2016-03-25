#ifndef __FONT_PAINTER__H
#define __FONT_PAINTER__H

#include <string>
#include <vector>

#include <glm/vec2.hpp>

#include <FTGL/ftgl.h>

struct TextBoundingBox 
{
	//��ʾ���ֱ߿�;
	glm::dvec2 upper; //����;
	glm::dvec2 lower; //����;
};
//----------------------------------------------------------------
class FontManager
{
	//������ʼ�����壬����������ʹ��;
	//Ҫ����������ʹ��TextPainter��;

public:
	static FontManager* Instance();
	
	//Ϊ���ڳ���ʼ��ʼ��ʱ���״̬;
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


	//������ע�⣺���һ������"\n"�Ļ��������"\n"֮���һ���ո�;
	//���������ʹ��wstring, ( L"" );
	void addTextLine( const std::string& s );
	void addTextLine( const std::wstring& ws );
	//ֱ��ȫ���޸���������;
	//ͬ��Ҫ�ӿո�;
	void setText( const std::string& s );
	void setText( const std::wstring& ws );

	std::wstring getWString(){ return this->wstring_; }
	std::string	 getString();

	inline void clearText(){ this->wstring_.clear(); }

	inline void setBeginPoint( const glm::dvec2&  pos ){ this->begin_point_ = pos; }
	void setTotalSize( const glm::dvec2& vec );
	void setAlignment( TextPainter::Alignment alignment );

	//ֻҪ���ù��ú������Ͳ����Զ������г���;
	void setLineLenth( const float len );

	void setFontSize( float size );		//��С��Χ��1 - 128 ����60Ϊ��;
	float getFontSize(){ return this->font_size_; };

	float Advance( const std::string& s );
	float Advance( const std::wstring& ws );

	//����������Χ�߿�λ��;
	TextBoundingBox getBoundingBox();

	// �м��
	float getLineSpacing();
	void  setLineSpacing( float spacing );

	//����ĸ߶�;
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

	glm::dvec2 begin_point_;			//��ʼ����λ��;

	glm::dvec2 total_size_;			//cell�Ĵ�С�����ڼ�����������;
	double base_size_;				//���ڼ�����������;
	float font_size_;				//������cell�еĴ�С����; ���ı�face_size; ֻ�Ǹı�����������;
	double scale_factor_;			//��������

	float max_advance_;				//�Զ�����lineLenthʱ����LineLenth;

	bool fixed_line_lenth_;			//�̶�lineLenth; trueʱ���Զ������г���;

	unsigned int line_count_;

};


#endif
