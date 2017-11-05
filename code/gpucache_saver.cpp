
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saver.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_saver.h"

#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "nv_dds\nv_dds.h"

///////////////////////////////////////////////////////////////////////////////////////////////////////
// CGPUCacheSaver

CGPUCacheSaver::CGPUCacheSaver()
{
	mQuery = nullptr;
}

bool CGPUCacheSaver::Save(const char *filename, CGPUCacheSaverQuery *pQuery )
//bool WriteObjectsToXML(const char *fileName, const char *modelFileName, const char *texturesFileName, FBModelList &pList)
{
	mQuery = pQuery;
	if (mQuery == nullptr || mQuery->Init(filename) == false)
	{
		return false;
	}

	//
	TiXmlDocument doc;

	TiXmlElement head("Header");
	head.SetAttribute( "models", mQuery->GetModelsCount() );
	head.SetAttribute( "version", mQuery->GetVersion() );

	head.SetAttribute( "filename", mQuery->GetSourceFilename() );

	//
	
	TiXmlElement models("Models");
	int numberOfModels = mQuery->GetModelsCount();
	models.SetAttribute( "count", numberOfModels );
	
	for (int i=0; i<numberOfModels; ++i)
	{
		WriteModelToXML( i, &models );
	}

	
	models.SetAttribute( "submeshes", mQuery->GetSubMeshesCount() );

	// store bb of the models
	vec4 vmin, vmax;
	mQuery->GetBoundingBox(vmin, vmax);

	TiXmlElement bbItem( "BoundingBox" );
	bbItem.SetDoubleAttribute( "minx", vmin[0] );
	bbItem.SetDoubleAttribute( "miny", vmin[1] );
	bbItem.SetDoubleAttribute( "minz", vmin[2] );
	bbItem.SetDoubleAttribute( "maxx", vmax[0] );
	bbItem.SetDoubleAttribute( "maxy", vmax[1] );
	bbItem.SetDoubleAttribute( "maxz", vmax[2] );

	models.InsertEndChild( bbItem );

	unsigned int verticesCount, indicesCount;
	mQuery->GetTotalCounts(verticesCount, indicesCount);

	models.SetAttribute( "totalVertices", verticesCount );
	models.SetAttribute( "totalIndices", indicesCount );

	//
	// MATERIALS

	TiXmlElement materials("Materials");
	WriteMaterialsToXML( &materials );

	//
	// TEXTURES

	TiXmlElement textures("Textures");
	WriteTexturesToXML( &textures );

	//
	// SHADERS

	TiXmlElement shaders("Shaders");
	WriteShadersToXML( &shaders );
	
	//
	// LIGHTS

	TiXmlElement lights("Lights");
	WriteLightsToXML( &lights );

	head.SetAttribute( "materials",	mQuery->GetMaterialsCount() );
	head.SetAttribute( "textures",	mQuery->GetSamplersCount() );
	head.SetAttribute( "media",		mQuery->GetVideoCount() );
	head.SetAttribute( "shaders",	mQuery->GetShadersCount() );
	head.SetAttribute( "lights",	mQuery->GetLightsCount() );

	doc.InsertEndChild(head);
	doc.InsertEndChild(models);
	doc.InsertEndChild(materials);
	doc.InsertEndChild(textures);
	doc.InsertEndChild(shaders);
	doc.InsertEndChild(lights);
	doc.SaveFile( filename );

	if (doc.Error() )
	{
		printf( doc.ErrorDesc() );
		return false;
	}

	// store geometry and texture resources
	std::string		geometry_filename( filename );

	auto iter = geometry_filename.find_last_of( "." );
	geometry_filename.erase( iter );
	geometry_filename.append( "_Geometry.pck" );
	
	FILE *modelFile = nullptr;
	int err = fopen_s( &modelFile, geometry_filename.c_str(), "wb" );
	try
	{
		if (err != 0)
			throw "Failed to open geometry file for writing";

		FileGeometryHeader geomHeader;
		FileGeometryHeader::Set( 1, numberOfModels, verticesCount, indicesCount, geomHeader );

		fwrite( &geomHeader, sizeof(FileGeometryHeader), 1, modelFile );

		for (int i=0; i<numberOfModels; ++i)
		{
			if (false == WriteModelGeometry(  modelFile, i ) )
				throw "Failed to save geometry model to cache";
		}
	}
	catch (const char *error)
	{
		printf( "Cache Error - %s\n", error );
	}
	if (modelFile) fclose(modelFile);

	//
	//
	std::string textures_filename( filename );

	auto iter2 = textures_filename.find_last_of( "." );
	textures_filename.erase( iter2 );
	textures_filename.append( "_Textures.pck" );

	return SaveTextures( textures_filename.c_str(), mQuery );

}

bool CGPUCacheSaver::SaveTextures(const char *filename, CGPUCacheSaverQuery *pQuery)
{
	//
	// store samplers/image data
	
	BYTE *localImageBuffer = new BYTE[16384 * 16384 * 4];
	
	int fh=0;

	printf ("try to open a file\n" );

	//
	

	errno_t err = _sopen_s( &fh, filename, _O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC, _SH_DENYRW, _S_IREAD | _S_IWRITE);

	try
	{

		printf ("%d %d\n", err, fh );
		if ( err != 0)
			throw std::exception("Failed to open textures file for writing\n");

		_lseeki64( fh, 0, 0 );

		const int numberOfMedias = pQuery->GetVideoCount();
		const int numberOfSamplers = pQuery->GetSamplersCount();

		// !!!
		// added version 2 format - support image sequences

		FileTexturesHeader texHeader;
		FileTexturesHeader::Set( 2, numberOfMedias, numberOfSamplers, texHeader );
		texHeader.imagesOffset = 0;
		texHeader.samplersOffset = 0;

		int bytteswritten = _write( fh, &texHeader, sizeof(texHeader) );
		
		if (bytteswritten != sizeof(texHeader))
			throw std::exception("Failed to save texture file header");

		// STORE image data first of all
		//

		texHeader.imagesOffset = _telli64(fh);

		for (int i=0; i<numberOfMedias; ++i)
		{
			const char *szVideoName = pQuery->GetVideoName(i);
			printf( "save video data %d - %s\n", i, szVideoName );

			const char *szFilename = pQuery->GetVideoFilename(i);
			if (szFilename && strstr( (char*)filename, ".dds" ) != nullptr )
			{
				
				if (false == SaveDDSData( fh, filename ) )
					throw std::exception("Failed to save DDS data - ");
	
			}
			else
			{
				// write empty texture
				if (false == SaveImageEmpty( fh ) )
					throw std::exception("Failed to save empty image data - ");
			}
		}
		/*
		if (canDoCompression == false)
		{
			FBMessageBox( "Caching export", "Textures were saved without compression, not enough memory!", "Ok" );
		}
		*/
		// STORE Samplers
		//
		
		texHeader.samplersOffset = _telli64(fh);

		for (int i=0; i<numberOfSamplers; ++i)
		{
			const int videoId = pQuery->GetSamplerVideoIndex(i);

			if (false == SaveSampler( fh, i, videoId ) )
				throw std::exception("Failed to save sampler for texture\n");
		}

		// rewrite header with offsets
		_lseeki64(fh, 0, 0);
		
		bytteswritten = _write( fh, &texHeader, sizeof(FileTexturesHeader) );

		if (bytteswritten != sizeof(FileTexturesHeader) )
			throw std::exception("Failed to write textures file header\n");

		printf ( "images offset - %u, samplers offset - %u\n", texHeader.imagesOffset, texHeader.samplersOffset );
	}
	catch (std::exception &e)
	{
		printf( "Cache Error during save operation - %s", e.what() );
	}

	if (fh > 0) _close(fh);

	if (localImageBuffer)
	{
		delete [] localImageBuffer;
		localImageBuffer = nullptr;
	}

	return true;
}

bool CGPUCacheSaver::WriteLightsToXML( TiXmlElement *parentElem )
{
	if (parentElem == nullptr)
		return false;

	int numberOfLights = mQuery->GetLightsCount();
	parentElem->SetAttribute( "count", numberOfLights );
	for (int i=0; i<numberOfLights; ++i)
	{
		TiXmlElement ligItem("Light");
		ligItem.SetAttribute( "name", mQuery->GetLightName(i) );

		parentElem->InsertEndChild( ligItem );
	}

	return true;
}


bool CGPUCacheSaver::SaveDDSData( int fh, const char *filename )
{
	nv_dds::CDDSImage	image;

	if (false == image.load( filename ) )
		return false;

	const short width = (short) image.get_width();
	const short height = (short) image.get_height();

	const GLint internalFormat = image.get_format();
	const GLint format = (image.get_components() == 4) ? GL_RGBA : GL_RGB;

	const GLint imageSize = (GLint) image.get_size();
	const unsigned char numberOfLods = (unsigned char) image.get_num_mipmaps();

	// store header
	ImageHeader2 header;
	ImageHeader2::Set( width, height, internalFormat, format, imageSize, 0, numberOfLods, header);
	
	long long pos = _telli64(fh);
	printf( "current file position - %u\n", pos );

	printf( "writing dds - %d, %d, size %d\n", width, height, imageSize );

	int byttesWritten = 0;

	try
	{
		byttesWritten = _write(fh, &header, sizeof(header) );

		if (byttesWritten != sizeof(header))
			throw std::exception( "error while writing a texture header!\n" );
			
		const unsigned char *imageData = image;

		// store texture
		if (imageSize > 0 && imageData != nullptr)
		{
			byttesWritten = _write( fh, imageData, sizeof(BYTE) * imageSize );
	
			if (byttesWritten != imageSize * sizeof(BYTE))
				throw std::exception( "error while writing a texture data\n" );
		}

		// store lods
		for (int i=0; i<numberOfLods; ++i)
		{

			nv_dds::CSurface mipmap = image.get_mipmap(i);

			// get lod and save it
			ImageLODHeader2	lodHeader;
			ImageLODHeader2::Set( mipmap.get_width(), mipmap.get_height(), mipmap.get_size(), lodHeader );

			byttesWritten = _write( fh, &lodHeader, sizeof(lodHeader) );
			if (byttesWritten != sizeof(lodHeader) )
				throw std::exception( "error while writing a mipmap header!\n" );
				
			//
			const unsigned char *mipmapData = mipmap;
			byttesWritten = _write( fh, mipmapData, mipmap.get_size() );
	
			if (byttesWritten != (int) mipmap.get_size() )
				throw std::exception( "error while writing a mipmap data\n" );
		}
	}
	catch (std::exception &e)
	{
		printf( "%s\n", e.what() );
		return false;
	}

	return true;
}


bool CGPUCacheSaver::SaveSampler( int fh, const int index, const int videoIndex )
{
	mat4 mf;
	mQuery->GetSamplerMatrix(index, mf);

	SamplerHeader header;
	SamplerHeader::Set( mf.mat_array, videoIndex, GL_REPEAT, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, header );

	int byttesWritten = _write( fh, &header, sizeof(SamplerHeader) );
	if (byttesWritten != sizeof(SamplerHeader) )
	{
		printf( "error while writing sampler header\n" );
		return false;
	}

	return true;
}


bool CGPUCacheSaver::WriteTexturesToXML( TiXmlElement *parentItem )
{
	int numberOfSamplers = mQuery->GetSamplersCount();
	parentItem->SetAttribute( "count", numberOfSamplers );

	//
	double totalUncompressedSize = 0.0;
	for (int i=0; i<numberOfSamplers; ++i)
	{
		TiXmlElement texItem("Texture");
		texItem.SetAttribute( "name", mQuery->GetSamplerName(i) );

		const int videoId = mQuery->GetSamplerVideoIndex(i);

		if (videoId >= 0)
		{
			texItem.SetAttribute( "width", mQuery->GetVideoWidth(videoId) );
			texItem.SetAttribute( "height", mQuery->GetVideoHeight(videoId) );
			texItem.SetAttribute( "filename", mQuery->GetVideoFilename(videoId) );
			texItem.SetAttribute( "format", mQuery->GetVideoFormat(videoId) );
			texItem.SetAttribute( "startFrame", mQuery->GetVideoStartFrame(videoId) );
			texItem.SetAttribute( "stopFrame", mQuery->GetVideoStopFrame(videoId) );
			texItem.SetAttribute( "frameRate", mQuery->GetVideoFrameRate(videoId) );

			texItem.SetAttribute( "imageSequence", (mQuery->IsVideoImageSequence(videoId)) ? 1 : 0 );
		}
		parentItem->InsertEndChild( texItem );
	}
	parentItem->SetAttribute( "UncompressedSize", mQuery->GetTotalUncompressedSize() );

	return true;
}

bool CGPUCacheSaver::SaveImageEmpty( int fh )
{
	//
	auto fn_writeSafe = [] (const int fh, void *data, const int size) {

		if (size != _write(fh, data, size ) )
			throw std::exception( "error while writing data!\n" );

	};

	int numberOfFrames = 0;
	int imageSize=0;

	ImageHeader2			header;
	ImageSequenceHeader2	seqHeader;

	__int64 pos = _telli64(fh);
	printf( "current file position - %u\n", pos );
	printf( "writing image - size %d\n", imageSize );

	BYTE imageType = IMAGE_TYPE_STILL;

	try
	{
		imageType = IMAGE_TYPE_STILL;

		GLubyte	*imageData = nullptr;

		fn_writeSafe( fh, &imageType, sizeof(BYTE) );
			
		if (imageData == nullptr)
		{
			header.width = 0;
			header.height = 0;
			header.size = 0;
			header.numberOfLODs = 0;
		}

			
		fn_writeSafe( fh, &header, sizeof(header) );

		// OpenGL texture limitation
		if (imageData != nullptr)
			fn_writeSafe( fh, imageData, sizeof(BYTE) * header.size );
			
	}
	catch (std::exception &e)
	{
		printf( "%s\n", e.what() );
		return false;
	}

	return true;
}



bool CGPUCacheSaver::WriteModelToXML( const int index, TiXmlElement *models )
{
	TiXmlElement modelItem("Model");
	modelItem.SetAttribute( "name", mQuery->GetModelName(index) );
	modelItem.SetAttribute( "visible", mQuery->GetModelVisible(index) );
	modelItem.SetAttribute( "castShadow", mQuery->GetModelCastsShadows(index) );
	modelItem.SetAttribute( "receiveShadow", mQuery->GetModelReceiveShadows(index) );

	vec4 t, r, s, vmin, vmax;

	mQuery->GetModelTranslation(index, t);
	mQuery->GetModelRotation(index, r);
	mQuery->GetModelScaling(index, s);
	mQuery->GetBoundingBox(vmin, vmax);

	TiXmlElement tItem( "Translation" );
	tItem.SetDoubleAttribute( "x", t[0] );
	tItem.SetDoubleAttribute( "y", t[1] );
	tItem.SetDoubleAttribute( "z", t[2] );
		
	TiXmlElement rItem( "Rotation" );
	rItem.SetDoubleAttribute( "x", r[0] );
	rItem.SetDoubleAttribute( "y", r[1] );
	rItem.SetDoubleAttribute( "z", r[2] );

	TiXmlElement sItem( "Scaling" );
	sItem.SetDoubleAttribute( "x", s[0] );
	sItem.SetDoubleAttribute( "y", s[1] );
	sItem.SetDoubleAttribute( "z", s[2] );
		
	modelItem.InsertEndChild( tItem );
	modelItem.InsertEndChild( rItem );
	modelItem.InsertEndChild( sItem );


	TiXmlElement bbItem( "BoundingBox" );
	bbItem.SetDoubleAttribute( "minx", vmin[0] );
	bbItem.SetDoubleAttribute( "miny", vmin[1] );
	bbItem.SetDoubleAttribute( "minz", vmin[2] );
	bbItem.SetDoubleAttribute( "maxx", vmax[0] );
	bbItem.SetDoubleAttribute( "maxy", vmax[1] );
	bbItem.SetDoubleAttribute( "maxz", vmax[2] );

	modelItem.InsertEndChild( bbItem );

	// store sub-patches
	TiXmlElement patches( "Patches" );

	
	const int subpatchcount = mQuery->GetModelSubPatchCount(index);
	for (int j=0; j<subpatchcount; ++j)
	{
		int offset, size, materialId;
		mQuery->GetModelSubPatchInfo(index, j, offset, size, materialId); 

		TiXmlElement patchItem( "Patch" );
		patchItem.SetAttribute( "offset", offset );
		patchItem.SetAttribute( "size", size );
		patchItem.SetAttribute( "material", (materialId >= 0) ? mQuery->GetMaterialName(materialId) : "None" );
		patchItem.SetAttribute( "materialId", index );

		patches.InsertEndChild( patchItem );
	}
	modelItem.SetAttribute( "vertices", mQuery->GetModelVertexCount(index) );
	modelItem.InsertEndChild( patches );

	// store shaders
	TiXmlElement shaders( "Shaders" );
	const int numberOfShaders = mQuery->GetModelShadersCount(index);
	for (int j=0; j<numberOfShaders; ++j)
	{
		const int shaderId = mQuery->GetModelShaderId(index, j);

		TiXmlElement shaderItem( "Shader" );
		shaderItem.SetAttribute( "name", mQuery->GetShaderName(shaderId) );
		shaderItem.SetAttribute( "shaderId", shaderId );
		shaders.InsertEndChild(shaderItem);
	}
	modelItem.InsertEndChild( shaders );

	models->InsertEndChild( modelItem );

	return true;
}

bool CGPUCacheSaver::WriteModelGeometry( FILE *modelFile, const int index )
{	
//bool CGPUVertexData::SaveModel( FILE *fp, FBModelVertexData *pVertexData, FBMatrix *tm )

	const bool afterDeform = true;
	const int numberOfVertices = mQuery->GetModelVertexCount(index); // pVertexData->GetVertexCount();

	mQuery->ModelVertexArrayRequest(index);

	const float *pVertices	= mQuery->GetModelVertexArrayPoint( afterDeform );
	const float *pNormals	= mQuery->GetModelVertexArrayNormal( afterDeform );
	const float *pTangents	= mQuery->GetModelVertexArrayTangent( afterDeform );
	const float *pUVs		= mQuery->GetModelVertexArrayUV( 0, afterDeform );
	const int *indices		= mQuery->GetModelIndexArray();

	mQuery->ModelVertexArrayRelease();

	
	int pointStride =	(pVertices) ? mQuery->GetModelVertexArrayPointStride(index) : 0;
	int normalStride =	(pNormals) ?  mQuery->GetModelVertexArrayNormalStride(index) : 0;
	int tangentStride =	(pTangents) ?  mQuery->GetModelVertexArrayTangentStride(index) : 0;
	int uvStride =		(pUVs) ?  mQuery->GetModelVertexArrayUVStride(index) : 0;
	//const int indexStride = (indices) ? sizeof(unsigned int) : 0;

	int numberOfIndices = 0;

	for (int i=0; i<mQuery->GetModelSubPatchCount(index); ++i)
	{
		int offset, size, matId;
		mQuery->GetModelSubPatchInfo( index, i, offset, size, matId );
		
		numberOfIndices = std::max( numberOfIndices, offset+size );
	}

	// we have strong specified strides for arrays
	if (pointStride != gPointStride) pointStride = 0;
	if (normalStride != gNormalStride) normalStride = 0;
	if (tangentStride != gTangentStride) tangentStride = 0;
	if (uvStride != gUVStride) uvStride = 0;

	// store header
	VertexDataHeader header;
	VertexDataHeader::Set( numberOfVertices, numberOfIndices, pointStride, normalStride, tangentStride, uvStride, header );
	
	size_t elementsWritten = 0;

	try
	{
		long long pos = _ftelli64(modelFile);
		elementsWritten = fwrite( &header, sizeof(header), 1, modelFile );
		if (elementsWritten != 1)
			throw std::exception( "ERROR: failed to write model header!\n" );

		float *pNewArray = (float*) pVertices;
		/*
		if (tm)
		{
			pNewArray = new FBVertex[numberOfVertices];

			for (int i=0; i<numberOfVertices; ++i)
			{
				FBVertexMatrixMult( pNewArray[i], *tm, FBVertex(pVertices[i][0], pVertices[i][1], pVertices[i][2], 1.0) );
				pNewArray[i][3] = 1.0;
			}
		
		}
		*/
		//
		// store data into the file storage
		header.positionOffset = _ftelli64(modelFile);
		elementsWritten = fwrite( (pNewArray) ? pNewArray : pVertices, pointStride, numberOfVertices, modelFile );
		
		//SAFE_ARRAY_DELETE(pNewArray);
		
		if (elementsWritten != numberOfVertices)
			throw std::exception( "ERROR: failed to write vertices!\n" );
		
		header.normalOffset = _ftelli64(modelFile);
		elementsWritten = fwrite( pNormals, normalStride, numberOfVertices, modelFile );
		if (elementsWritten != numberOfVertices)
			throw std::exception( "ERROR: failed to write vertices!\n" );
		
		header.tangentOffset = _ftelli64(modelFile);
		elementsWritten = fwrite( pTangents, tangentStride, numberOfVertices, modelFile );
		if (elementsWritten != numberOfVertices)
			throw std::exception( "ERROR: failed to write vertices!\n" );
			
		header.uvOffset = _ftelli64(modelFile);
		elementsWritten = fwrite( pUVs, uvStride, numberOfVertices, modelFile );
		if (elementsWritten != numberOfVertices)
			throw std::exception( "ERROR: failed to write vertices!\n" );
			

		unsigned int *newIndices = new unsigned int[numberOfIndices];
		if (newIndices == nullptr)
			throw std::exception( "Failed to allocate memoryfor indices\n" );

		for (int i=0; i<numberOfIndices; ++i)
			newIndices[i] = (unsigned int) indices[i];

		header.indicesOffset = _ftelli64(modelFile);
		elementsWritten = fwrite( newIndices, sizeof(unsigned int), numberOfIndices, modelFile );

		if(newIndices)
		{
			delete [] newIndices;
			newIndices = nullptr;
		}

		if (elementsWritten != numberOfIndices)
			throw std::exception( "ERROR: failed to write indices!\n" );

		// now store header with offsets
		header.endOffset = _ftelli64(modelFile);
		_fseeki64(modelFile, pos, 0);
		elementsWritten = fwrite( &header, sizeof(header), 1, modelFile );
		if (elementsWritten != 1)
			throw std::exception( "ERROR: failed to write model header!\n" );

		_fseeki64(modelFile, header.endOffset, 0);
	}
	catch (const std::exception &e)
	{
		printf( "%s\n", e.what() );
		return false;
	}

	return true;

}


bool CGPUCacheSaver::WriteShadersToXML( TiXmlElement *parentElem )
{
	const int count = mQuery->GetShadersCount();
	parentElem->SetAttribute( "count", count );
	for (int i=0; i<count; ++i)
	{
		TiXmlElement shdItem("Shader");
		WriteOneShaderToXML( &shdItem, i );

		parentElem->InsertEndChild( shdItem );
	}
	return true;
}

bool CGPUCacheSaver::WriteOneShaderToXML( TiXmlElement *shdItem, const int index )
{
	shdItem->SetAttribute( "name", mQuery->GetShaderName(index) );

	ShaderGLSL data;
	mQuery->ConvertShader(index, data);

	// transparency mode
	int alphaSource = mQuery->GetShaderAlphaSource(index);
	shdItem->SetAttribute( "alpha", alphaSource );
	shdItem->SetDoubleAttribute( "transparency", (double) data.transparency );
	shdItem->SetAttribute( "type", data.shaderType );

	// additional properties
	switch(data.shaderType)
	{
	case eShaderTypeSuperLighting:
		{
			TiXmlElement colorCorrItem("ColorCorrection");
		
			TiXmlElement customColorItem("CustomColor");
			customColorItem.SetDoubleAttribute( "r", data.customColor.x );
			customColorItem.SetDoubleAttribute( "g", data.customColor.y );
			customColorItem.SetDoubleAttribute( "b", data.customColor.z );
		
			colorCorrItem.InsertEndChild( customColorItem );
		
			colorCorrItem.SetAttribute( "blendType", (int) data.customColor.w );
			colorCorrItem.SetDoubleAttribute( "contrast", (double) data.contrast );
			colorCorrItem.SetDoubleAttribute( "saturation", (double) data.saturation );
			colorCorrItem.SetDoubleAttribute( "brightness", (double) data.brightness );
			colorCorrItem.SetDoubleAttribute( "gamma", (double) data.gamma );
		
			shdItem->InsertEndChild( colorCorrItem );

			// Toon settings
			TiXmlElement shadingItem( "Shading" );
			shadingItem.SetAttribute( "type", (int) data.shadingType );
			shadingItem.SetAttribute( "toonEnabled", 0 );
			shadingItem.SetDoubleAttribute( "toonSteps", data.toonSteps );
			shadingItem.SetDoubleAttribute( "toonDistribution", data.toonDistribution );
			shadingItem.SetDoubleAttribute( "toonShadowPosition", data.toonShadowPosition );
		
			shdItem->InsertEndChild( shadingItem );
		} break;

	case eShaderTypeColorCorrection:
		{
			TiXmlElement colorCorrItem("ColorCorrection");
		
			TiXmlElement customColorItem("CustomColor");
			customColorItem.SetDoubleAttribute( "r", data.customColor.x );
			customColorItem.SetDoubleAttribute( "g", data.customColor.y );
			customColorItem.SetDoubleAttribute( "b", data.customColor.z );
		
			colorCorrItem.InsertEndChild( customColorItem );
		
			colorCorrItem.SetAttribute( "blendType", (int) data.customColor.w );
			colorCorrItem.SetDoubleAttribute( "contrast", (double) data.contrast );
			colorCorrItem.SetDoubleAttribute( "saturation", (double) data.saturation );
			colorCorrItem.SetDoubleAttribute( "brightness", (double) data.brightness );
			colorCorrItem.SetDoubleAttribute( "gamma", (double) data.gamma );
		
			shdItem->InsertEndChild( colorCorrItem );

		} break;

	case eShaderTypeShading:
		{
			// Toon settings
			TiXmlElement shadingItem( "Shading" );
			shadingItem.SetAttribute( "type", (int) data.shadingType );
			shadingItem.SetAttribute( "toonEnabled", 0 );
			shadingItem.SetDoubleAttribute( "toonSteps", data.toonSteps );
			shadingItem.SetDoubleAttribute( "toonDistribution", data.toonDistribution );
			shadingItem.SetDoubleAttribute( "toonShadowPosition", data.toonShadowPosition );
		
			shdItem->InsertEndChild( shadingItem );
		} break;
	}

	return true;
}

bool CGPUCacheSaver::WriteMaterialsToXML( TiXmlElement *parentElem )
{
	const int numberOfMaterials = mQuery->GetMaterialsCount();
	parentElem->SetAttribute( "count", numberOfMaterials );

	for ( int i=0; i<numberOfMaterials; ++i )
	{
		MaterialGLSL data;
		mQuery->ConvertMaterial(i, data);

		TiXmlElement matItem("Material");
		matItem.SetAttribute( "name", mQuery->GetMaterialName(i) );

		TiXmlElement difItem("Diffuse");
		difItem.SetDoubleAttribute( "r", (double) data.diffuseColor.x );
		difItem.SetDoubleAttribute( "g", (double) data.diffuseColor.y );
		difItem.SetDoubleAttribute( "b", (double) data.diffuseColor.z );
		difItem.SetDoubleAttribute( "factor", (double) data.diffuseColor.w );

		difItem.SetAttribute( "map", (data.diffuse>=0) ? mQuery->GetSamplerName(data.diffuse) : "None" );
		difItem.SetAttribute( "mapId", data.diffuse );

		matItem.InsertEndChild(difItem);

		parentElem->InsertEndChild( matItem );
	}

	return true;
}