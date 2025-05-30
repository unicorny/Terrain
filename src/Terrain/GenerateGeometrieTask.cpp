#include "GenerateGeometrieTask.h"
#include "DREngine/DRLogging.h"
#include "DRCore2/Utils/DRProfiler.h"

namespace Terrain {

	DRReturn GenerateGeometrieTask::run()
	{
        DRProfiler timeUsed;
		DRVector2i stepsCount = mSize / mStepSize;
		if (stepsCount * mStepSize != mSize) {
			LOG_ERROR("please check params, stepsCount * mStepSize != mSize", DR_ERROR);
		}
        // for the edges
        // stepsCount += {1, 1};

		mPositions = std::make_shared<std::vector<DRVector2>>();
		mPositions->reserve(stepsCount.x * stepsCount.y);
		mIndices = std::make_shared<std::vector<unsigned int>>();
		mIndices->reserve((stepsCount.x - 1) * (stepsCount.y - 1) * 4);
		for (int y = 0; y < stepsCount.y; y++) 
		{
			for (int x = 0; x < stepsCount.x; x++) 
			{
				mPositions->push_back({ x * mStepSize.x, y * mStepSize.y });
				if (y + 1 >= stepsCount.y || x + 1 >= stepsCount.x) continue;
				int topLeft = y * stepsCount.x + x;
				int topRight = topLeft + 1;
				int bottomLeft = topLeft + stepsCount.x;
				int bottomRight = bottomLeft + 1;

				mIndices->push_back(topLeft);
				mIndices->push_back(topRight);
				mIndices->push_back(bottomRight);
				mIndices->push_back(bottomLeft);
			}
		}
        DRLog.writeToLog("[GenerateGeometrieTask] %s for generating %d 2d Vertices + Indices", timeUsed.string().data(), mPositions->size());
		return DR_OK;
	}

}