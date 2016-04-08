#include "Canvas.h"

#include <iostream>
#include <stdio.h>
#include <string>

#include <QtGui/QtEvents>
#include <QtCore/QString>
#include <SOIL.h>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Rectangle.h"

#include "freetype-gl.h"
#include "vertex-buffer.h"
#include "distance-field.h"

#define GL_CHECK_ERRORS assert( glGetError()== GL_NO_ERROR );

GLSLShader shader;
vertex_buffer* buffer;
texture_atlas *atlas;
texture_font* FONT;


glm::mat4 model, view, projection;

typedef struct {
	float x, y, z;    // position
	float s, t;       // texture
	float r, g, b, a; // color
} vertex_t;


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
			vertex_t vertices[4] = { 
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

	glClearColor( 0.2f, 0.3f, 0.3f, 1.0f );

// 	glEnable(GL_BLEND);
// 	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

//--------------------------------------------------------

	atlas = new texture_atlas( 512, 512, 1 );
	const char * filename = "C:/Windows/Fonts/msyh.ttf";
	char * text = "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~		";
	
	buffer = new vertex_buffer( "vertex:3f,tex_coord:2f,color:4f" );
	glm::vec2 pen( 5,400 );
	glm::vec4 black( 0, 0, 0, 1 );

	FONT = new texture_font( atlas, 32, filename );
	pen.x = 5;
	pen.y -= FONT->getHeight();
	FONT->loadGlyphs( text );
	add_text( buffer, FONT, text, &black, &pen );
	
	//delete font;

	unsigned char* map;

	map = ftgl::make_distance_mapb( atlas->getData(), atlas->getWidth(), atlas->getHeight()  );

	memcpy( atlas->getData(), map, atlas->getWidth()*atlas->getHeight()*sizeof( unsigned char ) );
	free(map);
	FONT->getAtlas()->upload();

	GL_CHECK_ERRORS

	unsigned int a = atlas->getTexID();
	glBindTexture( GL_TEXTURE_2D, atlas->getTexID() );

	shader.LoadFromFile( GL_VERTEX_SHADER, "E:/work/Qt5/Test_14_ogl_basic/Test/freetype-gl/shaders/v3f-t2f-c4f.vert" );
	shader.LoadFromFile( GL_FRAGMENT_SHADER,"E:/work/Qt5/Test_14_ogl_basic/Test/freetype-gl/shaders/v3f-t2f-c4f.frag" );
	shader.CreateAndLinkProgram();

	GL_CHECK_ERRORS

	projection = glm::mat4( 1.0 );
	model	   = glm::mat4( 1.0 );
	view	   = glm::mat4( 1.0 );

	shader.Use();
	shader.AddUniform( "texture" );
	shader.AddUniform( "model" );
	shader.AddUniform( "view" );
	shader.AddUniform( "projection" );
	shader.UnUse();

	GL_CHECK_ERRORS


		//	char* text = "王王";
		// 	std::vector< char > vec;
		// 	vec.push_back( text[0] );
		// 	vec.push_back( text[1] );
		// 	vec.push_back( text[2] );
		// 	vec.push_back( text[3] );
		// 	vec.push_back( text[4] );
		// 	vec.push_back( text[5] );
		// 	vec.push_back( text[6] );
}

void Canvas::paintGL()
{
	GL_CHECK_ERRORS

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
//----------------------------------------------------------
	glBindTexture( GL_TEXTURE_2D, atlas->getTexID() );
	glColor3f( 1.0, 1.0, 0.0 );

// 	std::string s( "!\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijkl" ); 
// 
// 	glm::vec2 pen( 1, 50 );
// 
// 	for ( int i = 0; i < 100; ++i )
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
// 		pen.y += 10;
// 		pen.x = 1;
// 	}

	auto glyph = FONT->getGlyph( "Q" );
	float s0 = glyph->s0;
	float t0 = glyph->t0;
	float s1 = glyph->s1;
	float t1 = glyph->t1;

	glm::vec2 pen2( 50.0, 50.0 );
	int x0  = (int)( pen2.x + glyph->offset_x );
	int y0  = (int)( pen2.y + glyph->offset_y );
	int x1  = (int)( x0 + glyph->width );
	int y1  = (int)( y0 - glyph->height );

	glBegin( GL_POLYGON );
	{
		glTexCoord2f(s0,t0 );glVertex3f( x0, y0, 0 );
		glTexCoord2f(s0,t1 );glVertex3f( x0, y1, 0 );
		glTexCoord2f(s1,t1 );glVertex3f( x1, y1, 0 );
		glTexCoord2f(s1,t0 );glVertex3f( x1, y0, 0 );

	}
	glEnd();
	GL_CHECK_ERRORS
//----------------------------------------------------------

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, atlas->getTexID() );
	shader.Use();
	{
		glUniform1i( shader( "texture" ), 0 );
		glUniformMatrix4fv( shader( "model" ), 1, 0, glm::value_ptr( model ) );
		glUniformMatrix4fv( shader( "view" ),  1, 0, glm::value_ptr( view ) );
		glUniformMatrix4fv( shader( "projection" ), 1, 0, glm::value_ptr( projection ) ) ;
		buffer->render(  GL_TRIANGLES );
	}
	shader.UnUse();
	auto e =  glGetError();

	GL_CHECK_ERRORS
}

void Canvas::resizeGL( int w, int h )
{

	this->width_ = w;
	this->height_ = h;

	glViewport ( 0, 0, ( GLsizei ) w, ( GLsizei ) h );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	//gluPerspective( 45.0f, ( GLfloat )w / h, 0.1f, 1000.f );

	glOrtho( 0, w, 0, h, -1, 1 );

	glMatrixMode( GL_MODELVIEW );
	glLoadIdentity();

	projection = glm::ortho( 0, w, 0, h, -1, 1 );
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
