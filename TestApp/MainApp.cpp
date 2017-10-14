
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MainApp.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

#include "NvAppBase\NvSampleApp.h"
#include "NvAppBase\NvInputTransformer.h"

#include "gpucache_visitor.h"
#include "Shader.h"
#include "shared_camera.h"
#include "gpucache_model.h"
#include "gpucache_loader.h"
#include "shared_lights.h"

/*
	TODO:
	1 + uber shader
	2 + load renderModelCached from command line
	3 + render geomCache
	4 - compute cluster lighting, use lights from xml
	5 - cast shadows, switch logarithmic depth

	25.10.15
	Fixed transformation bug and textures bug for loaded meshes

	17.10.15
	Geometry cache is loaded with bugs, wrong indices and black textures.
*/

#define UBERSHADER_EFFECT			"ProjectiveMapping.glslfx"

///////////////////////////////////////////////////////////////////////////
class App : public NvSampleApp
{
public:
	App(NvPlatformContext *platform)
		: NvSampleApp( platform, "Viewer App")
	{
		mCacheModel = nullptr;
		mUberShader = nullptr;

		m_transformer->setTranslationVec(nv::vec3f(0.0f, 1000.0f, -2000.2f));
		m_transformer->setRotationVec(nv::vec3f(NV_PI*-.35f, 0.0f, 0.0f));
		m_transformer->setMaxTranslationVel( 5000.0f );
		//m_transformer->setMotionMode( NvCameraMotionType::FIRST_PERSON );
		// Required in all subclasses to avoid silent link issues
		forceLinkHack();

		mCameraCache.width = 800;
		mCameraCache.height = 600;
		mCameraCache.fov = 45.0f * 2.0f*3.14159f / 360.0f;
		mCameraCache.nearPlane = 10.0f;
		mCameraCache.farPlane = 40000.0f;
		mCameraCache.realFarPlane = 40000.0f;
		mCameraCache.pUserData = nullptr;
	}

	~App()
	{
		if (mCacheModel)
		{
			delete mCacheModel;
			mCacheModel = nullptr;
		}

		if (mUberShader)
		{
			delete mUberShader;
			mUberShader = nullptr;
		}
	}

	std::string getEnvVar( std::string const &key ) const
	{
		char * val = getenv( key.c_str() );
		return val == NULL ? std::string("") : std::string(val);
	}

	void initRendering(void)
	{
		// Check extensions; exit on failure
		if(!requireExtension("GL_NV_vertex_buffer_unified_memory")) return;
		if(!requireExtension("GL_NV_shader_buffer_load")) return;
		if(!requireExtension("GL_EXT_direct_state_access")) return;
		if(!requireExtension("GL_NV_bindless_texture")) return;

		auto &cmds = getPlatformContext()->getCommandLine();

		//showDialog( "command line", cmds[0].c_str(), false);

		if (cmds.size() == 1 && cmds[0].find(".xml") > 0)
		{
			mCacheModel = new CGPUCacheModel();

			CGPUCacheLoader					loader;
			CGPUCacheLoaderVisitorImpl		visitor(mCacheModel);

			if (false == loader.Load( cmds[0].c_str(), &visitor ) )
			{
				delete mCacheModel;
				mCacheModel = nullptr;
			}
			else
			{
				nv::vec3f bmin, bmax;
				mCacheModel->GetBoundingBox(bmin, bmax);
				float scale = nv::length(bmax-bmin);
				//m_transformer->setTranslationVec( bmin + 0.5 * (bmax-bmin) );
				m_transformer->setTranslationVec(nv::vec3f(0.0f, 0.0f, -scale) );
				//m_transformer->setScale( 0.5f );

				m_modelMatrix.make_identity();
				nv::vec3f v( bmin + 0.5 * (bmax-bmin) );
				m_modelMatrix.set_translate( -1.0f * v );

				mCameraCache.farPlane = 2.0f * scale;
				mCameraCache.realFarPlane = 2.0f * scale;
		
				mCacheModel->OverrideShading = true;
				mCacheModel->ShadingType = eShadingTypeFlat;

				reshape( mCameraCache.width, mCameraCache.height );
			}
		}

		CHECK_GL_ERROR();

		//
		mUberShader = new Graphics::ShaderEffect();
		// "D:\\Work\\MOPLUGS\\_Plugins\\GLSLFX\\"
		if( !mUberShader->Initialize( getEnvVar( "MOPLUGS_SHADERS" ).c_str(), UBERSHADER_EFFECT, 512, 512, 1.0) )
		{
			delete mUberShader;
			mUberShader = nullptr;
		}

		CHECK_GL_ERROR();
	}
	
	void initUI(void)
	{
	}

	void draw(void)
	{
		glEnable(GL_DEPTH_TEST);

		glClearColor(0.4f, .4f, .4f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		nv::matrix4f modelView = m_transformer->getModelViewMat();
		nv::matrix4f invModelView = nv::inverse(modelView);
		
		mCameraCache.mv4 = modelView.get_value();
		mCameraCache.mvInv4 = modelView.get_value();
		mCameraCache.pos = m_transformer->getTranslationVec().get_value();

		mLightManager.Prep( mCameraCache, nullptr );

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glMultMatrixf( m_projectionMatrix.get_value() );

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glMultMatrixf( modelView.get_value() );

		float axisScale = 1000.0f;

		glBegin(GL_LINES);

		glColor3f(1.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(5.0f * axisScale, 0.0f, 0.0f);

		glColor3f(0.0f, 1.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 5.0f * axisScale, 0.0f);

		glColor3f(0.0f, 0.0f, 1.0f);
		glVertex3f(0.0f, 0.0f, 0.0f);
		glVertex3f(0.0f, 0.0f, 5.0f * axisScale);

		glEnd();

		
		if (mCacheModel && mUberShader)
		{
			mCacheModel->SetMatrix( m_modelMatrix._array );
			mCacheModel->Render( mCameraCache, mUberShader, &mLightManager, false, nullptr );

			CHECK_GL_ERROR();
		}
		
	}

	void reshape(int32_t width, int32_t height)
	{
		glViewport( 0, 0, (GLint) width, (GLint) height );

		nv::perspective(m_projectionMatrix, (float)mCameraCache.fov, (float)width/(float)height, (float)mCameraCache.nearPlane, (float)mCameraCache.farPlane);

		mCameraCache.width = width;
		mCameraCache.height = height;
		
		mCameraCache.p4 = m_projectionMatrix.get_value();
	}


protected:

	nv::matrix4f				m_projectionMatrix;
	nv::matrix4f				m_modelMatrix;
	CCameraInfoCache			mCameraCache;

	Graphics::ShaderEffect		*mUberShader;
	CGPUCacheModel				*mCacheModel;
	CGPULightsManager			mLightManager;
};

NvAppBase *NvAppFactory(NvPlatformContext *platform) {
	return new App(platform);
}

/*
int main(int argc, char** argv)
{
}
*/