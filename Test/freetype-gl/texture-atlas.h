#ifndef __TEXTURE_ATLAS_H__
#define __TEXTURE_ATLAS_H__

#include "glm/glm.hpp"
#include "glm/fwd.hpp"

#include <stdlib.h>

#include <vector>



namespace ftgl 
{
	/**
	* A texture atlas is used to pack several small regions into a single texture.
	*
	* The actual implementation is based on the article by Jukka Jylänki : "A
	* Thousand Ways to Pack the Bin - A Practical Approach to Two-Dimensional
	* Rectangle Bin Packing", February 27, 2010.
	* More precisely, this is an implementation of the Skyline Bottom-Left
	* algorithm based on C++ sources provided by Jukka Jylänki at:
	* http://clb.demon.fi/files/RectangleBinPack/
	*
	*
	* Example Usage:
	* @code
	* #include "texture-atlas.h"
	*
	* ...
	*
	* / Creates a new atlas of 512x512 with a depth of 1
	* texture_atlas_t * atlas = texture_atlas_new( 512, 512, 1 );
	*
	* // Allocates a region of 20x20
	* ivec4 region = texture_atlas_get_region( atlas, 20, 20 );
	*
	* // Fill region with some data
	* texture_atlas_set_region( atlas, region.x, region.y, region.width, region.height, data, stride )
	*
	* ...
	*
	* @endcode
	*
	* @{
	*/


	/**
	* A texture atlas is used to pack several small regions into a single texture.
	*/

	class texture_atlas
	{
	public:

		/*
		* @param   width   width of the atlas
		* @param   height  height of the atlas
		* @param   depth   bit depth of the atlas
		*/
		texture_atlas( const size_t width, const size_t height, const size_t depth );

		~texture_atlas();

		//Upload atlas to video memory.
		void upload();				

		/**
		*  Allocate a new region in the atlas.
		*  @param width  width of the region to allocate
		*  @param height height of the region to allocate
		*  @return       Coordinates of the allocated region
		*
		*/
		glm::ivec4 getRegion( const size_t width, const size_t height );


		/**
		*  Upload data to the specified atlas region.
		*
		*  @param x      x coordinate the region
		*  @param y      y coordinate the region
		*  @param width  width of the region
		*  @param height height of the region
		*  @param data   data to be uploaded into the specified region
		*  @param stride stride of the data
		*
		*/
		void setRegion( const size_t x, const size_t y, 
						const size_t width, const size_t height, 
						const unsigned char *data, const size_t stride );

		//Remove all allocated regions from the atlas.
		void clear();

		inline size_t getWidth() { return this->width_; }
		inline size_t getHeight(){ return this->height_; }
		inline size_t getDepth() { return this->depth_; }
		inline unsigned int getTexID(){ return this->id_; }
		unsigned char* getData(){ return this->data_; }

		//必须在添加文字之前执行;对已经添加的文字没有效果;
		void enableDistanceField( bool b ){ this->enable_distance_field_ = b; }

		//慎用;
		void makeDistanceField();
	private:

		int fit( const size_t index, const size_t width, const size_t height );
		void merge();

	private:

		//注意，修改数据时需用引用; glm::ivec3* v = &nodes[i];
		std::vector<glm::ivec3>  nodes;			//Allocated nodes

		size_t width_;				//Width (in pixels) of the underlying texture

		size_t height_;				//Height (in pixels) of the underlying texture

		size_t depth_;				//Depth (in bytes) of the underlying texture

		size_t used_;				//Allocated surface size

		unsigned int id_;			//Texture identity (OpenGL)

		unsigned char * data_;		//Atlas data


		bool enable_distance_field_;

	};
}

#endif /* __TEXTURE_ATLAS_H__ */
