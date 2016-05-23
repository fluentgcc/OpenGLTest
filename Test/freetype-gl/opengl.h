#ifndef __OPEN_GL_H__
#define __OPEN_GL_H__

#if defined(_WIN32) || defined(_WIN64)
#  include <GL/glew.h>
#  include <GL/wglew.h>
#else
#  include <GL/glew.h>
#  include <GL/gl.h>
#endif

#endif /* OPEN_GL_H */
