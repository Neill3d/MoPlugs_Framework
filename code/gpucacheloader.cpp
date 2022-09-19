
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_loader.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucacheloader.h"
#include "IO\tinyxml.h"


#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <share.h>
#include <string>

#include <vector>

///////////////////////////////////

#define LOG_LEVEL_INFO		1
#define	LOG_LEVEL_WARNING	2
#define LOG_LEVEL_ERROR		3
#define LOG_LEVEL_NONE		4

#define LOADER_LOG_PRINTF	3

///////////////////////////////////////////////////////////////////////////////////////////////////
// GPUCacheLoader

CGPUCacheLoader::CGPUCacheLoader()
{
	mVisitor = nullptr;
}

//
#define SAFE_READ(fh, data, size) \
		if (size != _read(fh, data, size ) ) \
			throw std::exception( "error while reading data!\n" );

int CalculateImageSize( BYTE *stream, size_t &imageSize )
{
	// retrieve a header
	ImageHeader			*header=nullptr;
	ImageLODHeader		*lodHeader=nullptr;
	
	BYTE				*lstream = stream;
	imageSize = 0;

	try
	{
		header = (ImageHeader*) lstream;
		imageSize += sizeof(ImageHeader);

		if (header->size <= 0 || header->height <= 0 || header->width <= 0)		
			throw std::exception(" empty texture !" );
		
		// skip main image
		imageSize += sizeof(BYTE) * header->size;
		
		for (int i=0; i<header->numberOfLODs; ++i)
		{
			lstream = stream + imageSize;
			lodHeader = (ImageLODHeader*) lstream;

			imageSize += sizeof(ImageLODHeader);
			imageSize += sizeof(BYTE) * lodHeader->size;
		}
		
	}
	catch (std::exception &e)
	{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
		printf("Read texture size - %s\n", e.what() );
#endif
		return 0;
	}

	return 1;
}

// calculate size for the second file version
int CalculateImageSize2( BYTE *stream, size_t &imageSize )
{
	// retrieve a header
	BYTE					*imageType=nullptr;
	ImageHeader2			*header=nullptr;
	ImageLODHeader2			*lodHeader=nullptr;
	ImageSequenceHeader2	*seqHeader=nullptr;
	
	BYTE				*lstream = stream;
	
	imageSize = 0;
	
	try
	{
		imageType = stream;
		imageSize += sizeof(BYTE);

		if (*imageType == IMAGE_TYPE_SEQUENCE)
		{
			lstream = stream + imageSize;
			seqHeader = (ImageSequenceHeader2*) lstream;
			
			// total size for storing all sequence information
			imageSize += sizeof(ImageSequenceHeader2);
			imageSize += seqHeader->size;
		}
		else if (*imageType == IMAGE_TYPE_STILL)
		{
			lstream = stream + imageSize;
			header = (ImageHeader2*) lstream;
			imageSize += sizeof(ImageHeader2);

			if (header->size <= 0 || header->height <= 0 || header->width <= 0)		
				throw std::exception(" empty texture !" );
		
			// skip main image
			imageSize += sizeof(BYTE) * header->size;
		
			// !! NOTE: first LOD is the main image
			for (int i=1; i<header->numberOfLODs; ++i)
			{
				lstream = stream + imageSize;
				lodHeader = (ImageLODHeader2*) lstream;

				imageSize += sizeof(ImageLODHeader2);
				imageSize += sizeof(BYTE) * lodHeader->size;
			}
		}
	}
	catch (std::exception &e)
	{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
		printf("Read texture size - %s\n", e.what() );
#endif
		return 0;
	}

	return 1;
}

void ConvertImageHeader( ImageHeader &src, ImageHeader2 &dst )
{
	ImageHeader2::Set( src.width, src.height, src.internalFormat, src.format, src.size, 0, src.numberOfLODs, dst );
}

size_t CalculateSamplerSize( BYTE *stream )
{
	return sizeof(SamplerHeader);
}

#define SAFE_CALL(_POINTER, _METHOD) \
	if (_POINTER) { _METHOD; }

bool CGPUCacheLoader::ReadTextures(const char *textures_filename, TiXmlElement *parentElem)
{

	TiXmlAttribute  *attrib = nullptr;
	int xmlNumberOfTextures = 0;

	// enumerate attribs and find name for each sampler !
	for( attrib = parentElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if ( strcmp(attrib->Name(), "count") == 0 )
		{
			xmlNumberOfTextures = attrib->IntValue();
			
		}
	}

	//
	if (xmlNumberOfTextures == 0)
		return true;

	//
	// read texture names
	std::vector<std::string>	names;
	std::vector<std::string>	filenames;
	names.resize(xmlNumberOfTextures);
	filenames.resize(xmlNumberOfTextures);

	TiXmlElement *texelem = parentElem->FirstChildElement( "Texture" );

	int index=0;
	while(texelem)
	{

		// enumerate attribs
		for( attrib = texelem->FirstAttribute();
				attrib;
				attrib = attrib->Next() )
		{
			if ( strcmp(attrib->Name(), "name") == 0 )
			{
				names[index] = attrib->Value();
			}
			else if (strcmp(attrib->Name(), "filename") == 0)
			{
				filenames[index] = attrib->Value();
			}
		}

		index++;
		texelem = texelem->NextSiblingElement();
	}

	// 2 - for each texture read it's content in textures package
	int fh = 0;

	fh = _open( textures_filename, _O_BINARY | _O_RDONLY, 0 );

	try
	{
		if (fh == -1)
			throw std::exception("FAILED to read textures package!");

		_lseeki64( fh, 0, SEEK_END );
		size_t		totalFileSize = _telli64(fh);
		size_t		totalFilePos = 0;

		// pre-cache all images pack into memory

		std::vector<BYTE>	fileCache(totalFileSize);

		if (fileCache.size() == 0)
			throw std::exception( "NOT ENOUGH MEMORY FOR READING TEXTURES" );

		_lseeki64( fh, 0, SEEK_SET );
		SAFE_READ( fh, fileCache.data(), sizeof(BYTE)*totalFileSize );

		if (fh > 0) 
		{
			_close(fh);
			fh = 0;
		}

		// read global pack header
		FileTexturesHeader	*fileHeader = (FileTexturesHeader*) fileCache.data();
		
		if (fileHeader->imagesOffset == 0 || fileHeader->samplersOffset == 0)
			throw std::exception("CORRUPTED FILE FORMAT");

		//
		bool proceed = false;

		if (mVisitor)
			proceed = mVisitor->OnReadTexturesBegin( textures_filename, fileHeader->numberOfSamplers, fileHeader->numberOfImages);

		if (proceed == false)
		{
			return true;
		}

		// 2 - load image data

		totalFilePos = fileHeader->imagesOffset;
		
		// calculate image size, cache from disk and transfer to visitor
		
		size_t fileOffset = 0;

		int successImages = 0;
		for (int i=0; i<fileHeader->numberOfImages; ++i)
		{
			ImageHeader		*imageHeader1=nullptr;
			ImageHeader2	*imageHeader2=nullptr;
			size_t imageSize = 0;

#if LOADER_LOG_PRINTF <= LOG_LEVEL_INFO
			printf ("process image %d\n", i );
#endif
			fileOffset = totalFilePos;

			switch(fileHeader->version)
			{
			case 1:

				if (0 == CalculateImageSize( fileCache.data() + fileOffset, imageSize ) )
				{
					totalFilePos += imageSize;

					if (mVisitor)
						mVisitor->OnReadEmptyImage();
					break;
				}

				// read full image data
				totalFilePos += imageSize;
				imageHeader1 = (ImageHeader*) (fileCache.data() + fileOffset);
				
				if (mVisitor)
					mVisitor->OnReadTexturesImage1( imageHeader1, fileOffset, imageSize, fileCache.data()+fileOffset );

				//
				successImages++;
				break;
			case 2:

				if (0 == CalculateImageSize2( fileCache.data() + fileOffset, imageSize ) )
				{
					totalFilePos += imageSize;

					if (mVisitor)
						mVisitor->OnReadEmptyImage();
					break;
				}

				// read full image data
				totalFilePos += imageSize;	
				imageHeader2 = (ImageHeader2*) (fileCache.data()+sizeof(BYTE)+fileOffset);
			
				if (mVisitor)
				{
					BYTE *poffset = fileCache.data()+fileOffset;
					mVisitor->OnReadTexturesImage2( *poffset, imageHeader2, fileOffset, imageSize, poffset );
				}

				//
				successImages++;
				break;
			}
		}
#if LOADER_LOG_PRINTF <= LOG_LEVEL_INFO
		printf( "succesful %d of %d images\n", successImages, fileHeader.numberOfImages );
#endif

		//
		// 3 - load samplers
		SamplerHeader	*samplerHeader = nullptr;
		size_t			samplerSize = 0;

		totalFilePos = fileHeader->samplersOffset;

		for (int i=0; i<fileHeader->numberOfSamplers; ++i)
		{
			fileOffset = totalFilePos;
			samplerSize = CalculateSamplerSize( fileCache.data() + fileOffset );

			if (samplerSize == 0)
				break;

			// read full image data
			totalFilePos += samplerSize;
			samplerHeader = (SamplerHeader*) (fileCache.data() + fileOffset);
				
			if (mVisitor)
				mVisitor->OnReadTexturesSampler( names[i].c_str(), filenames[i].c_str(), samplerHeader, fileOffset, samplerSize, fileCache.data()+fileOffset );
		}
	}
	catch (std::exception &e)
	{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
		printf("ERROR in reading textures = %s\n", e.what() );
#endif

		if (fh > 0) _close(fh);

		if (mVisitor)
			mVisitor->OnReadTexturesError( e.what() );

		return false;
	}

	if (fh > 0) 
		_close(fh);

	if (mVisitor)
		mVisitor->OnReadTexturesEnd();

	return true;
}


void CGPUCacheLoader::EmptyMaterial( MaterialGLSL &mat )
{
	memset( &mat, 0, sizeof(MaterialGLSL) );
}

void CGPUCacheLoader::ConstructMaterialFromXML( TiXmlElement *matelem, MaterialGLSL &mat )
{
	TiXmlAttribute  *attrib = nullptr;
	TiXmlElement	*diffuse = nullptr;

	EmptyMaterial( mat );
	
	diffuse = matelem->FirstChildElement( "Diffuse" );
	if (diffuse)
	{
		// enumerate attribs
		for( attrib = diffuse->FirstAttribute();
				attrib;
				attrib = attrib->Next() )
		{
			if ( strcmp(attrib->Name(), "r") == 0 )
			{
				mat.diffuseColor[0] = (float) attrib->DoubleValue();
			}
			else if ( strcmp(attrib->Name(), "g") == 0 )
			{
				mat.diffuseColor[1] = (float) attrib->DoubleValue();
			}
			else if ( strcmp(attrib->Name(), "b") == 0 )
			{
				mat.diffuseColor[2] = (float) attrib->DoubleValue();
			}
			else if ( strcmp(attrib->Name(), "factor") == 0 )
			{
				mat.diffuseColor[3] = (float) attrib->DoubleValue();
			}
			else if ( strcmp(attrib->Name(), "mapId") == 0 )
			{
				mat.diffuse = attrib->IntValue();
				mat.useDiffuse = (mat.diffuse >= 0) ? 1.0f : 0.0f;
			}
		}
	}
}

bool CGPUCacheLoader::ReadMaterials(TiXmlElement *parentElem)
{
	if (mVisitor == nullptr)
	{
		return false;
	}

	TiXmlAttribute  *attrib = nullptr;
	int numberOfMaterials = 0;

	// enumerate attribs
	for( attrib = parentElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if ( strcmp(attrib->Name(), "count") == 0 )
		{
			numberOfMaterials = attrib->IntValue();
		}
	}

	if (numberOfMaterials == 0)
		return true;

	bool proceed = mVisitor->OnReadMaterialsBegin(numberOfMaterials);

	if (proceed == false)
		return true;

	TiXmlElement *matelem = parentElem->FirstChildElement( "Material" );

	MaterialGLSL	materialData;
	std::string		materialName("");
	bool			catchname = false;

	while(matelem)
	{
		catchname = false;
		ConstructMaterialFromXML( matelem, materialData );

		// enumerate attribs
		for( attrib = matelem->FirstAttribute();
				attrib;
				attrib = attrib->Next() )
		{
			if ( strcmp(attrib->Name(), "name") == 0 )
			{
				catchname = true;
				materialName = attrib->Value();
			}
		}

		mVisitor->OnReadMaterial( (catchname) ? materialName.c_str() : "default", materialData );

		matelem = matelem->NextSiblingElement();
	}


	mVisitor->OnReadMaterialsEnd();
	return true;
}
/*
void ConstructDefaultShader( ShaderGLSL &shader )
{
	shader.shadingType = eShadingTypeDynamic;
	shader.shaderType = eShaderTypeDefault;

	shader.shader1 = -1;
	shader.shader2 = -1;
	shader.shader3 = -1;
	shader.shader4 = -1;
	shader.shader5 = -1;
	
	shader.transparency = 1.0f;

	shader.toonEnabled = 0.0f;
	shader.toonDistribution = 1.0f;
	shader.toonShadowPosition = 0.5f;
	shader.toonSteps = 4.0f;

	shader.customColor = vec4(1.0, 1.0, 1.0, 3.0);

	shader.contrast = 1.0f;
	shader.saturation = 1.0f;
	shader.brightness = 1.0f;
	shader.gamma = 1.0f;

	shader.depthDisplacement = 0.0f;
}
*/
void ConstructShaderFromXML( TiXmlElement *shaderelem, ShaderGLSL &shader )
{
	DefaultShader( shader );
	//ConstructDefaultShader( shader );
	
	// enumerate attribs
	TiXmlAttribute  *attrib = nullptr;
	for( attrib = shaderelem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if (strcmp(attrib->Name(), "transparency") == 0)
		{
			shader.transparency = (float)attrib->DoubleValue();
		}
		else if (strcmp(attrib->Name(), "type") == 0)
		{
			shader.shaderType = (EShaderType) attrib->IntValue();
		}
	}

	TiXmlElement *shadingElem = shaderelem->FirstChildElement( "Shading" );
	if (shadingElem)
	{
		for( attrib = shadingElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
		{
			if (strcmp(attrib->Name(), "type") == 0)
			{
				shader.shadingType = (int) attrib->IntValue();
			}
			else if (strcmp(attrib->Name(), "toonEnabled") == 0)
			{
				shader.toonEnabled = (float) attrib->IntValue();
			}
			else if (strcmp(attrib->Name(), "toonSteps") == 0)
			{
				shader.toonSteps = (float) attrib->DoubleValue();
			}
			else if (strcmp(attrib->Name(), "toonDistribution") == 0)
			{
				shader.toonDistribution = (float) attrib->DoubleValue();
			}
			else if (strcmp(attrib->Name(), "toonShadowPosition") == 0)
			{
				shader.toonShadowPosition = (float) attrib->DoubleValue();
			}
		}
	}

	TiXmlElement *colorCorrectionElem = shaderelem->FirstChildElement( "ColorCorrection" );
	if (colorCorrectionElem)
	{
		TiXmlElement *customColorElem = colorCorrectionElem->FirstChildElement( "CustomColor" );
		if (customColorElem)
		{
			for( attrib = customColorElem->FirstAttribute();
				attrib;
				attrib = attrib->Next() )
			{
				if (strcmp(attrib->Name(), "r") == 0)
				{
					shader.customColor[0] = (float) attrib->DoubleValue();
				}
				else if (strcmp(attrib->Name(), "g") == 0)
				{
					shader.customColor[1] = (float) attrib->DoubleValue();
				}
				else if (strcmp(attrib->Name(), "b") == 0)
				{
					shader.customColor[2] = (float) attrib->DoubleValue();
				}
			}
		}

		for( attrib = colorCorrectionElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
		{
			if (strcmp(attrib->Name(), "blendType") == 0)
			{
				shader.customColor[3] = (float) attrib->IntValue();
			}
			else if (strcmp(attrib->Name(), "contrast") == 0)
			{
				shader.contrast = 1.0f + 0.01f * (float) attrib->DoubleValue();
			}
			else if (strcmp(attrib->Name(), "saturation") == 0)
			{
				shader.saturation = 1.0f + 0.01f * (float) attrib->DoubleValue();
			}
			else if (strcmp(attrib->Name(), "brightness") == 0)
			{
				shader.brightness = 1.0f + 0.01f * (float) attrib->DoubleValue();
			}
			else if (strcmp(attrib->Name(), "gamma") == 0)
			{
				shader.gamma = 0.01f * (float) attrib->DoubleValue();
			}
		}
	}

	shader.applyColorCorrection = 0.0f;

	if ( 3 != (int) shader.customColor[3] )
		shader.applyColorCorrection = 1.0f;
	else if (shader.customColor[0] != 1.0f || shader.customColor[1] != 1.0f || shader.customColor[2] != 1.0f)
		shader.applyColorCorrection = 1.0f;
	else if (shader.contrast != 1.0f || shader.brightness != 1.0f || shader.saturation != 1.0f)
		shader.applyColorCorrection = 1.0f;
}

bool CGPUCacheLoader::ReadShaders(TiXmlElement *parentElem)
{
	if (mVisitor == nullptr)
	{
		return false;
	}

	TiXmlAttribute  *attrib = nullptr;
	int numberOfShaders = 0;

	// enumerate attribs
	for( attrib = parentElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if ( strcmp(attrib->Name(), "count") == 0 )
		{
			numberOfShaders = attrib->IntValue();
		}
	}

	bool proceed = mVisitor->OnReadShadersBegin(numberOfShaders);
	if (proceed == false)
		return true;

	//

	TiXmlElement *shaderelem = parentElem->FirstChildElement( "Shader" );

	ShaderGLSL		shaderdata;
	std::string		shadername("");
	int				shaderalpha=0;
	bool			catchname=false;

	while(shaderelem)
	{
		catchname = false;
		shaderalpha = 0;
		ConstructShaderFromXML( shaderelem, shaderdata );

		// enumerate attribs
		for( attrib = shaderelem->FirstAttribute();
				attrib;
				attrib = attrib->Next() )
		{
			if ( strcmp(attrib->Name(), "name") == 0 )
			{
				catchname = true;
				shadername = attrib->Value();
			}
			else
				if ( strcmp(attrib->Name(), "alpha") == 0 )
				{
					shaderalpha = attrib->IntValue();
				}
		}

		mVisitor->OnReadShader( (catchname) ? shadername.c_str() : "default", shaderalpha, shaderdata);
		
		shaderelem = shaderelem->NextSiblingElement();
	}

	mVisitor->OnReadShadersEnd();

	return true;
}

bool CGPUCacheLoader::Load(const char *filename, CGPUCacheLoaderVisitor *pVisitor)
{
	TiXmlDocument	doc;

	TiXmlNode *node = nullptr;
	TiXmlElement *headerElem = nullptr;
	TiXmlElement *texturesElem = nullptr;
	TiXmlElement *materialsElem = nullptr;
	TiXmlElement *shadersElem = nullptr;
	TiXmlElement *modelsElem = nullptr;
	TiXmlAttribute  *attrib = nullptr;

	std::string sourceFileName("");

	mVisitor = pVisitor;

	try
	{
		if (doc.LoadFile( filename ) == false)
			throw std::exception( "failed to load cache file" );
	
		// get main header attributes
		node = doc.FirstChild( "Header" );
	
		if (node)
		{
			headerElem = node->ToElement();

			if (headerElem)
			{
				// enumerate attribs
				for( attrib = headerElem->FirstAttribute();
						attrib;
						attrib = attrib->Next() )
				{
					if ( strcmp(attrib->Name(), "filename") == 0 )
					{
						sourceFileName = attrib->Value();
					}
				}
			}
		}

		//
		if (mVisitor)
			mVisitor->OnReadHeader(filename, sourceFileName.c_str() );

		//
		//
		
		node = doc.FirstChild("Textures");
		if (node == nullptr)
			throw std::exception("Failed to find Textures group in cache");

		texturesElem = node->ToElement();
		if (texturesElem)
		{
			//
			std::string textures_filename( filename );

			auto iter = textures_filename.find_last_of( "." );
			textures_filename.erase( iter );
			textures_filename.append( "_Textures.pck" );

			if (false == ReadTextures( textures_filename.c_str(), texturesElem ) )
				throw std::exception( "Failed to read textures" );
		}

		//
		//

		node = doc.FirstChild("Materials");
		if (node == nullptr)
			throw std::exception( "Failed to find materials group in cache" );

		materialsElem = node->ToElement();
		if (materialsElem)
		{
			if (false == ReadMaterials(materialsElem) )
				throw std::exception( "Failed to read materials" );
		}

		//
		//

		node = doc.FirstChild("Shaders");
		if (node == nullptr)
			throw std::exception( "Failed to find shaders group in cache" );

		shadersElem = node->ToElement();
		if (shadersElem)
		{
			ReadShaders( shadersElem );
		}
		
		//
		//

		node = doc.FirstChild("Models");
		if (node == nullptr)
			throw std::exception( "failed to find models group in cache" );

		modelsElem = node->ToElement();
		if (modelsElem)
		{
			std::string		geometry_filename( filename );

			auto iter = geometry_filename.find_last_of( "." );
			geometry_filename.erase( iter );
			geometry_filename.append( "_Geometry.pck" );

			if (false == ReadModels( geometry_filename.c_str(), modelsElem ) )
				throw std::exception( "failed to load model render from xml" );
		}


	}
	catch (const std::exception &e)
	{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
		printf ( "%s\n", e.what() );
#endif
		return false;
	}

	return true;
}

void ReadBoundingBoxFromXML(TiXmlElement *elem, double *lmin, double *lmax)
{
	TiXmlElement *bbElem = elem->FirstChildElement( "BoundingBox" );
	if (bbElem == nullptr)
	{
		return;
	}
	// enumerate attribs
	for( TiXmlAttribute *attrib = bbElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if ( strcmp(attrib->Name(), "minx") == 0 )
		{
			lmin[0] = attrib->DoubleValue();	
		}
		else if ( strcmp(attrib->Name(), "miny") == 0 )
		{
			lmin[1] = attrib->DoubleValue();	
		}
		else if ( strcmp(attrib->Name(), "minz") == 0 )
		{
			lmin[2] = attrib->DoubleValue();	
		}
		else if ( strcmp(attrib->Name(), "maxx") == 0 )
		{
			lmax[0] = attrib->DoubleValue();	
		}
		else if ( strcmp(attrib->Name(), "maxy") == 0 )
		{
			lmax[1] = attrib->DoubleValue();	
		}
		else if ( strcmp(attrib->Name(), "maxz") == 0 )
		{
			lmax[2] = attrib->DoubleValue();	
		}
	}
}

bool CGPUCacheLoader::ReadModels(const char *geometry_filename, TiXmlElement *parentElem)
{
	if (mVisitor == nullptr)
	{
		return false;
	}

	int numberOfSubModels = 0;
	int numberOfSubMeshes = 0;

	TiXmlAttribute  *attrib = nullptr;

	// enumerate attribs
	for( attrib = parentElem->FirstAttribute();
			attrib;
			attrib = attrib->Next() )
	{
		if ( strcmp(attrib->Name(), "count") == 0 )
		{
			numberOfSubModels = attrib->IntValue();
		}
		else
		if ( strcmp(attrib->Name(), "submeshes") == 0 )
		{
			numberOfSubMeshes = attrib->IntValue();
		}
	}

	if (numberOfSubModels == 0)
		return true;

	double bmin[3] = {0.0, 0.0, 0.0};
	double bmax[3] = {0.0, 0.0, 0.0};

	ReadBoundingBoxFromXML( parentElem, bmin, bmax );
	
	bool proceed = mVisitor->OnReadModelsBegin( numberOfSubModels, numberOfSubMeshes, bmin, bmax );

	if (proceed == false)
		return true;

	// DONE: load models
	FILE *fp = nullptr;

	try
	{

		errno_t err = fopen_s( &fp, geometry_filename, "rb" );
		if (err != 0)
			throw std::exception( "failed to read geometry package" );
			
		_fseeki64(fp, 0, SEEK_END);
		__int64 fileLen = _ftelli64(fp);

		if (fileLen == 0)
			throw std::exception( "geometry file length is zero!" );
			
		std::vector<BYTE>	geomcache(fileLen);

		if (geomcache.size() == 0)
			throw std::exception( "not enough memory for the geom file" );
			
		_fseeki64(fp, 0, SEEK_SET);
		if (fileLen != fread_s( geomcache.data(), fileLen, sizeof(BYTE), fileLen, fp ) )
			throw std::exception( "failed to read all geom file data!" );
			
		if (fp) fclose(fp);

		//
		// !!! check if package has the same number of models with xml

		FileGeometryHeader *pHeader = (FileGeometryHeader*) geomcache.data();

		if (pHeader->numberOfModels != numberOfSubModels)
			throw std::exception( "geometry package is not conform the xml information" );

		mVisitor->OnReadVertexData( pHeader, geomcache.data() );

		//
		//

		TiXmlElement *shaders = nullptr;
		TiXmlElement *shaderElem = nullptr;
		TiXmlElement *patches = nullptr;
		TiXmlElement *patchElem = nullptr;
		TiXmlElement *modelElem = nullptr;
		TiXmlElement *tElem, *rElem, *sElem;

		VertexDataHeader *pVertexHeader = (VertexDataHeader*) (geomcache.data() + sizeof(FileGeometryHeader));

		
		std::string						modelname("");
		bool							catchname = false;

		double tr[3];
		double rt[3];
		double scl[3];

		modelElem = parentElem->FirstChildElement( "Model" );
		while( modelElem )
		{
			catchname = false;

			// enumerate attribs
			for( attrib = modelElem->FirstAttribute();
					attrib;
					attrib = attrib->Next() )
			{
				if ( strcmp(attrib->Name(), "name") == 0 )
				{
					modelname = attrib->Value();
					catchname = true;
				}
			}

			tElem = modelElem->FirstChildElement( "Translation" );
			rElem = modelElem->FirstChildElement( "Rotation" );
			sElem = modelElem->FirstChildElement( "Scaling" );

			auto read_vector_func = [] (TiXmlElement *elem, double *values)
			{
				if (elem)
				{
					for( TiXmlAttribute *attrib = elem->FirstAttribute();
					attrib;
					attrib = attrib->Next() )
					{
						if ( strcmp(attrib->Name(), "x") == 0 ) values[0] = attrib->DoubleValue();
						else if ( strcmp(attrib->Name(), "y") == 0 ) values[1] = attrib->DoubleValue();
						else if ( strcmp(attrib->Name(), "z") == 0 ) values[2] = attrib->DoubleValue();
					}
				}
			};

			read_vector_func( tElem, tr );
			read_vector_func( rElem, rt );
			read_vector_func( sElem, scl );

			// read bb and compute bsphere
			ReadBoundingBoxFromXML(modelElem, bmin, bmax);

			// read shaders used by the model
			
			int	shaderCount = 0;
			int shaderId[MAX_NUMBER_OF_SHADERS_PER_MODEL] = {-1, -1, -1, -1, -1};

			shaders = modelElem->FirstChildElement( "Shaders" );
			if (shaders)
			{
				shaderElem = shaders->FirstChildElement( "Shader" );

				while(shaderElem)
				{
					// enumerate attribs
					for( attrib = shaderElem->FirstAttribute();
							attrib;
							attrib = attrib->Next() )
					{
						if ( strcmp(attrib->Name(), "shaderId") == 0 )
						{
							shaderId[shaderCount] = attrib->IntValue();
							
							// TODO: check why I've commented that before ?!
							shaderId[shaderCount] += 1;	// we have one default shader at the beginning

							shaderCount++;
							if (shaderCount >= MAX_NUMBER_OF_SHADERS_PER_MODEL)
							{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
								printf("ERROR: so many shaders per model is not supported!");
#endif
								shaderCount=MAX_NUMBER_OF_SHADERS_PER_MODEL-1;
							}
						}
					
					}
					shaderElem = shaderElem->NextSiblingElement("Shader");
				}
			}
	
			//
			mVisitor->OnReadModel( (catchname) ? modelname.c_str() : "default", 
									tr, 
									rt, 
									scl, 
									bmin, 
									bmax, 
									shaderCount, 
									shaderId, 
									pVertexHeader, 
									geomcache.data() );

			//
			// read patch inside the model

			patches = modelElem->FirstChildElement( "Patches" );
			if (patches)
			{
				patchElem = patches->FirstChildElement( "Patch" );

				int offset=0, size=0, matId=0;
			
				while(patchElem)
				{
					// enumerate attribs
					for( attrib = patchElem->FirstAttribute();
							attrib;
							attrib = attrib->Next() )
					{
						if ( strcmp(attrib->Name(), "offset") == 0 )
						{
							offset = attrib->IntValue();
						}
						else
						if ( strcmp(attrib->Name(), "size") == 0 )
						{
							size = attrib->IntValue();
						}
						else
						if ( strcmp(attrib->Name(), "materialId") == 0 )
						{
							matId = attrib->IntValue();
						}
					}

					mVisitor->OnReadModelPatch( offset, size, matId );

					patchElem = patchElem->NextSiblingElement("Patch");
				}

			}

			mVisitor->OnReadModelFinish();
		
			// go to next model
			pVertexHeader = (VertexDataHeader*) (geomcache.data() + pVertexHeader->endOffset);

			modelElem = modelElem->NextSiblingElement("Model");
		}

		mVisitor->OnReadModelsEnd();
	}
	catch (const std::exception &e)
	{
#if LOADER_LOG_PRINTF <= LOG_LEVEL_ERROR
		printf ( "%s\n", e.what() );
#endif
		return false;
	}

	return true;
}