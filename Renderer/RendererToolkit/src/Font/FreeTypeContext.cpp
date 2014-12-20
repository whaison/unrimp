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
#include "RendererToolkit/Font/FreeTypeContext.h"

#include <Renderer/IRenderer.h>

#include <ft2build.h>
#include FT_FREETYPE_H


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererToolkit
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	FreeTypeContext::FreeTypeContext(Renderer::IRenderer &renderer) :
		mRenderer(&renderer),
		mFTLibrary(new FT_Library)
	{
		// Add our renderer reference
		mRenderer->addReference();

		// Initialize the FreeType library object
		const FT_Error ftError = FT_Init_FreeType(mFTLibrary);
		if (0 != ftError)
		{
			// Error!
			delete mFTLibrary;
			mFTLibrary = nullptr;
		}
	}

	FreeTypeContext::~FreeTypeContext()
	{
		// Destroy the FreeType library object
		if (nullptr != mFTLibrary)
		{
			FT_Done_FreeType(*mFTLibrary);
			delete mFTLibrary;
		}

		// Release our renderer reference
		mRenderer->release();
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererToolkit