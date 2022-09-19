
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: gpucache_types.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <GL\glew.h>
#include <memory>

#include <Windows.h>

#define MAX_NUMBER_OF_SHADERS_PER_MODEL		5

namespace GPUCache
{
	// !!! TODO: strides are standartized, so give some info about that

	constexpr short gPointStride = 16;
	constexpr short gNormalStride = 16;
	constexpr short gTangentStride = 16;
	constexpr short gUVStride = 8;
	constexpr short gIndexStride = 4;

	///////////////////////////////////////////////////////////////////////////// MODELS
	//

	struct FileGeometryHeader
	{
		short	version;
		int		numberOfModels;
		int		totalNumberOfVertices;
		int		totalNumberOfIndices;

		static void Set(const short _version,
			const int _numberOfModels,
			const int _totalNumberOfVertices,
			const int _totalNumberOfIndices,
			FileGeometryHeader& header)
		{
			header.version = _version;
			header.numberOfModels = _numberOfModels;
			header.totalNumberOfVertices = _totalNumberOfVertices;
			header.totalNumberOfIndices = _totalNumberOfIndices;
		}
	};

	struct VertexDataHeader
	{
		int			numVertices;
		int			numIndices;
		//	strides are standartize !
		short		pointStride;
		short		normalStride;
		short		tangentStride;
		short		uvStride;

		__int64		positionOffset;
		__int64		normalOffset;
		__int64		tangentOffset;
		__int64		uvOffset;
		__int64		indicesOffset;
		__int64		endOffset;

		static void Set(const int _numVertices,
			const int _numIndices,
			const short _pointStride,
			const short _normalStride,
			const short _tangentStride,
			const short _uvStride,
			VertexDataHeader& header)
		{
			header.numVertices = _numVertices;
			header.numIndices = _numIndices;

			header.pointStride = _pointStride;
			header.normalStride = _normalStride;
			header.tangentStride = _tangentStride;
			header.uvStride = _uvStride;

			header.positionOffset = 0;
			header.normalOffset = 0;
			header.tangentOffset = 0;
			header.uvOffset = 0;
			header.indicesOffset = 0;
			header.endOffset = 0;
		}
	};




	///////////////////////////////////////////////////////////////// TEXTURES
	//
	struct FileTexturesHeader
	{
		short	version;
		int		numberOfImages;
		int		numberOfSamplers;
		__int64	imagesOffset;
		__int64	samplersOffset;

		static void Set(const short _version,
			const int _numberOfImages,
			const int _numberOfSamplers,
			FileTexturesHeader& header)
		{
			header.version = _version;
			header.numberOfImages = _numberOfImages;
			header.numberOfSamplers = _numberOfSamplers;
		}
	};

	struct ImageHeader
	{
		short		width;
		short		height;
		GLint		internalFormat;
		GLint		format;
		GLint		size;
		BYTE		numberOfLODs;

		static void Set(const short _width,
			const short _height,
			const GLint _internalFormat,
			const GLint _format,
			const GLint _size,
			const unsigned char lods,
			ImageHeader& header)
		{
			header.width = _width;
			header.height = _height;
			header.internalFormat = _internalFormat;
			header.format = _format;
			header.size = _size;
			header.numberOfLODs = lods;
		}
	};

#define IMAGE_TYPE_STILL		1
#define IMAGE_TYPE_SEQUENCE		2

	struct ImageHeader2
	{
		short		width;
		short		height;
		GLint		internalFormat;
		GLint		format;
		GLint		size;

		short		numberOfFrames;

		BYTE		numberOfLODs;
		BYTE		type;			// is that image still or sequence

		BYTE		compressed;

		GLint		dummy;

		static void Set(const short _width,
			const short _height,
			const GLint _internalFormat,
			const GLint _format,
			const GLint _size,
			const short _frames,
			const unsigned char lods,
			ImageHeader2& header)
		{
			header.width = _width;
			header.height = _height;
			header.internalFormat = _internalFormat;
			header.format = _format;
			header.size = _size;
			header.numberOfFrames = _frames;
			header.numberOfLODs = lods;
			header.type = IMAGE_TYPE_STILL;
			header.compressed = 0;
			header.dummy = 0;
		}
	};

	struct ImageSequenceHeader2 : public ImageHeader2
	{
		double		timeOffset;
		GLint		startFrame;
		GLint		stopFrame;
		double		frameRate;
		double		playSpeed;
		BYTE		freeRunning;
		BYTE		loop;

		BYTE		useSystemFrameRate;


		static void Set(const double _timeOffset,
			const GLint _startFrame,
			const GLint _stopFrame,
			const double _frameRate,
			const double _playSpeed,
			const BYTE _freeRunning,
			const BYTE _loop,
			const BYTE _useSystemFrameRate,
			ImageSequenceHeader2& header)
		{
			header.timeOffset = _timeOffset;
			header.startFrame = _startFrame;
			header.stopFrame = _stopFrame;
			header.frameRate = _frameRate;
			header.playSpeed = _playSpeed;
			header.freeRunning = _freeRunning;
			header.loop = _loop;
			header.useSystemFrameRate = _useSystemFrameRate;
			header.dummy = 0;
		}
	};

	struct ImageSequenceLink
	{
		__int64		link;	// link to a specified frame in stream
	};

	// for image or for frame
	struct ImageLODHeader
	{
		short	width;
		short	height;
		GLint	size;

		static void Set(const short _width,
			const short _height,
			const GLint _size,
			ImageLODHeader& header)
		{
			header.width = _width;
			header.height = _height;
			header.size = _size;
		}
	};


	struct ImageLODHeader2
	{
		short	width;
		short	height;
		GLint	size;
		GLint	dummy;

		static void Set(const short _width,
			const short _height,
			const GLint _size,
			ImageLODHeader2& header)
		{
			header.width = _width;
			header.height = _height;
			header.size = _size;
			header.dummy = 0;
		}
	};

	struct SamplerHeader
	{
		float		matrix[16];	// matrix that is assigned for that combination of sampler/image

		int			videoIndex;

		GLenum		sWrap;
		GLenum		tWrap;
		GLenum		rWrap;
		GLenum		minFilter;
		GLenum		magFilter;

		static void Set(const float* src_m,
			const int _videoIndex,
			const GLenum _sWrap,
			const GLenum _tWrap,
			const GLenum _rWrap,
			const GLenum _minFilter,
			const GLenum _magFilter,
			SamplerHeader& header)
		{
			memcpy(header.matrix, src_m, sizeof(float) * 16);
			header.videoIndex = _videoIndex;
			header.sWrap = _sWrap;
			header.tWrap = _tWrap;
			header.rWrap = _rWrap;
			header.minFilter = _minFilter;
			header.magFilter = _magFilter;
		}
	};
}