#include "Canvas.h"

#include <iostream>
#include <stdio.h>
#include <string>
#include <Windows.h>

#include <QtGui/QtEvents>
#include <QtCore/QString>
#include <SOIL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Rectangle.h"

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "distance-field.h"
#include "utf8-utils.h"


#include "zyFontManager.h"
#include "zyTextLayout.h"

#define GL_CHECK_ERRORS assert( glGetError()== GL_NO_ERROR );

GLSLShader shader;
vertex_buffer* buffer;
texture_atlas *atlas;
texture_font* FONT;

zyTextLayout* text_layout;


glm::mat4 model, view, projection;

typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_tt;


void add_text( vertex_buffer* buffer, texture_font* font,
	char * text, glm::vec4 * color, glm::vec2 * pen )
{
	size_t i;
	float r = color->r, g = color->g, b = color->b, a = color->a;
	for( i = 0; i < strlen(text); ++i )
	{
		texture_glyph *glyph = font->getGlyph( text + i );
		if( glyph != NULL )
		{
			float kerning =  0.0f;
			if( i > 0)
			{
				kerning = glyph->getKerning( text + i - 1 );
			}
			pen->x += kerning;
			int x0  = (int)( pen->x + glyph->offset_x );
			int y0  = (int)( pen->y + glyph->offset_y );
			int x1  = (int)( x0 + glyph->width );
			int y1  = (int)( y0 - glyph->height );
			float s0 = glyph->s0;
			float t0 = glyph->t0;
			float s1 = glyph->s1;
			float t1 = glyph->t1;
			GLuint indices[6] = {0,1,2, 0,2,3};
			vertex_tt vertices[4] = { 
			{ x0,y0,0,  s0,t0,  r,g,b,a },
			{ x0,y1,0,  s0,t1,  r,g,b,a },
			{ x1,y1,0,  s1,t1,  r,g,b,a },
			{ x1,y0,0,  s1,t0,  r,g,b,a } };
			buffer->pushBack( vertices, 4, indices, 6 );
			pen->x += glyph->advance_x;
		}
	}
}


Canvas::Canvas(QWidget *parent)
	: QOpenGLWidget(parent)
{
// 	std::vector< unsigned int > ints;
// 	std::wstring s = L"王";
// 	for ( auto it = s.begin(); it != s.end(); ++ it )
// 	{
// 		ints.push_back( *it );
// 
// 	}
// 
// 	const char* ss = "王";
// 
// 	WCHAR * str1;  
// 	int n = MultiByteToWideChar( CP_ACP, 0,ss, -1, NULL, 0 );
// 	str1 = new WCHAR[n];
// 	MultiByteToWideChar( CP_ACP, 0, ss, -1, str1, n );
// 
// 	n = WideCharToMultiByte( CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);  
// 	char * str2 = new char[n];
// 	WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL); 
// 
// 	for ( int i = 0; i < n; ++i )
// 	{
// 		unsigned char a = str2[i];
// 		ints.push_back( a );
// 	}
// 
// 	int bbb= 211;
}

Canvas::~Canvas()
{
}


void Canvas::initializeGL()
{
	//初始化glew;-------------------------------------
	glewExperimental = GL_TRUE;

	GLenum err = glewInit();

	if (GLEW_OK != err)	
	{
		std::cerr<<"Error: "<<glewGetErrorString(err)<<endl;
	} 
	else 
	{
		if ( GLEW_VERSION_3_1 )
		{
			std::cout<<"Driver supports OpenGL 3.1\nDetails:"<<endl;
		}
	}

	err = glGetError();
	GL_CHECK_ERRORS

	std::cout << "\tUsing GLEW "	<< glewGetString( GLEW_VERSION )<<endl;
	std::cout << "\tVendor: "		<< glGetString (  GL_VENDOR )<<endl;
	std::cout << "\tRenderer: "		<< glGetString (  GL_RENDERER )<<endl;
	std::cout << "\tVersion: "		<< glGetString (  GL_VERSION )<<endl;
	std::cout << "\tGLSL: "			<< glGetString (  GL_SHADING_LANGUAGE_VERSION )<<endl;
	GL_CHECK_ERRORS

	//glClearColor( 0.30f, 0.30f, 0.20f, 1.0f );
	glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );

// 	glEnable(GL_BLEND);
// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//--------------------------------------------------------

// 	atlas = new texture_atlas( 512, 512, 1 );
// 	const char * filename = "C:/Windows/Fonts/msyh.ttf";
// 	char * text = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~	王送萨芬合家欢疯狂青蛙净额汇入";
// 	const char* text2 = GBK_to_utf8( text );
// 
// 	FONT = new texture_font( atlas, 32, filename );
// 	FONT->loadGlyphs( text2 );
// 	
// 	const char* str = "王";
// 	const char* str2 = GBK_to_utf8( str );
// 
// 	FONT->loadGlyphs( str2 );
// 
// 	atlas->upload();
//-----------------------------------------------------------------------------------

// 	
// 	QString str = QString::fromLocal8Bit( "ABCDEFGHIJKLMNOPQRSTUVWXYZ" );
// 
// 	FONT->loadGlyphs( str.toStdString().c_str() ); 
// 
// 	//delete font;
// 
// 	unsigned char* map;
// 
// 	map = ftgl::make_distance_mapb( atlas->getData(), atlas->getWidth(), atlas->getHeight()  );
// 
// 	memcpy( atlas->getData(), map, atlas->getWidth()*atlas->getHeight()*sizeof( unsigned char ) );
// 	free(map);
// 	FONT->getAtlas()->upload();
//-----------------------------------------------------------------------------------
	zyFontManager::instance()->init( 2048, 2048, 1 );

	FONT = zyFontManager::instance()->getFont();

	std::string vert_file = "./freetype-gl/shaders/my-sdf.vert";
	std::string frag_file = "./freetype-gl/shaders/my-sdf.frag";
	
	shader.LoadFromFile( GL_VERTEX_SHADER, vert_file );
	shader.LoadFromFile( GL_FRAGMENT_SHADER, frag_file );
	shader.CreateAndLinkProgram();
// 	shader.Use();
// 	{
// 		shader.AddUniform( "MVP" );
// 		shader.AddUniform( "Color" );
// 	}
// 	shader.UnUse();


	text_layout = new zyTextLayout( FONT );
	text_layout->setLineLength( 1000 );

	//glm::vec2 pos( 2,200 );

	//text_layout->setBeginPos( pos );
	text_layout->addText(
		 "Adsfdsfefe\n"
		"bdfewafefaaefewq89890\n"
		"sdfawefwfwqefqfaC\n"
		"王送使得房价owe放假哈撒\n"
		"撒反对撒地方去维护荣辱观\n"
		"ada\n"
		"af\n"
		
		) ;
	//text_layout->setAlign( ALIGN_CENTER );

	//text_layout->setAlign( Align::ALIGN_RIGHT );
// 	glm::vec2 pos2( 20,250 );
// 	text_layout->addText( &pos2,	"Adsfdsfefe\n"
// 		"bdfewafefaaefewq89890\n"
// 		"sdfawefwfwqefqfaC\n"
// 		"王送使得房价owe放假哈撒\n"
// 		"撒反对撒地方去维护荣辱观\n"
// 		);
//-----------------------------------------------------------
	text_layout->moveTo( glm::vec2( 2, 500 ) );
// 	text_layout->addText( "中sfdsa1\n哇ggew\n");
// 	text_layout->clear();
 	char* text2 = "会议指出，权力就是责任，责任就要担当，忠诚干净担当是党对领导干部提出的政治要求。"
 				"我们党95年奋斗取得的伟大成就，充分展现了共产党人的担当精神。"
 				"实现“两个一百年”奋斗目标、实现中华民族伟大复兴的中国梦，"
 				"关键是各级党组织尤其是党员领导干部要担当责任，"
 				"做到在党忧党，为党尽职、为民尽责。"
 				"只要各级领导干部心系使命、扛起责任，就没有过不去的坎。"
 				"要唤醒责任意识，激发担当精神，永葆党的凝聚力和战斗力。";
	text_layout->addText( text2 );

	text_layout->addText( 
		"根据Effective C++第三版第一条款的描述，C++由以下四个“子语言”组成："
		"1、C子语言。C++支持C语言的几乎全部功能，主要是c89的部分"
		"在语法上与C语言仅有极微妙的差别(如括号表达式的左右值性，具体请参考C++标准文献)。"
		"这部分功能对应于传统的面向" 

		);

	text_layout->setAlign( ALIGN_RIGHT );
	text_layout->setAlign( ALIGN_CENTER );
	
	//text_layout->setAlign( ALIGN_LEFT );

	zyFontManager::instance()->getFont()->getAtlas()->upload();
	

	
	GL_CHECK_ERRORS
}

void Canvas::paintGL()
{
	GL_CHECK_ERRORS

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

// 	glScalef( 2.0, 2.0, 2.0 );
// 	glTranslatef(0, - 250, 0 );
	glEnable( GL_TEXTURE_2D );
 	glEnable( GL_BLEND );
 	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//----------------------------------------------------------
	glBindTexture( GL_TEXTURE_2D, FONT->getAtlas()->getTexID() );;

// 	std::string s( "!\*+,-.9:;<BCRSTabcdefghijkl" ); 
// 
// 	glm::vec2 pen( 1, 50 );
// 
// 	for ( int i = 0; i < 10; ++i )
// 	{
// 		for ( auto it = s.begin(); it != s.end(); ++it )
// 		{
// 			char cha = *it;
// 
// 			auto glyph = FONT->getGlyph( &cha );
// 
// 			float kerning =  0.0f;
// 			if( it !=  s.begin() )
// 			{
// 				kerning = glyph->getKerning( &cha );
// 			}
// 			pen.x += kerning;
// 
// 
// 			float s0 = glyph->s0;
// 			float t0 = glyph->t0;
// 			float s1 = glyph->s1;
// 			float t1 = glyph->t1;
// 
// 			int x0  = (int)( pen.x + glyph->offset_x );
// 			int y0  = (int)( pen.y + glyph->offset_y );
// 			int x1  = (int)( x0 + glyph->width );
// 			int y1  = (int)( y0 - glyph->height );
// 
// 			glBegin( GL_POLYGON );
// 			{
// 				glTexCoord2f(s0,t0 );glVertex3f( x0, y0, 0 );
// 				glTexCoord2f(s0,t1 );glVertex3f( x0, y1, 0 );
// 				glTexCoord2f(s1,t1 );glVertex3f( x1, y1, 0 );
// 				glTexCoord2f(s1,t0 );glVertex3f( x1, y0, 0 );
// 
// 			}
// 
// 			pen.x += glyph->advance_x;
// 			glEnd();
// 		}
// 
// 		pen.y += 30;
// 		pen.x = 1;
// 	}

//--------------------------------------
// 	glMatrixMode( GL_MODELVIEW );
// 	glLoadIdentity();
// 	glPushMatrix();
// 	glLoadMatrixd( glm::value_ptr( this->model_view_ ) );
	shader.Use();
	text_layout->render();
	shader.UnUse();
	glBindTexture( GL_TEXTURE_2D, 0 );

	auto a = text_layout->getBounds();
	glColor4f( 1.0, 1.0, 1.0, 1.0 );
	glBegin( GL_LINE_LOOP );
	{
		glVertex3f( a.left,				a.top,				0 );
		glVertex3f( a.left + a.width,	a.top,				0 );
		glVertex3f( a.left + a.width,	a.top - a.height,	0 );
		glVertex3f( a.left,				a.top - a.height,	0 );
	}
	glEnd();
	
// 	glScalef( 0.5, 0.5, 0 );
// 	glTranslatef( 100, 50, 0 );
// 	text_layout->render();
//--------------------------------------
// 	const char* str = "王";
// 	const char* str2 = GBK_to_utf8( str );
// 
// 
// 	auto glyph = FONT->getGlyph( str2 );
// 	float s0 = glyph->s0;
// 	float t0 = glyph->t0;
// 	float s1 = glyph->s1;
// 	float t1 = glyph->t1;
// 
// 	glm::vec2 pen2( 50.0, 50.0 );
// 	int x0  = (int)( pen2.x + glyph->offset_x );
// 	int y0  = (int)( pen2.y + glyph->offset_y );
// 	int x1  = (int)( x0 + glyph->width );
// 	int y1  = (int)( y0 - glyph->height );
// 
// 	glBegin( GL_POLYGON );
// 	{
// 		glTexCoord2f(s0,t0 );glVertex3f( x0, y0, 0 );
// 		glTexCoord2f(s0,t1 );glVertex3f( x0, y1, 0 );
// 		glTexCoord2f(s1,t1 );glVertex3f( x1, y1, 0 );
// 		glTexCoord2f(s1,t0 );glVertex3f( x1, y0, 0 );
// 
// 	}
// 	glEnd();
// 	GL_CHECK_ERRORS
//----------------------------------------------------------


	GL_CHECK_ERRORS
}

void Canvas::resizeGL( int w, int h )
{

	this->width_ = w;
	this->height_ = h;

	//glViewport ( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
	glViewport ( 0, 0, ( GLsizei ) w /2, ( GLsizei ) h /2 );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//gluPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );

	glOrtho( 0, w, 0, h, -1, 1 );
	projection = glm::ortho( 0, w, 0, h, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

}

void Canvas::mouseMoveEvent( QMouseEvent* event )
{

	if ( Qt::LeftButton != event->buttons() )
	{
		return;
	}

	QPoint p = event->pos();

	float delta_x = ( p.x() - this->pos0_.x() ) * 0.1;
	float delta_y = ( p.y() - this->pos0_.y() ) * 0.1;

	this->pos0_ = p;

	this->update();
}

void Canvas::mousePressEvent( QMouseEvent* event )
{
	this->pos0_ = event->pos();
}

void Canvas::mouseReleaseEvent(QMouseEvent* event)
{

}

void Canvas::wheelEvent(QWheelEvent * event)
{
	QPoint numPixels = event->pixelDelta();
	QPoint numDegrees = event->angleDelta() / 8;

	int a = event->delta();
	float b = 1 + a / 1000.0;

	this->update();

	event->accept();


}
