#include "QuadraticGridLogic.h"
#include <cassert>
#include <vector>

namespace Terrain {
	std::array<u32, 4> QuadraticGridLogic::quadIndices(u16 x, u16 y) const
	{
		if (y + 1 >= mQuads || x + 1 >= mQuads) {
			throw InvalidQuadIndex("[quadIndices] invalid quad index, must be < (quads - 1)");
		}
		u32 topLeft = y * vertices() + x;

		return { 
			topLeft, 
			topLeft + 1, 
			topLeft + vertices(),
			topLeft + vertices() + 1
		};
	}

	void QuadraticGridLogic::triangleStripIndices(u32* indexBuffer) const
	{
		u16 vertsPerRow = vertices();
		u32 indexCursor = 0;
		auto totalIndexCount = totalIndices(GL_TRIANGLE_STRIP);

		for (u16 y = 0; y < mQuads; ++y)
		{
			if (y % 2 == 0)
			{
				// even lane: left to right
				for (u16 x = 0; x <= mQuads; ++x)
				{
					indexBuffer[indexCursor++] = y * vertsPerRow + x;
					indexBuffer[indexCursor++] = (y + 1) * vertsPerRow + x;
				}
			}
			else
			{
				// uneven lane: right to left
				for (int x = mQuads; x >= 0; --x)
				{
					indexBuffer[indexCursor++] = y * vertsPerRow + x;
					indexBuffer[indexCursor++] = (y + 1) * vertsPerRow + x;
				}
			}
			if (y + 1 < mQuads) {
				// use as for GL_PRIMITIVE_RESTART_FIXED_INDEX 
				indexBuffer[indexCursor++] = UINT_MAX;
			}
		}
		assert(indexCursor == totalIndices(GL_TRIANGLE_STRIP));
	}

	void QuadraticGridLogic::allVertices(DRVector2* verticesBuffer) const
	{
		size_t verticesCursor = 0;
		for (int y = 0; y < vertices(); y++)
		{
			for (int x = 0; x < vertices(); x++)
			{
				verticesBuffer[verticesCursor++] = vertex(x, y);
			}
		}
	}

	size_t QuadraticGridLogic::totalIndices(GLenum openGLDrawType) const
	{
		switch (openGLDrawType) {
		case GL_TRIANGLES: return static_cast<size_t>(totalQuads()) * 6;
		case GL_TRIANGLE_STRIP:
			return (mQuads * vertices() * 2) + (mQuads - 1);
		case GL_QUADS: return static_cast<size_t>(totalQuads()) * 4;
		default: throw InvalidRenderMode(
				"[Terrain::QuadraticGridDimensions::totalIndices] invalid render mode, expect one of these: GL_TRIANGLES, GL_TRIANGLE_STRIP, GL_QUADS"
			);
		}
		
	}
}