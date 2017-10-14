
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_shaders.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-- 
#include <GL\glew.h>

//
#include "algorithm\nv_math.h"

#include "graphics\OGL_Utils.h"
#include "shared_glsl.h"

#include "shared_common.h"

////////////////////////////////////////////////////////////////////////////////
// forward declaration
class CGPUCacheLoaderVisitorImpl;

////////////////////////////////////////////////////////////////////////////
//

class CShadersReference : public CResourceGPUModel<ShaderGLSL>
{

protected:

	virtual void Allocate(const int count);

	// free memory
	virtual void Free();

	// clear all data to zero
	virtual void Clear();

public:

	//! a constructor
	CShadersReference();
	//! a destructor
	virtual ~CShadersReference();


	const int	GetAlphaSource(const int shaderIndex) const;
	const char	*GetShaderName(const int index) const;

	std::vector<int> *GetAlphaVectorPtr() {
		return &mShadersAlpha;
	}

	const int GetNumberOfOpaqueShaders() const;
	const int GetNumberOfTransparencyShaders() const;

protected:

	std::vector<std::string>		mShadersNames;	// store names for UI
	std::vector<int>				mShadersAlpha;	// store information about alpha states

	friend class CGPUCacheLoaderVisitorImpl;
};
