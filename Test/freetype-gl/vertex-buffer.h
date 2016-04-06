#ifndef __VERTEX_BUFFER_H__
#define __VERTEX_BUFFER_H__

#include "opengl.h"
#include "vertex-attribute.h"
#include <vector>

namespace ftgl 
{

class vertex_buffer
{
public:

	vertex_buffer( const char* format );	/**format a string describing vertex format.**/
	~vertex_buffer();

	size_t getSize();
	const char*  getFormat();
	void printInfo();

	void renderSetup(  GLenum mode );
	void renderFinish();
	void render( GLenum mode );
	void renderItem( size_t index  );

	void upload();
	void clear();

	void pushBackIndices( const GLuint * indices, const size_t icount );
	void pushBackVertices ( const void * vertices, const size_t vcount );
	
	void insertIndices ( const size_t index, const GLuint *indices, const size_t icount );
	void insertVertices ( const size_t index, const void *vertices, const size_t vcount );
	
	void eraseIndices ( const size_t first, const size_t last );
	void eraseVertices ( const size_t first, const size_t last );

	size_t pushBack( const void * vertices, const size_t vcount,
					const GLuint * indices, const size_t icount );

	size_t insert( const size_t index,
		const void * vertices, const size_t vcount,
		const GLuint * indices, const size_t icount );

	void erase( const size_t index );


private:

    char * format;					/** Format of the vertex buffer. */

#ifdef FREETYPE_GL_USE_VAO
    GLuint VAO_id;					/** GL identity of the Vertex Array Object */
#endif

    GLuint vertices_id;				/** GL identity of the vertices buffer. */

	vector_t * vertices;			/** Vector of vertices. */
    vector_t * indices;				/** Vector of indices. */

    
    GLuint indices_id;				/** GL identity of the indices buffer. */

    size_t GPU_vsize;				/** Current size of the vertices buffer in GPU */
    size_t GPU_isize;				/** Current size of the indices buffer in GPU*/

    GLenum mode;					/** GL primitives to render. */

    char state;						/** Whether the vertex buffer needs to be uploaded to GPU memory. */

    /** Individual items */
    vector_t * items;

    /** Array of attributes. */
    vertex_attribute *attributes[MAX_VERTEX_ATTRIBUTE];
};

}


#endif /* __VERTEX_BUFFER_H__ */
