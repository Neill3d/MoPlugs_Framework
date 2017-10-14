
#pragma once

//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_models.h
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


//-- 
#include <GL\glew.h>

//
#include "algorithm\nv_math.h"
#include "algorithm\math3d.h"

#include "shared_glsl.h"
#include "shared_common.h"
#include "shared_misc.h"

#include "shared_textures.h"
#include "shared_materials.h"
#include "shared_shaders.h"

#include "graphics\OGL_Utils.h"

#include <vector>

////////////////////////////////////////////////////////////////////////////////
// forward declaration
class CGPUCacheLoaderVisitorImpl;

//////////////////////////////////////////////////////////////////////



void ModelUpdateNormalMatrix( const mat4 &modelview, ModelGLSL *data );


// store some per mesh information on client side for future recalculations
	/*
struct TClientModelDATA
{
	mat4	transform;
	int		shader;
};

struct TClientMeshDATA
{
	int		model;
	int		material;
};
*/
//////////////////////////////////////////////////////
// base class for rendering model data
class CGPUVertexData;

class CGPUModelRender
{

public:
	//! a constructor
	CGPUModelRender(CGPUVertexData *vertexdata);
	//! a destructor
	virtual ~CGPUModelRender();

	// free gpu mem
	virtual void Free()
	{}

	// clear buffers
	virtual void Clear()
	{}

	virtual void Render();

	CGPUVertexData *GetVertexDataPtr() {
		return mVertexData;
	}

protected:

	CGPUVertexData		*mVertexData;	// render using that vertex data

};

// render model from FBModel patches
class CGPUModelRenderFB : public CGPUModelRender
{
public:
	//! a constructor
	CGPUModelRenderFB(CGPUVertexData *vertexdata)
		: CGPUModelRender(vertexdata)
	{}

	virtual void Render()
	{}
};

///////////////////////////////////////////

typedef  struct {
        GLuint  count;
        GLuint  primCount;
        GLuint  firstIndex;
        GLuint  baseVertex;
        GLuint  baseInstance;
    } DrawElementsIndirectCommand;

typedef struct {
          GLuint   index;
          GLuint   reserved; 
          GLuint64 address;
          GLuint64 length;
        } BindlessPtrNV; 

typedef struct {
          DrawElementsIndirectCommand cmd;
          GLuint                      reserved; 
          BindlessPtrNV               indexBuffer;
          BindlessPtrNV               vertexBuffers[1];
        } DrawElementsIndirectBindlessCommandNV;

//////////////////////////////////////////////////////////////////////////
// render model from cached values (indirect commands)
//  this models consists of all cached models as sub-models
class CGPUModelRenderCached : public CGPUModelRender
{
public:
	//! a constructor
	CGPUModelRenderCached(CGPUVertexData *vertexdata);
	//! a destructor
	~CGPUModelRenderCached();

	void	Reserve(const int numberOfModels, const int numberOfMeshes);

	virtual void Free();
	virtual void Clear();

	void PrepRender();

	void RenderCulling();

	void RenderNormals();

	void RenderSelection(const int index);	// make a render for the picking (selection) buffer

	void	ReCalculatePerModelInfo(CMaterialsReference *materials, CShadersReference *shaders);
	void	ReCalculatePerMeshInfo(CMaterialsReference *materials, CShadersReference *shaders);

	void BindModelInfoAsUniform( const GLuint programId, const GLint uniformLoc ) {
		mBufferPerModel.BindAsUniform( programId, uniformLoc, 0 );
	}
	void BindMeshInfoAsUniform( const GLuint programId, const GLint uniformLoc ) {
		mBufferPerMesh.BindAsUniform( programId, uniformLoc, 0 );
	}

	//
	void		UpdateColorId(const vec3 &colorId);
	void		UpdateReceiveShadows(const int flag);
	void		UpdateGPUBuffer(const mat4 *m4_parent, const mat4 *modelview);
	bool			RenderBegin();
	void			RenderOpaque();
	void			RenderTransparency();
	void			RenderEnd();
	
	//

	const float CalculateFarDistance(const CFrustum &frustum, const vec3 &eyePos);

	const int GetNumberOfSubModels()
	{
		return (int) mSubModelNames.size();
	}
	const char *GetSubModelName(const int index)
	{
		return mSubModelNames[index].c_str();
	}
	const mat4 &GetModelTransform(const int index)
	{
		return mModelInfos[index].transform;
	}

	//
	// MESHES (each model could has several meshes inside)

	const int GetNumberOfSubMeshes()
	{
		return (int) mMeshInfos.size();
	}

	const int SubMeshIndexToModelIndex(const int index)
	{
		return mMeshInfos[index].model;
	}

	const int GetMeshFirstIndex(const int index)
	{
		return mCommands[index].firstIndex;
	}
	const int GetMeshIndexCount(const int index)
	{
		return mCommands[index].count;
	}


	void	BindBufferIndirect();
	void	BindBufferIndirectTransparency();

	void	BindBufferIndirectToWrite();
	void	BindBufferInfos(const GLuint location);

	void	SetBufferRealFarValue(const float value);
	float	GetBufferRealFarValue();

	void GetBoundingBox(float *bmin, float *bmax)
	{
		memcpy(bmin, mBoundingBoxMin.vec_array, sizeof(float)*3);
		memcpy(bmax, mBoundingBoxMax.vec_array, sizeof(float)*3);
	}

	const bool HasTransparentGeometry() const
	{
		return (mCommandsTransparency.size() > 0);
	}

protected:

	std::vector<std::string>					mSubModelNames;	// for UI

	vec4					mBoundingBoxMin;
	vec4					mBoundingBoxMax;

	// draw culling
	GLuint					mBufferBSphere;		// buffer with bspehere coords for frustum culling of gpu
	std::vector<vec4>		mBSphereCoords;		// store xyz position and radius in w for each mesh or for each submodel
	GLuint					mBufferBShader;
	std::vector<vec4>		mBShaderInfo;	// store x (0.0 - opaque, 1.0 - transparency shader for this mesh)

	// atomic counter for calculating real far distance (for cluster lighting)
	GLuint					mBufferAz;

	// draw all by one call
	// devide into two command list, one for fully opaque, others - the rest
	std::vector<DrawElementsIndirectCommand>				mCommands;
	std::vector<DrawElementsIndirectCommand>				mCommandsTransparency;	

	GLuint													mBufferIndirect;
	GLuint													mBufferIndirectTransparency;

	std::vector<DrawElementsIndirectBindlessCommandNV>		mBindlessCommands;
	GLuint													mBufferIndirectBindless;
	/*
	std::vector<TClientModelDATA>					mClientModelInfos;	// hold each mesh transformation to prepare per mesh normal matrix
	std::vector<TClientMeshDATA>					mClientMeshInfos;	// hold each mesh transformation to prepare per mesh normal matrix
	*/
	// this one is assigned as a attribute (location=4)
	std::vector<MeshGLSL>						mMeshInfos;	// allocate and collect all information about meshes for render
	std::vector<ModelGLSL>						mModelInfos;
	GLuint										mBufferInfos;	// SSBO with submeshes data

	CGPUBufferNV				mBufferPerMesh;
	CGPUBufferNV				mBufferPerModel;

	void	PrepareBufferIndirect();
	void	PrepareBufferInfosSSBO();

	void	UpdatePerModelGPUBuffer();
	void	UpdatePerMeshGPUBuffer();	// UBO for nvidia gpu pointer

	void	PrepareBufferBSphere();
	void	BindBufferBSphere();

	void	PrepareBufferRealFar();
	void	BindBufferRealFar();

	//
	friend class CGPUCacheLoaderVisitorImpl;
};

//////////////////////////////////////
// base class for holding model data
enum 
{
	VERTEX_BUFFER_POINT,
	VERTEX_BUFFER_UV,
	VERTEX_BUFFER_NORMAL,
	VERTEX_BUFFER_TANGENT,
	VERTEX_BUFFER_BINORMAL,
	VERTEX_BUFFER_INDEX,
	VERTEX_BUFFER_MAX
};

class CGPUVertexData
{
public:
	//! a constructor
	CGPUVertexData();

	virtual ~CGPUVertexData();

public:

	static	void renderPrep();
	static	void renderFinish();
	static	void QueryAttributes(const GLuint programHandle);
	
	// bind ogl arrays 
	virtual void	Bind();
	virtual void	UnBind();

	//void UpdateFromTempArrays( const unsigned int vertexCount, const unsigned int indexCount );

	virtual bool UpdateFromCache( const unsigned char *models_data );

	const int GetNumberOfVertices()
	{
		return mNumberOfVertices;
	}

	const int GetNumberOfIndices()
	{
		return mNumberOfIndices;
	}
	/*
	const BYTE *GetPoints()
	{
		return points;
	}
	*/

	const float *MapPositionBuffer();
	void UnMapPositionBuffer();

	const float *MapNormalBuffer();
	void UnMapNormalBuffer();

	const unsigned int *MapIndexBuffer();
	void UnMapIndexBuffer();


	bool	PrepCacheBuffers( const int numberOfVertices, const int numberOfIndices, const BYTE *pointData, const BYTE *normalData, const BYTE *tangentData, const BYTE *uvData, const BYTE *indexData );

	void AssignBuffers(const GLuint positionId, const GLuint normalId, const GLuint tangentId, const GLuint binormalId, const GLuint uvId, const GLuint indexId)
	{
		mBuffersAllocated = false;

		mBuffersId[VERTEX_BUFFER_POINT] = positionId;
		mBuffersId[VERTEX_BUFFER_NORMAL] = normalId;
		mBuffersId[VERTEX_BUFFER_TANGENT] = tangentId;
		mBuffersId[VERTEX_BUFFER_BINORMAL] = binormalId;
		mBuffersId[VERTEX_BUFFER_UV] = uvId;
		mBuffersId[VERTEX_BUFFER_INDEX] = indexId;
	}

	void AssignBufferOffsets(GLvoid *pos, GLvoid *normal, GLvoid *tangent, 
		GLvoid *binormal, GLvoid *uv, GLvoid *index)
	{
		mBuffersOffsets[VERTEX_BUFFER_POINT] = pos;
		mBuffersOffsets[VERTEX_BUFFER_NORMAL] = normal;
		mBuffersOffsets[VERTEX_BUFFER_TANGENT] = tangent;
		mBuffersOffsets[VERTEX_BUFFER_BINORMAL] = binormal;
		mBuffersOffsets[VERTEX_BUFFER_UV] = uv;
		mBuffersOffsets[VERTEX_BUFFER_INDEX] = index;
	}

	bool IsReady() {
		return (mBuffersId[VERTEX_BUFFER_POINT] > 0);
	}

protected:

	GLuint			mVao;

	bool			mBuffersAllocated;
	GLuint			mBuffersId[VERTEX_BUFFER_MAX];	// do we need to free gpu buffers from this class destructor
	GLvoid			*mBuffersOffsets[VERTEX_BUFFER_MAX];

	//BYTE			*points;

	int				mNumberOfVertices;
	GLuint			mPointBuffer;
	GLuint			mNormalBuffer;
	GLuint			mTangentBuffer;
	GLuint			mUVBuffer;
	
	int				mNumberOfIndices;
	GLuint			mIndexBuffer;

	//
	int				mPointStride;
	int				mTexCoordStride;
	int				mNormalStride;
	int				mTangentStride;

	// store source data for use in newton collision

	std::vector<vec4>	mPositions;
	std::vector<vec4>	mNormals;

	std::vector<unsigned int>	mIndices;

	
};


// organize and bind arrays using VAO
class CGPUVertexDataVAO : public CGPUVertexData
{
public:

	// bind ogl arrays 
	virtual void	Bind();
	virtual void	UnBind();

	void		RenderPrepVAO();
	void		RenderFinishVAO();

protected:

	VertexArray		mVertexArray;		// vertex array objects

private:

	void UpdateVAO();

};

// organize and bind arrays using NV VBUM
class CGPUVertexDataVBUM : public CGPUVertexData
{
public:

	//! a constructor
	CGPUVertexDataVBUM();
	//! a destructor
	virtual ~CGPUVertexDataVBUM();

	// bind ogl arrays 
	virtual void	Bind();
	virtual void	UnBind();

	void		RenderPrepVBUM();
	void		RenderFinishVBUM();

protected:

	GLint			mPointBufferSize;
	GLint			mNormalBufferSize;
	GLint			mTangentBufferSize;
	GLint			mUVBufferSize;

	GLint			mIndexBufferSize;

	GLuint64EXT		mPointBufferGPUPtr;	// gpu pointer to position data
	GLuint64EXT		mNormalBufferGPUPtr;	
	GLuint64EXT		mTangentBufferGPUPtr;	
	GLuint64EXT		mUVBufferGPUPtr;	

	GLuint64EXT		mIndexBufferGPUPtr;		// gpu pointer to index data

private:

	void UpdateVBUM();

};


