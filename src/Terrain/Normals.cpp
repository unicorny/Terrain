#include "Normals.h"
#include "DREngine/DRLogging.h"

namespace Terrain {
	DRReturn CalculateNormalsTask::run()
	{
		DRProfiler timeUsed;
		const auto& heights = *mInterpolatedHeights->getResult();
		
		for (int y = 0; y < mGrid.stepsCount.y; y++)
		{
			for (int x = 0; x < mGrid.stepsCount.x; x++)
			{
				DRReal hLeft = heights[ y * mGrid.stepsCount.y + (x > 0 ? x - 1 : x) ];
				DRReal hRight = heights[y * mGrid.stepsCount.y + (x + 1 < mGrid.stepsCount.x ? x + 1 : x) ];
				DRReal hDown = heights[ (y > 0 ? y - 1 : y ) * mGrid.stepsCount.y + x];
				DRReal hUp = heights[( y + 1 < mGrid.stepsCount.y ?  y + 1 : y) * mGrid.stepsCount.y + x ];

				DRVector3 dX = { 2.0f * mGrid.stepSize.x, hRight - hLeft, 0.0f };
				DRVector3 dY = { 0.0f, hUp - hDown, 2.0f * mGrid.stepSize.y };
				DRVector3 normal = dY.cross(dX);
				mNormalsBuffer[y * mGrid.stepsCount.y + x] = normal.normalize();
			}
		}
		DRLog.writeToLog("[CalculateNormalsTask] %s for calculate %d normals", timeUsed.string().data(), mGrid.stepsCount.x * mGrid.stepsCount.y);
		return DR_OK;
	}
}