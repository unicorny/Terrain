#include "Exceptions.h"

namespace GPU {

	const char* DRGetGLErrorText(GLenum eError)
	{
		switch (eError)
		{
		case GL_INVALID_ENUM:		return "GL_INVALID_ENUM";
		case GL_INVALID_VALUE:		return "GL_INVALID_VALUE";
		case GL_INVALID_OPERATION:	return "GL_INVALID_OPERATION";
		case GL_STACK_OVERFLOW:		return "GL_STACK_OVERFLOW";
		case GL_STACK_UNDERFLOW:	return "GL_STACK_UNDERFLOW";
		case GL_OUT_OF_MEMORY:		return "GL_OUT_OF_MEMORY";
		case GL_NO_ERROR:			return "GL_NO_ERROR";
		default: return "- gl Unknown error-";
		}
		return "- error -";
	}

	void throwOnOpenGLError() {
		GLenum GLError = glGetError();
		if (GLError) {
			std::string error = "OpenGL Error: ";
			error += DRGetGLErrorText(GLError);
			throw OpenGLError(error);
		}
	};
}