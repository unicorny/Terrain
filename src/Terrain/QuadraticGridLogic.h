#ifndef __TERRAIN_GRID_DIMENSIONS_H
#define __TERRAIN_GRID_DIMENSIONS_H

#include "Exceptions.h"

#include "DRCore2/DRTypes.h"
#include "DRCore2/Foundation/DRVector2.h"
#include "DREngine/OpenGL.h"

#include <array>

namespace Terrain {
	/*
	* \brief Helper Class for generating a grid from quads
	* Example:
	* size = 2
	* area = 4
	* quads = 2
	* totalQuads = 4
	* quadSize = 1
	* vertices = 3
	* totalVertices = 9
	* 
	* 0 1 2
	* 3 4 5
	* 6 7 8
	* 
	* Quads[0] = 0, 1, 3, 4
	* Quads[1] = 1, 2, 4, 5
	* Quads[2] = 3, 4, 6, 7
	* Quads[3] = 4, 5, 7, 8
	*/
	class QuadraticGridLogic
	{
	public:
		//! \param size in one direction, surface area is size * size
		//! \param quads quad count in one direction
		QuadraticGridLogic(u16 size, u16 height, u16 quads)
		: mSize(size), mHeight(height), mQuads(quads) {}
		QuadraticGridLogic() : mSize(0), mHeight(0), mQuads(0) {}
		~QuadraticGridLogic() {}

		// easy calculations are sometimes faster than store and load from memory
		//! \return surface area
		inline u32 area() const;
		//! \return count of quads in total
		inline u32 totalQuads() const;
		//! \return size in one direction
		inline u16 size() const;
		//! \return terrain height
		inline u16 height() const;
		//! \return quad count in one direction
		inline u16 quads() const;
		//! \return quadSize 
		inline DRReal quadSize() const;
		//! \return get four indices for quad on declared index, assuming GL_QUADS as render mode
		std::array<u32, 4> quadIndices(u16 x, u16 y) const;
		void triangleStripIndices(u32* indexBuffer) const;
		//! \return vertices count in one direction
		inline u16 vertices() const;
		//! \return vertices total count for whole grid
		inline u32 totalVertices() const;
		//! \param openGLDrawType expect GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_QUADS !deprecated
		//! GL_TRIANGLE_STRIP assume configuration with using primitive restart (vertices() + 1) * quads
		//! \return total count for indices for rendering
		size_t totalIndices(GLenum openGLDrawType) const;
		//! \param return vertex x * quadSize, y * quadSize
		inline DRVector2 vertex(u16 x, u16 y) const;
		void allVertices(DRVector2* verticesBuffer) const;

	protected:
		// size of on side, total size = mSize * mSize
		u16 mSize;
		u16 mHeight;
		u16 mQuads;
	};

	// Exceptions for QuadraticGridLogic
	class InvalidQuadIndex : LogicalException
	{
	public:
		using LogicalException::LogicalException;
	};

	class InvalidRenderMode : LogicalException
	{
	public:
		using LogicalException::LogicalException;
	};

	// -----  inline implementations -----------
	u32 QuadraticGridLogic::area() const
	{
		return static_cast<u32>(mSize) * static_cast<u32>(mSize);
	}

	u32 QuadraticGridLogic::totalQuads() const
	{ 
		return static_cast<size_t>(mQuads) * static_cast<size_t>(mQuads); 
	}

	u16 QuadraticGridLogic::size() const
	{ 
		return mSize; 
	}

	u16 QuadraticGridLogic::height() const
	{
		return mHeight;
	}

	u16 QuadraticGridLogic::quads() const
	{ 
		return mQuads; 
	}

	DRReal QuadraticGridLogic::quadSize() const
	{
		return static_cast<DRReal>(mSize) / static_cast<DRReal>(mQuads);
	}

	u16 QuadraticGridLogic::vertices() const
	{
		return mQuads + 1;
	}

	u32 QuadraticGridLogic::totalVertices() const
	{
		return vertices() * vertices();
	}

	DRVector2 QuadraticGridLogic::vertex(u16 x, u16 y) const
	{
		if (y > vertices() || x > vertices()) {
			throw InvalidQuadIndex("[vertex] invalid vertex index, must be < vertices()");
		}
		return {
			static_cast<DRReal>(x) * quadSize(),
			static_cast<DRReal>(y) * quadSize()
		};
	}

	
}

#endif //__TERRAIN_GRID_DIMENSIONS_H