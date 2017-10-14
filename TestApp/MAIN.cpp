//========================================================================
// Full screen anti-aliasing test
// Copyright (c) Camilla Berglund <elmindreda@elmindreda.org>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================
//
// This test renders two high contrast, slowly rotating quads, one aliased
// and one (hopefully) anti-aliased, thus allowing for visual verification
// of whether FSAA is indeed enabled
//
//========================================================================

//------------------------- glew libr
#include <GL\glew.h>

//#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>

#include <stdio.h>
#include <stdlib.h>

#include "getopt.h"

//------------------------- Effect GLSL Module
#include "FxParser.h"

#include <assert.h>

#   define  LOGI(...)  printf(__VA_ARGS__)
#   define  LOGW(...)  printf(__VA_ARGS__)
#   define  LOGE(...)  printf(__VA_ARGS__)
#   define  LOGOK(...)  printf(__VA_ARGS__)

char	gResourcesPath[256];

//------------------------------------------------------------------------------
void errorCallbackFunc(const char *errMsg)
{
#ifdef WIN32
    OutputDebugString(errMsg);
	printf(errMsg);
#endif
    printf(errMsg);
}
void includeCallbackFunc(const char *incName, FILE *&fp, const char *&buf)
{
    char fullpath[200];
    fopen_s(&fp, incName, "r");
    if(fp) return;

    sprintf_s(fullpath, 200, "%s\\%s", gResourcesPath, incName);
    fopen_s(&fp, fullpath, "r");
    if(fp) return;
}

class Shader
{
public:

	//! a constructor
	Shader()
	{
		fx_EffectMaterial = nullptr;

		fx_TechCurrent = nullptr;
		fx_pass = nullptr;
	}
	//! a destructor
	virtual ~Shader()
	{
		Free();
	}

	bool InitializeEffectParams()
	{
		//
		// Let's keep track in interface pointers everything, for purpose of clarity
		//
	
		fx_TechCurrent = fx_EffectMaterial->findTechnique("t0");
		if(fx_TechCurrent && (!fx_TechCurrent->validate()))
			return false;

		return true;
	}
	
	bool loadMaterialEffect(const char *effectFileName)
	{
		if(fx_EffectMaterial)
		{
			LOGI("Desroying previous material Effect\n");
			//fx_EffectMaterial->destroy();
			//or
			LOGI("=========> Destroying effect\n");
			nvFX::IContainer::destroy(fx_EffectMaterial);
			fx_EffectMaterial = NULL;
		}
		LOGI("Creating Effect (material)\n");
		fx_EffectMaterial = nvFX::IContainer::create("material");
		bool bRes = nvFX::loadEffectFromFile(fx_EffectMaterial, effectFileName);
        
		if(!bRes)
		{
			LOGE("Failed\n");
			return false;
		}
		LOGOK("Loaded\n");
    
		if (InitializeEffectParams() == false)
		{
			LOGE("Failed to initialized uniforms\n");
			return false;
		}

		return true;
	}

    // Call after contruction.
    virtual bool	Initialize(const char* resourcePath, const char *effectName)
	{
		sprintf_s( gResourcesPath, 256, "%s", resourcePath );
		char szMaterialEffectPath[256];

		sprintf_s( szMaterialEffectPath, 256, "%s\\%s", resourcePath, effectName );

		//
		// Effects
		//
		nvFX::setErrorCallback(errorCallbackFunc);
		nvFX::setIncludeCallback(includeCallbackFunc);
		if (false == loadMaterialEffect( szMaterialEffectPath ) ) 
			return false;

		return true;
	}
	// on destructor
	virtual void	Free()
	{
		if (fx_EffectMaterial)
		{
			nvFX::IContainer::destroy(fx_EffectMaterial);
			fx_EffectMaterial = NULL;
		}
	}
	// not used
	void			ReSize(const int w, const int h)
	{
		nvFX::getResourceRepositorySingleton()->setParams(0,0,w,h,1,0,NULL );
		bool failed = nvFX::getResourceRepositorySingleton()->validateAll() ? false : true;
		if(failed)
			assert(!"Oops");
	}

	
	// bind and unbind a zero pass of a current technique
	virtual void		Bind()
	{
		if (fx_TechCurrent) fx_pass = fx_TechCurrent->getPass(0);
		if (fx_pass) fx_pass->execute();
	}

	virtual void		UnBind()
	{
		if (fx_pass)
			fx_pass->unbindProgram();
	}

protected:

	//
	// stuff effect, tech and pass interfaces
	//
	nvFX::IContainer	*fx_EffectMaterial;

	nvFX::ITechnique	*fx_TechCurrent;
	nvFX::IPass			*fx_pass;
};

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
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

static void drop_callback(GLFWwindow*,int,const char**)
{
	// TODO: read some new xml file
}

static void usage(void)
{
    printf("Usage: fsaa [-h] [-s SAMPLES]\n");
}

void draw(float time)
{
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(0.0, 0.0, 0.0, 1.0);
    glLoadIdentity();
    glTranslatef(0.25f, 0.25f, 0.f);
    glRotatef(time, 0.f, 0.f, 1.f);

    glDisable(GL_MULTISAMPLE_ARB);
    glRectf(-0.15f, -0.15f, 0.15f, 0.15f);

    glLoadIdentity();
    glTranslatef(0.75f, 0.25f, 0.f);
    glRotatef(time, 0.f, 0.f, 1.f);

    glEnable(GL_MULTISAMPLE_ARB);
    glRectf(-0.15f, -0.15f, 0.15f, 0.15f);
}

int main(int argc, char** argv)
{
    int ch, samples = 4;
    GLFWwindow* window=nullptr;

    while ((ch = getopt(argc, argv, "hs:")) != -1)
    {
        switch (ch)
        {
            case 'h':
                usage();
                exit(EXIT_SUCCESS);
            case 's':
                samples = atoi(optarg);
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
        printf("Requesting FSAA with %i samples\n", samples);
    else
        printf("Requesting that FSAA not be available\n");

    glfwWindowHint(GLFW_SAMPLES, samples);
    glfwWindowHint(GLFW_VISIBLE, GL_FALSE);

    window = glfwCreateWindow(800, 400, "Aliasing Detector", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

	// Check extensions; exit on failure
    
    if (!glfwExtensionSupported("GL_ARB_multisample")
		|| !glfwExtensionSupported("GL_NV_vertex_buffer_unified_memory")
		|| !glfwExtensionSupported("GL_NV_shader_buffer_load")
		|| !glfwExtensionSupported("GL_EXT_direct_state_access")
		|| !glfwExtensionSupported("GL_NV_bindless_texture") )
    {
        printf("some of extensions is not supported\n");

        glfwTerminate();
        exit(EXIT_FAILURE);
    }

    glfwShowWindow(window);

    glGetIntegerv(GL_SAMPLES_ARB, &samples);
    if (samples)
        printf("Context reports FSAA is available with %i samples\n", samples);
    else
        printf("Context reports FSAA is unavailable\n");

    glMatrixMode(GL_PROJECTION);
    glOrtho(0.f, 1.f, 0.f, 0.5f, 0.f, 1.f);
    glMatrixMode(GL_MODELVIEW);

	Shader	shader;
	shader.Initialize( "C:\\Work\\TestApp\\bin\\", "Test.glsl" );

	// drop any geom cache file here (*.xml)
	glfwSetDropCallback(window, drop_callback);

    while (!glfwWindowShouldClose(window))
    {
        GLfloat time = (GLfloat) glfwGetTime();

		shader.Bind();

        draw(time);

		shader.UnBind();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    exit(EXIT_SUCCESS);
}

