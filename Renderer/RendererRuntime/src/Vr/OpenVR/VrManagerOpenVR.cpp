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
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "RendererRuntime/Vr/OpenVR/VrManagerOpenVR.h"
#include "RendererRuntime/Vr/OpenVR/OpenVRRuntimeLinking.h"
#include "RendererRuntime/Resource/Scene/ISceneResource.h"
#include "RendererRuntime/Resource/Scene/Node/ISceneNode.h"
#include "RendererRuntime/Resource/Scene/Item/MeshSceneItem.h"
#include "RendererRuntime/Resource/Mesh/MeshResourceManager.h"
#include "RendererRuntime/Resource/Compositor/CompositorInstance.h"
#include "RendererRuntime/Resource/Texture/TextureResourceManager.h"
#include "RendererRuntime/Resource/Material/MaterialResourceManager.h"
#include "RendererRuntime/Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h"
#include "RendererRuntime/Resource/MaterialBlueprint/Listener/IMaterialBlueprintResourceListener.h"
#include "RendererRuntime/Core/Math/Transform.h"
#include "RendererRuntime/IRendererRuntime.h"

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4464)	// warning C4464: relative include path contains '..'
	#include <glm/gtx/matrix_decompose.hpp>
#pragma warning(pop)

#include <string>
#include <chrono>
#include <thread>
#include <cassert>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		// TODO(co) We need a central vertex input layout management
		// Vertex input layout
		const Renderer::VertexAttribute VertexAttributesLayout[] =
		{
			{ // Attribute 0
				// Data destination
				Renderer::VertexAttributeFormat::FLOAT_3,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
				"Position",									// name[32] (char)
				"POSITION",									// semanticName[32] (char)
				0,											// semanticIndex (uint32_t)
				// Data source
				0,											// inputSlot (uint32_t)
				0,											// alignedByteOffset (uint32_t)
				// Data source, instancing part
				0											// instancesPerElement (uint32_t)
			},
			{ // Attribute 1
				// Data destination
				Renderer::VertexAttributeFormat::SHORT_2,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
				"TexCoord",									// name[32] (char)
				"TEXCOORD",									// semanticName[32] (char)
				0,											// semanticIndex (uint32_t)
				// Data source
				0,											// inputSlot (uint32_t)
				sizeof(float) * 3,							// alignedByteOffset (uint32_t)
				// Data source, instancing part
				0											// instancesPerElement (uint32_t)
			},
			{ // Attribute 2
				// Data destination
				Renderer::VertexAttributeFormat::SHORT_4,	// vertexAttributeFormat (Renderer::VertexAttributeFormat)
				"QTangent",									// name[32] (char)
				"TEXCOORD",									// semanticName[32] (char)
				1,											// semanticIndex (uint32_t)
				// Data source
				0,											// inputSlot (uint32_t)
				sizeof(float) * 3 + sizeof(short) * 2,		// alignedByteOffset (uint32_t)
				// Data source, instancing part
				0											// instancesPerElement (uint32_t)
			}
		};
		const Renderer::VertexAttributes VertexAttributes(glm::countof(VertexAttributesLayout), VertexAttributesLayout);


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		std::string getTrackedDeviceString(vr::IVRSystem& vrSystem, vr::TrackedDeviceIndex_t trackedDeviceIndex, vr::TrackedDeviceProperty trackedDeviceProperty, vr::TrackedPropertyError* trackedPropertyError = nullptr)
		{
			uint32_t requiredBufferLength = vrSystem.GetStringTrackedDeviceProperty(trackedDeviceIndex, trackedDeviceProperty, nullptr, 0, trackedPropertyError);
			if (0 == requiredBufferLength)
			{
				return "";
			}

			char* temp = new char[requiredBufferLength];
			requiredBufferLength = vrSystem.GetStringTrackedDeviceProperty(trackedDeviceIndex, trackedDeviceProperty, temp, requiredBufferLength, trackedPropertyError);
			std::string result = temp;
			delete [] temp;
			return result;
		}

		glm::mat4 convertOpenVrMatrixToGlmMat4(const vr::HmdMatrix44_t& vrHmdMatrix34)
		{
			return glm::mat4(
				vrHmdMatrix34.m[0][0], vrHmdMatrix34.m[1][0], vrHmdMatrix34.m[2][0], vrHmdMatrix34.m[3][0],
				vrHmdMatrix34.m[0][1], vrHmdMatrix34.m[1][1], vrHmdMatrix34.m[2][1], vrHmdMatrix34.m[3][1],
				vrHmdMatrix34.m[0][2], vrHmdMatrix34.m[1][2], vrHmdMatrix34.m[2][2], vrHmdMatrix34.m[3][2],
				vrHmdMatrix34.m[0][3], vrHmdMatrix34.m[1][3], vrHmdMatrix34.m[2][3], vrHmdMatrix34.m[3][3]
				);
		}

		glm::mat4 convertOpenVrMatrixToGlmMat4(const vr::HmdMatrix34_t& vrHmdMatrix34)
		{
			return glm::mat4(
				vrHmdMatrix34.m[0][0], vrHmdMatrix34.m[1][0], vrHmdMatrix34.m[2][0], 0.0f,
				vrHmdMatrix34.m[0][1], vrHmdMatrix34.m[1][1], vrHmdMatrix34.m[2][1], 0.0f,
				vrHmdMatrix34.m[0][2], vrHmdMatrix34.m[1][2], vrHmdMatrix34.m[2][2], 0.0f,
				vrHmdMatrix34.m[0][3], vrHmdMatrix34.m[1][3], vrHmdMatrix34.m[2][3], 1.0f
				);
		}

		// TODO(co) Awful quick'n'dirty implementation. Implement asynchronous render texture loading.
		RendererRuntime::AssetId setupRenderModelDiffuseTexture(RendererRuntime::IRendererRuntime& rendererRuntime, const std::string& renderModelName, const vr::RenderModel_t& vrRenderModel)
		{
			// Get the texture name and convert it into an runtime texture asset ID
			const std::string diffuseTextureName = "OpenVR_" + std::to_string(vrRenderModel.diffuseTextureId);
			RendererRuntime::AssetId diffuseTextureAssetId = RendererRuntime::StringId(diffuseTextureName.c_str());

			// Check whether or not we need to generate the runtime texture asset right now
			RendererRuntime::TextureResourceManager& textureResourceManager = rendererRuntime.getTextureResourceManager();
			RendererRuntime::TextureResourceId textureResourceId = textureResourceManager.loadTextureResourceByAssetId(diffuseTextureAssetId);
			if (RendererRuntime::isUninitialized(textureResourceId))
			{
				// Load the render model texture
				vr::IVRRenderModels* vrRenderModels = vr::VRRenderModels();
				vr::RenderModel_TextureMap_t* vrRenderModelTextureMap = nullptr;
				vr::EVRRenderModelError vrRenderModelError = vr::VRRenderModelError_Loading;
				while (vrRenderModelError == vr::VRRenderModelError_Loading)
				{
					vrRenderModelError = vrRenderModels->LoadTexture_Async(vrRenderModel.diffuseTextureId, &vrRenderModelTextureMap);
					if (vrRenderModelError == vr::VRRenderModelError_Loading)
					{
						using namespace std::chrono_literals;
						std::this_thread::sleep_for(1ms);
					}
				}
				if (vr::VRRenderModelError_None != vrRenderModelError)
				{
					RENDERERRUNTIME_OUTPUT_DEBUG_PRINTF("Error: Unable to load OpenVR diffuse texture %d of render model \"%s\": %s", vrRenderModel.diffuseTextureId, renderModelName.c_str(), vrRenderModels->GetRenderModelErrorNameFromEnum(vrRenderModelError));
					renderModelName.empty();	// TODO(co) Hack to get rid of "warning C4100: 'renderModelName': unreferenced formal parameter" in release static build
					return RendererRuntime::getUninitialized<RendererRuntime::AssetId>();
				}

				// Create the renderer texture instance
				Renderer::ITexture2D* texture2D = rendererRuntime.getRenderer().createTexture2D(vrRenderModelTextureMap->unWidth, vrRenderModelTextureMap->unHeight, Renderer::TextureFormat::R8G8B8A8, static_cast<const void*>(vrRenderModelTextureMap->rubTextureMapData), Renderer::TextureFlag::GENERATE_MIPMAPS);

				// We need to generate the runtime texture asset right now
				// -> Takes over the given 2D texture
				textureResourceId = textureResourceManager.createTextureResourceByAssetId(diffuseTextureAssetId, *texture2D);

				// Free the render model texture
				vrRenderModels->FreeTexture(vrRenderModelTextureMap);
			}

			// Done
			return diffuseTextureAssetId;
		}

		RendererRuntime::MaterialResourceId setupRenderModelMaterial(RendererRuntime::IRendererRuntime& rendererRuntime, vr::TextureID_t vrTextureId, const RendererRuntime::AssetId diffuseTextureAssetId)
		{
			// Get the texture name and convert it into an runtime material asset ID
			const std::string materialName = "OpenVR_" + std::to_string(vrTextureId);
			RendererRuntime::AssetId materialAssetId = RendererRuntime::StringId(materialName.c_str());

			// Check whether or not we need to generate the runtime material asset right now
			RendererRuntime::MaterialResourceManager& materialResourceManager = rendererRuntime.getMaterialResourceManager();
			RendererRuntime::MaterialResourceId materialResourceId = materialResourceManager.loadMaterialResourceByAssetId(materialAssetId);
			if (RendererRuntime::isUninitialized(materialResourceId))
			{
				// We need to generate the runtime material asset right now
				materialResourceId = materialResourceManager.createMaterialResourceByAssetId(materialAssetId, RendererRuntime::StringId("Example/MaterialBlueprint/Default/FirstMesh"));	// TODO(co) It must be possible to set the material blueprint ID from the outside
				if (RendererRuntime::isInitialized(materialResourceId))
				{
					// TODO(co) Get rid of the evil const-cast
					RendererRuntime::MaterialResource* materialResource = const_cast<RendererRuntime::MaterialResource*>(materialResourceManager.getMaterialResources().tryGetElementById(materialResourceId));
					if (nullptr != materialResource)
					{
						// TODO(co) It must be possible to set the property name from the outside
						RendererRuntime::MaterialProperties& materialProperties = materialResource->getMaterialProperties();
						materialProperties.setPropertyById("DiffuseMap", RendererRuntime::MaterialPropertyValue::fromTextureAssetId(diffuseTextureAssetId));
						materialProperties.setPropertyById("UseDiffuseMap", RendererRuntime::MaterialPropertyValue::fromBoolean(true));
					}
				}
			}

			// Done
			return materialResourceId;
		}

		// TODO(co) Awful quick'n'dirty implementation. Implement asynchronous render model loading.
		void setupRenderModel(RendererRuntime::IRendererRuntime& rendererRuntime, const std::string& renderModelName, RendererRuntime::MeshResource& meshResource)
		{
			vr::IVRRenderModels* vrRenderModels = vr::VRRenderModels();

			// Load the render model
			vr::RenderModel_t* vrRenderModel = nullptr;
			vr::EVRRenderModelError vrRenderModelError = vr::VRRenderModelError_Loading;
			while (vrRenderModelError == vr::VRRenderModelError_Loading)
			{
				vrRenderModelError = vrRenderModels->LoadRenderModel_Async(renderModelName.c_str(), &vrRenderModel);
				if (vrRenderModelError == vr::VRRenderModelError_Loading)
				{
					using namespace std::chrono_literals;
					std::this_thread::sleep_for(1ms);
				}
			}
			if (vr::VRRenderModelError_None != vrRenderModelError)
			{
				RENDERERRUNTIME_OUTPUT_DEBUG_PRINTF("Error: Unable to load OpenVR render model \"%s\": %s", renderModelName.c_str(), vrRenderModels->GetRenderModelErrorNameFromEnum(vrRenderModelError));
				return;
			}

			// Load the render model texture
			// -> We don't care if loading of the diffuse texture fails in here, isn't that important and the show must go on
			const RendererRuntime::AssetId diffuseTextureAssetId = setupRenderModelDiffuseTexture(rendererRuntime, renderModelName, *vrRenderModel);

			// Setup the material asset
			const RendererRuntime::MaterialResourceId materialResourceId = setupRenderModelMaterial(rendererRuntime, vrRenderModel->diffuseTextureId, diffuseTextureAssetId);

			{ // Fill the mesh resource
				// Tell the mesh resource about the number of vertices and indices
				const uint32_t numberOfVertices = vrRenderModel->unVertexCount;
				const uint32_t numberOfIndices = vrRenderModel->unTriangleCount * 3;
				meshResource.setNumberOfVertices(numberOfVertices);
				meshResource.setNumberOfIndices(numberOfIndices);

				{ // TODO(co) Tell the mesh resource about the vertex array
					Renderer::IRenderer& renderer = rendererRuntime.getRenderer();

					// Create the vertex buffer
					Renderer::IVertexBuffer* vertexBuffer = nullptr;
					const uint32_t numberOfBytesPerVertex = sizeof(float) * 3 + sizeof(short) * 6;
					{
						const uint32_t numberOfBytes = numberOfVertices * numberOfBytesPerVertex;
						uint8_t* temp = new uint8_t[numberOfBytes];
						uint8_t* currentTemp = temp;
						const vr::RenderModel_Vertex_t* currentVrRenderModelVertex = vrRenderModel->rVertexData;
						for (uint32_t i = 0; i < numberOfVertices; ++i, ++currentVrRenderModelVertex)
						{
							{ // 32 bit position
								float* position = reinterpret_cast<float*>(currentTemp);
								position[0] = currentVrRenderModelVertex->vPosition.v[0];
								position[1] = currentVrRenderModelVertex->vPosition.v[1];
								position[2] = currentVrRenderModelVertex->vPosition.v[2];
								currentTemp += sizeof(float) * 3;
							}

							{ // 16 bit texture coordinate
								short* textureCoordinate = reinterpret_cast<short*>(currentTemp);
								textureCoordinate[0] = static_cast<short>(currentVrRenderModelVertex->rfTextureCoord[0] * SHRT_MAX);
								textureCoordinate[1] = static_cast<short>(currentVrRenderModelVertex->rfTextureCoord[1] * SHRT_MAX);
								currentTemp += sizeof(short) * 2;
							}

							{ // TODO(co) 16 bit QTangent
								short* qTangent = reinterpret_cast<short*>(currentTemp);
								qTangent[0] = static_cast<short>(0.2f * SHRT_MAX);
								qTangent[1] = static_cast<short>(0.2f * SHRT_MAX);
								qTangent[2] = static_cast<short>(0.2f * SHRT_MAX);
								qTangent[3] = static_cast<short>(0.2f * SHRT_MAX);
								currentTemp += sizeof(short) * 4;
							}
						}
						vertexBuffer = renderer.createVertexBuffer(numberOfBytes, temp, Renderer::BufferUsage::STATIC_DRAW);
						delete [] temp;
					}

					// Create the index buffer
					// -> We need to flip the vertex winding so we don't need to modify rasterizer states
					Renderer::IIndexBuffer* indexBuffer = nullptr;
					{
						uint16_t* temp = new uint16_t[numberOfIndices];
						uint16_t* currentTemp = temp;
						for (uint32_t i = 0; i < vrRenderModel->unTriangleCount; ++i, currentTemp += 3)
						{
							const uint32_t offset = i * 3;
							currentTemp[0] = vrRenderModel->rIndexData[offset + 2];
							currentTemp[1] = vrRenderModel->rIndexData[offset + 1];
							currentTemp[2] = vrRenderModel->rIndexData[offset + 0];
						}
						indexBuffer = renderer.createIndexBuffer(numberOfIndices * sizeof(uint16_t), Renderer::IndexBufferFormat::UNSIGNED_SHORT, temp, Renderer::BufferUsage::STATIC_DRAW);
						delete [] temp;
					}

					// Create vertex array object (VAO)
					const Renderer::VertexArrayVertexBuffer vertexArrayVertexBuffers[] =
					{
						{ // Vertex buffer 0
							vertexBuffer,			// vertexBuffer (Renderer::IVertexBuffer *)
							numberOfBytesPerVertex	// strideInBytes (uint32_t)
						}
					};
					meshResource.setVertexArray(renderer.createVertexArray(::detail::VertexAttributes, glm::countof(vertexArrayVertexBuffers), vertexArrayVertexBuffers, indexBuffer));
				}

				// Tell the mesh resource about the sub-mesh
				meshResource.getSubMeshes().push_back(RendererRuntime::SubMesh(materialResourceId, Renderer::PrimitiveTopology::TRIANGLE_LIST, 0, meshResource.getNumberOfIndices()));
			}

			// Free the render model
			vrRenderModels->FreeRenderModel(vrRenderModel);
		}


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public virtual RendererRuntime::IVrManager methods    ]
	//[-------------------------------------------------------]
	bool VrManagerOpenVR::isHmdPresent() const
	{
		return (mOpenVRRuntimeLinking->isOpenVRAvaiable() && vr::VR_IsRuntimeInstalled() && vr::VR_IsHmdPresent());
	}

	void VrManagerOpenVR::setSceneResource(ISceneResource* sceneResource)
	{
		// TODO(co) Decent implementation so it's no problem to change the scene resource at any time
		mSceneResource = sceneResource;
	}

	bool VrManagerOpenVR::startup()
	{
		assert(nullptr == mVrSystem);
		if (nullptr == mVrSystem)
		{
			// OpenVR "IVRSystem_012" (Jun 10, 2016) only works with OpenGL and DirectX 11
			Renderer::IRenderer& renderer = mRendererRuntime.getRenderer();
			const bool isOpenGLRenderer = (0 == strcmp(renderer.getName(), "OpenGL"));
			const bool isDirect3D11Renderer = (0 == strcmp(renderer.getName(), "Direct3D11"));
			if (!isOpenGLRenderer && !isDirect3D11Renderer)
			{
				// Error!
				RENDERERRUNTIME_OUTPUT_DEBUG_STRING("Error: OpenVR \"IVRSystem_012\" (Jun 10, 2016) only works with OpenGL and DirectX 11");
				return false;
			}
			mVrGraphicsAPIConvention = isOpenGLRenderer ? vr::API_OpenGL : vr::API_DirectX;

			// Initialize the OpenVR system
			vr::EVRInitError vrInitError = vr::VRInitError_None;
			mVrSystem = vr::VR_Init(&vrInitError, vr::VRApplication_Scene);
			if (vr::VRInitError_None != vrInitError)
			{
				// Error!
				RENDERERRUNTIME_OUTPUT_DEBUG_PRINTF("Error: Unable to initialize OpenVR runtime: %s", vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError));
				return false;
			}

			// Get the OpenVR render models interface
			mVrRenderModels = static_cast<vr::IVRRenderModels*>(vr::VR_GetGenericInterface(vr::IVRRenderModels_Version, &vrInitError));
			if (nullptr == mVrRenderModels)
			{
				// De-initialize the OpenVR system
				vr::VR_Shutdown();
				mVrSystem = nullptr;

				// Error!
				RENDERERRUNTIME_OUTPUT_DEBUG_PRINTF("Error: Unable to retrieve the OpenVR render models interface: %s", vr::VR_GetVRInitErrorAsEnglishDescription(vrInitError));
				return false;
			}

			// Setup all render models for tracked devices
			for (uint32_t trackedDeviceIndex = vr::k_unTrackedDeviceIndex_Hmd + 1; trackedDeviceIndex < vr::k_unMaxTrackedDeviceCount; ++trackedDeviceIndex)
			{
				if (mVrSystem->IsTrackedDeviceConnected(trackedDeviceIndex))
				{
					setupRenderModelForTrackedDevice(trackedDeviceIndex);
				}
			}

			{ // Create renderer resources
				// Create the texture instance
				uint32_t width = 0;
				uint32_t height = 0;
				mVrSystem->GetRecommendedRenderTargetSize(&width, &height);
				Renderer::ITexture* colorTexture2D = mColorTexture2D = renderer.createTexture2D(width, height, Renderer::TextureFormat::R8G8B8A8, nullptr, Renderer::TextureFlag::RENDER_TARGET);
				Renderer::ITexture* depthStencilTexture2D = renderer.createTexture2D(width, height, Renderer::TextureFormat::D32_FLOAT, nullptr, Renderer::TextureFlag::RENDER_TARGET);

				// Create the framebuffer object (FBO) instance
				mFramebuffer = renderer.createFramebuffer(1, &colorTexture2D, depthStencilTexture2D);
			}
		}

		// Done
		return true;
	}

	void VrManagerOpenVR::shutdown()
	{
		if (nullptr != mVrSystem)
		{
			{ // Release renderer resources
				mFramebuffer = nullptr;
				mColorTexture2D = nullptr;
			}

			// De-initialize the OpenVR system
			vr::VR_Shutdown();
			mVrSystem = nullptr;
		}
	}

	void VrManagerOpenVR::updateHmdMatrixPose()
	{
		assert(nullptr != mVrSystem);

		{ // Process OpenVR events
			vr::VREvent_t vrVREvent;
			while (mVrSystem->PollNextEvent(&vrVREvent, sizeof(vr::VREvent_t)))
			{
				switch (vrVREvent.eventType)
				{
					case vr::VREvent_TrackedDeviceActivated:
						setupRenderModelForTrackedDevice(vrVREvent.trackedDeviceIndex);
						break;
				}
			}
		}

		// Request poses from OpenVR
		vr::VRCompositor()->WaitGetPoses(mVrTrackedDevicePose, vr::k_unMaxTrackedDeviceCount, nullptr, 0);

		// Gather all valid poses
		mNumberOfValidDevicePoses = 0;
		for (int32_t deviceIndex = 0; deviceIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIndex)
		{
			if (mVrTrackedDevicePose[deviceIndex].bPoseIsValid)
			{
				++mNumberOfValidDevicePoses;
				const glm::mat4& devicePoseMatrix = mDevicePoseMatrix[deviceIndex] = ::detail::convertOpenVrMatrixToGlmMat4(mVrTrackedDevicePose[deviceIndex].mDeviceToAbsoluteTracking);
				ISceneNode* sceneNode = mSceneNodes[deviceIndex];
				if (nullptr != sceneNode)
				{
					glm::vec3 scale;
					glm::quat rotation;
					glm::vec3 translation;
					glm::vec3 skew;
					glm::vec4 perspective;
					glm::decompose(devicePoseMatrix, scale, rotation, translation, skew, perspective);

					// TODO(co) Why is the rotation inverted?
					rotation = glm::inverse(rotation);

					// Tell the scene node about the new position and rotation, scale doesn't change
					sceneNode->setPositionRotation(translation, rotation);
				}
			}
		}

		// Backup HMD pose
		if (mVrTrackedDevicePose[vr::k_unTrackedDeviceIndex_Hmd].bPoseIsValid)
		{
			mHmdPoseMatrix = glm::inverse(mDevicePoseMatrix[vr::k_unTrackedDeviceIndex_Hmd]);
		}
	}

	glm::mat4 VrManagerOpenVR::getHmdViewSpaceToClipSpaceMatrix(VrEye vrEye, float nearZ, float farZ) const
	{
		assert(nullptr != mVrSystem);
		return ::detail::convertOpenVrMatrixToGlmMat4(mVrSystem->GetProjectionMatrix(static_cast<vr::Hmd_Eye>(vrEye), nearZ, farZ, vr::API_DirectX));
	}

	glm::mat4 VrManagerOpenVR::getHmdEyeSpaceToHeadSpaceMatrix(VrEye vrEye) const
	{
		assert(nullptr != mVrSystem);
		return glm::inverse(::detail::convertOpenVrMatrixToGlmMat4(mVrSystem->GetEyeToHeadTransform(static_cast<vr::Hmd_Eye>(vrEye))));
	}

	void VrManagerOpenVR::executeCompositorInstance(CompositorInstance& compositorInstance, Renderer::IRenderTarget&, CameraSceneItem* cameraSceneItem)
	{
		assert(nullptr != mVrSystem);

		IMaterialBlueprintResourceListener& materialBlueprintResourceListener = mRendererRuntime.getMaterialBlueprintResourceManager().getMaterialBlueprintResourceListener();
		for (int8_t eyeIndex = 0; eyeIndex < 2; ++eyeIndex)
		{
			// Execute the compositor instance
			materialBlueprintResourceListener.setCurrentRenderedVrEye(static_cast<IMaterialBlueprintResourceListener::VrEye>(eyeIndex));
			compositorInstance.execute(*mFramebuffer, cameraSceneItem);

			// Submit the rendered texture to the OpenVR compositor
			const vr::Texture_t vrTexture = { mColorTexture2D->getInternalResourceHandle(), mVrGraphicsAPIConvention, vr::ColorSpace_Auto };
			vr::VRCompositor()->Submit(static_cast<vr::Hmd_Eye>(eyeIndex), &vrTexture);
		}
		materialBlueprintResourceListener.setCurrentRenderedVrEye(IMaterialBlueprintResourceListener::VrEye::UNKNOWN);

		// TODO(co) Optionally mirror the result on the given render target
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	VrManagerOpenVR::VrManagerOpenVR(IRendererRuntime& rendererRuntime) :
		mRendererRuntime(rendererRuntime),
		mSceneResource(nullptr),
		mOpenVRRuntimeLinking(new OpenVRRuntimeLinking()),
		mVrGraphicsAPIConvention(vr::API_OpenGL),
		mVrSystem(nullptr),
		mVrRenderModels(nullptr),
		mNumberOfValidDevicePoses(0)
	{
		memset(mSceneNodes, 0, sizeof(ISceneNode*) * vr::k_unMaxTrackedDeviceCount);
	}

	VrManagerOpenVR::~VrManagerOpenVR()
	{
		shutdown();
		delete mOpenVRRuntimeLinking;
	}

	void VrManagerOpenVR::setupRenderModelForTrackedDevice(vr::TrackedDeviceIndex_t trackedDeviceIndex)
	{
		assert(trackedDeviceIndex < vr::k_unMaxTrackedDeviceCount);

		// Get the render model name and convert it into an runtime mesh asset ID
		const std::string renderModelName = ::detail::getTrackedDeviceString(*mVrSystem, trackedDeviceIndex, vr::Prop_RenderModelName_String);
		const AssetId assetId(renderModelName.c_str());

		// Check whether or not we need to generate the runtime mesh asset right now
		MeshResourceManager& meshResourceManager = mRendererRuntime.getMeshResourceManager();
		MeshResourceId meshResourceId = meshResourceManager.loadMeshResourceByAssetId(assetId);
		if (isUninitialized(meshResourceId))
		{
			// We need to generate the runtime mesh asset right now
			meshResourceId = meshResourceManager.createEmptyMeshResourceByAssetId(assetId);
			if (isInitialized(meshResourceId))
			{
				const MeshResource* meshResource = meshResourceManager.getMeshResources().tryGetElementById(meshResourceId);
				if (nullptr != meshResource)
				{
					// TODO(co) Get rid of the evil const-cast
					::detail::setupRenderModel(mRendererRuntime, renderModelName, *const_cast<MeshResource*>(meshResource));
				}
			}
		}

		// Create and setup scene node with mesh item, this is what's controlled during runtime
		if (nullptr != mSceneResource)
		{
			ISceneNode* sceneNode = mSceneNodes[trackedDeviceIndex] = mSceneResource->createSceneNode(Transform());
			if (nullptr != sceneNode)
			{
				MeshSceneItem* meshSceneItem = static_cast<MeshSceneItem*>(mSceneResource->createSceneItem(MeshSceneItem::TYPE_ID, *sceneNode));
				if (nullptr != meshSceneItem)
				{
					meshSceneItem->setMeshResourceId(meshResourceId);
				}
			}
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
