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
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Direct3D12Renderer/TessellationControlShaderHlsl.h"
#include "Direct3D12Renderer/Guid.h"	// For "WKPDID_D3DDebugObjectName"
#include "Direct3D12Renderer/D3D12.h"
#include "Direct3D12Renderer/Direct3D12Renderer.h"
#include "Direct3D12Renderer/ShaderLanguageHlsl.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace Direct3D12Renderer
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	TessellationControlShaderHlsl::TessellationControlShaderHlsl(Direct3D12Renderer &direct3D12Renderer, const uint8_t *, uint32_t) :
		TessellationControlShader(direct3D12Renderer)
	//	mD3D12HullShader(nullptr)	// TODO(co) Direct3D 12 update
	{
		// Create the Direct3D 12 hull shader
		// TODO(co) Direct3D 12 update
		// direct3D12Renderer.getD3D12Device()->CreateHullShader(bytecode, numberOfBytes, nullptr, &mD3D12HullShader);

		// Don't assign a default name to the resource for debugging purposes, Direct3D 12 automatically sets a decent default name
	}

	TessellationControlShaderHlsl::TessellationControlShaderHlsl(Direct3D12Renderer &direct3D12Renderer, const char *sourceCode) :
		TessellationControlShader(direct3D12Renderer)
	//	mD3D12HullShader(nullptr)	// TODO(co) Direct3D 12 update
	{
		// Create the Direct3D 12 binary large object for the hull shader
		ID3DBlob *d3dBlob = ShaderLanguageHlsl::loadShader("hs_5_0", sourceCode, nullptr);
		if (nullptr != d3dBlob)
		{
			// TODO(co) Direct3D 12 update
			// Create the Direct3D 12 hull shader
			// direct3D12Renderer.getD3D12Device()->CreateHullShader(d3dBlob->GetBufferPointer(), d3dBlob->GetBufferSize(), nullptr, &mD3D12HullShader);

			// Release the Direct3D 12 shader binary large object
			d3dBlob->Release();
		}

		// Don't assign a default name to the resource for debugging purposes, Direct3D 12 automatically sets a decent default name
	}

	TessellationControlShaderHlsl::~TessellationControlShaderHlsl()
	{
		// Release the Direct3D 12 hull shader
		// TODO(co) Direct3D 12 update
		/*
		if (nullptr != mD3D12HullShader)
		{
			mD3D12HullShader->Release();
		}
		*/
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IResource methods            ]
	//[-------------------------------------------------------]
	void TessellationControlShaderHlsl::setDebugName(const char *)
	{
		// TODO(co) Direct3D 12 update
		/*
		#ifndef DIRECT3D12RENDERER_NO_DEBUG
			// Valid Direct3D 12 hull shader?
			if (nullptr != mD3D12HullShader)
			{
				// Set the debug name
				// -> First: Ensure that there's no previous private data, else we might get slapped with a warning!
				mD3D12HullShader->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
				mD3D12HullShader->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
			}
		#endif
		*/
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IShader methods              ]
	//[-------------------------------------------------------]
	const char *TessellationControlShaderHlsl::getShaderLanguageName() const
	{
		return ShaderLanguageHlsl::NAME;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // Direct3D12Renderer
