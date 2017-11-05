
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saver.h
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

//////////////////////////////////////////////////////////////////////////
//

class CGPUCacheSaverQuery
{
public:

	CGPUCacheSaverQuery()
	{}

	virtual bool Init(const char *filename)
	{
		return false;
	}

	//
	// query infromation for header

	virtual const int GetVersion() {
		return 1;
	}
	virtual const char *GetSourceFilename()
	{
		return "";
	}

	//
	// query information for lights

	virtual const int GetLightsCount() {
		return 0;
	}
	
	virtual const char *GetLightName(const int index) = 0;
	virtual void GetLightPosition(const int index, vec4 &pos) = 0;
	virtual void GetLightDirection(const int index, vec4 &dir) = 0;
	virtual void GetLightColor(const int index, vec4 &color) = 0;
	virtual void GetLightAttenuation(const int index, vec4 &att) = 0;
	virtual void GetLightShadowing(const int index, bool &castshadow) = 0;

	// query information for textures

	virtual const int GetVideoCount() {
		return 0;
	}
	virtual const char *GetVideoName(const int index) = 0;
	virtual const int GetVideoWidth(const int index) = 0;
	virtual const int GetVideoHeight(const int index) = 0;
	virtual const int GetVideoFormat(const int index) = 0;
	virtual const bool IsVideoImageSequence(const int index) = 0;
	virtual const int GetVideoStartFrame(const int index) = 0;
	virtual const int GetVideoStopFrame(const int index) = 0;
	virtual const int GetVideoFrameRate(const int index) = 0;
	virtual const char *GetVideoFilename(const int index) = 0;
	virtual const double GetVideoSize(const int index) = 0;
	virtual const bool IsVideoUsedMipmaps(const int index) = 0;

	// information about media
	virtual double GetTotalUncompressedSize()
	{
		return 0.0;
	}

	//

	virtual const int GetSamplersCount() = 0;
	virtual const char *GetSamplerName(const int index) = 0; // pTexture->LongName
	virtual const int GetSamplerVideoIndex(const int index) = 0;	// which video is used for that sampler

	virtual void GetSamplerMatrix( const int index, mat4 &mat ) = 0;


	//
	// query information for materials

	virtual const int GetMaterialsCount() 
	{
		return 0;
	}

	virtual const char *GetMaterialName(const int index) = 0; // pMaterial->LongName 
	virtual void ConvertMaterial(const int index, MaterialGLSL &data) = 0;

	//
	// query information for shaders

	virtual const int GetShadersCount()
	{
		return 0;
	}

	virtual const char *GetShaderName(const int index) = 0;
	virtual const int GetShaderAlphaSource(const int index) = 0;
	virtual void ConvertShader(const int index, ShaderGLSL &data) = 0;

	//
	// query information for models

	

	virtual const int GetModelsCount() = 0;
	virtual const int GetSubMeshesCount() = 0;
	virtual const unsigned int GetTotalCounts(unsigned int &vertices, unsigned int &indices) = 0;
	// NOTE: should be calculated in global world space !
	virtual void GetBoundingBox(vec4 &bmin, vec4 &bmax) = 0;

	virtual const char *GetModelName(const int modelId) = 0; // longname
	virtual const int GetModelVisible(const int modelId) = 0; // (pModel->IsVisible()) ? 1 : 0
	virtual const int GetModelCastsShadows(const int modelId) = 0; // (pModel->CastsShadows) ? 1 : 0
	virtual const int GetModelReceiveShadows(const int modelId) = 0; // (pModel->ReceiveShadows) ? 1 : 0
	virtual void GetModelMatrix(const int modelId, mat4 &mat) = 0;
	virtual void GetModelTranslation(const int modelId, vec4 &pos) = 0;
	virtual void GetModelRotation(const int modelId, vec4 &rot) = 0;
	virtual void GetModelScaling(const int modelId, vec4 &scaling) = 0;
	// NOTE: should be calculated in global world space !
	virtual void GetModelBoundingBox(const int modelId, vec4 &bmin, vec4 &bmax) = 0;

	// model geometry

	virtual const int GetModelVertexCount(const int modelId) = 0; // pVertexData->GetVertexCount()
	virtual const int GetModelUVCount(const int modelId) = 0;

	virtual void ModelVertexArrayRequest(const int modelId) = 0;
	virtual const float *GetModelVertexArrayPoint( const bool afterDeform ) = 0;
	virtual const float *GetModelVertexArrayNormal( const bool afterDeform ) = 0;
	virtual const float *GetModelVertexArrayTangent( const bool afterDeform ) = 0;
	virtual const float *GetModelVertexArrayUV( const int uvset, const bool afterDeform ) = 0;
	virtual const int *GetModelIndexArray() = 0;
	virtual void ModelVertexArrayRelease() = 0;

	// CGPUVertexData::GetStrideFromArrayElementType( pVertexData->GetVertexArrayType(kFBGeometryArrayID_Point) ) 
	virtual const int GetModelVertexArrayPointStride(const int modelId) = 0;
	virtual const int GetModelVertexArrayNormalStride(const int modelId) = 0;
	virtual const int GetModelVertexArrayTangentStride(const int modelId) = 0;
	virtual const int GetModelVertexArrayUVStride(const int modelId) = 0;

	virtual const int GetModelSubPatchCount(const int index) = 0;
	virtual void GetModelSubPatchInfo(const int modelid, const int patchid, int &offset, int &size, int &materialId) = 0;

	virtual const unsigned int GetModelShadersCount(const int index) = 0;
	virtual const int GetModelShaderId(const int index, const int nshader) = 0;

};



//////////////////////////////////////////////////////////////////////////
//
class CGPUCacheSaver
{
public:
	//! a constructor
	CGPUCacheSaver();

	bool Save(const char *filename, CGPUCacheSaverQuery *pQuery );
	bool SaveTextures(const char *filename, CGPUCacheSaverQuery *pQuery);

protected:

	CGPUCacheSaverQuery		*mQuery;

	bool WriteModelToXML( const int index, TiXmlElement *parentElem );
	bool WriteModelGeometry( FILE *modelFile, const int index );

	bool WriteMaterialsToXML( TiXmlElement *parentElem );
	bool WriteTexturesToXML( TiXmlElement *parentElem );
	bool WriteShadersToXML( TiXmlElement *parentElem );
	bool WriteOneShaderToXML( TiXmlElement *parentElem, const int index );
	bool WriteLightsToXML( TiXmlElement *parentElem );

	bool SaveDDSData( int fh, const char *filename );
	// index in samplers array
	bool SaveSampler( int fh, const int index, const int videoIndex );
	// index in video clips array
	bool SaveImageEmpty( int fh );
};