
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_materials.h
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

// STL
#include <vector>
#include <map>

//
#include "algorithm\nv_math.h"

#include "graphics\OGL_Utils.h"

#include "shared_textures.h"
#include "shared_glsl.h"

#include "shared_common.h"

////////////////////////////////////////////////////////////////////////////////
// forward declaration
class CGPUCacheLoaderVisitorImpl;

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//
// collect materials from FBScene or locally from gpu cache
//

class CMaterialsReference : public CResourceGPUModel<MaterialGLSL>
{

protected:

	virtual void Allocate(const int count);
	virtual void Free();
	virtual void Clear();

public:

	//! a constructor
	CMaterialsReference(CResourceGPUModel<TextureGLSL>	*pTextures);

	//! a destructor
	virtual ~CMaterialsReference();

	//
	// process materials

	const int GetNumberOfMaterials();
	const char *GetMaterialName(const int index);

	const GLuint64 GetMaterialDiffuseChannel(const int matindex);

	

protected:

	CResourceGPUModel<TextureGLSL>	*mTextures;

	// store names for UI
	std::vector<std::string>		mMaterialsNames;

	friend class CGPUCacheLoaderVisitorImpl;
};