/*********************************************************\
 * Copyright (c) 2012-2015 Christian Ofenberg
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
//[ Header guard                                          ]
//[-------------------------------------------------------]
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "OpenGLRenderer/IContext.h"

#include <Renderer/PlatformTypes.h>
#include <Renderer/WindowsHeader.h>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace OpenGLRenderer
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Windows OpenGL context class
	*/
	class ContextWindows : public IContext
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] nativeWindowHandle
		*    Optional native main window handle, can be a null handle
		*/
		explicit ContextWindows(handle nativeWindowHandle);

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~ContextWindows();

		/**
		*  @brief
		*    Return the primary device context
		*
		*  @return
		*    The primary device context, null pointer on error
		*/
		inline HDC getDeviceContext() const;

		/**
		*  @brief
		*    Return the primary render context
		*
		*  @return
		*    The primary render context, null pointer on error
		*/
		inline HGLRC getRenderContext() const;


	//[-------------------------------------------------------]
	//[ Public virtual OpenGLRenderer::IContext methods       ]
	//[-------------------------------------------------------]
	public:
		inline virtual bool isInitialized() const override;
		void makeCurrent(handle nativeWindowHandle) const override;


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		/**
		*  @brief
		*    Create a OpenGL context
		*
		*  @return
		*    The created OpenGL context, null pointer on error
		*/
		HGLRC createOpenGLContext();


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		handle mNativeWindowHandle;		///< OpenGL window, can be a null pointer (HWND)
		handle mDummyWindow;			///< OpenGL dummy window, can be a null pointer (HWND)
		HDC	   mWindowDeviceContext;	///< The device context of the OpenGL dummy window, can be a null pointer
		HGLRC  mWindowRenderContext;	///< The render context of the OpenGL dummy window, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // OpenGLRenderer


//[-------------------------------------------------------]
//[ Implementation                                        ]
//[-------------------------------------------------------]
#include "OpenGLRenderer/Windows/ContextWindows.inl"
