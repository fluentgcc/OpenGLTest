#include "Canvas.h"

#include <iostream>
#include <string>

#include <QtGui/QtEvents>
#include <SOIL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Rectangle.h"

#include <ft2build.h>
#include FT_FREETYPE_H
#include <FTGL/ftgl.h>

#include "FZFont.h"

#define GL_CHECK_ERRORS assert( glGetError()== GL_NO_ERROR );


//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;


FZFont* fzfont;

FTFont* FONT;
FTSimpleLayout* LAYOUT;


Canvas::Canvas(QWidget *parent)
	: QOpenGLWidget(parent)
{
	this->camera_ = new Camera();
}

Canvas::~Canvas()
{
	glDeleteTextures( 1, &this->g_tex_render_id_ );
	glDeleteRenderbuffers( 1, &this->g_rbo_id_ );
	glDeleteFramebuffers( 1, &this->g_fbo_id_ );

	delete this->camera_;
	this->camera_ = nullptr;
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

	glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );

	//this->rectangle_ = new CRectangle();

//--------------------------------------------
	fzfont = new FZFont( "C:/Windows/Fonts/msyh.ttf" );
//--------------------------------------------

// 	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

}

void Canvas::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

	glm::mat4 P = this->camera_->getProjectionMatrix();
	glm::mat4 V = this->camera_->getViewMatrix();

	//this->rectangle_->render( glm::value_ptr( P*V ) );
// 	for ( unsigned int i = 0; i < 100; ++ i ) 
// 	{
// 
// 		float de = i /100;
// 		fzfont->renderText( std::wstring( L"abcd.?...{}[+-gthzsaehjkrhekrqjekwjkvkewqhrjhlalkdjfhkq" ),glm::vec3( -1.0 , 0.5 -de , -1.0 ), P*V, 1.0, glm::vec3( 1.0, 1.0, 0.0 ) );
// 
// 	}

	fzfont->renderText( std::wstring( L"abcd.?...{}[+-gthz" ),glm::vec3( -1.0, 0.5, -1.0 ), P*V, 1.0, glm::vec3( 1.0, 1.0, 0.0 ) );
	fzfont->renderText( std::wstring( L"aadfdfdfdf" ),glm::vec3( -1.0, 0.4, -1.0 ), P*V, 1.0, glm::vec3( 1.0, 1.0, 0.0 ) );

//	Character c = fzfont->getCharactor( 71 );
//
// 	glEnable( GL_TEXTURE_2D );
// 	glColor3f( 1.0 , 1.0, 1.0 ); 
// 	glBindTexture( GL_TEXTURE_2D, c.TextureID );
// 	glBegin( GL_QUADS );
// 	{
// 		glTexCoord2d( 0, 0 );glVertex3f( -0.5f, -0.5f, -1.0 );
// 		glTexCoord2d( 0, 1 );glVertex3f(  0.5f, -0.5f, -1.0 );
// 		glTexCoord2d( 1, 1 );glVertex3f(  0.5f,  0.5f, -1.0 );
// 		glTexCoord2d( 1, 0 );glVertex3f( -0.5f,  0.5f, -1.0 );
// 	}
// 	glEnd();

}

void Canvas::resizeGL( int w, int h )
{

	this->width_ = w;
	this->height_ = h;

	glViewport ( 0, 0, ( GLsizei ) w, ( GLsizei ) h );
	this->camera_->setPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );
	this->camera_->lookAt( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, -100 ), glm::vec3( 0, 1, 0 ) );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	gluPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );

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

	//this->camera_->translate( glm::vec3( delta_x, - delta_y, 0.0 ) );

	//this->camera_->rotate( -delta_x, -delta_y, 0 );
	this->camera_->rotate( delta_x, -delta_y, 0 );

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

	//this->camera_->zoom( b );

	//this->camera_->zoom ( a  /1000.0f);
	this->camera_->move(0, 0, a/1000.0 );

	this->update();

	event->accept();


}
