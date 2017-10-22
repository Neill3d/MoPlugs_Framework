
//////////////////////////////////////////////////////////////////////////////////////////////////
//
// file: MAIN.cpp
//
//	Author Sergey Solokhin (Neill3d)
//
//
//	GitHub page - https://github.com/Neill3d/MoPlugs_Framework
//	Licensed under BSD 3-Clause - https://github.com/Neill3d/MoPlugs_Framework/blob/master/LICENSE
//
///////////////////////////////////////////////////////////////////////////////////////////////////

//------------------------- glew libr
#include <GL\glew.h>

//#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

#include "gpucache_visitorImpl.h"
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

#define MOPLUGS_SYS_ENV				"MOPLUGS_SHADERS"
#define UBERSHADER_EFFECT			"ProjectiveMapping.glslfx"

vec3						m_cameraPos(100.0f, 0.0f, 100.0f);
mat4						m_projectionMatrix;
mat4						m_modelMatrix;
CCameraInfoCache			mCameraCache;

Graphics::ShaderEffect		*mUberShader = nullptr;
CGPUCacheModel				*mCacheModel = nullptr;
CGPULightsManager			mLightManager;

//
bool loadModel(const char *filename);

//////////////////////////////////////////////////////////////////////////////////////
//

void DebugOGL_Callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const char *message, const void*userParam)
{
	if (GL_DEBUG_SEVERITY_HIGH == severity)
	{
		printf("%s\n", message);
	}
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);

	perspective(m_projectionMatrix, (float)mCameraCache.fov, (float)width/(float)height, 
		(float)mCameraCache.nearPlane, (float)mCameraCache.farPlane);

	mCameraCache.width = width;
	mCameraCache.height = height;
		
	mCameraCache.p4 = m_projectionMatrix.mat_array;
}

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (action != GLFW_PRESS)
        return;

    switch (key)
    {
        case GLFW_KEY_SPACE:
            glfwSetTime(0.0);
            break;
    }
}

static void drop_callback(GLFWwindow*,int count,const char**files)
{
	// TODO: read some new xml file

	if (count > 0)
	{
		loadModel( files[0] );
	}
}

static void usage(void)
{
    printf("Usage: fsaa [-h] [-s SAMPLES]\n");
}

void draw(float time)
{
	glEnable(GL_DEPTH_TEST);

	glClearColor(0.4f, .4f, .4f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	vec3 center(0.0f, 0.0f, 0.0f);
	vec3 up(0.0f, 1.0f, 0.0f);

	mat4 modelView, invModelView;
	look_at(modelView, m_cameraPos, center, up);
	invert(invModelView, modelView);
		
	mCameraCache.mv4 = modelView.mat_array;
	mCameraCache.mvInv4 = modelView.mat_array;
	mCameraCache.pos = m_cameraPos.vec_array;

	mLightManager.Prep( mCameraCache, nullptr );

	if (mCacheModel && mUberShader)
	{
		mUberShader->UploadCameraUniforms(mCameraCache);

		mUberShader->SetNumberOfProjectors(0);
		mUberShader->UploadModelTransform( mat4(array16_id) );
	
		mUberShader->UploadLightingInformation( false,
				vec4(0.3f, 0.3f, 0.3f, 0.0f), 
				0, 0);

		// models normal matrix update, according to camera modelview
		mCacheModel->PrepRender( mCameraCache, mUberShader, false, nullptr);

		mCacheModel->RenderBegin( mCameraCache, mUberShader, false, 
		 false, nullptr );
	
		mCacheModel->RenderOpaque( mCameraCache, mUberShader );
		mCacheModel->RenderEnd( mCameraCache, mUberShader);
		
		CHECK_GL_ERROR();
	}
}

bool loadModel(const char *filename)
{
	if (mCacheModel)
	{
		delete mCacheModel;
		mCacheModel = nullptr;
	}

	mCacheModel = new CGPUCacheModel();

	CGPUCacheLoader					loader;
	CGPUCacheLoaderVisitorImpl		visitor(mCacheModel);

	if (false == loader.Load( filename, &visitor ) )
	{
		delete mCacheModel;
		mCacheModel = nullptr;
		return false;
	}
	else
	{
		vec3 bmin, bmax;
		mCacheModel->GetBoundingBox(bmin.vec_array, bmax.vec_array);
		
		const float scale = nv_norm(bmax-bmin);
		m_cameraPos = bmin;
		//m_cameraPos.z *= -1.0f;

		mCameraCache.farPlane = 2.0f * scale;
		mCameraCache.realFarPlane = 2.0f * scale;
		
		mCacheModel->OverrideShading = true;
		mCacheModel->ShadingType = eShadingTypeFlat;

		//reshape( mCameraCache.width, mCameraCache.height );
	}

	return true;
}

std::string getEnvVar( std::string const &key )
{
	char * val = getenv( key.c_str() );
	return val == NULL ? std::string("") : std::string(val);
}

bool loadShader()
{
	if (mUberShader)
	{
		delete mUberShader;
		mUberShader = nullptr;
	}

	//
	mUberShader = new Graphics::ShaderEffect();
	// "D:\\Work\\MOPLUGS\\_Plugins\\GLSLFX\\"
	//const std::string envVar (getEnvVar( MOPLUGS_SYS_ENV ));
	const std::string envVar ("D:\\Work\\MOPLUGS_TheWALL\\_Plugins\\GLSLFX\\");
	
	if( !mUberShader->Initialize( envVar.c_str(), UBERSHADER_EFFECT, 512, 512, 1.0) )
	{
		delete mUberShader;
		mUberShader = nullptr;
		return false;
	}

	CHECK_GL_ERROR();

	return true;
}

bool freeResources()
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
	return true;
}

int main(int argc, char** argv)
{
    int ch, samples = 4;
    GLFWwindow* window=nullptr;

	std::string	filename("");

    while ((ch = getopt(argc, argv, "hsf:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                samples = atoi(optarg);
                break;
			case 'f':
				filename = optarg;
				break;
            default:
                usage();
                exit(EXIT_FAILURE);
        }
    }
	
    glfwSetErrorCallback(error_callback);

    if (!glfwInit())
        exit(EXIT_FAILURE);

    if (samples)
        printf("Requesting OpenGL context with %i samples\n", samples);
    else
        printf("Requesting that FSAA not be available\n");

    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    window = glfwCreateWindow(800, 600, "Viewer App [OpenGL]", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

	// moplugs framework uses glew
	const GLenum err = glewInit();
	if (GLEW_OK != err)
    {
        printf ( "glewInit failed, aborting.\n" );
		glfwTerminate();
        exit (EXIT_FAILURE);
	}
	// Check extensions; exit on failure
    
	try
	{
		const int extCount = 6;
		const char *extensions[extCount] = { "GL_ARB_multisample", "GL_NV_vertex_buffer_unified_memory",
			"GL_NV_shader_buffer_load", "GL_EXT_direct_state_access", "GL_NV_bindless_texture", "GL_ARB_get_program_binary" };

		for (int i=0; i<extCount; ++i)
		{
			if (!glewIsExtensionSupported( extensions[i] ))
				throw extensions[i];
			//if ( !glfwExtensionSupported( extensions[i] ) )
			//	throw extensions[i];
		}
	}
	catch (const char *what)
    {
        printf( "[ERROR] %s is not supported\n", what );

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

#ifdef _DEBUG
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback( DebugOGL_Callback, nullptr );
#endif

	//
	mCameraCache.fov = 35.0;
	mCameraCache.nearPlane = 10.0;
	mCameraCache.farPlane = 4000.0;
	mCameraCache.width = 800.0;
	mCameraCache.height = 600.0;

    glfwShowWindow(window);

    glGetIntegerv(GL_SAMPLES_ARB, &samples);
    if (samples)
        printf("Context reports FSAA is available with %i samples\n", samples);
    else
        printf("Context reports FSAA is unavailable\n");

	//
	if (false == loadShader() )
	{
		printf( "[ERROR] Failed to load a shader ! \n" );

        glfwTerminate();
        exit(EXIT_FAILURE);
	}

	if ( filename.size() > 0 )
	{
		loadModel(filename.c_str() );
		framebuffer_size_callback(window, 800, 600);
	}

	// drop any geom cache file here (*.xml)
	glfwSetDropCallback(window, drop_callback);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat time = (GLfloat) glfwGetTime();

        draw(time);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

