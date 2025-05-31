#include "HeightMap.h"
#include "TTP.h"

#include "DRCore2/DRTypes.h"
#include "DRCore2/Utils/DRProfiler.h"
#include "DREngine/DRLogging.h"
#include "DREngine/DRIImage.h"

#include <filesystem>
#include <cassert>

namespace Terrain {

	// multiplication on cpu is usually faster than division
	constexpr DRReal INV_255 = 1.0 / 255.0;

	DRReal HeightMap::getInterpolatedHeight(const DRVector2 pos) const
	{
		assert(pos.x >= 0 && pos.x < static_cast<DRReal>(width));
		assert(pos.y >= 0 && pos.y < static_cast<DRReal>(height));
		assert(map.size() >= width * height);

		// position in terrain coords only integer part
		////////////////////////
		//  left  right
		//  0/0   1/0   top 
		//  0/1   1/1   bottom
		////////////////////////
		// static_cast<u16> is faster than floor, but work only with > 0 like floor
		u16 iLeft = static_cast<u16>(pos.x);
		u16 iTop = static_cast<u16>(pos.y);

		u16 iRight = (iLeft + 1 < width) ? iLeft + 1 : iLeft;
		u16 iBottom = (iTop + 1 < height) ? iTop + 1 : iTop;

		DRReal hLeftTop = static_cast<DRReal>(map[iLeft + iTop * width]) * INV_255;
		DRReal hRightTop = static_cast<DRReal>(map[iRight + iTop * width]) * INV_255;
		DRReal hLeftBottom = static_cast<DRReal>(map[iLeft + iBottom * width]) * INV_255;
		DRReal hRightBottom = static_cast<DRReal>(map[iRight + iBottom * width]) * INV_255;

		// billinear filtering
		DRVector2 diff(pos.x - static_cast<DRReal>(iLeft), pos.y - static_cast<DRReal>(iTop));
		DRReal hTop = (hLeftTop * (1.0 - diff.x)) + (hRightTop * diff.x);
		DRReal hBottom = (hLeftBottom * (1.0 - diff.x)) + (hRightBottom * diff.x);
		return (hTop * (1.0 - diff.y)) + (hBottom * diff.y);
	}



	DRReturn CollectInterpolatedHeights::run()
	{
		DRProfiler timeUsed;
		auto& heightMap = mHeightMap;
		if (!heightMap && mHeightMapLoaderTask) {
			heightMap = mHeightMapLoaderTask->getResult();
		}
		auto& positions = mPositions;
		if (!mPositions && mPositionsGeneratingTask) {
			positions = mPositionsGeneratingTask->getPositions();
		}
		assert(positions && heightMap);
		mHeights = std::make_shared<std::vector<DRReal>>();
		mHeights->reserve(positions->size());
		const auto& posVec = *positions;
		for (int i = 0; i < positions->size(); i++) {
			mHeights->push_back(heightMap->getInterpolatedHeight(posVec[i]));
		}
		DRLog.writeToLog("[CollectInterpolatedHeights] %s for interpolate %d heights from Terrain Height Map", timeUsed.string().data(), mHeights->size());
		return DR_OK;
	}

	std::shared_ptr<const std::vector<DRVector2>> CollectInterpolatedHeights::getPositions()
	{
		auto positions = mPositions;
		if (!mPositions && mPositionsGeneratingTask) {
			positions = mPositionsGeneratingTask->getPositions();
		}
		return positions;
	}


	DRReturn LoadHeightMapFromHme::run()
	{
		DRProfiler timeUsed;
		try {
			auto fileSize = std::filesystem::file_size(mFileName);
			if (fileSize < 100) {
				DRLog.writeToLog("Try to open file: %s, reported size: %llu", mFileName.data(), fileSize);
				LOG_ERROR("File is smaller than expected header size", DR_ERROR);
			}
			std::ifstream file(mFileName.data(), std::ios::binary);
			int file_header[25];
			file.read(reinterpret_cast<char*>(file_header), 100);
			mHeightMap = std::make_shared<HeightMap>();
			mHeightMap->width = file_header[0];
			mHeightMap->height = file_header[1];
			auto size = file_header[0] * file_header[1];
			if (fileSize < 100 + size) {
				DRLog.writeToLog("Try to open file: %s, reported size: %llu, heigt map width: %d, height: %d", 
					mFileName.data(),
					fileSize,
					mHeightMap->width,
					mHeightMap->height
				);
				LOG_ERROR("File is smaller than expected header size + payload size", DR_ERROR);
			}
			mHeightMap->map.resize(size, 0);
			file.read(reinterpret_cast<char*>(mHeightMap->map.data()), size);

			file.close();
			DRLog.writeToLog("[LoadHeightMapFromHme] %s for loading Height Map from Hme file with %d x %d", 
				timeUsed.string().data(), 
				mHeightMap->width,
				mHeightMap->height
			);
			return DR_OK;
		}
		catch (std::filesystem::filesystem_error& e) {
			DRLog.writeToLog("Try to open file: %s, exception thrown: %s", mFileName.data(), e.what());
			LOG_ERROR("Couldn't open file", DR_ERROR);
		}		
	}

	DRReturn LoadHeightMapFromImage::run()
	{
		DRProfiler timeUsed;
		auto image = DRIImage::newImage();
		if (image->loadFromFile(mFileName.data())) {
			LOG_ERROR("error loading image from file", DR_ERROR);
		}
		mHeightMap = std::make_shared<HeightMap>();
		mHeightMap->width = image->getWidth();
		mHeightMap->height = image->getHeight();
		u32 size = mHeightMap->width * mHeightMap->height;
		DRColor* colorBuffer = new DRColor[size];
		mHeightMap->map.reserve(size);
		image->getPixel(colorBuffer);
		for (u32 i = 0; i < size; i++) {
			mHeightMap->map.push_back(colorBuffer[i].r);
		}
		DRIImage::deleteImage(image);
		DRLog.writeToLog("[LoadHeightMapFromImage] %s for loading Height Map from Image file (%s) with %d x %d",
			timeUsed.string().data(),
			mFileName.data(),
			mHeightMap->width,
			mHeightMap->height
		);
		return DR_OK;		
	}

	DRReturn LoadHeightMapFromTTP::run()
	{
		DRProfiler timeUsed;
		auto fileSize = std::filesystem::file_size(mFileName);
		if (fileSize < sizeof(TTPHeader)) {
			DRLog.writeToLog("Try to open file: %s, reported size: %llu", mFileName.data(), fileSize);
			LOG_ERROR("File is smaller than expected header size", DR_ERROR);
		}
		std::ifstream file(mFileName.data(), std::ios::binary);
		TTPHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(TTPHeader));
		auto size = header.heightMapSize * header.heightMapSize;
		if (fileSize < sizeof(TTPHeader) + size) {
			DRLog.writeToLog("[Try to open file: %s, reported size: %llu, heigt map width: %d, height: %d",
				mFileName.data(),
				fileSize,
				header.heightMapSize,
				header.heightMapSize
			);
			LOG_ERROR("File is smaller than expected header size + payload size", DR_ERROR);
		}
		mHeightMap = std::make_shared<HeightMap>();
		mHeightMap->width = mHeightMap->height = header.heightMapSize;
		mHeightMap->map.reserve(size);
		DRColor* buffer = new DRColor[size];
		file.read(reinterpret_cast<char*>(buffer), size * sizeof(DRColor));
		for (int i = 0; i < size; i++) {
			mHeightMap->map.push_back(buffer[i].r * 255.0);
		}
		delete[] buffer;

		file.close();
		DRLog.writeToLog("[LoadHeightMapFromTTP] %s for loading Height Map from TTP file (%s) with %d x %d",
			timeUsed.string().data(),
			mFileName.data(),
			mHeightMap->width,
			mHeightMap->height
		);
		return DR_OK;
	}

}