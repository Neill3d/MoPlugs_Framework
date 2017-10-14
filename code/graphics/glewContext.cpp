
#include "glewContext.h"

#ifdef GLEW_MX
GLEWContext ctx;

GLEWContext *glewGetContext()
{
	return &ctx;
}
#endif