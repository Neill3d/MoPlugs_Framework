//
// Simple OpenGL error checks
//

#include	<stdio.h>
#include	"checks.h"

#ifdef  _WIN32
    #include    <windows.h>
#endif

#include <gl\glew.h>
//#include	"graphics\glewContext.h"

const char * getGlErrorString ()
{
	int error;
	
	if ( (error = glGetError()) != GL_NO_ERROR)
	{
		switch ( error )
		{
		case GL_NO_ERROR:				// no error, return NULL
			return NULL;
			
		case GL_INVALID_ENUM:
			return "GL_INVALID_ENUM";
			
		case GL_INVALID_VALUE:
			return "GL_INVALID_VALUE";
		
		case GL_INVALID_OPERATION:
			return "GL_INVALID_OPERATION";
			
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			return "GL_INVALID_FRAMEBUFFER_OPERATION";
			
		case GL_OUT_OF_MEMORY:
			return  "GL_OUT_OF_MEMORY";
			
		default:
			return "UNKNOWN ERROR";
		}
	}
	
	return NULL;
}

const char * checkFramebuffer ()
{
	GLenum status = glCheckFramebufferStatus ( GL_FRAMEBUFFER );
	
	if ( status == GL_FRAMEBUFFER_COMPLETE )		// ok
		return NULL;
		
	switch ( status )
	{
		case GL_FRAMEBUFFER_UNDEFINED:
			return  "GL_FRAMEBUFFER_UNDEFINED";
			
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			
		case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
			
		case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
			return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
			
		case GL_FRAMEBUFFER_UNSUPPORTED:
			return "GL_FRAMEBUFFER_UNSUPPORTED";
			
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
			
		case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS:
			return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
			
		default:
			return "UNKNOWN ERROR";
	}

	return NULL;
}

bool checkGlError ( const char * title )
{
	const char * str = getGlErrorString ();

	if ( str != NULL )
	{
		printf ( "%s - OpenGL error: %s\n", title, str );
		return false;
	}
	return true;
}
