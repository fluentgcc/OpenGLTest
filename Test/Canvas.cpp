#include "Canvas.h"

#include <iostream>
#include <string>

#include <QtGui/QtEvents>
#include <SOIL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Triangle.h"

#include <ft2build.h>

#include FT_FREETYPE_H


#define GL_CHECK_ERRORS assert( glGetError()== GL_NO_ERROR );


//screen resolution
const int WIDTH  = 1280;
const int HEIGHT = 960;




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

void Canvas::initFBO()
{
// 	//产生FBO;
// 	glGenFramebuffers( 1, &this->g_fbo_id_ );
// 	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, this->g_fbo_id_ );
// 	//产生RBO;
// 	glGenRenderbuffers( 1, &this->g_rbo_id_ );
// 	glBindRenderbufferEXT( GL_RENDERBUFFER, this->g_rbo_id_ );
// 	//设置RBO存储;
// 	glRenderbufferStorage( GL_RENDERBUFFER, GL_DEPTH_COMPONENT32, WIDTH, HEIGHT );
// 
// 	//产生用于离屏渲染的texture;
// 	glGenTextures( 1, &this->g_tex_render_id_ );
// 	glBindTexture( GL_TEXTURE_2D, this->g_tex_render_id_ );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
// 	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );
// 	glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, WIDTH, HEIGHT, 0, GL_BGRA, GL_UNSIGNED_BYTE, NULL );
// 
// 	//！FBO并没有任何图像存储，但是有多个挂靠点，
// 	//：颜色color（可能有多个，为了渲染到多个目标），深度depth，模板stencil；
// 	//: FBO切换挂靠的图像速度很快，要比切换FBO快;
// 
// 	//将纹理绑定到FBO的颜色挂靠点 (color attachment);
// 	glFramebufferTexture2D( GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, this->g_tex_render_id_, 0 );
// 	
// 	//将RBO绑定到FBO的深度挂靠点 (depth attachment);
// 	glFramebufferRenderbuffer( GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, this->g_fbo_id_ );
// 
// 	//检查FBO状态;
// 	GLuint status = glCheckFramebufferStatus( GL_DRAW_FRAMEBUFFER );
// 	if ( GL_FRAMEBUFFER_COMPLETE == status )
// 	{
// 		std::cout << "Sucess: FBO init  \n";
// 	} 
// 	else
// 	{	
// 		std::cout << "Error: FBO init \n";
// 	}
// 
// 	glBindTexture( GL_TEXTURE_2D, 0 );
// 	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );

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
//-------------------------------------------------

	glEnable( GL_DEPTH_TEST );

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
//-------------------------------------------

	this->triangle_ = new CTriangle();

//-------------------------------------------
	FT_Library ft;
	if ( FT_Init_FreeType( &ft ) )
	{
		std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
	}

	FT_Face face;
	if ( FT_New_Face( ft, "C:/Windows/Fonts/arial.ttf", 0, &face ) )
	{
		std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl; 
	}

	 FT_Set_Pixel_Sizes( face, 0, 48);
	//---------------------------------------------

	 std::string s1 =  QString::fromLocal8Bit( "字体" ).toStdString();
	 std::string s2 = "aa";
	 std::wstring s3  = L"字体";
	 std::wstring s4  =  QString::fromLocal8Bit( "字体" ).toStdWString();
	 auto sz = s1.size();
	 auto sz2 = s2.size();
	 int a = s1[0];


	 int c = 0;
	 for ( auto it = s3.begin(); it != s3.end(); it++) 
	 {
		 ++c;
	 }

	 int z =32;

}

void Canvas::paintGL()
{
	glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

	//glm::mat4 P = this->camera_->getProjectionMatrix();
	//glm::mat4 V = this->camera_->getViewMatrix();

	glm::mat4 P = this->camera_->getProjectionMatrix();
	glm::mat4 V = this->camera_->getViewMatrix();

	this->triangle_->render( glm::value_ptr( P*V ) );
}

void Canvas::resizeGL( int w, int h )
{
	glViewport ( 0, 0, ( GLsizei ) w, ( GLsizei ) h );

	//投影-----视景体; 对称透视投影;
	//P = glm::perspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );				
	//	P = glm::ortho( -1, 1, -1, 1 ); //正交投影;

	//视图-----相机位置，视线中心，上方向;
	//MV = glm::lookAt( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, -100 ), glm::vec3( 0, 1, 0 ) );

	//this->camera_->setPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f  );
	//this->camera_->lookAt( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, -100 ), glm::vec3( 0, 1, 0 ) );
	this->camera_->setPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );
	this->camera_->lookAt( glm::vec3( 0, 0, 0 ), glm::vec3( 0, 0, -100 ), glm::vec3( 0, 1, 0 ) );

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
// 
// 	if( Qt::RightButton == event->buttons() )
// 	{
// 	}

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
