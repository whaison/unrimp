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
#include "Direct3D12Renderer/RootSignature.h"
#include "Direct3D12Renderer/Guid.h"	// For "WKPDID_D3DDebugObjectName"
#include "Direct3D12Renderer/Direct3D12Renderer.h"
#include "Direct3D12Renderer/Direct3D12RuntimeLinking.h"

#include <Renderer/RootSignatureTypes.h>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace Direct3D12Renderer
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	RootSignature::RootSignature(Direct3D12Renderer &direct3D12Renderer, const Renderer::RootSignature& rootSignature) :
		IRootSignature(direct3D12Renderer),
		mD3D12RootSignature(nullptr)
	{
		// "Renderer::RootSignature" is identical to "D3D12_ROOT_SIGNATURE_DESC" so there's no additional mapping work to be done in here
		ID3DBlob* d3dBlobSignature = nullptr;
		ID3DBlob* d3dBlobError = nullptr;
		if (SUCCEEDED(D3D12SerializeRootSignature(reinterpret_cast<const D3D12_ROOT_SIGNATURE_DESC*>(&rootSignature), D3D_ROOT_SIGNATURE_VERSION_1, &d3dBlobSignature, &d3dBlobError)))
		{
			if (FAILED(direct3D12Renderer.getD3D12Device()->CreateRootSignature(0, d3dBlobSignature->GetBufferPointer(), d3dBlobSignature->GetBufferSize(), IID_PPV_ARGS(&mD3D12RootSignature))))
			{
				RENDERER_OUTPUT_DEBUG_STRING("Direct3D 12 error: Failed to create the root signature instance")
			}
			d3dBlobSignature->Release();
		}
		else
		{
			RENDERER_OUTPUT_DEBUG_STRING("Direct3D 12 error: Failed to create the root signature instance")
			d3dBlobError->Release();
		}

		// Assign a default name to the resource for debugging purposes
		#ifndef DIRECT3D12RENDERER_NO_DEBUG
			setDebugName("Root signature");
		#endif
	}

	RootSignature::~RootSignature()
	{
		// Release the Direct3D 12 root signature
		if (nullptr != mD3D12RootSignature)
		{
			mD3D12RootSignature->Release();
		}
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IResource methods            ]
	//[-------------------------------------------------------]
	void RootSignature::setDebugName(const char *name)
	{
		#ifndef DIRECT3D12RENDERER_NO_DEBUG
			// Valid Direct3D 12 root signature?
			if (nullptr != mD3D12RootSignature)
			{
				// Set the debug name
				// -> First: Ensure that there's no previous private data, else we might get slapped with a warning!
				mD3D12RootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, 0, nullptr);
				mD3D12RootSignature->SetPrivateData(WKPDID_D3DDebugObjectName, strlen(name), name);
			}
		#endif
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // Direct3D12Renderer