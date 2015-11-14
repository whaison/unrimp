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
#include "Framework/Stopwatch.h"
#include "Framework/IApplicationRendererRuntime.h"

#include <RendererRuntime/Manager/Resource/Font/FontResource.h>


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class MeshResource;
	class TextureResource;
}
namespace RendererToolkit
{
	class IProject;
}


//[-------------------------------------------------------]
//[ Classes                                               ]
//[-------------------------------------------------------]
/**
*  @brief
*    First scene example
*
*  @remarks
*    Demonstrates:
*    - Compositor
*    - Scene manager
*/
class FirstScene : public IApplicationRendererRuntime
{


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
public:
	/**
	*  @brief
	*    Constructor
	*
	*  @param[in] rendererName
	*    Case sensitive ASCII name of the renderer to instance, if null pointer or unknown renderer no renderer will be used.
	*    Example renderer names: "Null", "OpenGL", "OpenGLES2", "Direct3D9", "Direct3D10", "Direct3D11", "Direct3D12"
	*/
	explicit FirstScene(const char *rendererName);

	/**
	*  @brief
	*    Destructor
	*/
	virtual ~FirstScene();


//[-------------------------------------------------------]
//[ Public virtual IApplication methods                   ]
//[-------------------------------------------------------]
public:
	virtual void onInitialization() override;
	virtual void onDeinitialization() override;
	virtual void onUpdate() override;
	virtual void onDraw() override;


//[-------------------------------------------------------]
//[ Private data                                          ]
//[-------------------------------------------------------]
private:
	Renderer::IUniformBufferPtr			  mUniformBuffer;			///< Uniform buffer object (UBO), can be a null pointer
	Renderer::IRootSignaturePtr			  mRootSignature;			///< Root signature, can be a null pointer
	Renderer::IPipelineStatePtr			  mPipelineState;			///< Pipeline state object (PSO), can be a null pointer
	Renderer::IProgramPtr				  mProgram;					///< Program, can be a null pointer
	RendererRuntime::FontResource*		  mFontResource;			///< Font resource, can be a null pointer
	RendererRuntime::MeshResource*		  mMeshResource;			///< Mesh resource, can be a null pointer
	RendererRuntime::TextureResource*	  mDiffuseTextureResource;
	RendererRuntime::TextureResource*	  mNormalTextureResource;
	RendererRuntime::TextureResource*	  mSpecularTextureResource;
	RendererRuntime::TextureResource*	  mEmissiveTextureResource;
	Renderer::ISamplerStatePtr			  mSamplerState;			///< Sampler state, can be a null pointer
	// Optimization: Cache data to not bother the renderer API to much
	handle	 mObjectSpaceToClipSpaceMatrixUniformHandle;	///< Object space to clip space matrix uniform handle, can be NULL_HANDLE
	handle	 mObjectSpaceToViewSpaceMatrixUniformHandle;	///< Object space to view space matrix uniform handle, can be NULL_HANDLE
	// For timing
	Stopwatch mStopwatch;	///< Stopwatch instance
	float	  mGlobalTimer;	///< Global timer

	// TODO(co) First asset hot-reloading test
	RendererToolkit::IProject* mProject;


};