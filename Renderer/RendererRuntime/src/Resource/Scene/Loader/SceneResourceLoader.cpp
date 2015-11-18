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
#include "RendererRuntime/Resource/Scene/Loader/SceneResourceLoader.h"
#include "RendererRuntime/Resource/Scene/Loader/SceneFileFormat.h"
#include "RendererRuntime/Resource/Scene/Item/ISceneItem.h"
#include "RendererRuntime/Resource/Scene/ISceneResource.h"

#include <fstream>


// TODO(co) Possible performance improvement: Inside "SceneResourceLoader::onDeserialization()" load everything directly into memory,
// create the instances inside "SceneResourceLoader::onProcessing()" while other stuff can continue reading from disk.
// TODO(co) Error handling


//[-------------------------------------------------------]
//[ Global functions in anonymous namespace               ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{
		void itemDeserialization(std::istream& istream, RendererRuntime::ISceneResource& sceneResource, RendererRuntime::ISceneNode& sceneNode)
		{
			// Read the scene item header
			RendererRuntime::v1Scene::ItemHeader itemHeader;
			istream.read(reinterpret_cast<char*>(&itemHeader), sizeof(RendererRuntime::v1Scene::ItemHeader));

			// Create the scene item
			RendererRuntime::ISceneItem* sceneItem = sceneResource.createSceneItem(itemHeader.typeId, sceneNode);
			if (nullptr != sceneItem && 0 != itemHeader.numberOfBytes)
			{
				// Load in the scene item data
				// TODO(co) Get rid of the new/delete in here
				uint8_t* data = new uint8_t[itemHeader.numberOfBytes];
				istream.read(reinterpret_cast<char*>(data), itemHeader.numberOfBytes);

				// Deserialize the scene item
				sceneItem->deserialize(itemHeader.numberOfBytes, data);

				// Cleanup
				delete [] data;
			}
			else
			{
				// TODO(co) Error handling
			}
		}

		void nodeDeserialization(std::istream& istream, RendererRuntime::ISceneResource& sceneResource)
		{
			// Read in the scene node
			RendererRuntime::v1Scene::Node node;
			istream.read(reinterpret_cast<char*>(&node), sizeof(RendererRuntime::v1Scene::Node));

			// Create the scene node
			RendererRuntime::ISceneNode* sceneNode = sceneResource.createSceneNode(node.transform);
			if (nullptr != sceneNode)
			{
				// Read in the scene items
				for (uint32_t i = 0; i < node.numberOfItems; ++i)
				{
					itemDeserialization(istream, sceneResource, *sceneNode);
				}
			}
			else
			{
				// TODO(co) Error handling
			}
		}

		void nodesDeserialization(std::istream& istream, RendererRuntime::ISceneResource& sceneResource)
		{
			// Read in the scene nodes
			RendererRuntime::v1Scene::Nodes nodes;
			istream.read(reinterpret_cast<char*>(&nodes), sizeof(RendererRuntime::v1Scene::Nodes));

			// Read in the scene nodes
			for (uint32_t i = 0; i < nodes.numberOfNodes; ++i)
			{
				nodeDeserialization(istream, sceneResource);
			}
		}
	}
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	const ResourceLoaderTypeId SceneResourceLoader::TYPE_ID("scene");


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IResourceLoader methods ]
	//[-------------------------------------------------------]
	ResourceLoaderTypeId SceneResourceLoader::getResourceLoaderTypeId() const
	{
		return TYPE_ID;
	}

	void SceneResourceLoader::onDeserialization()
	{
		// TODO(co) Error handling
		try
		{
			std::ifstream ifstream(mAsset.assetFilename, std::ios::binary);

			// Read in the scene header
			v1Scene::Header sceneHeader;
			ifstream.read(reinterpret_cast<char*>(&sceneHeader), sizeof(v1Scene::Header));

			// Read in the scene resource nodes
			::detail::nodesDeserialization(ifstream, *mSceneResource);
		}
		catch (const std::exception& e)
		{
			RENDERERRUNTIME_OUTPUT_ERROR_PRINTF("Renderer runtime failed to load scene asset %d: %s", mAsset.assetId, e.what());
		}
	}

	void SceneResourceLoader::onProcessing()
	{
		// Nothing here
	}

	void SceneResourceLoader::onRendererBackendDispatch()
	{
		// Nothing here
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	SceneResourceLoader::~SceneResourceLoader()
	{
		// Nothing here
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
