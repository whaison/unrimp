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


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "RendererRuntime/PrecompiledHeader.h"
#include "RendererRuntime/Resource/Skeleton/SkeletonResourceManager.h"

#include <cassert>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceManager methods ]
	//[-------------------------------------------------------]
	IResource& SkeletonResourceManager::getResourceByResourceId(ResourceId resourceId) const
	{
		IResource* resource = tryGetResourceByResourceId(resourceId);
		assert(nullptr != resource);
		return *resource;
	}

	IResource* SkeletonResourceManager::tryGetResourceByResourceId(ResourceId) const
	{
		// TODO(co) Implement me
		return nullptr;
	}

	void SkeletonResourceManager::reloadResourceByAssetId(AssetId assetId)
	{
		// TODO(co) Implement me
		assetId = assetId;
	}

	void SkeletonResourceManager::update()
	{
		// TODO(co) Implement me
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	SkeletonResourceManager::SkeletonResourceManager(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime)
	{
		// Nothing here
	}

	SkeletonResourceManager::~SkeletonResourceManager()
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
