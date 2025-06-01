#ifndef __TERRAIN_EXCEPTION_H
#define __TERRAIN_EXCEPTION_H

#include <stdexcept>
#include <string>

namespace Terrain {
	class LogicalException : std::logic_error
	{
	public:
		using std::logic_error::logic_error;

	};
}

#endif //__TERRAIN_EXCEPTION_H