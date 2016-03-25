#ifndef CANVAS_H
#define CANVAS_H
#include <gl/glew.h>
#include <gl/glu.h>
#include <gl/gl.h>

#include <QOpenGLWidget>
#include <QGLWidget>
#include <map>
#include <string>
#include <glm/glm.hpp>

#include "GLSLShader.h"
#include "GLCamera.h"


class CTriangle;

class Canvas : public QOpenGLWidget
{
	Q_OBJECT

public:
	Canvas(QWidget *parent = 0);
	~Canvas();

	void initFBO();

protected:
	virtual void initializeGL();
	virtual void paintGL();
	virtual void resizeGL( int w, int h );

	void mouseMoveEvent(	QMouseEvent* event );
	void mousePressEvent(	QMouseEvent* event );
	void mouseReleaseEvent( QMouseEvent* event );
	void wheelEvent( QWheelEvent * event );

private:

	int width_;
	int height_;

	GLuint g_fbo_id_;
	GLuint g_rbo_id_;

	GLuint g_tex_render_id_;	//offscreen render texture ID
// 
 	CTriangle* triangle_;
	Camera* camera_;
	
	QPoint pos0_;

};

#endif // CANVAS_H
