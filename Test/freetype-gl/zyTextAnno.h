#ifndef __ZY_TEXT_ANNO__H
#define __ZY_TEXT_ANNO__H
#include "zyTextLayout.h"

#include <glm/glm.hpp>
#include <GLSLShader.h>

class zyTextAnno
{
public:

	enum AnnoPos
	{
		TopLeft,
		TopRight,
	
		BottomLeft,
		BottomRight,

		totalNum
	};

	zyTextAnno( );
	~zyTextAnno();

	void setViewport( int x, int y , int w, int h );
	void addText( AnnoPos pos, const char* text );

	void render();
	
private:
	zyTextLayout* layouts[totalNum];

	glm::ivec4 viewport_;

};




#endif
