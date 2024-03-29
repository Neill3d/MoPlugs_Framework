
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
class CGPUCacheLoaderVisitor
{
public:
	
	CGPUCacheLoaderVisitor()
	{}

	virtual ~CGPUCacheLoaderVisitor()
	{}

	// main header
	virtual void OnReadHeader(const char *xmlFilename, const char *sourceFilename) = 0;
	
	// textures
	virtual bool OnReadTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages ) = 0;
	virtual void OnReadTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) = 0;
	virtual void OnReadTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData) = 0;
	virtual void OnReadEmptyImage() = 0;
	virtual void OnReadTexturesSampler(const char *samplerName, const char *clipFile, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData) = 0;
	virtual void OnReadTexturesError(const char *what) = 0;
	virtual void OnReadTexturesEnd() = 0;
	
	// materials, shaders, etc.
	virtual bool OnReadMaterialsBegin(const int numberOfMaterials) = 0;
	virtual void OnReadMaterial(const char *material_name, const MaterialGLSL &material ) = 0;
	virtual void OnReadMaterialsEnd() = 0;

	virtual bool OnReadShadersBegin(const int numberOfShaders) = 0;
	virtual void OnReadShader(const char *shader_name, const int alphatype, const ShaderGLSL &shaderData ) = 0;
	virtual void OnReadShadersEnd() = 0;

	// geometry, models
	virtual bool OnReadModelsBegin(const int numberOfModels, const int numberOfMeshes, const double *bounding_min, const double *bounding_max) = 0;
	virtual void OnReadVertexData( FileGeometryHeader *const pHeader, const BYTE *data ) = 0;

	// NOTE: data is a global vertex data, GET from pheader local model segment
	virtual void OnReadModel(const char *name, 
						const double *translation, 
						const double *rotation, 
						const double *scaling, 
						const double *bounding_min,
						const double *bounding_max,
						const int numberOfShaders,
						const int *shaders,
						const VertexDataHeader *pheader, 
						const BYTE *data) = 0;
	virtual void OnReadModelPatch(const int offset, const int size, const int materialId) = 0;
	virtual void OnReadModelFinish() = 0;
	virtual void OnReadModelsEnd() = 0;
};

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

