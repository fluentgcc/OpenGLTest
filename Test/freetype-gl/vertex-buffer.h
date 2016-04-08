#ifndef __VERTEX_BUFFER_H__
#define __VERTEX_BUFFER_H__

#include "opengl.h"
#include "vertex-attribute.h"
#include <vector_t.h>
#include <vector>

namespace ftgl 
{

class vertex_buffer
{
public:

	/**@param format a string describing vertex format.**/
	vertex_buffer( const char* format );
	//删除需释放GPU资源;
	~vertex_buffer();

	/**
	*  Returns the number of items in the vertex buffer
	*  @return       number of items
	*/
	size_t getSize();

	/**
	*  Returns vertex format
	*  @return       vertex format
	*/
	const char*  getFormat();

	void print();								//Print information about a vertex buffer

	void render( GLenum mode );					//Render vertex buffer. @mode: render mode
	void renderItem( size_t index  );			//Render 特定Item. @index: index of the item to be rendered
	void renderSetup(  GLenum mode );			//Prepare for render;
	void renderFinish();						//Finish reder;

	void upload();								// Upload buffer to GPU memory.
	void clear();								// Clear all items.

	//-----对verteces和indices的操作-------;
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
	//---------------------------------------;

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

    vector_t * items;				 /** Individual items */
  
    vertex_attribute *attributes[MAX_VERTEX_ATTRIBUTE];		  /** Array of attributes. */
};

}


#endif /* __VERTEX_BUFFER_H__ */
