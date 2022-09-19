
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_saverImpl.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include "gpucachesaverqueryimpl.h"

///////////////////////////////////////////////////////////////////////////

CGPUCacheSaverQueryImpl::CGPUCacheSaverQueryImpl( CGPUCacheModel *pModel )
	: CGPUCacheSaverQuery()
	, mModel(pModel)
{
}

CGPUCacheSaverQueryImpl::~CGPUCacheSaverQueryImpl()
{
}

bool CGPUCacheSaverQueryImpl::Init(const char *filename)
{
	return true;
}

	// query information for textures

const int CGPUCacheSaverQueryImpl::GetVideoCount()
{
	return 0;
}

const char *CGPUCacheSaverQueryImpl::GetVideoName(const int index)
{
	return "";
}

const int CGPUCacheSaverQueryImpl::GetVideoWidth(const int index)
{
	return 0;
}

const int CGPUCacheSaverQueryImpl::GetVideoHeight(const int index)
{
	return 0;
}

const int CGPUCacheSaverQueryImpl::GetVideoFormat(const int index)
{
	return 0;
}

const bool CGPUCacheSaverQueryImpl::IsVideoImageSequence(const int index)
{
	return false;
}

const int CGPUCacheSaverQueryImpl::GetVideoStartFrame(const int index)
{
	return 0;
}

const int CGPUCacheSaverQueryImpl::GetVideoStopFrame(const int index)
{
	return 0;
}

const int CGPUCacheSaverQueryImpl::GetVideoFrameRate(const int index)
{
	return 0;
}

const char *CGPUCacheSaverQueryImpl::GetVideoFilename(const int index)
{
	return "";
}

const double CGPUCacheSaverQueryImpl::GetVideoSize(const int index)
{
	return 0.0;
}

const bool CGPUCacheSaverQueryImpl::IsVideoUsedMipmaps(const int index)
{
	return false;
}


// information about media
double CGPUCacheSaverQueryImpl::GetTotalUncompressedSize()
{
	return 0.0;
}

//

const int CGPUCacheSaverQueryImpl::GetSamplersCount()
{
	return 0;
}

const char *CGPUCacheSaverQueryImpl::GetSamplerName(const int index) // pTexture->LongName
{
	return "";
}

const int CGPUCacheSaverQueryImpl::GetSamplerVideoIndex(const int index)	// which video is used for that sampler
{
	return 0;
}

void CGPUCacheSaverQueryImpl::GetSamplerMatrix( const int index, mat4 &mat )
{

}

/*

bool CTexturesReference::ReSaveTexturesPackage(const char *filename)
{
	//
	auto fn_writeSafe = [] (const int fh, void *data, const int size) {

		if (size != _write(fh, data, size ) )
			throw std::exception( "error while writing data!\n" );

	};

	//
	BYTE *localImageBuffer = nullptr;

	int fh=0;
	int err = _sopen_s( &fh, filename, _O_BINARY | _O_CREAT | _O_WRONLY | _O_TRUNC, _SH_DENYRW, _S_IREAD | _S_IWRITE);

	try
	{

		printf ("%d %d\n", err, fh );
		if ( err != 0)
			throw std::exception("Failed to open textures file for writing\n");

		_lseeki64( fh, 0, 0 );

		const int numberOfImages = (int) mResourceIds.size();
		const int numberOfSamplers = (int) mTextureResources.size();

		FileTexturesHeader texHeader;
		FileTexturesHeader::Set( 2, numberOfImages, numberOfSamplers, texHeader );
		texHeader.imagesOffset = 0;
		texHeader.samplersOffset = 0;

		fn_writeSafe( fh, &texHeader, sizeof(texHeader) );
		
		//
		// save images
		//
		localImageBuffer = new BYTE[16384 * 16384 * 4];

		texHeader.imagesOffset = _telli64(fh);

		BYTE	imageType = IMAGE_TYPE_STILL;
		ImageHeader2 header;

		for (int i=0; i<numberOfImages; ++i)
		{
			if (mResourceIds[i] == 0)
			{
				// store image type
				imageType = IMAGE_TYPE_STILL;
				fn_writeSafe( fh, &imageType, sizeof(BYTE) );

				// store header
				ImageHeader2::Set( 0, 0, 0, 0, 0, 0, 0, header);
				fn_writeSafe( fh, &header, sizeof(header) );
				
				continue;
			}


			if (mResourceFrames[i].frames != nullptr)
			{
				imageType = IMAGE_TYPE_SEQUENCE;
				fn_writeSafe( fh, &imageType, sizeof(BYTE) );

				// store seq header
				fn_writeSafe( fh, &mResourceFrames[i].seqHeader, sizeof(ImageSequenceHeader2) );

				// store frames
				fn_writeSafe( fh, &mResourceFrames[i].frames, mResourceFrames[i].seqHeader.size );
			}
			else
			{
				imageType = IMAGE_TYPE_STILL;
				fn_writeSafe( fh, &imageType, sizeof(BYTE) );

				// store still image - grab data from video memory
			
				glBindTexture(GL_TEXTURE_2D, mResourceIds[i] );
			
				bool isCompressed=false;	// !!
				header.numberOfFrames = 1;
				int pixelMemorySize = 0;

				TextureObjectGetData( localImageBuffer, header, isCompressed, pixelMemorySize );

				// store header
	
				printf( "writing image - size %d\n", header.size );
				header.dummy = 0;
				fn_writeSafe( fh, &header, sizeof(header) );

				// store texture
				if (header.size > 0 && localImageBuffer != nullptr)
				{
					fn_writeSafe( fh, localImageBuffer, sizeof(BYTE) * header.size );
				}

				// store lods
				ImageLODHeader2	lodHeader;

				for (int j=1; j<header.numberOfLODs; ++j)
				{
			
					TextureObjectGetMipmapData( localImageBuffer, j, isCompressed, pixelMemorySize, header.format, lodHeader );

					// get lod and save it
					lodHeader.dummy = 0;
					fn_writeSafe( fh, &lodHeader, sizeof(lodHeader) );
				
					// DONE: store lod data here
					if (lodHeader.size > 0 && localImageBuffer != nullptr)
					{
						fn_writeSafe( fh, localImageBuffer, sizeof(BYTE) * lodHeader.size );
					}
				}
			}
		}

		//
		/// save samplers
		//
		texHeader.samplersOffset = _telli64(fh);

		for (int i=0; i<numberOfSamplers; ++i)
		{
			SamplerHeader header;
			SamplerHeader::Set( mTextureMatrix[i].mat_array, mTextureResources[i], GL_REPEAT, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR, header );

			fn_writeSafe( fh, &header, sizeof(header) );
		}


		// save offsets to a header
		//
		_lseeki64(fh, 0, 0);
		
		fn_writeSafe( fh, &texHeader, sizeof(texHeader) );

	}
	catch (std::exception &e)
	{
		if(localImageBuffer)
		{
			delete [] localImageBuffer;
			localImageBuffer = nullptr;
		}
		glBindTexture(GL_TEXTURE_2D, 0);

		if (fh > 0) _close(fh);

		//FBTraceWithLevel(kFBCRITICAL_TRACE, e.what() );
		return false;
	}
	if(localImageBuffer)
	{
		delete [] localImageBuffer;
		localImageBuffer = nullptr;
	}
	glBindTexture(GL_TEXTURE_2D, 0);

	if (fh > 0) _close(fh);

	return true;
}
*/