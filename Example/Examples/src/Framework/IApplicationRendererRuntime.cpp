/*********************************************************\
 * Copyright (c) 2012-2014 Christian Ofenberg
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software
 * and associated documentation files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
\*********************************************************/


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "PrecompiledHeader.h"


//[-------------------------------------------------------]
//[ Preprocessor                                          ]
//[-------------------------------------------------------]
#ifndef RENDERER_NO_RUNTIME


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Framework/IApplicationRendererRuntime.h"
#ifdef SHARED_LIBRARIES
	// Dynamically linked libraries
	#ifdef WIN32
		#include "Framework/WindowsHeader.h"

	#elif defined LINUX
		#include "Framework/LinuxHeader.h"

		#include <dlfcn.h>
	#else
		#error "Unsupported platform"
	#endif

	#include <stdio.h>
#endif

#include <string.h>


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
IApplicationRendererRuntime::~IApplicationRendererRuntime()
{
	// Nothing to do in here
	// m_pRendererRuntime is destroyed within onDeinitialization()
}


//[-------------------------------------------------------]
//[ Public virtual IApplication methods                   ]
//[-------------------------------------------------------]
void IApplicationRendererRuntime::onInitialization()
{
	// Call the base implementation
	IApplicationRenderer::onInitialization();

	// Is there a valid renderer instance?
	Renderer::IRenderer *renderer = getRenderer();
	if (nullptr != renderer)
	{
		// Create the renderer runtime instance
		mRendererRuntime = createRendererRuntimeInstance(*renderer);
	}
}

void IApplicationRendererRuntime::onDeinitialization()
{
	// Delete the renderer runtime instance
	mRendererRuntime = nullptr;

	// Destroy the shared library instance
	#ifdef SHARED_LIBRARIES
		#ifdef WIN32
			if (nullptr != mRendererRuntimeSharedLibrary)
			{
				::FreeLibrary(static_cast<HMODULE>(mRendererRuntimeSharedLibrary));
				mRendererRuntimeSharedLibrary = nullptr;
			}
		#elif defined LINUX
			if (nullptr != mRendererRuntimeSharedLibrary)
			{
				::dlclose(mRendererRuntimeSharedLibrary);
				mRendererRuntimeSharedLibrary = nullptr;
			}
		#else
			#error "Unsupported platform"
		#endif
	#endif
}


//[-------------------------------------------------------]
//[ Protected methods                                     ]
//[-------------------------------------------------------]
IApplicationRendererRuntime::IApplicationRendererRuntime(const char *rendererName) :
	IApplicationRenderer(rendererName),
	mRendererRuntimeSharedLibrary(nullptr)
{
	// Nothing to do in here
}


//[-------------------------------------------------------]
//[ Private methods                                       ]
//[-------------------------------------------------------]
RendererRuntime::IRendererRuntime *IApplicationRendererRuntime::createRendererRuntimeInstance(Renderer::IRenderer &renderer)
{
	#ifdef SHARED_LIBRARIES
		// Dynamically linked libraries
		#ifdef WIN32
			// Load in the dll
			#ifdef _DEBUG
				static const char RENDERER_RUNTIME_FILENAME[] = "RendererRuntimeD.dll";
			#else
				static const char RENDERER_RUNTIME_FILENAME[] = "RendererRuntime.dll";
			#endif
			mRendererRuntimeSharedLibrary = ::LoadLibraryExA(RENDERER_RUNTIME_FILENAME, nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
			if (nullptr != mRendererRuntimeSharedLibrary)
			{
				// Get the "createRendererRuntimeInstance()" function pointer
				void *symbol = ::GetProcAddress(static_cast<HMODULE>(mRendererRuntimeSharedLibrary), "createRendererRuntimeInstance");
				if (nullptr != symbol)
				{
					// "createRendererRuntimeInstance()" signature
					typedef RendererRuntime::IRendererRuntime *(__cdecl *createRendererRuntimeInstance)(Renderer::IRenderer &renderer);

					// Create the renderer runtime instance
					return static_cast<createRendererRuntimeInstance>(symbol)(renderer);
				}
				else
				{
					// Error!
					OUTPUT_DEBUG_PRINTF("Failed to locate the entry point \"createRendererRuntimeInstance\" within the renderer runtime shared library \"%s\"", RENDERER_RUNTIME_FILENAME)
				}
			}
			else
			{
				OUTPUT_DEBUG_PRINTF("Failed to load in the shared library \"%s\"\n", RENDERER_RUNTIME_FILENAME)
			}
		#elif defined LINUX
			// Load in the shared library
			#ifdef _DEBUG
				static const char RENDERER_RUNTIME_FILENAME[] = "RendererRuntimeD.so";
			#else
				static const char RENDERER_RUNTIME_FILENAME[] = "RendererRuntime.so";
			#endif
			mRendererRuntimeSharedLibrary = dlopen(RENDERER_RUNTIME_FILENAME, RTLD_NOW);
			if (nullptr != mRendererRuntimeSharedLibrary)
			{
				// Get the "createRendererRuntimeInstance()" function pointer
				void *symbol = dlsym(mRendererRuntimeSharedLibrary, "createRendererRuntimeInstance");
				if (nullptr != symbol)
				{
					// "createRendererRuntimeInstance()" signature
					typedef RendererRuntime::IRendererRuntime *(*createRendererRuntimeInstance)(Renderer::IRenderer &renderer);

					// Create the renderer runtime instance
					return reinterpret_cast<createRendererRuntimeInstance>(symbol)(renderer);
				}
				else
				{
					// Error!
					OUTPUT_DEBUG_PRINTF("Failed to locate the entry point \"createRendererRuntimeInstance\" within the renderer runtime shared library \"%s\"", RENDERER_RUNTIME_FILENAME)
				}
			}
			else
			{
				OUTPUT_DEBUG_PRINTF("Failed to load in the shared library \"%s\"\n", RENDERER_RUNTIME_FILENAME)
			}
		#else
			#error "Unsupported platform"
		#endif

		// Error!
		return nullptr;
	#else
		// Statically linked libraries

		// "createRendererRuntimeInstance()" signature
		extern RendererRuntime::IRendererRuntime *createRendererRuntimeInstance(Renderer::IRenderer &renderer);

		// Create the renderer runtime instance
		return createRendererRuntimeInstance(renderer);
	#endif
}


//[-------------------------------------------------------]
//[ Preprocessor                                          ]
//[-------------------------------------------------------]
#endif // RENDERER_NO_RUNTIME