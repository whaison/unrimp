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
#include "RendererToolkit/AssetCompiler/SceneAssetCompiler.h"

#include <RendererRuntime/Asset/AssetPackage.h>
#include <RendererRuntime/Resource/Scene/Item/MeshSceneItem.h>
#include <RendererRuntime/Resource/Scene/Item/CameraSceneItem.h>
#include <RendererRuntime/Resource/Scene/Loader/SceneFileFormat.h>

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4251)	// warning C4251: 'Poco::StringTokenizer::_tokens': class 'std::vector<std::string,std::allocator<_Kty>>' needs to have dll-interface to be used by clients of class 'Poco::StringTokenizer'
	#include <Poco/StringTokenizer.h>
#pragma warning(pop)

#include <fstream>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererToolkit
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	SceneAssetCompiler::SceneAssetCompiler()
	{
	}

	SceneAssetCompiler::~SceneAssetCompiler()
	{
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererToolkit::IAssetCompiler methods ]
	//[-------------------------------------------------------]
	void SceneAssetCompiler::compile(const Input& input, const Configuration& configuration, Output& output)
	{
		// Input, configuration and output
		const std::string&			   assetInputDirectory	= input.assetInputDirectory;
		const std::string&			   assetOutputDirectory	= input.assetOutputDirectory;
		Poco::JSON::Object::Ptr		   jsonAssetRootObject	= configuration.jsonAssetRootObject;
		RendererRuntime::AssetPackage& outputAssetPackage	= *output.outputAssetPackage;

		// Get the JSON asset object
		Poco::JSON::Object::Ptr jsonAssetObject = jsonAssetRootObject->get("Asset").extract<Poco::JSON::Object::Ptr>();

		// Read configuration
		// TODO(co) Add required properties
		std::string inputFile;
		uint32_t test = 0;
		{
			// Read Scene asset compiler configuration
			Poco::JSON::Object::Ptr jsonConfigurationObject = jsonAssetObject->get("SceneAssetCompiler").extract<Poco::JSON::Object::Ptr>();
			inputFile = jsonConfigurationObject->getValue<std::string>("InputFile");
			test	  = jsonConfigurationObject->optValue<uint32_t>("Test", test);
		}

		// Open the input file
		std::ifstream ifstream(assetInputDirectory + inputFile, std::ios::binary);
		const std::string assetName = jsonAssetObject->get("AssetMetadata").extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("AssetName");
		const std::string assetFilename = assetOutputDirectory + assetName + ".scene";
		std::ofstream ofstream(assetFilename, std::ios::binary);

		{ // Scene
			// Parse JSON
			Poco::JSON::Parser jsonParser;
			jsonParser.parse(ifstream);
			Poco::JSON::Object::Ptr jsonRootObject = jsonParser.result().extract<Poco::JSON::Object::Ptr>();
		
			{ // Check whether or not the file format matches
				Poco::JSON::Object::Ptr jsonFormatObject = jsonRootObject->get("Format").extract<Poco::JSON::Object::Ptr>();
				if (jsonFormatObject->get("Type").convert<std::string>() != "SceneAsset")
				{
					throw std::exception("Invalid JSON format type, must be \"SceneAsset\"");
				}
				if (jsonFormatObject->get("Version").convert<uint32_t>() != 1)
				{
					throw std::exception("Invalid JSON format version, must be 1");
				}
			}

			{ // Write down the scene resource header
				RendererRuntime::v1Scene::Header sceneHeader;
				sceneHeader.formatType	  = RendererRuntime::v1Scene::FORMAT_TYPE;
				sceneHeader.formatVersion = RendererRuntime::v1Scene::FORMAT_VERSION;
				ofstream.write(reinterpret_cast<const char*>(&sceneHeader), sizeof(RendererRuntime::v1Scene::Header));
			}

			Poco::JSON::Object::Ptr jsonSceneObject = jsonRootObject->get("SceneAsset").extract<Poco::JSON::Object::Ptr>();
			Poco::JSON::Object::Ptr jsonSceneNodesObject = jsonSceneObject->get("Nodes").extract<Poco::JSON::Object::Ptr>();

			{ // Write down the scene nodes
				RendererRuntime::v1Scene::Nodes nodes;
				nodes.numberOfNodes = jsonSceneNodesObject->size();
				ofstream.write(reinterpret_cast<const char*>(&nodes), sizeof(RendererRuntime::v1Scene::Nodes));

				// Loop through all scene nodes
				Poco::JSON::Object::ConstIterator nodesIterator = jsonSceneNodesObject->begin();
				Poco::JSON::Object::ConstIterator nodesIteratorEnd = jsonSceneNodesObject->end();
				while (nodesIterator != nodesIteratorEnd)
				{
					Poco::JSON::Object::Ptr jsonSceneNodeObject = nodesIterator->second.extract<Poco::JSON::Object::Ptr>();
					Poco::JSON::Object::Ptr jsonItemsObject = jsonSceneNodeObject->has("Items") ? jsonSceneNodeObject->get("Items").extract<Poco::JSON::Object::Ptr>() : Poco::JSON::Object::Ptr();

					{ // Write down the scene node
						RendererRuntime::v1Scene::Node node;

						// Get the scene node transform
						node.transform.scale = glm::vec3(1.0f, 1.0f, 1.0f);
						if (jsonSceneNodeObject->has("Properties"))
						{
							Poco::JSON::Object::Ptr jsonPropertiesObject = jsonSceneNodeObject->get("Properties").extract<Poco::JSON::Object::Ptr>();

							{ // Parse the position string
								Poco::Dynamic::Var position = jsonPropertiesObject->get("Position");
								if (!position.isEmpty())
								{
									Poco::StringTokenizer stringTokenizer(position.convert<std::string>(), " ");
									if (stringTokenizer.count() == 3)
									{
										for (size_t i = 0; i < 3; ++i)
										{
											node.transform.position[static_cast<int>(i)] = static_cast<float>(std::atof(stringTokenizer[i].c_str()));
										}
									}
									else
									{
										// TODO(co) Error handling
									}
								}
							}

							{ // Parse the rotation string
								Poco::Dynamic::Var rotation = jsonPropertiesObject->get("Rotation");
								if (!rotation.isEmpty())
								{
									Poco::StringTokenizer stringTokenizer(rotation.convert<std::string>(), " ");
									if (stringTokenizer.count() == 4)
									{
										for (size_t i = 0; i < 4; ++i)
										{
											node.transform.rotation[static_cast<int>(i)] = static_cast<float>(std::atof(stringTokenizer[i].c_str()));
										}
									}
									else
									{
										// TODO(co) Error handling
									}
								}
							}

							{ // Parse the scale string
								Poco::Dynamic::Var scale = jsonPropertiesObject->get("Scale");
								if (!scale.isEmpty())
								{
									Poco::StringTokenizer stringTokenizer(scale.convert<std::string>(), " ");
									if (stringTokenizer.count() == 3)
									{
										for (size_t i = 0; i < 3; ++i)
										{
											node.transform.scale[static_cast<int>(i)] = static_cast<float>(std::atof(stringTokenizer[i].c_str()));
										}
									}
									else
									{
										// TODO(co) Error handling
									}
								}
							}
						}

						// Write down the scene node
						node.numberOfItems = jsonItemsObject->size();
						ofstream.write(reinterpret_cast<const char*>(&node), sizeof(RendererRuntime::v1Scene::Node));
					}

					{ // Write down the scene items
						Poco::JSON::Object::ConstIterator itemsIterator = jsonItemsObject->begin();
						Poco::JSON::Object::ConstIterator itemsIteratorEnd = jsonItemsObject->end();
						while (itemsIterator != itemsIteratorEnd)
						{
							Poco::JSON::Object::Ptr jsonItemObject = itemsIterator->second.extract<Poco::JSON::Object::Ptr>();
							const RendererRuntime::SceneItemTypeId typeId = RendererRuntime::StringId(itemsIterator->first.c_str());

							// Get the scene item type specific data number of bytes
							// TODO(co) Make this more generic via scene factory
							uint32_t numberOfBytes = 0;
							if (RendererRuntime::CameraSceneItem::TYPE_ID == typeId)
							{
								numberOfBytes = sizeof(RendererRuntime::v1Scene::CameraItem);
							}
							else if (RendererRuntime::MeshSceneItem::TYPE_ID == typeId)
							{
								numberOfBytes = sizeof(RendererRuntime::v1Scene::MeshItem);
							}

							{ // Write down the scene item header
								RendererRuntime::v1Scene::ItemHeader itemHeader;
								itemHeader.typeId		 = typeId;
								itemHeader.numberOfBytes = numberOfBytes;
								ofstream.write(reinterpret_cast<const char*>(&itemHeader), sizeof(RendererRuntime::v1Scene::ItemHeader));
							}

							// Write down the scene item type specific data, if there is any
							if (0 != numberOfBytes)
							{
								if (RendererRuntime::CameraSceneItem::TYPE_ID == typeId)
								{
									// Nothing here
								}
								else if (RendererRuntime::MeshSceneItem::TYPE_ID == typeId)
								{
									RendererRuntime::v1Scene::MeshItem meshItem;
									meshItem.meshAssetId = static_cast<uint32_t>(std::atoi(jsonItemObject->get("MeshAssetId").convert<std::string>().c_str()));
									ofstream.write(reinterpret_cast<const char*>(&meshItem), sizeof(RendererRuntime::v1Scene::MeshItem));
								}
							}

							// Next scene item, please
							++itemsIterator;
						}
					}

					// Next scene node, please
					++nodesIterator;
				}
			}
		}

		{ // Update the output asset package
			const std::string assetCategory = jsonAssetObject->get("AssetMetadata").extract<Poco::JSON::Object::Ptr>()->getValue<std::string>("AssetCategory");
			const std::string assetIdAsString = input.projectName + "/Scene/" + assetCategory + '/' + assetName;

			// Output asset
			RendererRuntime::Asset outputAsset;
			outputAsset.assetId = RendererRuntime::StringId(assetIdAsString.c_str());
			strcpy(outputAsset.assetFilename, assetFilename.c_str());	// TODO(co) Buffer overflow test
			outputAssetPackage.getWritableSortedAssetVector().push_back(outputAsset);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererToolkit
