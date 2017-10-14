
#pragma once

/*
    GitHub page - https://github.com/Neill3d/MoPlugs_Framework
	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
*/

#include	<string.h>

// get current OpenGL error desc or NULL if no error
const char * getGlErrorString ();

// get bound framebuffer error or NULL if no error
const char * checkFramebuffer ();

// check for error and print if any
bool checkGlError ( const char * title );
