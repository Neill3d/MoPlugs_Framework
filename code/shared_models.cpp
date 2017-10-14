
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: shared_models.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////


#include <assert.h>
#include "shared_models.h"

#include "graphics\CheckGLError.h"
//#include "graphics\particlesDrawHelper.h"


#include "utils_shaders.h"

	
	const bool gUseBindlessUniforms = true;

#define SAFE_ARRAY_DELETE(a)	if(a) { delete [] a; a = nullptr; }



extern void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam);

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void ModelUpdateNormalMatrix(const mat4 &modelView, ModelGLSL *data)
{
	const mat4 tm = modelView * data->transform;
    
	invert(data->normalMatrix, tm);
	transpose(data->normalMatrix);
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGPUModelRender::CGPUModelRender(CGPUVertexData *vertexdata)
	: mVertexData(vertexdata)
{
}

CGPUModelRender::~CGPUModelRender()
{
}

void CGPUModelRender::Render()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGPUModelRenderCached::CGPUModelRenderCached(CGPUVertexData *vertexdata)
	: CGPUModelRender(vertexdata)
{
	mBufferIndirect = 0;
	mBufferIndirectTransparency = 0;
	mBufferIndirectBindless = 0;
	mBufferInfos = 0;

	mBufferBSphere = 0;
	mBufferBShader = 0;
	mBufferAz = 0;

	mBoundingBoxMin = vec4(0.0, 0.0, 0.0, 1.0);
	mBoundingBoxMax = vec4(0.0, 0.0, 0.0, 1.0);
}

CGPUModelRenderCached::~CGPUModelRenderCached()
{
	Free();
}

void CGPUModelRenderCached::Free()
{
	if (mBufferIndirect)
	{
		glDeleteBuffers(1, &mBufferIndirect);
		mBufferIndirect = 0;
	}
	if (mBufferIndirectTransparency)
	{
		glDeleteBuffers(1, &mBufferIndirectTransparency);
		mBufferIndirectTransparency = 0;
	}
	if (mBufferIndirectBindless)
	{
		glDeleteBuffers(1, &mBufferIndirectBindless);
		mBufferIndirectBindless = 0;
	}
	if (mBufferInfos)
	{
		glDeleteBuffers(1, &mBufferInfos);
		mBufferInfos = 0;
	}
	if (mBufferBSphere)
	{
		glDeleteBuffers(1, &mBufferBSphere);
		mBufferBSphere = 0;
	}
	if (mBufferBShader)
	{
		glDeleteBuffers(1, &mBufferBShader);
		mBufferBShader = 0;
	}
}

void CGPUModelRenderCached::Clear()
{
	mSubModelNames.clear();
	mCommands.clear();
	mCommandsTransparency.clear();
	mBindlessCommands.clear();
	mMeshInfos.clear();
	mModelInfos.clear();
	//mClientModelInfos.clear();
	//mClientMeshInfos.clear();
	mBSphereCoords.clear();
	mBShaderInfo.clear();
}


void CGPUModelRenderCached::Reserve(const int numberOfModels, const int numberOfMeshes)
{
	mSubModelNames.reserve(numberOfModels);
	//
	mCommands.reserve( numberOfMeshes );
	mCommandsTransparency.reserve( numberOfMeshes );

	mBSphereCoords.reserve( numberOfMeshes );
	mBShaderInfo.reserve( numberOfMeshes );

	mModelInfos.reserve( numberOfModels );
	//mClientModelInfos.reserve( numberOfSubModels );

	mMeshInfos.reserve( numberOfMeshes );
	//mClientMeshInfos.reserve( numberOfSubMeshes );
}

void CGPUModelRenderCached::PrepareBufferRealFar()
{
	if (mBufferAz == 0)
		glGenBuffers(1, &mBufferAz);

	vec4 defValue(0.0, 0.0, 0.0, 0.0);

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferAz);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float)*4, &defValue, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
}

void CGPUModelRenderCached::BindBufferRealFar()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, mBufferAz);
}

void CGPUModelRenderCached::SetBufferRealFarValue(const float value)
{
	if (mBufferAz == 0) return;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferAz);
	
	float *data = (float*) glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*4, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	if (data)
		data[0] = value;

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(float), &value, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 

	CHECK_GL_ERROR();
}

float CGPUModelRenderCached::GetBufferRealFarValue()
{
	if (mBufferAz == 0) return 0.0f;

	float value;

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, mBufferAz);
	float* ptr = (float*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(float)*4, GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT );
	
	glMemoryBarrier(GL_CLIENT_MAPPED_BUFFER_BARRIER_BIT);

	GLsync fence = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	glClientWaitSync(fence, 0, 0);

	if (ptr)
	{
		value = ptr[0];
	}
	else
	{
		value = 20000.0f;
	}
	
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0); 

	CHECK_GL_ERROR();

	return value;
}

void CGPUModelRenderCached::PrepareBufferBSphere()
{
	if (mBSphereCoords.size() > 0)
	{
		if (mBufferBSphere == 0)
		{
			glGenBuffers(1, &mBufferBSphere);
		}

		glBindBuffer(GL_ARRAY_BUFFER, mBufferBSphere);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*mBSphereCoords.size(), mBSphereCoords.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0 );
	}

	if (mBShaderInfo.size() > 0)
	{
		if (mBufferBShader == 0)
		{
			glGenBuffers(1, &mBufferBShader);
		}

		glBindBuffer(GL_ARRAY_BUFFER, mBufferBShader);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vec4)*mBShaderInfo.size(), mBShaderInfo.data(), GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0 );
	}
}

void CGPUModelRenderCached::BindBufferBSphere()
{
	if (mBufferBSphere != 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBufferBSphere );
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 

		glEnableVertexAttribArray(0);
	}
	if (mBufferBShader != 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBufferBShader );
		glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 

		glEnableVertexAttribArray(1);
	}
}

void CGPUModelRenderCached::PrepareBufferIndirect()
{
	if (mBindlessCommands.size() > 0)
	{
		if (mBufferIndirectBindless == 0)
			glGenBuffers(1, &mBufferIndirectBindless);

		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirectBindless );
		glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectBindlessCommandNV) * mBindlessCommands.size(), mBindlessCommands.data(), GL_STATIC_DRAW );
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
	}
	else
	{
		if (mCommands.size() > 0)
		{
			if (mBufferIndirect == 0)
				glGenBuffers(1, &mBufferIndirect);
		
			glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirect );
			glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) * mCommands.size(), mCommands.data(), GL_STREAM_DRAW );
			glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
		}
		if (mCommandsTransparency.size() > 0)
		{
			if (mBufferIndirectTransparency == 0)
				glGenBuffers(1, &mBufferIndirectTransparency);
		
			glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirectTransparency );
			glBufferData( GL_DRAW_INDIRECT_BUFFER, sizeof(DrawElementsIndirectCommand) * mCommandsTransparency.size(), mCommandsTransparency.data(), GL_STREAM_DRAW );
			glBindBuffer( GL_DRAW_INDIRECT_BUFFER, 0 );
		}
	}
}

void CGPUModelRenderCached::BindBufferIndirect()
{
	if (mBufferIndirectBindless > 0)
	{
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirectBindless );
	}
	else
	if (mBufferIndirect > 0)
	{
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirect );
	}
}

void CGPUModelRenderCached::BindBufferIndirectTransparency()
{
	if (mBufferIndirectTransparency > 0)
	{
		glBindBuffer( GL_DRAW_INDIRECT_BUFFER, mBufferIndirectTransparency );
	}
}

void CGPUModelRenderCached::BindBufferIndirectToWrite()
{
	if (mBufferIndirect > 0)
	{
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 1, mBufferIndirect );
	}
	if (mBufferIndirectTransparency > 0)
	{
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, 2, mBufferIndirectTransparency );
	}
}

void CGPUModelRenderCached::PrepareBufferInfosSSBO()
{
	if (mBufferInfos == 0)
		glGenBuffers(1, &mBufferInfos);

	if (mMeshInfos.size() > 0)
	{
		glBindBuffer( GL_SHADER_STORAGE_BUFFER, mBufferInfos );
		glBufferData( GL_SHADER_STORAGE_BUFFER, sizeof(MeshGLSL) * mMeshInfos.size(), mMeshInfos.data(), GL_STATIC_DRAW );
		glBindBuffer( GL_SHADER_STORAGE_BUFFER, 0 );
	}
}

void CGPUModelRenderCached::UpdatePerModelGPUBuffer()
{
	mBufferPerModel.UpdateData( sizeof(ModelGLSL), mModelInfos.size(), mModelInfos.data() );
}

void CGPUModelRenderCached::UpdatePerMeshGPUBuffer()
{
	mBufferPerMesh.UpdateData( sizeof(MeshGLSL), mMeshInfos.size(), mMeshInfos.data() );
}

void CGPUModelRenderCached::BindBufferInfos(const GLuint location)
{
	if (mBufferInfos > 0)
		glBindBufferBase( GL_SHADER_STORAGE_BUFFER, location, mBufferInfos );
}

void CGPUModelRenderCached::PrepRender()
{
	// move buffers to GPU
	if (gUseBindlessUniforms)
	{

		UpdatePerModelGPUBuffer();
		UpdatePerMeshGPUBuffer();
		/*
		// update bindless commands
		const GLuint64	ptr64 = mBufferPerMesh.GetGPUPtr();
		const GLuint perMeshLocation = 5;

		mBindlessCommands.resize( mCommands.size() );

		for (size_t i=0; i<mBindlessCommands.size(); ++i)
		{
			GLuint64 perMeshUniformsGPUPtr = ptr64 + sizeof(MeshGLSL) * i;

			DrawElementsIndirectBindlessCommandNV &bindlessCmd = mBindlessCommands[i];

			bindlessCmd.cmd = mCommands[i];
			bindlessCmd.indexBuffer.address = 0;
			bindlessCmd.indexBuffer.index = 0;
			bindlessCmd.indexBuffer.length = 0;
			bindlessCmd.indexBuffer.reserved = 0;
			bindlessCmd.vertexBuffers[0].address = perMeshUniformsGPUPtr;
			bindlessCmd.vertexBuffers[0].index = perMeshLocation;
			bindlessCmd.vertexBuffers[0].length = sizeof(MeshGLSL);
			bindlessCmd.vertexBuffers[0].reserved = 0;

			bindlessCmd.reserved = 0;
		}
		*/
	}
	else
	{
		PrepareBufferInfosSSBO();
	}
	PrepareBufferIndirect();
	PrepareBufferBSphere();
}

void CGPUModelRenderCached::RenderCulling()
{
	if (mBSphereCoords.size() > 0)
	{

		//BindBufferRealFar();

		// bind bounding attributes buffer 
		BindBufferBSphere();

		// bind indirect buffer to write data to
		BindBufferIndirectToWrite();

		//checkGlError( "bind buffers" );

		// draw points 
		glDrawArrays(GL_POINTS, 0, (GLsizei) mBSphereCoords.size() );

		//checkGlError( "draw occulusion bspheres" );
		/*
		int drawnInstances = 0;
		glBindBuffer(GL_DRAW_INDIRECT_BUFFER, mBufferIndirect);
		DrawElementsIndirectCommand* pointer = (DrawElementsIndirectCommand*)glMapBufferRange(GL_DRAW_INDIRECT_BUFFER, 0, sizeof(DrawElementsIndirectCommand)*mCommands.size(), GL_MAP_READ_BIT);
		for(size_t i = 0; i < mCommands.size(); i++)
		{
			drawnInstances += pointer[i].primCount;
		}
		glUnmapBuffer(GL_DRAW_INDIRECT_BUFFER);
		printf( "Drawing %d out of %d instances\n", drawnInstances, mCommands.size() );

		checkGlError( "output inderect buffer" );
		*/

		//float realZ = GetBufferRealFarValue();
		//printf( "realZ - %.2f\n", realZ );
		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);
	}
}

void CGPUModelRenderCached::RenderNormals()
{
	const int numberOfVertices = mVertexData->GetNumberOfVertices();
	if (numberOfVertices > 0)
	{
		mVertexData->Bind();
		CGPUVertexData::renderPrep();

		glDrawArrays( GL_POINTS, 0, numberOfVertices );

		mVertexData->UnBind();
		CGPUVertexData::renderFinish();
	}
}

void CGPUModelRenderCached::RenderSelection(const int index)
{
	const int numberOfVertices = mVertexData->GetNumberOfVertices();
	if (numberOfVertices > 0)
	{
		if (index <= 0)
		{
			for (size_t i=0; i<mCommands.size(); ++i)
			{
				glLoadName(i+1);

				DrawElementsIndirectCommand	&command = mCommands[i];
				glDrawElements( GL_TRIANGLES, command.count, GL_UNSIGNED_INT, (GLvoid*) command.firstIndex ); 
			}
		}
		else
		{
			DrawElementsIndirectCommand	&command = mCommands[index-1];
			glDrawElements( GL_TRIANGLES, command.count, GL_UNSIGNED_INT, (GLvoid*) command.firstIndex ); 
		}
	}
}

void CGPUModelRenderCached::ReCalculatePerModelInfo( CMaterialsReference*, CShadersReference* )
{
	/*
	const size_t numberOfModels = mModelInfos.size();
	if (numberOfModels > 0)
	{
		mat4 inv;
	
		const int numberOfShaders = shadersManager->GetNumberOfShaders();

		for (size_t i=0; i<numberOfModels; ++i)
		{
			ModelGLSL &data = mModelInfos[i];

			int shaderId = mClientModelInfos[i].shader;

			if (shaderId < 0 || shaderId >= numberOfShaders )
			{
				printf ("wrong material or shader index!!\n");
			}

			data.shader = shadersManager->GetGPUPtr(shaderId);
		}
		
		UpdatePerModelUniforms();
	}
	*/
	UpdatePerModelGPUBuffer();
}

void CGPUModelRenderCached::ReCalculatePerMeshInfo( CMaterialsReference*, CShadersReference* )
{
	/*
	const GLuint64 modelPtr = mBufferPerModel.GetGPUPtr();

	const size_t numberOfMeshes = mMeshInfos.size();
	if (numberOfMeshes > 0)
	{
		mat4 inv;
		
		const int numberOfModels = (int) mClientModelInfos.size();
		const int numberOfMaterials = materialsManager->GetNumberOfMaterials();
		
		for (size_t i=0; i<numberOfMeshes; ++i)
		{
			MeshGLSL &data = mMeshInfos[i];

			int matId = mClientMeshInfos[i].material;
			int modelId = mClientMeshInfos[i].model;

			if (matId < 0 || matId >= numberOfMaterials || modelId < 0 || modelId >= numberOfModels )
			{
				printf ("wrong material or model index!!\n");
			}

			data.material = materialsManager->GetGPUPtr(matId);
			data.model = modelPtr + sizeof(ModelGLSL) * modelId;
		}
		
		UpdatePerMeshGPUBuffer();
	}
	*/

	UpdatePerMeshGPUBuffer();
}

void CGPUModelRenderCached::UpdateColorId(const vec3 &colorId)
{
	for (auto iter=begin(mMeshInfos); iter!=end(mMeshInfos); ++iter)
	{
		iter->color = vec4(colorId.x, colorId.y, colorId.z, 1.0f);
	}
	UpdatePerMeshGPUBuffer();
}

void CGPUModelRenderCached::UpdateReceiveShadows(const int flag)
{
	for (auto iter=begin(mMeshInfos); iter!=end(mMeshInfos); ++iter)
	{
		iter->lightmap = flag;
	}
	UpdatePerMeshGPUBuffer();
}


void CGPUModelRenderCached::UpdateGPUBuffer(const mat4 *m4_parent, const mat4 *modelview)
{
	// update per mesh uniforms
	const bool updateEachFrame = true;

	const size_t numberOfModels = mModelInfos.size();
	// numberOfModels != mBufferPerModel.GetCount() 
	if (updateEachFrame && m4_parent != nullptr && modelview != nullptr)
	{
		mat4 m4_World, inv;
		ModelGLSL *pData = mModelInfos.data();

		for (size_t i=0; i<numberOfModels; ++i, ++pData)
		{
			m4_World = pData->transform;

			pData->normalMatrix = (*m4_parent) * m4_World;
			pData->normalMatrix = (*modelview) * pData->normalMatrix;
			//data.transform = m4_World;
			pData->normalMatrix.set_translation( vec3(0.0f, 0.0f, 0.0f) );
			invert(inv, pData->normalMatrix);
			pData->normalMatrix = transpose(inv);
		}
		
		UpdatePerModelGPUBuffer();
	}
}

bool CGPUModelRenderCached::RenderBegin()
{
	if (mBufferInfos == 0 && mBufferIndirect == 0)
		return false;

	//
	/*
#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif
	*/
	
	
	mVertexData->Bind();		// <<== ERROR HERE ??!!
	CGPUVertexData::renderPrep();
	
	//BindBufferInfos(5);
	

	CHECK_GL_ERROR();

	return true;
}

void CGPUModelRenderCached::RenderOpaque()
{
	BindBufferIndirect();

	// assign attribute with per mesh information
	//const GLuint perMeshLocation = 5;
	//const GLuint perModelLocation = 6;
	
	// use nvidia bindless multidraw
	if (mBindlessCommands.size() > 0)
	{
		glMultiDrawElementsIndirectBindlessNV( GL_TRIANGLES, GL_UNSIGNED_INT, (const GLvoid*) 0, (GLsizei) mBindlessCommands.size(), 0, 1 );
	}
	else
	if (mCommands.size() > 0)
	{
		//
		//mBufferPerMesh.BindAsAttribute( perMeshLocation, 0 );
		//mBufferPerModel.BindAsAttribute( perModelLocation, 0 );

		glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (const GLvoid*) 0, (GLsizei) mCommands.size(), 0 );
	}
	// no multi draw indirect
	/*
	for (size_t i=0; i<mCommands.size(); ++i)
	{
		DrawElementsIndirectCommand &command = mCommands[i];
		MeshDATA &data = mMeshInfos[i];

		
		const GLuint indexSize = command.count;
		const GLuint indexOffset = command.firstIndex;

		glDrawElements( GL_TRIANGLES, indexSize, GL_UNSIGNED_INT, (void*) (indexOffset * sizeof(unsigned int)) );
	}
	*/

}

void CGPUModelRenderCached::RenderTransparency()
{
	BindBufferIndirectTransparency();

	// assign attribute with per mesh information
	//const GLuint perMeshLocation = 5;
	//const GLuint perModelLocation = 6;
	
	if (mCommandsTransparency.size() > 0)
	{
		//
		//mBufferPerMesh.BindAsAttribute( perMeshLocation, 0 );
		//mBufferPerModel.BindAsAttribute( perModelLocation, 0 );

		glMultiDrawElementsIndirect( GL_TRIANGLES, GL_UNSIGNED_INT, (const GLvoid*) 0, (GLsizei) mCommandsTransparency.size(), 0 );
	}
}

void CGPUModelRenderCached::RenderEnd()
{
	mVertexData->UnBind();
	CGPUVertexData::renderFinish();
}

const float CGPUModelRenderCached::CalculateFarDistance(const CFrustum &frustum, const vec3 &eyePos)
{

	float realFarDistance = 1000.0f;

	for (size_t i=0; i<mBSphereCoords.size(); ++i)
	{
		vec4 bsphere = mBSphereCoords[i];
		vec3 pos(bsphere.x, bsphere.y, bsphere.z);
		const float radius = bsphere.w;

		bool IsVisible = frustum.SphereInFrustum( pos.x, pos.y, pos.z, radius );

		if (IsVisible)
		{
			vec3 dir = pos - eyePos;
			float len = radius + sqrt(dir.x*dir.x + dir.y*dir.y + dir.z*dir.z);

			if (len > realFarDistance)
				realFarDistance = len;
		}
	}

	return realFarDistance;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//

CGPUVertexData::CGPUVertexData()
{
	mVao = 0;

	mPointBuffer = 0;
	mNormalBuffer = 0;
	mTangentBuffer = 0;
	mUVBuffer = 0;
	mIndexBuffer = 0;

	mNumberOfVertices = 0;
	mNumberOfIndices = 0;

	mBuffersAllocated = false;
	memset( &mBuffersId, 0, sizeof(GLuint) * VERTEX_BUFFER_MAX );

	for (int i=0; i<VERTEX_BUFFER_MAX; ++i)
		mBuffersOffsets[i] = nullptr;
}

CGPUVertexData::~CGPUVertexData()
{
	//SAFE_ARRAY_DELETE (points);

	if (mVao > 0)
	{
		glDeleteVertexArrays(1, &mVao);
	}

	if (mBuffersAllocated && mBuffersId[0] > 0)
	{
		glDeleteBuffers( VERTEX_BUFFER_MAX, mBuffersId );
	}
}

void CGPUVertexData::QueryAttributes(const GLuint handle)
{
	// ! use extensions from OGL 4.3 - ARB_program_interface_query

	GLint programHandle = handle;
	if (programHandle == 0)
	{
		glGetIntegerv( GL_CURRENT_PROGRAM, &programHandle );
	}

	GLint numberOfAttributes;
	glGetProgramInterfaceiv( programHandle, GL_PROGRAM_INPUT, GL_ACTIVE_RESOURCES, &numberOfAttributes );

	GLenum properties[] = { GL_NAME_LENGTH, GL_TYPE, GL_LOCATION };

	printf( "Active attributes:\n" );
	char name[256];
	for (int i=0; i<numberOfAttributes; ++i)
	{
		GLint results[3];

		glGetProgramResourceiv( programHandle, GL_PROGRAM_INPUT, i, 3, properties, 3, nullptr, results );

		GLint nameBufSize = results[0] + 1;
		memset( name, 0, sizeof(char) * 256 );

		glGetProgramResourceName(programHandle, GL_PROGRAM_INPUT, i, nameBufSize, nullptr, name);

		printf( "%-5d %s (%d)\n", results[2], name, results[1]);
	}

}


// TODO: temp code-  remove it!
void UpdateFromTempArrays( const unsigned int vertexCount, const unsigned int indexCount )
{
	BYTE *points=nullptr;
	BYTE *normals=nullptr;
	BYTE *tangents=nullptr;
	BYTE *uvs = nullptr;
	BYTE *indices = nullptr;
	/*
	points = new BYTE[gPointStride * fileHeader->totalNumberOfVertices];
	if (!points) throw "failed to allocate memory for points\n";	
		
	normals = new BYTE[gNormalStride * fileHeader->totalNumberOfVertices];
	if (!normals) throw "failed to allocate memory for normals\n";
		
	tangents = new BYTE[gTangentStride * fileHeader->totalNumberOfVertices];
	if (!tangents) throw "failed to allocate memory for tangents\n";
		
	uvs = new BYTE[gUVStride * fileHeader->totalNumberOfVertices];
	if (!uvs) throw "failed to allocate memory for uvs\n";

	indices = new BYTE[gIndexStride * fileHeader->totalNumberOfIndices];
	if (!indices) throw "failed to allocate memory for indices\n";
	*/
}

bool CGPUVertexData::UpdateFromCache( const unsigned char *models_data )
{
	if (models_data == nullptr) return false;

	// read global pack header
	FileGeometryHeader *fileHeader = (FileGeometryHeader*) models_data;

	BYTE *points=nullptr;
	BYTE *normals=nullptr;
	BYTE *tangents=nullptr;
	BYTE *uvs = nullptr;
	BYTE *indices = nullptr;

	try
	{
		if (fileHeader->numberOfModels == 0 || fileHeader->totalNumberOfIndices == 0 || fileHeader->totalNumberOfVertices == 0)
			throw "> geometry file is empty\n";
	

		// check if strides are supported
		BYTE *ptr = (BYTE*) models_data + sizeof(FileGeometryHeader);

		points = new BYTE[gPointStride * fileHeader->totalNumberOfVertices];
		if (!points) throw "failed to allocate memory for points\n";	
		
		normals = new BYTE[gNormalStride * fileHeader->totalNumberOfVertices];
		if (!normals) throw "failed to allocate memory for normals\n";
		
		tangents = new BYTE[gTangentStride * fileHeader->totalNumberOfVertices];
		if (!tangents) throw "failed to allocate memory for tangents\n";
		
		uvs = new BYTE[gUVStride * fileHeader->totalNumberOfVertices];
		if (!uvs) throw "failed to allocate memory for uvs\n";

		indices = new BYTE[gIndexStride * fileHeader->totalNumberOfIndices];
		if (!indices) throw "failed to allocate memory for indices\n";
	
		BYTE *lp = points;
		BYTE *ln = normals;
		BYTE *lt = tangents;
		BYTE *lu = uvs;
		BYTE *li = indices;

		unsigned int accumNumberOfVertices = 0;

		for (int i=0; i<fileHeader->numberOfModels; ++i)
		{
			VertexDataHeader	*header = (VertexDataHeader*) ptr;
		
			// WTF - different strides ?!
			if (header->normalStride != gNormalStride || header->pointStride != gPointStride || header->tangentStride != gTangentStride || header->uvStride != gUVStride )
			{
				printf (" strides from geom file are not supported!\n" );
				return false;
			}
			// skip zero geometry
			if (header->numVertices == 0 || header->numIndices == 0)
			{
				continue;
			}

			memcpy( lp, models_data + header->positionOffset, gPointStride * header->numVertices );
			memcpy( ln, models_data + header->normalOffset, gNormalStride * header->numVertices );
			memcpy( lt, models_data + header->tangentOffset, gTangentStride * header->numVertices );
			memcpy( lu, models_data + header->uvOffset, gUVStride * header->numVertices );

			memcpy( li, models_data + header->indicesOffset, gIndexStride * header->numIndices );

			// DONE: we need to add some offset to indices elements
			unsigned int *pIndex = (unsigned int*) li;
			for (int i=0; i<header->numIndices; ++i)
			{
				*pIndex += accumNumberOfVertices;
				pIndex++;
			}

			accumNumberOfVertices += header->numVertices;

			lp += gPointStride * header->numVertices;
			ln += gNormalStride * header->numVertices;
			lt += gTangentStride * header->numVertices;
			lu += gUVStride * header->numVertices;
			li += gIndexStride * header->numIndices;

			//
			ptr = (BYTE*) models_data + header->endOffset;
		}

		//
		// move arrays to gpu !
	
		mNumberOfVertices = fileHeader->totalNumberOfVertices;
		mNumberOfIndices = fileHeader->totalNumberOfIndices;
		// DONE: gpu work
		PrepCacheBuffers( fileHeader->totalNumberOfVertices, fileHeader->totalNumberOfIndices, points, normals, tangents, uvs, indices );
	}
	catch( const char *msg )
	{
		//
		// free
	
		SAFE_ARRAY_DELETE (points);
		SAFE_ARRAY_DELETE (normals);
		SAFE_ARRAY_DELETE (tangents);
		SAFE_ARRAY_DELETE (uvs);
		SAFE_ARRAY_DELETE (indices);

		printf( "ERROR - %s\n", msg );
		return false;
	}

	//
	// free
	
	SAFE_ARRAY_DELETE (points);
	SAFE_ARRAY_DELETE (normals);
	SAFE_ARRAY_DELETE (tangents);
	SAFE_ARRAY_DELETE (uvs);
	SAFE_ARRAY_DELETE (indices);
	
	return true;
}

bool CGPUVertexData::PrepCacheBuffers( const int numberOfVertices, const int numberOfIndices, const BYTE *pointData, const BYTE *normalData, const BYTE *tangentData, const BYTE *uvData, const BYTE *indexData )
{
	if (!pointData || !normalData || !tangentData || !uvData || !indexData)
		return false;

	// store data for ray casting
	mPositions.resize(numberOfVertices);
	mNormals.resize(numberOfVertices);

	memcpy( mPositions.data(), pointData, sizeof(vec4) * numberOfVertices );
	memcpy( mNormals.data(), normalData, sizeof(vec4) * numberOfVertices );

	mIndices.resize(numberOfIndices);
	memcpy( mIndices.data(), indexData, sizeof(unsigned int) * numberOfIndices );

	if (mBuffersId[0] == 0)
	{
		glGenBuffers( VERTEX_BUFFER_MAX, mBuffersId );
		mBuffersAllocated = true;
	}

	// TODO: replace with 1 vertex stream

	glBindBuffer(GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_POINT] );
	glBufferData(GL_ARRAY_BUFFER, gPointStride * numberOfVertices, pointData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_UV] );
	glBufferData(GL_ARRAY_BUFFER, gUVStride * numberOfVertices, uvData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_NORMAL] );
	glBufferData(GL_ARRAY_BUFFER, gNormalStride * numberOfVertices, normalData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_TANGENT] );
	glBufferData(GL_ARRAY_BUFFER, gTangentStride * numberOfVertices, tangentData, GL_STATIC_DRAW);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_INDEX] );
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, gIndexStride * numberOfIndices, indexData, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	CHECK_GL_ERROR();

	return true;
}


const float *CGPUVertexData::MapPositionBuffer()
{
	if (mBuffersId[VERTEX_BUFFER_POINT] == 0)
		return nullptr;

	//return (float*) glMapNamedBuffer(mBuffersId[VERTEX_BUFFER_POINT], GL_READ_ONLY);
	return (float*) mPositions.data();
}

void CGPUVertexData::UnMapPositionBuffer()
{
	//glUnmapNamedBuffer(mBuffersId[VERTEX_BUFFER_POINT]);
}

const float *CGPUVertexData::MapNormalBuffer()
{
	if (mBuffersId[VERTEX_BUFFER_NORMAL] == 0)
		return nullptr;

	//return (float*) glMapNamedBuffer(mBuffersId[VERTEX_BUFFER_NORMAL], GL_READ_ONLY);
	return (float*) mNormals.data();
}

void CGPUVertexData::UnMapNormalBuffer()
{
	//glUnmapNamedBuffer(mBuffersId[VERTEX_BUFFER_NORMAL]);
}

const unsigned int *CGPUVertexData::MapIndexBuffer()
{
	if (mBuffersId[VERTEX_BUFFER_INDEX] == 0)
		return nullptr;

	//return (unsigned int*) glMapNamedBuffer(mBuffersId[VERTEX_BUFFER_INDEX], GL_READ_ONLY);
	return mIndices.data();
}

void CGPUVertexData::UnMapIndexBuffer()
{
	//glUnmapNamedBuffer(mBuffersId[VERTEX_BUFFER_INDEX]);
}


void CGPUVertexData::renderPrep()
{
	
	glEnableVertexAttribArray(0);		// position
	glEnableVertexAttribArray(1);		// tex coords
	glEnableVertexAttribArray(2);		// normal
	//glEnableVertexAttribArray(3);		// tangent
	
	/*
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnableClientState(GL_INDEX_ARRAY);
	*/
}

void CGPUVertexData::renderFinish()
{
	
	glDisableVertexAttribArray(0);		// position
	glDisableVertexAttribArray(1);		// tex coords
	glDisableVertexAttribArray(2);		// normal
	glDisableVertexAttribArray(3);		// tangent
	//glDisableVertexAttribArray(4);		// binormal
	/*
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisableClientState(GL_INDEX_ARRAY);
	*/
}

// DONE: make bind and unbind functions
void CGPUVertexData::Bind()
{
	/*
	if (mVao > 0)
	{
		glBindVertexArray(mVao);
	}
	else
	*/
	if (mBuffersId[0] > 0)
	{
		glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_POINT] );
		//glVertexPointer( 4, GL_FLOAT, 0, (const GLvoid*) 0 );
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) mBuffersOffsets[VERTEX_BUFFER_POINT] ); 
		
		glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_UV] );
		//glTexCoordPointer( 2, GL_FLOAT, 0, (const GLvoid*) 0 );
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) mBuffersOffsets[VERTEX_BUFFER_UV] ); 

		glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_NORMAL] );
		//glNormalPointer( GL_FLOAT, 0, (const GLvoid*) 0 );
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) mBuffersOffsets[VERTEX_BUFFER_NORMAL] ); 

		//glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_TANGENT] );
		//glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, gTangentStride, (const GLvoid*) 0 ); 
	}

	if (mBuffersId[VERTEX_BUFFER_TANGENT] > 0)
	{
		// TODO: maybe put somewhere else ?!
		glEnableVertexAttribArray(3);		// tangent
		//glEnableVertexAttribArray(4);		// binormal

		glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_TANGENT] );
		glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 
		/*
		glBindBuffer( GL_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_BINORMAL] );
		glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 0, (const GLvoid*) 0 ); 
		*/
	}

	if (mBuffersId[VERTEX_BUFFER_INDEX] > 0)
		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mBuffersId[VERTEX_BUFFER_INDEX] );
		//glIndexPointer( GL_UNSIGNED_INT, 0, (const GLvoid*) 0 );
	

}

void CGPUVertexData::UnBind()
{
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// VAO

void CGPUVertexDataVAO::Bind()
{
	mVertexArray.Bind();
}

void CGPUVertexDataVAO::UnBind()
{
	mVertexArray.UnBind();
}

void CGPUVertexDataVAO::UpdateVAO()
{
	// Using Vertex Array Objects (VAO)
	mVertexArray.Bind();

	glBindBuffer( GL_ARRAY_BUFFER, mPointBuffer );
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, mPointStride, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ARRAY_BUFFER, mUVBuffer );
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, mTexCoordStride, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ARRAY_BUFFER, mNormalBuffer );
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, mNormalStride, (const GLvoid*) 0 ); 

	glBindBuffer( GL_ARRAY_BUFFER, mTangentBuffer );
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, mTangentStride, (const GLvoid*) 0 ); 

	//glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );

	mVertexArray.UnBind();
}

void CGPUVertexDataVAO::RenderPrepVAO()
{
	// For the Vertex Array Objects (VAO), enable the vertex attributes
		
	glEnableVertexAttribArray(0);		// position
	glEnableVertexAttribArray(1);		// tex coords
	glEnableVertexAttribArray(2);		// normal
	glEnableVertexAttribArray(3);		// tangent
	glEnableVertexAttribArray(4);		// per mesh uniform pointer
}

void CGPUVertexDataVAO::RenderFinishVAO()
{
	// VAO
	glDisableVertexAttribArray(0);		// position
	glDisableVertexAttribArray(1);		// tex coords
	glDisableVertexAttribArray(2);		// normal
	glDisableVertexAttribArray(3);		// tangent
	glDisableVertexAttribArray(4);		// per mesh uniform pointer

	glBindVertexArray ( 0 );	
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
// Vertex Buffer Unified Memory (VBUM)

CGPUVertexDataVBUM::CGPUVertexDataVBUM()
{
	mPointBufferSize = 0;
	mNormalBufferSize = 0;
	mTangentBufferSize = 0;
	mUVBufferSize = 0;
	mIndexBufferSize = 0;

	mPointBufferGPUPtr = 0;
	mNormalBufferGPUPtr = 0;	
	mTangentBufferGPUPtr = 0;	
	mUVBufferGPUPtr = 0;	
	mIndexBufferGPUPtr = 0;	
}

CGPUVertexDataVBUM::~CGPUVertexDataVBUM()
{

}

void CGPUVertexDataVBUM::Bind()
{
	// Set up the pointers in GPU memory to the vertex attributes.
	// Set GPU pointer to the vertex buffer was stored in Mesh::update() after the buffer was filled
	glBufferAddressRangeNV( GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 0, mPointBufferGPUPtr, mPointBufferSize );
	glBufferAddressRangeNV( GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 1, mUVBufferGPUPtr, mUVBufferSize );
	glBufferAddressRangeNV( GL_VERTEX_ATTRIB_ARRAY_ADDRESS_NV, 2, mNormalBufferGPUPtr, mNormalBufferSize );

	// set up the pointer in GPU memory to the index buffer
	glBufferAddressRangeNV( GL_ELEMENT_ARRAY_ADDRESS_NV, 0, mIndexBufferGPUPtr, mIndexBufferSize );
}

void CGPUVertexDataVBUM::UnBind()
{

}

void CGPUVertexDataVBUM::UpdateVBUM()
{
	//
	// get the GPU pointer for the vertex buffer and make the vertex buffer resident on the GPU
	glBindBuffer( GL_ARRAY_BUFFER, mPointBuffer );
	glGetBufferParameterui64vNV( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mPointBufferGPUPtr );
	glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &mPointBufferSize );
	glMakeBufferResidentNV( GL_ARRAY_BUFFER, GL_READ_ONLY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, mNormalBuffer );
	glGetBufferParameterui64vNV( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mNormalBufferGPUPtr );
	glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &mNormalBufferSize );
	glMakeBufferResidentNV( GL_ARRAY_BUFFER, GL_READ_ONLY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, mTangentBuffer );
	glGetBufferParameterui64vNV( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mTangentBufferGPUPtr );
	glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &mTangentBufferSize );
	glMakeBufferResidentNV( GL_ARRAY_BUFFER, GL_READ_ONLY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	glBindBuffer( GL_ARRAY_BUFFER, mUVBuffer );
	glGetBufferParameterui64vNV( GL_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mUVBufferGPUPtr );
	glGetBufferParameteriv( GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &mUVBufferSize );
	glMakeBufferResidentNV( GL_ARRAY_BUFFER, GL_READ_ONLY );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );

	//
	// get the GPU pointer to the index buffer and make the index buffer resident on the GPU
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, mIndexBuffer );
	glGetBufferParameterui64vNV( GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_GPU_ADDRESS_NV, &mIndexBufferGPUPtr );
	glGetBufferParameteriv( GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &mIndexBufferSize );
	glMakeBufferResidentNV( GL_ELEMENT_ARRAY_BUFFER, GL_READ_ONLY );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void CGPUVertexDataVBUM::RenderPrepVBUM()
{
	// Specify the vertex format
	glVertexAttribFormatNV( 0, 3, GL_FLOAT, GL_FALSE, mPointStride );		// position attribute 0 - 4 floats
	glVertexAttribFormatNV( 1, 2, GL_FLOAT, GL_FALSE, mTexCoordStride );		// texcoords attribute 1 - 4 floats (two channels)
	glVertexAttribFormatNV( 2, 3, GL_FLOAT, GL_FALSE, mNormalStride );		// normal attribute 2 - 3 floats
	glVertexAttribFormatNV( 3, 3, GL_FLOAT, GL_FALSE, mTangentStride );		// tangent attribute 3 - 3 floats

	// Enable the relevent attributes
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	// Enable Vertex Buffer Unified Memory (VBUM) for the vertex attributes
	glEnableClientState( GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV );

	// Enable Vertex Buffer Unified Memory (VBUM) for the indices
	glEnableClientState( GL_ELEMENT_ARRAY_UNIFIED_NV );
}

void CGPUVertexDataVBUM::RenderFinishVBUM()
{
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(3);

	glDisableClientState( GL_VERTEX_ATTRIB_ARRAY_UNIFIED_NV );
	glDisableClientState( GL_ELEMENT_ARRAY_UNIFIED_NV );
}


