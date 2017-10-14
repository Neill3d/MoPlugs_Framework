
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_materials.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "shared_materials.h"
//#include "io_materials.h"

/////////////////////////////////////////////////////////////////////////////// Materials Manager ////////////////////////////////////////
// 

CMaterialsReference::CMaterialsReference(CResourceGPUModel<TextureGLSL> *pTextures)
	: CResourceGPUModel<MaterialGLSL>()
	, mTextures(pTextures)
{
}

CMaterialsReference::~CMaterialsReference()
{
	Free();
}

void CMaterialsReference::Allocate(const int count)
{
	CResourceGPUModel<MaterialGLSL>::Allocate(count);
	mMaterialsNames.resize(count);
}

void CMaterialsReference::Free()
{
	CResourceGPUModel<MaterialGLSL>::Free();
}

void CMaterialsReference::Clear()
{
	CResourceGPUModel<MaterialGLSL>::Clear();
	mMaterialsNames.clear();

}

////////////////////////////////////////
// Materials

//

const int CMaterialsReference::GetNumberOfMaterials()
{
	return (int) mMaterialsNames.size();
}
const char *CMaterialsReference::GetMaterialName(const int index)
{
	return mMaterialsNames[index].c_str();
}

const GLuint64 CMaterialsReference::GetMaterialDiffuseChannel(const int matindex)
{
	if (matindex >=0 && matindex < mGPUData.size() )
		return mGPUData[matindex].diffuse;
	return 0;
}