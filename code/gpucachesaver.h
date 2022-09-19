
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

	bool SaveImageSimple( int fh, const int index );
};