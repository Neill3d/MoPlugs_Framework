
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saverImpl.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucachesaverquery.h"
#include "gpucachemodel.h"

//////////////////////////////////////////////////////////////////////////
// class for saving data from gpu cache model

class CGPUCacheSaverQueryImpl : public CGPUCacheSaverQuery
{
public:

	//! a constructor
	CGPUCacheSaverQueryImpl(CGPUCacheModel	*pModel);

	//! a destructor
	virtual ~CGPUCacheSaverQueryImpl();

public:

	virtual bool Init(const char *filename);

	// query information for textures

	virtual const int GetVideoCount();
	virtual const char *GetVideoName(const int index);
	virtual const int GetVideoWidth(const int index);
	virtual const int GetVideoHeight(const int index);
	virtual const int GetVideoFormat(const int index);
	virtual const bool IsVideoImageSequence(const int index);
	virtual const int GetVideoStartFrame(const int index);
	virtual const int GetVideoStopFrame(const int index);
	virtual const int GetVideoFrameRate(const int index);
	virtual const char *GetVideoFilename(const int index);
	virtual const double GetVideoSize(const int index);
	virtual const bool IsVideoUsedMipmaps(const int index);

	// information about media
	virtual double GetTotalUncompressedSize();

	//

	virtual const int GetSamplersCount();
	virtual const char *GetSamplerName(const int index); // pTexture->LongName
	virtual const int GetSamplerVideoIndex(const int index);	// which video is used for that sampler

	virtual void GetSamplerMatrix( const int index, mat4 &mat );


protected:

	CGPUCacheModel		*mModel;

};