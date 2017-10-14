
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: utils_shaders.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_glsl.h"
#include "shared_common.h"

// pass shader id -1 if slot is empty
// return shader combination index in shaders vector
const int AddShaderCombination( CResourceGPUModel<ShaderGLSL> *shaders, 
									const int shader1, 
									const int shader2, 
									const int shader3, 
									const int shader4, 
									const int shader5,
									std::vector<int> *alpha_vector=nullptr);