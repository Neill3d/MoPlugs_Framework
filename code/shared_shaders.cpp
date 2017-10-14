
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_shaders.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_shaders.h"
//#include "io_shaders.h"


///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CShadersReference::CShadersReference()
	: CResourceGPUModel<ShaderGLSL>()
{
	mNumberOfBaseShaders = 0;
}

CShadersReference::~CShadersReference()
{
	Free();
}

void CShadersReference::Allocate(const int count)
{
	CResourceGPUModel<ShaderGLSL>::Allocate(count);

	mShadersNames.resize(count);
	mShadersAlpha.resize(count, 0);
}

void CShadersReference::Free()
{
	CResourceGPUModel<ShaderGLSL>::Free();
}

void CShadersReference::Clear()
{
	CResourceGPUModel<ShaderGLSL>::Clear();
	
	mShadersNames.clear();
	mShadersAlpha.clear();
}


const int CShadersReference::GetAlphaSource(const int shaderIndex) const
{
	return mShadersAlpha[shaderIndex];
}

const int CShadersReference::GetNumberOfOpaqueShaders() const
{
	int count=0;
	for (auto it=mShadersAlpha.begin(); it!=mShadersAlpha.end(); ++it)
	{
		if (*it == 0) // kFBAlphaSourceNoAlpha
			count++;
	}

	return count;
}

const int CShadersReference::GetNumberOfTransparencyShaders() const
{
	int count=0;
	for (auto it=mShadersAlpha.begin(); it!=mShadersAlpha.end(); ++it)
	{
		if (*it != 0) // kFBAlphaSourceNoAlpha
			count++;
	}

	return count;
}

const char *CShadersReference::GetShaderName(const int index) const
{
	return mShadersNames[index].c_str();
}