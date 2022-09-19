
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_loader.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "gpucache_types.h"
#include "shared_glsl.h"

#include "IO\tinyxml.h"



///////////////////////////////////////////////////////////////////////////
//

class CGPUCacheLoader
{

public:
	//! a constructor
	CGPUCacheLoader();

	bool Load(const char *filename, CGPUCacheLoaderVisitor *pVisitor);

protected:

	CGPUCacheLoaderVisitor *mVisitor;

	//

	bool ReadTextures(const char *textures_filename, TiXmlElement *parentElem);
	bool ReadImageData();
	bool ReadImageData2();
	bool ReadSampler();

	bool ReadMaterials(TiXmlElement *parentElem);
	void EmptyMaterial( MaterialGLSL &mat );
	void ConstructMaterialFromXML( TiXmlElement *matelem, MaterialGLSL &mat );

	bool ReadShaders(TiXmlElement *parentElem);

	bool ReadModels(const char *models_filename, TiXmlElement *parentElem);
};

