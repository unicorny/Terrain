#include "TTP.h"
#include "DREngine/DRLogging.h"
#include <filesystem>

namespace Terrain {
	TTPHeader loadTTPHeaderFromFile(const char* fileName)
	{
		auto fileSize = std::filesystem::file_size(fileName);
		if (fileSize < sizeof(TTPHeader)) {
			DRLog.writeToLog("Try to open file: %s, reported size: %llu", fileName, fileSize);
			throw std::runtime_error("File is smaller than expected header size");
		}
		std::ifstream file(fileName, std::ios::binary);
		TTPHeader header;
		file.read(reinterpret_cast<char*>(&header), sizeof(TTPHeader));
		file.close();
		return header;
	}
}