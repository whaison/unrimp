/*********************************************************\
 * Copyright (c) 2012-2016 Christian Ofenberg
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
#include "RendererRuntime/Core/Manager.h"

#include <Renderer/Public/Renderer.h>

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4201)	// warning C4201: nonstandard extension used: nameless struct/union
	#pragma warning(disable: 4464)	// warning C4464: relative include path contains '..'
	#pragma warning(disable: 4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/glm.hpp>
#pragma warning(pop)

#include <vector>


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class Transform;
	class IBufferManager;
	class IRendererRuntime;
	class MaterialBlueprintResource;
	class MaterialBlueprintResourceManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    Pass buffer manager
	*/
	class PassBufferManager : private Manager
	{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	public:
		struct PassData
		{
			glm::mat4 worldSpaceToClipSpaceMatrix;
			glm::mat4 worldSpaceToViewSpaceMatrix;
			glm::mat4 viewSpaceToWorldSpaceMatrix;
		};


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor
		*
		*  @param[in] rendererRuntime
		*    Renderer runtime instance to use
		*  @param[in] materialBlueprintResource
		*    Material blueprint resource
		*/
		PassBufferManager(IRendererRuntime& rendererRuntime, const MaterialBlueprintResource& materialBlueprintResource);

		/**
		*  @brief
		*    Destructor
		*/
		~PassBufferManager();

		/**
		*  @brief
		*    Fill the pass buffer
		*
		*  @param[in] worldSpaceToViewSpaceTransform
		*    World space to view space transform matrix
		*/
		void fillBuffer(const Transform& worldSpaceToViewSpaceTransform);

		/**
		*  @brief
		*    Return the pass data
		*
		*  @return
		*    The pass data filled inside "RendererRuntime::PassBufferManager::fillBuffer()"
		*/
		inline const PassData& getPassData() const;

		/**
		*  @brief
		*    Bind the currently used pass buffer into the given commando buffer
		*
		*  @param[out] commandBuffer
		*    Command buffer to fill
		*/
		void fillCommandBuffer(Renderer::CommandBuffer& commandBuffer) const;

		/**
		*  @brief
		*    Reset current pass buffer
		*
		*  @note
		*    - Should be called once per frame
		*/
		inline void resetCurrentPassBuffer();


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		PassBufferManager(const PassBufferManager&) = delete;
		PassBufferManager& operator=(const PassBufferManager&) = delete;


	//[-------------------------------------------------------]
	//[ Private definitions                                   ]
	//[-------------------------------------------------------]
	private:
		typedef std::vector<Renderer::IUniformBuffer*> UniformBuffers;
		typedef std::vector<uint8_t>				   ScratchBuffer;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		IRendererRuntime&						mRendererRuntime;
		Renderer::IBufferManager&				mBufferManager;
		const MaterialBlueprintResource&		mMaterialBlueprintResource;
		const MaterialBlueprintResourceManager&	mMaterialBlueprintResourceManager;
		PassData								mPassData;
		UniformBuffers							mUniformBuffers;
		uint32_t								mCurrentUniformBufferIndex;
		ScratchBuffer							mScratchBuffer;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime


//[-------------------------------------------------------]
//[ Implementation                                        ]
//[-------------------------------------------------------]
#include "RendererRuntime/Resource/MaterialBlueprint/BufferManager/PassBufferManager.inl"