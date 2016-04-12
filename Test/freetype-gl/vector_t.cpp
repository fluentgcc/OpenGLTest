#include "vector_t.h"

#include <assert.h>
#include <string.h>
#include <stdio.h>

ftgl::vector_t::vector_t(size_t item_size)
{
	this->item_size_ = item_size;
	this->size_      = 0;
	this->capacity_  = 1;
	this->items_     = malloc( this->item_size_ * this->capacity_ );
}

ftgl::vector_t::~vector_t()
{
	free( this->items_ );
}

void ftgl::vector_t::clear()
{
	this->size_ = 0;
}

int ftgl::vector_t::empty()
{
	return  0 == this->size_;
}

size_t ftgl::vector_t::size()
{
	return this->size_;
}

const void* ftgl::vector_t::front()
{
	assert( this->size_ );

	return this->at( 0 );
}

const void* ftgl::vector_t::back()
{
	assert( this->size_ );
	return this->at( this->size_ - 1 );
}

const void* ftgl::vector_t::at(size_t index)
{
	assert( this->size_ );
	assert( index  < this->size_ );

	return ( char* )( this->items_ ) + index * this->item_size_;
}

void ftgl::vector_t::set(const size_t index, const void* item)
{
	assert( this->size_ );
	assert( this->size_ > index );

	memcpy( ( char* )( this->items_ ) + index * this->item_size_, item, this->item_size_ );

}

void ftgl::vector_t::erase(const size_t index)
{
	assert( index < this->size_ );

	this->erase_range( index, index + 1 );
}

void ftgl::vector_t::erase_range(const size_t first, const size_t last)
{
	assert( first < this->size_ );
	assert( last  < this->size_ + 1 );
	assert( first < last );

	memmove( 
		( char * )( this->items_ ) + first * this->item_size_,
		( char * )( this->items_ ) + last  * this->item_size_,
		( this->size_ - last )   * this->item_size_ );
	this->size_ -= (last-first);
}

void ftgl::vector_t::push_back(const void* item)
{
	  this->insert( this->size_, item );
}

void ftgl::vector_t::pop_back()
{
	assert( this->size_ );

	this->size_--;
}

void ftgl::vector_t::insert(const size_t index, const void* item)
{
	assert( index <= this->size_ );
	
	if ( this->capacity_ <= this->size_ )
	{
		this->reserve( 2 * this->capacity_ );
	}
	if ( index < this->size_ )
	{
		memmove( 
			( char * )( this->items_ ) + (index + 1) * this->item_size_,
			( char * )( this->items_ ) + (index + 0) * this->item_size_,
			( this->size_ - index)  * this->item_size_ );
	}
	this->size_++;
	this->set( index, item );
}

void ftgl::vector_t::insert_data(const size_t index, const void* data, const size_t count)
{
	assert( index < this->size_ );
	assert( data );
	assert( count );

	if( this->capacity_ < (this->size_ + count ) )
	{
		this->reserve( this->size_ + count );
	}
	memmove( 
		( char * )( this->items_ ) + (index + count ) * this->item_size_,
		( char * )( this->items_ ) + (index ) * this->item_size_,
		count * this->item_size_ );

	memmove( 
		( char * )( this->items_ ) + index * this->item_size_, 
		data,
		count * this->item_size_ );

	this->size_ += count;
}

void ftgl::vector_t::push_back_data(const void* data, const size_t count)
{
	assert( data );
	assert( count );

	if( this->capacity_ < ( this->size_ + count ) )
	{
		this->reserve( this->size_ + count );
	}
	memmove( 
		( char * )( this->items_ ) + this->size_ * this->item_size_, 
		data,
		count * this->item_size_ );

	this->size_ += count;
}

int ftgl::vector_t::contains(const void* item, int( *cmp )( const void*, const void* ))
{
	size_t i;

	for ( i = 0; i < this->size_; ++i )
	{
		if ( 0 == ( *cmp )( item, this->at( i ) ) )
		{
			return 1;
		}
	}

	return 0;
}

void ftgl::vector_t::sort( int (*cmp)(const void *, const void *))
{
	assert( this->size_ );

	qsort( this->items_, this->size_, this->item_size_, cmp);
}

size_t ftgl::vector_t::capacity()
{
	return this->capacity_;
}

void ftgl::vector_t::reserve(const size_t size)
{
	if ( this->capacity_ < size )
	{
		this->items_ = realloc( this->items_, size* this->item_size_ );
		this->capacity_ = size;
	}
}

void ftgl::vector_t::resize(const size_t size)
{
	if( size > this->capacity_ )
	{
		this->reserve( size );
		this->size_ = this->capacity_;
	}
	else
	{
		this->size_ = size;
	}
}

void ftgl::vector_t::shrink()
{
	if ( this->capacity_ > this->size_ )
	{
		this->items_ = realloc( this->items_, this->size_ * this->item_size_ );
	}
	this->capacity_ = this->size_;
}
