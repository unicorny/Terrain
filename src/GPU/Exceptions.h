#ifndef __GPU_EXCEPTIONS_H
#define __GPU_EXCEPTIONS_H

#include <stdexcept>
#include <string>
#include "DREngine/OpenGL.h"

namespace GPU {
	class LogicalException : std::logic_error 
	{
	public:
		using std::logic_error::logic_error;

	};

	class OpenGLError : std::runtime_error 
	{
	public:
		using std::runtime_error::runtime_error;
	};

	void throwOnOpenGLError();
}

#endif //__GPU_EXCEPTIONS_H