#include "zyTextAnno.h"
#include "zyFontManager.h"

zyTextAnno::zyTextAnno( )
{

	ftgl::texture_font* ft = zyFontManager::instance()->getFont();

	layouts[TopLeft]	 = new zyTextLayout( ft );
	layouts[TopRight]	 = new zyTextLayout( ft );
	layouts[BottomLeft]	 = new zyTextLayout( ft );
	layouts[BottomRight] = new zyTextLayout( ft );
}

zyTextAnno::~zyTextAnno()
{
	delete this->layouts[TopLeft];
	delete this->layouts[TopRight];
	delete this->layouts[BottomLeft];
	delete this->layouts[BottomRight];
}

void zyTextAnno::addText( AnnoPos pos, const char* text)
{
	switch ( pos )
	{
	case TopLeft:
		
		this->layouts[TopLeft]->addText(  text );
		this->layouts[TopLeft]->setAlign( ALIGN_LEFT );
		//bounds_t bound = layouts[topLeft]->getBounds();
		
		//this->layouts[topLeft]->moveTo( )

		break;

	case TopRight:

		this->layouts[TopRight]->addText(  text );
		this->layouts[TopRight]->setAlign( ALIGN_RIGHT );
		
		break;

	case BottomLeft:
		
		this->layouts[BottomLeft]->addText(  text );
		this->layouts[BottomLeft]->setAlign( ALIGN_LEFT );

		break;

	case BottomRight:
		
		this->layouts[BottomRight]->addText(  text );
		this->layouts[BottomRight]->setAlign( ALIGN_RIGHT );
		break;

	default:
		break;
	}
}

void zyTextAnno::render()
{
	layouts[TopLeft]->render();
	layouts[TopRight]->render();
	layouts[BottomRight]->render();
	layouts[BottomLeft]->render();
}

void zyTextAnno::setViewport(int x, int y , int w, int h)
{
	this->viewport_ = glm::ivec4( x, y, w, h );

}
