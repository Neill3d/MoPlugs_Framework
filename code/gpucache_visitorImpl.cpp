
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_visitorImpl.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucache_visitorImpl.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <io.h>
#include <fcntl.h>
#include <sys\stat.h>
#include <stdlib.h>

#include "utils_shaders.h"

#include "glm\gtc\matrix_transform.hpp"
#include "glm\gtx\euler_angles.hpp"
#include "glm\gtc\type_ptr.hpp"

#ifndef M_DEG2RAD
#define 		M_DEG2RAD		0.0174532925
#endif

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGPUCacheLoaderVisitorImpl::CGPUCacheLoaderVisitorImpl(CGPUCacheModel *pModel)
	: CGPUCacheLoaderVisitor()
	, mModel(pModel)
{
	mModel->Allocate();
	mTextures = mModel->mTextures;
	mMaterials = mModel->mMaterials;
	mShaders = mModel->mShaders;
	mModelRender = mModel->mModelRender;
	mVertexData = mModel->mVertexData;

	mImageIndex = 0;
	mSamplerIndex = 0;
	mMaterialIndex = 0;
	mShaderIndex = 0;
}


CGPUCacheLoaderVisitorImpl::~CGPUCacheLoaderVisitorImpl()
{
}

// main header
void CGPUCacheLoaderVisitorImpl::OnReadHeader(const char *xmlFilename, const char *sourceFilename)
{
	mModel->mFilename = xmlFilename;
	mModel->mSourceFilename = sourceFilename;
}

#ifdef _DEBUG
static int gCountLock = 0;
static int gCountCount = 50;
#endif
// textures
bool CGPUCacheLoaderVisitorImpl::OnReadTexturesBegin( const char *textures_filename, const int numberOfSamplers, const int numberOfImages )
{
	mTextures->Clear();
	mTextures->Allocate(numberOfSamplers, numberOfImages);

	mImageIndex = 0;
	mSamplerIndex = 0;
#ifdef _DEBUG
	gCountLock = 0;
#endif
	return true;
}

void CGPUCacheLoaderVisitorImpl::OnReadEmptyImage()
{
	mImageIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadTexturesImage1(const ImageHeader *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	// DONE: make an offset for imageData
	const BYTE *stream = imageData + sizeof(ImageHeader);

	GLuint texId = 0;
	glGenTextures( 1, &texId );

	glBindTexture(GL_TEXTURE_2D, texId);

	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	/*
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		
	glTexEnvf( GL_TEXTURE_FILTER_CONTROL, GL_TEXTURE_LOD_BIAS, -0.5f );

	float max;
	glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
	glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, max );
	*/
	const int uncompress_size = header->width * header->height * ((header->format == GL_RGBA) ? 4 : 3);
	bool isCompressed = (uncompress_size != header->size);

	if (isCompressed)
	{ 
#ifdef _DEBUG
		if (gCountLock < gCountCount)
		{
#endif
		// load already compressed
		glCompressedTexImage2D( GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->size, stream );
#ifdef _DEBUG
		}
		else
		{
			unsigned char pixels[4] = {100, 100, 100, 255};
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels);
		}

		gCountLock += 1;
#endif
	}
	else
	{
		// compressed on load
		GLint internalFormat = (header->format == GL_RGBA) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
		glTexImage2D( GL_TEXTURE_2D, 0, internalFormat, header->width, header->height, 0, header->format, GL_UNSIGNED_BYTE, stream );

		CHECK_GL_ERROR();

		GLint compressed = 0;
		glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED, &compressed );

		if (compressed == GL_TRUE )
		{
			GLint compressed_size;

			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
			glGetTexLevelParameteriv( GL_TEXTURE_2D, 0, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size );

			printf( "compressed size - %d\n", compressed_size );
		}
		else
		{
			printf( "failed to compress!\n" );
		}
	}

	stream  = stream + header->size;

	if (header->numberOfLODs == 0)
	{
		glHint( GL_GENERATE_MIPMAP_HINT, GL_NICEST );
		glGenerateMipmap(GL_TEXTURE_2D);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
	}
	else
	{
		// DONE: load all lods and setUp them

		glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header->numberOfLODs );

		for (int j=1; j<=header->numberOfLODs; ++j)
		{
			ImageLODHeader	*lodHeader = (ImageLODHeader*) stream;
			stream = stream + sizeof(ImageLODHeader);

			if (lodHeader->size <= 0) continue;

			if (isCompressed)
			{
				// load already compressed
				glCompressedTexImage2D( GL_TEXTURE_2D, j, header->internalFormat, lodHeader->width, lodHeader->height, 0, lodHeader->size, stream );
			}
			else
			{
				// compressed on load
				GLint internalFormat = (header->format == GL_RGBA) ? GL_COMPRESSED_RGBA : GL_COMPRESSED_RGB;
				glTexImage2D( GL_TEXTURE_2D, j, internalFormat, lodHeader->width, lodHeader->height, 0, header->format, GL_UNSIGNED_BYTE, stream );

				CHECK_GL_ERROR();

				GLint compressed = 0;
				glGetTexLevelParameteriv( GL_TEXTURE_2D, j, GL_TEXTURE_COMPRESSED, &compressed );

				if (compressed == GL_TRUE )
				{
					GLint compressed_size;

					glGetTexLevelParameteriv( GL_TEXTURE_2D, j, GL_TEXTURE_INTERNAL_FORMAT, &internalFormat );
					glGetTexLevelParameteriv( GL_TEXTURE_2D, j, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, &compressed_size );

					printf( "compressed size - %d\n", compressed_size );
				}
				else
				{
					printf( "failed to compress!\n" );
				}
			}

			stream = stream + lodHeader->size;
		}
	}

	CHECK_GL_ERROR();

	glBindTexture(GL_TEXTURE_2D, 0);

	//
	// add to texture manager

	mTextures->mResourceIds[mImageIndex] = texId;
	mTextures->mTextureDimentions[mImageIndex] = vec2(header->width, header->height);

	mImageIndex += 1;
}



void CGPUCacheLoaderVisitorImpl::OnReadTexturesImage2(const BYTE type, const ImageHeader2 *header, const size_t fileImageOffset, const size_t imageSize, const BYTE *imageData)
{
	BYTE *stream = (BYTE*) imageData;
	GLuint texId=0;

	// skip image type
	stream += sizeof(BYTE);

	if (type == IMAGE_TYPE_STILL)
	{
		stream += sizeof(ImageHeader2);
	
		int components = (header->format==GL_RGBA) ? 4 : 3;
		int d = (header->format==GL_RGBA) ? 32 : 24;

		
		glGenTextures(1, &texId); 

		if (header->size == (header->width * header->height * components) )
		{
			glBindTexture(GL_TEXTURE_2D, texId);

			if (header->numberOfLODs > 1)
			{
				glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header->numberOfLODs-1 );
			}

			glTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->format, GL_UNSIGNED_BYTE, stream);
			
			stream += header->size;
			for (int i=1; i<header->numberOfLODs; ++i)
			{
				ImageLODHeader2		*lodHeader = (ImageLODHeader2*) stream;
				stream += sizeof(ImageLODHeader2);

				glTexImage2D(GL_TEXTURE_2D, i, header->internalFormat, lodHeader->width, lodHeader->height, 0, header->format, GL_UNSIGNED_BYTE, stream);
				stream += lodHeader->size;
			}

			// auto generate mipmaps if missing
			if (header->numberOfLODs <= 1)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			glBindTexture(GL_TEXTURE_2D, 0);
		}
		else
		{
			glBindTexture(GL_TEXTURE_2D, texId);
#ifdef _DEBUG
			if (gCountLock < 20)
			{
#endif
				if (header->numberOfLODs > 1)
				{
					glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, header->numberOfLODs-1 );
				}

				glCompressedTexImage2D(GL_TEXTURE_2D, 0, header->internalFormat, header->width, header->height, 0, header->size, stream);
			
				stream += header->size;
				for (int i=1; i<header->numberOfLODs; ++i)
				{
					ImageLODHeader2		*lodHeader = (ImageLODHeader2*) stream;
					stream += sizeof(ImageLODHeader2);

					glCompressedTexImage2D(GL_TEXTURE_2D, i, header->internalFormat, lodHeader->width, lodHeader->height, 0, lodHeader->size, stream);
					stream += lodHeader->size;
				}
#ifdef _DEBUG
			}
			else
			{
				unsigned char pixels = 0;
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, &pixels);
			}
			gCountLock += 1;
#endif

			// auto generate mipmaps if missing
			if (header->numberOfLODs <= 1)
			{
				glGenerateMipmap(GL_TEXTURE_2D);
			}

			glBindTexture(GL_TEXTURE_2D, 0);

			auto handle = glGetTextureHandleARB(texId);
			printf( "%d\n", handle );
		}
	}

	mTextures->mResourceIds[mImageIndex] = texId;
	mTextures->mTextureDimentions[mImageIndex] = vec2(header->width, header->height);

	CHECK_GL_ERROR();

	mImageIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadTexturesSampler(const char *sampler_name, const char *sampler_file, const SamplerHeader *header, const size_t fileSamplerOffset, const size_t samplerSize, const BYTE *samplerData)
{
	GLuint samplerId = mTextures->mTextureSamplers[mSamplerIndex];

	// DEBUG:
	if (header->sWrap != GL_REPEAT)
	{
		char buffer[128];
		memset( buffer, 0, sizeof(char) * 128 );
		sprintf_s( buffer, sizeof(char)*128, "samplerId - %d\0", samplerId );
		printf( "error - %s\n", buffer );
	}

	CHECK_GL_ERROR();

	glSamplerParameteri( samplerId, GL_TEXTURE_WRAP_S, header->sWrap );
	glSamplerParameteri( samplerId, GL_TEXTURE_WRAP_T, header->tWrap );

	//glSamplerParameteri( samplerId, GL_TEXTURE_MIN_FILTER, header.minFilter );
	//glSamplerParameteri( samplerId, GL_TEXTURE_MAG_FILTER, header.magFilter );
	glSamplerParameteri( samplerId, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
	glSamplerParameteri( samplerId, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	// EXT: GL_EXT_texture_filter_anisotropic
	
	float max;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max);
	glSamplerParameterf( samplerId, GL_TEXTURE_MAX_ANISOTROPY_EXT, max );
	
	// EXT: LOD BIAS
	//glSamplerParameterf( samplerId, GL_TEXTURE_LOD_BIAS, -0.5f );

	CHECK_GL_ERROR();

	mTextures->mTextureNames[mSamplerIndex] = (sampler_name!=nullptr) ? sampler_name : "None";
	memcpy( mTextures->mTextureMatrix[mSamplerIndex].mat_array, header->matrix, sizeof(float) * 16 );
	mTextures->mTextureResources[mSamplerIndex] = header->videoIndex;

	mSamplerIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadTexturesError(const char *what)
{
	printf( "texture error - %s\n", what );
}

void CGPUCacheLoaderVisitorImpl::OnReadTexturesEnd()
{
	// TODO: number of succefully readed textures should be equal to total amount
	if (mImageIndex != (int) mTextures->mResourceIds.size() )
	{
		printf( "textures big error!\n" );
	}

	if (mSamplerIndex != (int) mTextures->mTextureSamplers.size() )
	{
		printf( "texture samplers big error\n" );
	}

	mTextures->UpdateTexturePointers();
}
	
// materials, shaders, etc.
bool CGPUCacheLoaderVisitorImpl::OnReadMaterialsBegin(const int numberOfMaterials)
{
	mMaterials->Allocate(numberOfMaterials);

	mMaterialIndex = 0;

	return true;
}

void CGPUCacheLoaderVisitorImpl::OnReadMaterial(const char *material_name, const MaterialGLSL &materialData )
{
	mMaterials->GetDataItem(mMaterialIndex) = materialData;
	mMaterials->mMaterialsNames[mMaterialIndex] = material_name;

	mMaterialIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadMaterialsEnd()
{
	mMaterials->CauseAGPUUpdate();
}

bool CGPUCacheLoaderVisitorImpl::OnReadShadersBegin(const int numberOfShaders)
{
	mShaders->Allocate(numberOfShaders+1);

	ShaderGLSL defShader;
	DefaultShader(defShader);
	
	mShaders->GetDataItem(0) = defShader;
	mShaders->mShadersAlpha[0] = 0; // kFBAlphaSourceNoAlpha
	mShaders->mShadersNames[0] = "Default";

	mShaderIndex = 1;

	return true;
}

void CGPUCacheLoaderVisitorImpl::OnReadShader(const char *shader_name, const int alphatype, const ShaderGLSL &shaderData )
{
	mShaders->GetDataItem(mShaderIndex) = shaderData;
	mShaders->mShadersAlpha[mShaderIndex] = alphatype;
	mShaders->mShadersNames[mShaderIndex] = shader_name;

	mShaderIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadShadersEnd()
{
	mShaders->CauseAGPUUpdate();

	mModel->mNumberOfOpaque = mShaders->GetNumberOfOpaqueShaders();
	mModel->mNumberOfTransparency = mShaders->GetNumberOfTransparencyShaders();
}

// geometry, models
bool CGPUCacheLoaderVisitorImpl::OnReadModelsBegin(const int numberOfModels, const int numberOfMeshes, const double *bounding_min, const double *bounding_max)
{
	mModelRender->mBoundingBoxMin = vec4( (float)bounding_min[0], (float)bounding_min[1], (float)bounding_min[2], 1.0 );
	mModelRender->mBoundingBoxMax = vec4( (float)bounding_max[0], (float)bounding_max[1], (float)bounding_max[2], 1.0 );

	mModelRender->Reserve(numberOfModels, numberOfMeshes);

	mAccumNumberOfIndices = 0;
	mSubmodelIndex = 0;

	return true;
}

void CGPUCacheLoaderVisitorImpl::OnReadVertexData( FileGeometryHeader *const pHeader, const BYTE *data )
{
	mVertexData->UpdateFromCache(data);
}

void CGPUCacheLoaderVisitorImpl::OnReadModel(const char *name, 
					const double *translation, 
					const double *rotation, 
					const double *scaling, 
					const double *bounding_min,
					const double *bounding_max,
					const int numberOfShaders,
					const int *shaders,
					const VertexDataHeader *pheader, 
					const BYTE *data)
{
	mModelRender->mSubModelNames.push_back(name);
	mNumberOfIndices = 0;

	mModelShaderId = 0;
	mOpaqueModel = 0.0f;

	if (numberOfShaders == 0)
	{
		mModelShaderId = 0;
	}
	else if (numberOfShaders == 1)
	{
		mModelShaderId = shaders[0];
	}
	else
	{
		mModelShaderId = AddShaderCombination( mShaders, 
			shaders[0], 
			shaders[1], 
			shaders[2], 
			shaders[3], 
			shaders[4], 
			mShaders->GetAlphaVectorPtr() );
	}

	// NOTE: we add +1 cause we have zero defaultShader !
	if (mShaders->GetAlphaSource(mModelShaderId) != 0) // kFBAlphaSourceNoAlpha
		mOpaqueModel = 1.0f;

	ModelGLSL						modelData;
	
	// 
	mat4 m4_tm, m4_normal;

	glm::mat4x4		matrix, trMatrix, rtMatrix, rtX, rtY, rtZ, sclMatrix, normalMatrix;

	trMatrix = glm::translate( glm::mat4x4(1.0f), glm::vec3( translation[0], translation[1], translation[2] ) );
	sclMatrix = glm::scale( glm::mat4x4(1.0f), glm::vec3( scaling[0], scaling[1], scaling[2] ) );
	
	rtX = glm::eulerAngleX( (float) (M_DEG2RAD*rotation[0]) );
	rtY = glm::eulerAngleY( (float) (M_DEG2RAD*rotation[1]) );
	rtZ = glm::eulerAngleZ( (float) (M_DEG2RAD*rotation[2]) );
	
	rtMatrix = rtZ * rtY * rtX;

	matrix = trMatrix * rtMatrix * sclMatrix;

	normalMatrix = matrix;
	glm::inverse(normalMatrix);
	glm::transpose(normalMatrix);

	float *matrixPtr = glm::value_ptr(matrix);
	float *normalPtr = glm::value_ptr(normalMatrix);

	memcpy( modelData.transform.mat_array, matrixPtr, sizeof(float)*16 );
	memcpy( modelData.normalMatrix.mat_array, normalPtr, sizeof(float)*16 );

	mModelRender->mModelInfos.push_back(modelData);

	// DONE: calculate the center of bounding box
	double lcenter[3] = { bounding_min[0]+0.5*(bounding_max[0]-bounding_min[0]), 
		bounding_min[1]+0.5*(bounding_max[1]-bounding_min[1]), 
		bounding_min[2]+0.5*(bounding_max[2]-bounding_min[2]) };

	double ldiff[3] = { bounding_max[0]-lcenter[0], 
		bounding_max[1]-lcenter[1], 
		bounding_max[2]-lcenter[2] };

	const double dist = sqrt( ldiff[0]*ldiff[0] + ldiff[1]*ldiff[1] + ldiff[2]*ldiff[2] );
	mBSphere = vec4( (float)lcenter[0], (float)lcenter[1], (float)lcenter[2], (float)dist );
}

void CGPUCacheLoaderVisitorImpl::OnReadModelPatch(const int offset, const int size, const int materialId)
{
	DrawElementsIndirectCommand		command;
	memset( &command, 0, sizeof(DrawElementsIndirectCommand) );
	command.primCount = 1;

	command.count = size;
	command.firstIndex = offset + mAccumNumberOfIndices; // DONE: shift this value by prev meshes numberOfIndices
	command.baseInstance = (GLuint) mModelRender->mMeshInfos.size();
	command.primCount = (mOpaqueModel > 0.0f) ? 0 : 1;
	mModelRender->mCommands.push_back(command);
	command.primCount = (mOpaqueModel > 0.0f) ? 1 : 0;
	mModelRender->mCommandsTransparency.push_back(command);

	mModelRender->mBSphereCoords.push_back(mBSphere);
	mModelRender->mBShaderInfo.push_back(vec4(mOpaqueModel, 0.0f, 0.0f, 0.0f));
	
	//TClientMeshDATA clientMeshData;
	//clientMeshData.material = matId;
	//clientMeshData.model = modelId;
	//mClientMeshInfos.push_back(clientMeshData);

	MeshGLSL						meshData;

	//meshData.material = materialsManager->GetGPUPtr(matId);
	meshData.material = materialId;
	meshData.shader = mModelShaderId;
	meshData.model = mSubmodelIndex;
	meshData.lightmap = 1;

	mModelRender->mMeshInfos.push_back(meshData);
				
	//
	mNumberOfIndices = std::max( mNumberOfIndices, (unsigned int) (offset+size) );
}

void CGPUCacheLoaderVisitorImpl::OnReadModelFinish()
{
	mAccumNumberOfIndices += mNumberOfIndices;
	mSubmodelIndex += 1;
}

void CGPUCacheLoaderVisitorImpl::OnReadModelsEnd()
{
	mShaders->CauseAGPUUpdate();

	// DONE: update per mesh pointers to models
	mModelRender->PrepRender();

}
