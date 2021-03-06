/*********************************************************\
 * Copyright (c) 2012-2017 The Unrimp Team
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


// TODO(co) Rethink the header in here

// Public comfort header putting everything within a single header


//[-------------------------------------------------------]
//[ Header guard                                          ]
//[-------------------------------------------------------]
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "RendererRuntime/Core/StringId.h"

#include <Renderer/Public/Renderer.h>

#include <vector>


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
namespace RendererRuntime
{
	class IVrManager;
	class IFileManager;
	class AssetManager;
	class ThreadManager;
	class DebugGuiManager;
	class IRendererRuntime;
	class ResourceStreamer;
	class IResourceManager;
	class MeshResourceManager;
	class SceneResourceManager;
	class PipelineStateCompiler;
	class TextureResourceManager;
	class SkeletonResourceManager;
	class MaterialResourceManager;
	class ShaderPieceResourceManager;
	class CompositorNodeResourceManager;
	class ShaderBlueprintResourceManager;
	class MaterialBlueprintResourceManager;
	class CompositorWorkspaceResourceManager;
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Global definitions                                    ]
	//[-------------------------------------------------------]
	typedef StringId AssetId;	///< Asset identifier, internally just a POD "uint32_t", string ID scheme is "<project name>/<asset type>/<asset category>/<asset name>"


	//[-------------------------------------------------------]
	//[ Types                                                 ]
	//[-------------------------------------------------------]
	// RendererRuntime/IRendererRuntime.h
	class IRendererRuntime : public Renderer::RefCount<IRendererRuntime>
	{
	public:
		typedef std::vector<IResourceManager*> ResourceManagers;
	public:
		virtual ~IRendererRuntime();
		inline Renderer::IRenderer& getRenderer() const
		{
			return *mRenderer;
		}
		inline Renderer::IBufferManager& getBufferManager() const
		{
			return *mBufferManager;
		}
		inline Renderer::ITextureManager& getTextureManager() const
		{
			return *mTextureManager;
		}
		inline IFileManager& getFileManager() const
		{
			return *mFileManager;
		}
		inline ThreadManager& getThreadManager() const
		{
			return *mThreadManager;
		}
		inline AssetManager& getAssetManager() const
		{
			return *mAssetManager;
		}
		inline ResourceStreamer& getResourceStreamer() const
		{
			return *mResourceStreamer;
		}
		inline TextureResourceManager& getTextureResourceManager() const
		{
			return *mTextureResourceManager;
		}
		inline ShaderPieceResourceManager& getShaderPieceResourceManager() const
		{
			return *mShaderPieceResourceManager;
		}
		inline ShaderBlueprintResourceManager& getShaderBlueprintResourceManager() const
		{
			return *mShaderBlueprintResourceManager;
		}
		inline MaterialBlueprintResourceManager& getMaterialBlueprintResourceManager() const
		{
			return *mMaterialBlueprintResourceManager;
		}
		inline MaterialResourceManager& getMaterialResourceManager() const
		{
			return *mMaterialResourceManager;
		}
		inline SkeletonResourceManager& getSkeletonResourceManager() const
		{
			return *mSkeletonResourceManager;
		}
		inline MeshResourceManager& getMeshResourceManager() const
		{
			return *mMeshResourceManager;
		}
		inline SceneResourceManager& getSceneResourceManager() const
		{
			return *mSceneResourceManager;
		}
		inline CompositorNodeResourceManager& getCompositorNodeResourceManager() const
		{
			return *mCompositorNodeResourceManager;
		}
		inline CompositorWorkspaceResourceManager& getCompositorWorkspaceResourceManager() const
		{
			return *mCompositorWorkspaceResourceManager;
		}
		inline const ResourceManagers& getResourceManagers() const
		{
			return mResourceManagers;
		}
		inline PipelineStateCompiler& getPipelineStateCompiler() const
		{
			return *mPipelineStateCompiler;
		}
		inline DebugGuiManager& getDebugGuiManager() const
		{
			return *mDebugGuiManager;
		}
		inline IVrManager& getVrManager() const
		{
			return *mVrManager;
		}
	public:
		virtual void reloadResourceByAssetId(AssetId assetId) const = 0;
		virtual void update() const = 0;
	protected:
		IRendererRuntime();
		explicit IRendererRuntime(const IRendererRuntime &source);
		IRendererRuntime &operator =(const IRendererRuntime &source);
	private:
		Renderer::IRenderer*				mRenderer;
		Renderer::IBufferManager*			mBufferManager;
		Renderer::ITextureManager*			mTextureManager;
		IFileManager*						mFileManager;
		ThreadManager*						mThreadManager;
		AssetManager*						mAssetManager;
		ResourceStreamer*					mResourceStreamer;
		TextureResourceManager*				mTextureResourceManager;
		ShaderPieceResourceManager*			mShaderPieceResourceManager;
		ShaderBlueprintResourceManager*		mShaderBlueprintResourceManager;
		MaterialBlueprintResourceManager*	mMaterialBlueprintResourceManager;
		MaterialResourceManager*			mMaterialResourceManager;
		SkeletonResourceManager*			mSkeletonResourceManager;
		MeshResourceManager*				mMeshResourceManager;
		SceneResourceManager*				mSceneResourceManager;
		CompositorNodeResourceManager*		mCompositorNodeResourceManager;
		CompositorWorkspaceResourceManager*	mCompositorWorkspaceResourceManager;
		ResourceManagers					mResourceManagers;
		PipelineStateCompiler*				mPipelineStateCompiler;
		DebugGuiManager*					mDebugGuiManager;
		IVrManager*							mVrManager;
	};
	typedef Renderer::SmartRefCount<IRendererRuntime> IRendererRuntimePtr;


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
