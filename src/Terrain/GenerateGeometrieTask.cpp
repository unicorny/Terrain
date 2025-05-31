#include "GenerateGeometrieTask.h"
#include "DREngine/DRLogging.h"
#include "DRCore2/Utils/DRProfiler.h"

namespace Terrain {

	DRReturn GenerateGeometrieTask::run()
	{
        DRProfiler timeUsed;
		    
		mPositions = std::make_shared<std::vector<DRVector2>>();
		mPositions->reserve(mGrid.stepsCount.x * mGrid.stepsCount.y);
		u32 indicesCursor = 0;
		for (int y = 0; y < mGrid.stepsCount.y; y++)
		{
			for (int x = 0; x < mGrid.stepsCount.x; x++)
			{
				mPositions->push_back({ x * mGrid.stepSize.x, y * mGrid.stepSize.y });
				if (y + 1 >= mGrid.stepsCount.y || x + 1 >= mGrid.stepsCount.x) continue;
				int topLeft = y * mGrid.stepsCount.x + x;
				int topRight = topLeft + 1;
				int bottomLeft = topLeft + mGrid.stepsCount.x;
				int bottomRight = bottomLeft + 1;

				mIndicesBuffer[indicesCursor++] = topLeft;
				mIndicesBuffer[indicesCursor++] = topRight;
				mIndicesBuffer[indicesCursor++] = bottomRight;
				mIndicesBuffer[indicesCursor++] = bottomLeft;
			}
		}
        DRLog.writeToLog("[GenerateGeometrieTask] %s for generating %d 2d Vertices + %d Indices", timeUsed.string().data(), mPositions->size(), indicesCursor);
		return DR_OK;
	}

}