#ifndef __VECTOR_T__H__
#define __VECTOR_T__H__
#include <stddef.h>
#include <stdlib.h>

namespace ftgl
{

/*	主要简单实现类似std::vector的一个容器; 以字节存放数据; 
用于vertex_buffer 中存放 vetices 数据;

* <b>Example Usage</b>:
* @code
* #include "vector.h"
* int main( int arrgc, char *argv[] )
* {
*   int i,j = 1;
*   vector_t * vector = new vector_t( sizeof(int) );
*   vector->push_back( &i );
*
*   j = * (int *) vector->at( 0 );
*   delete  vector;
*
*   return 0;
* }
* @endcode
*/
class vector_t
 {

 public:

	 vector_t( size_t item_size );
	 ~vector_t();

	 void clear();
	 int empty();
	 size_t size();
	 inline size_t item_size(){ return this->item_size_; }

	 const void* front();
	 const void* back();
	 const void* at( size_t index );

	 void set( const size_t index, const void* item );
	 void erase( const size_t index );
	 void erase_range( const size_t first, const size_t last );
	 void push_back( const void* item );
	 void pop_back();
	 void insert( const size_t index, const void* item );
	 void insert_data( const size_t index, const void* data, const size_t count );
	 void push_back_data( const void* data, const size_t count );

	 int contains( const void* item, int( *cmp )( const void*, const void* ) );
	 void sort( int (*cmp)(const void *, const void *) );

	 size_t capacity();
	 void reserve( const size_t size );
	 void resize( const size_t size );
	 void shrink();

 private:
     /** Pointer to dynamically allocated items. */
     void * items_;

     /** Number of items that can be held in currently allocated storage. */
     size_t capacity_;

     /** Number of items. */
     size_t size_;

     /** Size (in bytes) of a single item. */
     size_t item_size_;
};


}


#endif /* __VECTOR_H__ */
