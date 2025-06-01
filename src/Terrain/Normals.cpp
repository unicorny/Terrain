#include "Normals.h"
#include "DREngine/DRLogging.h"

namespace Terrain {
	DRReturn CalculateNormalsTask::run()
	{
		DRProfiler timeUsed;
		const auto& heights = *mInterpolatedHeights->getResult();
		auto heightMap = mInterpolatedHeights->getHeightMap();
		size_t normalsCursor = 0;
		auto normalsBuffer = mNormalsBuffer->getVector3Ptr(GPU::DRGLBufferType::NORMALS_BUFFER);
		auto quadSize = mGrid.quadSize();

		for (int y = 0; y < mGrid.vertices(); y++)
		{
			for (int x = 0; x < mGrid.vertices(); x++)
			{				
				DRReal hLeft = heights[ y * mGrid.vertices() + (x > 0 ? x - 1 : x) ];
				DRReal hRight = heights[y * mGrid.vertices() + (x + 1 < mGrid.vertices() ? x + 1 : x) ];
				DRReal hDown = heights[ (y > 0 ? y - 1 : y ) * mGrid.vertices() + x];
				DRReal hUp = heights[( y + 1 < mGrid.vertices() ?  y + 1 : y) * mGrid.vertices() + x ];

				DRVector3 dX = { 2.0f * quadSize, (hRight - hLeft) * mGrid.height(), 0.0f};
				DRVector3 dY = { 0.0f, (hUp - hDown) * mGrid.height(), 2.0f * quadSize };
				DRVector3 normal = dY.cross(dX);
				normalsBuffer[normalsCursor++] = normal.normalize();
			}
		}
		DRLog.writeToLog("[CalculateNormalsTask] %s for calculate %d normals", timeUsed.string().data(), mGrid.totalVertices());
		return DR_OK;
	}
}