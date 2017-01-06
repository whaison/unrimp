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
#include "PrecompiledHeader.h"
#include "Runtime/FirstScene/VrController.h"

#include <RendererRuntime/IRendererRuntime.h>
#include <RendererRuntime/Core/Math/Math.h>
#include <RendererRuntime/Core/Math/Transform.h>
#include <RendererRuntime/Core/Math/EulerAngles.h>
#include <RendererRuntime/Resource/Scene/ISceneResource.h>
#include <RendererRuntime/Resource/Scene/Node/ISceneNode.h>
#include <RendererRuntime/Resource/Scene/Item/MeshSceneItem.h>
#include <RendererRuntime/Resource/Scene/Item/LightSceneItem.h>
#include <RendererRuntime/Resource/Scene/Item/CameraSceneItem.h>
#include <RendererRuntime/Resource/MaterialBlueprint/MaterialBlueprintResourceManager.h>
#include <RendererRuntime/Resource/MaterialBlueprint/Listener/MaterialBlueprintResourceListener.h>
#include <RendererRuntime/Vr/OpenVR/VrManagerOpenVR.h>
#include <RendererRuntime/Vr/OpenVR/IVrManagerOpenVRListener.h>

#include <imgui/imgui.h>

// Disable warnings in external headers, we can't fix them
PRAGMA_WARNING_PUSH
	PRAGMA_WARNING_DISABLE_MSVC(4464)	// warning C4464: relative include path contains '..'
	PRAGMA_WARNING_DISABLE_MSVC(4324)	// warning C4324: '<x>': structure was padded due to alignment specifier
	#include <glm/gtx/intersect.hpp>
	#include <glm/gtx/matrix_decompose.hpp>
PRAGMA_WARNING_POP


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
		#define DEFINE_CONSTANT(name) static const RendererRuntime::StringId name(#name);
			// Pass
			DEFINE_CONSTANT(IMGUI_OBJECT_SPACE_TO_CLIP_SPACE_MATRIX)
		#undef DEFINE_CONSTANT


		//[-------------------------------------------------------]
		//[ Classes                                               ]
		//[-------------------------------------------------------]
		/**
		*  @brief
		*    Virtual reality manager OpenVR listener
		*
		*  @todo
		*    - TODO(co) Support the dynamic adding and removal of VR controllers (index updates)
		*/
		class VrManagerOpenVRListener : public RendererRuntime::IVrManagerOpenVRListener
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			VrManagerOpenVRListener() :
				mVrManagerOpenVR(nullptr),
				mVrController(nullptr),
				mNumberOfVrControllers(0)
			{
				for (uint32_t i = 0; i < vr::k_unMaxTrackedDeviceCount; ++i)
				{
					mVrControllerTrackedDeviceIndices[i] = RendererRuntime::getUninitialized<vr::TrackedDeviceIndex_t>();
				}
			}

			virtual ~VrManagerOpenVRListener()
			{
				// Nothing here
			}

			void setVrManagerOpenVR(const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR, VrController& vrController)
			{
				mVrManagerOpenVR = &vrManagerOpenVR;
				mVrController = &vrController;
			}

			uint32_t getNumberOfVrControllers() const
			{
				return mNumberOfVrControllers;
			}

			vr::TrackedDeviceIndex_t getVrControllerTrackedDeviceIndices(uint32_t vrControllerIndex) const
			{
				assert(vrControllerIndex < vr::k_unMaxTrackedDeviceCount);
				return mVrControllerTrackedDeviceIndices[vrControllerIndex];
			}


		//[-------------------------------------------------------]
		//[ Private virtual RendererRuntime::IVrManagerOpenVRListener methods ]
		//[-------------------------------------------------------]
		private:
			virtual void onVrEvent(const vr::VREvent_t& vrVrEvent) override
			{
				switch (vrVrEvent.eventType)
				{
					// Handle quiting the application from Steam
					case vr::VREvent_DriverRequestedQuit:
					case vr::VREvent_Quit:
						// TODO(co)
						NOP;
						break;

					case vr::VREvent_ButtonPress:
					{
						// The first VR controller is used for teleporting
						// -> A green light indicates the position one will end up
						// -> When pressing the trigger button one teleports to this position
						if (mNumberOfVrControllers > 0 && mVrControllerTrackedDeviceIndices[0] == vrVrEvent.trackedDeviceIndex && vrVrEvent.data.controller.button == vr::k_EButton_SteamVR_Trigger && mVrController->getTeleportIndicationLightSceneItemSafe().isVisible())
						{
							// TODO(co) Why inversed position?
							mVrController->getCameraSceneItem().getParentSceneNodeSafe().setPosition(-mVrController->getTeleportIndicationLightSceneItemSafe().getParentSceneNodeSafe().getTransform().position);
						}
						break;
					}
				}
			}

			virtual void onMeshSceneItemCreated(vr::TrackedDeviceIndex_t trackedDeviceIndex, RendererRuntime::MeshSceneItem& meshSceneItem) override
			{
				if (mVrManagerOpenVR->getVrSystem()->GetTrackedDeviceClass(trackedDeviceIndex) == vr::TrackedDeviceClass_Controller)
				{
					// Remember the VR controller tracked device index
					mVrControllerTrackedDeviceIndices[mNumberOfVrControllers] = trackedDeviceIndex;
					++mNumberOfVrControllers;

					// Attach a point light to controllers, this way they can be seen easier and it's possible to illuminate the scene by using the hands
					meshSceneItem.getSceneResource().createSceneItem<RendererRuntime::LightSceneItem>(meshSceneItem.getParentSceneNodeSafe());
				}
			}


		//[-------------------------------------------------------]
		//[ Private methods                                       ]
		//[-------------------------------------------------------]
		private:
			VrManagerOpenVRListener(const VrManagerOpenVRListener&) = delete;
			VrManagerOpenVRListener& operator=(const VrManagerOpenVRListener&) = delete;


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			const RendererRuntime::VrManagerOpenVR*	mVrManagerOpenVR;
			VrController*							mVrController;
			uint32_t								mNumberOfVrControllers;
			vr::TrackedDeviceIndex_t				mVrControllerTrackedDeviceIndices[vr::k_unMaxTrackedDeviceCount];


		};

		class MaterialBlueprintResourceListener : public RendererRuntime::MaterialBlueprintResourceListener
		{


		//[-------------------------------------------------------]
		//[ Public methods                                        ]
		//[-------------------------------------------------------]
		public:
			MaterialBlueprintResourceListener() :
				mVrManagerOpenVR(nullptr),
				mVrManagerOpenVRListener(nullptr),
				mVrController(nullptr)
			{
				// Nothing here
			}

			virtual ~MaterialBlueprintResourceListener()
			{
				// Nothing here
			}

			void setVrManagerOpenVR(const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR, const VrManagerOpenVRListener& vrManagerOpenVRListener, VrController& vrController)
			{
				mVrManagerOpenVR = &vrManagerOpenVR;
				mVrManagerOpenVRListener = &vrManagerOpenVRListener;
				mVrController = &vrController;
			}


		//[-------------------------------------------------------]
		//[ Private virtual RendererRuntime::IMaterialBlueprintResourceListener methods ]
		//[-------------------------------------------------------]
		private:
			virtual bool fillPassValue(uint32_t referenceValue, uint8_t* buffer, uint32_t numberOfBytes) override
			{
				// The GUI is placed over the second VR controller
				if (::detail::IMGUI_OBJECT_SPACE_TO_CLIP_SPACE_MATRIX == referenceValue && mVrManagerOpenVRListener->getNumberOfVrControllers() >= 2)
				{
					assert(sizeof(float) * 4 * 4 == numberOfBytes);
					const ImGuiIO& imGuiIo = ImGui::GetIO();
					const glm::quat rotationOffset = RendererRuntime::EulerAngles::eulerToQuaternion(glm::vec3(glm::degrees(0.0f), glm::degrees(180.0f), 0.0f));
					const glm::mat4 guiScaleMatrix = glm::scale(RendererRuntime::Math::IDENTITY_MATRIX, glm::vec3(1.0f / imGuiIo.DisplaySize.x, 1.0f / imGuiIo.DisplaySize.y, 1.0f));
					const glm::mat4& devicePoseMatrix = mVrManagerOpenVR->getDevicePoseMatrix(mVrManagerOpenVRListener->getVrControllerTrackedDeviceIndices(1));
					const glm::mat4& cameraPositionMatrix = glm::translate(RendererRuntime::Math::IDENTITY_MATRIX, -mVrController->getCameraSceneItem().getParentSceneNodeSafe().getTransform().position);
					const glm::mat4 objectSpaceToClipSpaceMatrix = getPassData().worldSpaceToClipSpaceMatrix * cameraPositionMatrix * devicePoseMatrix * glm::mat4_cast(rotationOffset) * guiScaleMatrix;
					memcpy(buffer, glm::value_ptr(objectSpaceToClipSpaceMatrix), numberOfBytes);

					// Value filled
					return true;
				}
				else
				{
					// Call the base implementation
					return RendererRuntime::MaterialBlueprintResourceListener::fillPassValue(referenceValue, buffer, numberOfBytes);
				}
			}


		//[-------------------------------------------------------]
		//[ Private methods                                       ]
		//[-------------------------------------------------------]
		private:
			MaterialBlueprintResourceListener(const MaterialBlueprintResourceListener&) = delete;
			MaterialBlueprintResourceListener& operator=(const MaterialBlueprintResourceListener&) = delete;


		//[-------------------------------------------------------]
		//[ Private data                                          ]
		//[-------------------------------------------------------]
		private:
			const RendererRuntime::VrManagerOpenVR*	mVrManagerOpenVR;
			const VrManagerOpenVRListener*			mVrManagerOpenVRListener;
			VrController*							mVrController;


		};


		//[-------------------------------------------------------]
		//[ Global variables                                      ]
		//[-------------------------------------------------------]
		static VrManagerOpenVRListener defaultVrManagerOpenVRListener;
		static MaterialBlueprintResourceListener materialBlueprintResourceListener;


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Public methods                                        ]
//[-------------------------------------------------------]
VrController::VrController(RendererRuntime::CameraSceneItem& cameraSceneItem) :
	IController(cameraSceneItem),
	mRendererRuntime(cameraSceneItem.getSceneResource().getRendererRuntime()),
	mTeleportIndicationLightSceneItem(nullptr)
{
	// Register our listeners
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID)
	{
		RendererRuntime::VrManagerOpenVR& vrManagerOpenVR = static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager());
		::detail::defaultVrManagerOpenVRListener.setVrManagerOpenVR(vrManagerOpenVR, *this);
		vrManagerOpenVR.setVrManagerOpenVRListener(&::detail::defaultVrManagerOpenVRListener);
		::detail::materialBlueprintResourceListener.setVrManagerOpenVR(vrManagerOpenVR, ::detail::defaultVrManagerOpenVRListener, *this);
		mRendererRuntime.getMaterialBlueprintResourceManager().setMaterialBlueprintResourceListener(&::detail::materialBlueprintResourceListener);
	}

	{ // Create the teleport indication light scene item
		RendererRuntime::ISceneResource& sceneResource = cameraSceneItem.getSceneResource();
		RendererRuntime::ISceneNode* sceneNode = sceneResource.createSceneNode(RendererRuntime::Transform::IDENTITY);
		assert(nullptr != sceneNode);
		mTeleportIndicationLightSceneItem = sceneResource.createSceneItem<RendererRuntime::LightSceneItem>(*sceneNode);
		assert(nullptr != mTeleportIndicationLightSceneItem);
		mTeleportIndicationLightSceneItem->setColor(glm::vec3(0.0f, 1.0f, 0.0f));
		mTeleportIndicationLightSceneItem->setVisible(false);
	}
}

VrController::~VrController()
{
	// TODO(co) Destroy the teleport indication light scene item? (not really worth the effort here)

	// Unregister our listeners
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID)
	{
		static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager()).setVrManagerOpenVRListener(nullptr);
		mRendererRuntime.getMaterialBlueprintResourceManager().setMaterialBlueprintResourceListener(nullptr);
	}
}

const RendererRuntime::LightSceneItem& VrController::getTeleportIndicationLightSceneItemSafe() const
{
	assert(nullptr != mTeleportIndicationLightSceneItem);
	return *mTeleportIndicationLightSceneItem;
}


//[-------------------------------------------------------]
//[ Public virtual IController methods                    ]
//[-------------------------------------------------------]
void VrController::onUpdate(float pastMilliseconds)
{
	// The first VR controller is used for teleporting
	// -> A green light indicates the position one will end up
	// -> When pressing the trigger button one teleports to this position
	if (mRendererRuntime.getVrManager().getVrManagerTypeId() == RendererRuntime::VrManagerOpenVR::TYPE_ID && ::detail::defaultVrManagerOpenVRListener.getNumberOfVrControllers() >= 1 && nullptr != mTeleportIndicationLightSceneItem)
	{
		// Get VR controller transform data
		const RendererRuntime::VrManagerOpenVR& vrManagerOpenVR = static_cast<RendererRuntime::VrManagerOpenVR&>(mRendererRuntime.getVrManager());
		const glm::mat4& devicePoseMatrix = vrManagerOpenVR.getDevicePoseMatrix(::detail::defaultVrManagerOpenVRListener.getVrControllerTrackedDeviceIndices(0));
		glm::vec3 scale;
		glm::quat rotation;
		glm::vec3 translation;
		glm::vec3 skew;
		glm::vec4 perspective;
		glm::decompose(devicePoseMatrix, scale, rotation, translation, skew, perspective);

		// Everything must be relative to the camera world space position
		translation -= getCameraSceneItem().getParentSceneNodeSafe().getTransform().position;

		// TODO(co) Why is the rotation inverted?
		rotation = glm::inverse(rotation);

		// Construct ray
		const glm::vec3& rayOrigin = translation;
		const glm::vec3 rayDirection = rotation * -RendererRuntime::Math::FORWARD_VECTOR;

		// Simple ray-plane intersection
		static const float MAXIMUM_TELEPORT_DISTANCE = 10.0f;
		float distance = 0.0f;
		if (glm::intersectRayPlane(rayOrigin, rayDirection, RendererRuntime::Math::ZERO_VECTOR, RendererRuntime::Math::UP_VECTOR, distance) && !std::isnan(distance) && distance <= MAXIMUM_TELEPORT_DISTANCE)
		{
			mTeleportIndicationLightSceneItem->setVisible(true);
			mTeleportIndicationLightSceneItem->getParentSceneNode()->setPosition(rayOrigin + rayDirection * distance);
		}
		else
		{
			mTeleportIndicationLightSceneItem->setVisible(false);
		}
	}

	// Call the base implementation
	IController::onUpdate(pastMilliseconds);
}