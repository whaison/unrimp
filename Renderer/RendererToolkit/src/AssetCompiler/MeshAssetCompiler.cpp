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
#include "RendererToolkit/AssetCompiler/MeshAssetCompiler.h"
#include "RendererToolkit/Helper/StringHelper.h"

#include <RendererRuntime/Core/Math/Math.h>
#include <RendererRuntime/Asset/AssetPackage.h>
#include <RendererRuntime/Resource/Mesh/MeshResource.h>
#include <RendererRuntime/Resource/Mesh/Loader/MeshFileFormat.h>

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4061)	// warning C4061: enumerator 'FORCE_32BIT' in switch of enum 'aiMetadataType' is not explicitly handled by a case label
	#include <assimp/scene.h>
	#include <assimp/Importer.hpp>
	#include <assimp/postprocess.h>
PRAGMA_WARNING_POP

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4668)	// warning C4668: '__GNUC__' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	PRAGMA_WARNING_DISABLE_MSVC(4365)	// warning C4365: '=': conversion from 'int' to 'rapidjson::internal::BigInteger::Type', signed/unsigned mismatch
	PRAGMA_WARNING_DISABLE_MSVC(4625)	// warning C4625: 'rapidjson::GenericMember<Encoding,Allocator>': copy constructor was implicitly defined as deleted
	#include <rapidjson/document.h>
PRAGMA_WARNING_POP

#include <memory>
#include <fstream>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		static const uint32_t NUMBER_OF_BYTES_PER_VERTEX = 28;	///< Number of bytes per vertex (3 float position, 2 float texture coordinate, 4 short QTangent)
		typedef std::vector<RendererRuntime::v1Mesh::SubMesh> SubMeshes;


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Get the total number of vertices and indices by using a given Assimp node
		*
		*  @param[in]  assimpScene
		*    Assimp scene
		*  @param[in]  assimpNode
		*    Assimp node to gather the data from
		*  @param[out] numberOfVertices
		*    Receives the number of vertices
		*  @param[out] numberOfIndices
		*    Receives the number of indices
		*  @param[out] subMeshes
		*    Receives the sub-meshes data
		*/
		void getNumberOfVerticesAndIndicesRecursive(const RendererToolkit::IAssetCompiler::Input& input, const aiScene& assimpScene, const aiNode& assimpNode, uint32_t& numberOfVertices, uint32_t& numberOfIndices, SubMeshes& subMeshes)
		{
			// Loop through all meshes this node is using
			for (uint32_t i = 0; i < assimpNode.mNumMeshes; ++i)
			{
				// Get the used mesh
				const aiMesh& assimpMesh = *assimpScene.mMeshes[assimpNode.mMeshes[i]];

				// Update the number of vertices
				numberOfVertices += assimpMesh.mNumVertices;

				// Loop through all mesh faces and update the number of indices
				const uint32_t previousNumberOfIndices = numberOfIndices;
				for (uint32_t j = 0; j < assimpMesh.mNumFaces; ++j)
				{
					numberOfIndices += assimpMesh.mFaces[j].mNumIndices;
				}

				{ // Add sub-mesh
					// Get the source material asset ID
					// TODO(co) Error handling: Do we need to validate the material index or material name?
					aiString materialName;
					assimpScene.mMaterials[assimpMesh.mMaterialIndex]->Get(AI_MATKEY_NAME, materialName);
					const uint32_t sourceMaterialAssetId = static_cast<uint32_t>(std::atoi(materialName.C_Str()));

					// Add sub-mesh
					RendererRuntime::v1Mesh::SubMesh subMesh;
					subMesh.materialAssetId		= input.getCompiledAssetIdBySourceAssetId(sourceMaterialAssetId);
					subMesh.primitiveTopology	= static_cast<uint8_t>(Renderer::PrimitiveTopology::TRIANGLE_LIST);
					subMesh.startIndexLocation	= previousNumberOfIndices;
					subMesh.numberOfIndices		= numberOfIndices - previousNumberOfIndices;
					subMeshes.push_back(subMesh);
				}
			}

			// Loop through all child nodes recursively
			for (uint32_t i = 0; i < assimpNode.mNumChildren; ++i)
			{
				getNumberOfVerticesAndIndicesRecursive(input, assimpScene, *assimpNode.mChildren[i], numberOfVertices, numberOfIndices, subMeshes);
			}
		}

		/**
		*  @brief
		*    Fill the mesh data recursively
		*
		*  @param[in]  assimpScene
		*    Assimp scene
		*  @param[in]  assimpNode
		*    Assimp node to gather the data from
		*  @param[in]  vertexBuffer
		*    Vertex buffer to fill
		*  @param[in]  indexBuffer
		*    Index buffer to fill
		*  @param[in]  assimpTransformation
		*    Current absolute Assimp transformation matrix (local to global space)
		*  @param[out] numberOfVertices
		*    Receives the number of processed vertices
		*  @param[out] numberOfIndices
		*    Receives the number of processed indices
		*/
		void fillMeshRecursive(const aiScene &assimpScene, const aiNode &assimpNode, uint8_t *vertexBuffer, uint16_t *indexBuffer, const aiMatrix4x4 &assimpTransformation, uint32_t &numberOfVertices, uint32_t &numberOfIndices)
		{
			// Get the absolute transformation matrix of this Assimp node
			const aiMatrix4x4 currentAssimpTransformation = assimpTransformation * assimpNode.mTransformation;
			const aiMatrix3x3 currentAssimpNormalTransformation = aiMatrix3x3(currentAssimpTransformation);

			// Loop through all meshes this node is using
			for (uint32_t i = 0; i < assimpNode.mNumMeshes; ++i)
			{
				// Get the used mesh
				const aiMesh &assimpMesh = *assimpScene.mMeshes[assimpNode.mMeshes[i]];

				// Get the start vertex inside the our vertex buffer
				const uint32_t starVertex = numberOfVertices;

				// Loop through the Assimp mesh vertices
				uint8_t *currentVertexBuffer = vertexBuffer + numberOfVertices * NUMBER_OF_BYTES_PER_VERTEX;
				for (uint32_t j = 0; j < assimpMesh.mNumVertices; ++j)
				{
					{ // 32 bit position
						// Get the Assimp mesh vertex position
						aiVector3D assimpVertex = assimpMesh.mVertices[j];

						// Transform the Assimp mesh vertex position into global space
						assimpVertex *= currentAssimpTransformation;

						// Set our vertex buffer position
						float *currentVertexBufferFloat = reinterpret_cast<float*>(currentVertexBuffer);
						*currentVertexBufferFloat = assimpVertex.x;
						++currentVertexBufferFloat;
						*currentVertexBufferFloat = assimpVertex.y;
						++currentVertexBufferFloat;
						*currentVertexBufferFloat = assimpVertex.z;
						currentVertexBuffer += sizeof(float) * 3;
					}

					{ // 32 bit texture coordinate
						// Get the Assimp mesh vertex texture coordinate
						aiVector3D assimpTexCoord = assimpMesh.mTextureCoords[0][j];

						// Set our vertex buffer 32 bit texture coordinate
						float *currentVertexBufferFloat = reinterpret_cast<float*>(currentVertexBuffer);
						*currentVertexBufferFloat = assimpTexCoord.x;
						++currentVertexBufferFloat;
						*currentVertexBufferFloat = assimpTexCoord.y;
						currentVertexBuffer += sizeof(float) * 2;
					}

					{ // 16 bit QTangent
					  // - QTangent basing on http://dev.theomader.com/qtangents/ "QTangents" which is basing on
					  //   http://www.crytek.com/cryengine/presentations/spherical-skinning-with-dual-quaternions-and-qtangents "Spherical Skinning with Dual-Quaternions and QTangents"
						// Get the Assimp mesh vertex tangent, binormal and normal
						aiVector3D tangent = assimpMesh.mTangents[j];
						aiVector3D binormal = assimpMesh.mBitangents[j];
						aiVector3D normal = assimpMesh.mNormals[j];

						// Transform the Assimp mesh vertex data into global space
						tangent *= currentAssimpNormalTransformation;
						binormal *= currentAssimpNormalTransformation;
						normal *= currentAssimpNormalTransformation;

						// Generate tangent frame rotation matrix
						glm::mat3 tangentFrame(
							tangent.x,  tangent.y,  tangent.z,
							binormal.x, binormal.y, binormal.z,
							normal.x,   normal.y,   normal.z
						);

						// Calculate tangent frame quaternion
						const glm::quat tangentFrameQuaternion = RendererRuntime::Math::calculateTangentFrameQuaternion(tangentFrame);

						// Set our vertex buffer 16 bit QTangent
						short *currentVertexBufferShort = reinterpret_cast<short*>(currentVertexBuffer);
						*currentVertexBufferShort = static_cast<short>(tangentFrameQuaternion.x * SHRT_MAX);
						++currentVertexBufferShort;
						*currentVertexBufferShort = static_cast<short>(tangentFrameQuaternion.y * SHRT_MAX);
						++currentVertexBufferShort;
						*currentVertexBufferShort = static_cast<short>(tangentFrameQuaternion.z * SHRT_MAX);
						++currentVertexBufferShort;
						*currentVertexBufferShort = static_cast<short>(tangentFrameQuaternion.w * SHRT_MAX);
						currentVertexBuffer += sizeof(short) * 4;
					}
				}
				numberOfVertices += assimpMesh.mNumVertices;

				// Loop through all Assimp mesh faces
				uint16_t *currentIndexBuffer = indexBuffer + numberOfIndices;
				for (uint32_t j = 0; j < assimpMesh.mNumFaces; ++j)
				{
					// Get the Assimp face
					const aiFace &assimpFace = assimpMesh.mFaces[j];

					// Loop through all indices of the Assimp face and set our indices
					for (uint32_t assimpIndex = 0; assimpIndex < assimpFace.mNumIndices; ++assimpIndex, ++currentIndexBuffer)
					{
						//					  Assimp mesh vertex index	 								 Where the Assimp mesh starts within the our vertex buffer
						*currentIndexBuffer = static_cast<uint16_t>(assimpFace.mIndices[assimpIndex] + starVertex);
					}

					// Update the number if processed indices
					numberOfIndices += assimpFace.mNumIndices;
				}
			}

			// Loop through all child nodes recursively
			for (uint32_t assimpChild = 0; assimpChild < assimpNode.mNumChildren; ++assimpChild)
			{
				fillMeshRecursive(assimpScene, *assimpNode.mChildren[assimpChild], vertexBuffer, indexBuffer, currentAssimpTransformation, numberOfVertices, numberOfIndices);
			}
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererToolkit
{


	//[-------------------------------------------------------]
	//[ Public definitions                                    ]
	//[-------------------------------------------------------]
	const AssetCompilerTypeId MeshAssetCompiler::TYPE_ID("Mesh");


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	MeshAssetCompiler::MeshAssetCompiler()
	{
		// Nothing here
	}

	MeshAssetCompiler::~MeshAssetCompiler()
	{
		// Nothing here
	}


	//[-------------------------------------------------------]
	//[ Public virtual RendererToolkit::IAssetCompiler methods ]
	//[-------------------------------------------------------]
	AssetCompilerTypeId MeshAssetCompiler::getAssetCompilerTypeId() const
	{
		return TYPE_ID;
	}

	void MeshAssetCompiler::compile(const Input& input, const Configuration& configuration, Output& output)
	{
		// Input, configuration and output
		const std::string&			   assetInputDirectory	= input.assetInputDirectory;
		const std::string&			   assetOutputDirectory	= input.assetOutputDirectory;
		RendererRuntime::AssetPackage& outputAssetPackage	= *output.outputAssetPackage;

		// Get the JSON asset object
		const rapidjson::Value& rapidJsonValueAsset = configuration.rapidJsonDocumentAsset["Asset"];

		// Read configuration
		// TODO(co) Add required properties
		std::string inputFile;
		{
			// Read mesh asset compiler configuration
			const rapidjson::Value& rapidJsonValueMeshAssetCompiler = rapidJsonValueAsset["MeshAssetCompiler"];
			inputFile = rapidJsonValueMeshAssetCompiler["InputFile"].GetString();
		}

		// Open the input and output file
		const std::string assetName = rapidJsonValueAsset["AssetMetadata"]["AssetName"].GetString();
		const std::string outputAssetFilename = assetOutputDirectory + assetName + ".mesh";
		std::ofstream outputFileStream(outputAssetFilename, std::ios::binary);

		// Create an instance of the Assimp importer class
		Assimp::Importer assimpImporter;

		// Load the given mesh
		// -> "aiProcess_MakeLeftHanded" is added because the rasterizer states directly map to Direct3D
		const aiScene *assimpScene = assimpImporter.ReadFile(assetInputDirectory + inputFile, aiProcessPreset_TargetRealtime_MaxQuality | aiProcess_MakeLeftHanded);
		if (nullptr != assimpScene && nullptr != assimpScene->mRootNode)
		{
			// Get the total number of vertices and indices by using the Assimp root node
			uint32_t numberOfVertices = 0;
			uint32_t numberOfIndices = 0;
			::detail::SubMeshes subMeshes;
			::detail::getNumberOfVerticesAndIndicesRecursive(input, *assimpScene, *assimpScene->mRootNode, numberOfVertices, numberOfIndices, subMeshes);

			{ // Mesh header
				RendererRuntime::v1Mesh::Header meshHeader;
				meshHeader.formatType				= RendererRuntime::v1Mesh::FORMAT_TYPE;
				meshHeader.formatVersion			= RendererRuntime::v1Mesh::FORMAT_VERSION;
				meshHeader.numberOfBytesPerVertex	= ::detail::NUMBER_OF_BYTES_PER_VERTEX;
				meshHeader.numberOfVertices			= numberOfVertices;
				meshHeader.indexBufferFormat		= Renderer::IndexBufferFormat::UNSIGNED_SHORT;
				meshHeader.numberOfIndices			= numberOfIndices;
				meshHeader.numberOfVertexAttributes = static_cast<uint8_t>(RendererRuntime::MeshResource::VERTEX_ATTRIBUTES.numberOfAttributes);
				meshHeader.numberOfSubMeshes		= static_cast<uint8_t>(subMeshes.size());

				// Write down the mesh header
				outputFileStream.write(reinterpret_cast<const char*>(&meshHeader), sizeof(RendererRuntime::v1Mesh::Header));
			}

			{ // Vertex and index buffer data
				// Allocate memory for the local vertex and index buffer data
				uint8_t *vertexBufferData = new uint8_t[::detail::NUMBER_OF_BYTES_PER_VERTEX * numberOfVertices];
				uint16_t *indexBufferData = new uint16_t[numberOfIndices];

				{ // Fill the mesh data recursively
					uint32_t numberOfFilledVertices = 0;
					uint32_t numberOfFilledIndices  = 0;
					::detail::fillMeshRecursive(*assimpScene, *assimpScene->mRootNode, vertexBufferData, indexBufferData, aiMatrix4x4(), numberOfFilledVertices, numberOfFilledIndices);

					// TODO(co) ?
					numberOfVertices = numberOfFilledVertices;
					numberOfIndices  = numberOfFilledIndices;
				}

				// Write down the vertex and index buffer
				outputFileStream.write(reinterpret_cast<const char*>(vertexBufferData), ::detail::NUMBER_OF_BYTES_PER_VERTEX * numberOfVertices);
				outputFileStream.write(reinterpret_cast<const char*>(indexBufferData), static_cast<std::streamsize>(sizeof(uint16_t) * numberOfIndices));

				// Destroy local vertex and input buffer data
				delete [] vertexBufferData;
				delete [] indexBufferData;
			}

			// Write down the vertex array attributes
			outputFileStream.write(reinterpret_cast<const char*>(RendererRuntime::MeshResource::VERTEX_ATTRIBUTES.attributes), static_cast<std::streamsize>(sizeof(Renderer::VertexAttribute) * RendererRuntime::MeshResource::VERTEX_ATTRIBUTES.numberOfAttributes));

			// Write down the sub-meshes
			outputFileStream.write(reinterpret_cast<const char*>(subMeshes.data()), static_cast<std::streamsize>(sizeof(RendererRuntime::v1Mesh::SubMesh) * subMeshes.size()));
		}
		else
		{
			throw std::runtime_error("ASSIMP failed to load in the given mesh");
		}

		{ // Update the output asset package
			const std::string assetCategory = rapidJsonValueAsset["AssetMetadata"]["AssetCategory"].GetString();
			const std::string assetIdAsString = input.projectName + "/Mesh/" + assetCategory + '/' + assetName;

			// Output asset
			RendererRuntime::Asset outputAsset;
			outputAsset.assetId = StringHelper::getAssetIdByString(assetIdAsString.c_str());
			strcpy(outputAsset.assetFilename, outputAssetFilename.c_str());	// TODO(co) Buffer overflow test
			outputAssetPackage.getWritableSortedAssetVector().push_back(outputAsset);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererToolkit
