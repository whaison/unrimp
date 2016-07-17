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
#include "RendererRuntime/Resource/Material/MaterialResource.h"
#include "RendererRuntime/Resource/Material/MaterialResourceManager.h"

#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Structures                                            ]
		//[-------------------------------------------------------]
		struct OrderByMaterialResourceId
		{
			inline bool operator()(RendererRuntime::MaterialResourceId left, RendererRuntime::MaterialResourceId right) const
			{
				return (left < right);
			}
		};

		struct OrderByMaterialTechniqueId
		{
			inline bool operator()(const RendererRuntime::MaterialTechnique& left, RendererRuntime::MaterialTechniqueId right) const
			{
				return (left.getMaterialTechniqueId() < right);
			}

			inline bool operator()(RendererRuntime::MaterialTechniqueId left, const RendererRuntime::MaterialTechnique& right) const
			{
				return (left < right.getMaterialTechniqueId());
			}
		};


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
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	void MaterialResource::setParentMaterialResourceId(MaterialResourceId parentMaterialResourceId)
	{
		if (mParentMaterialResourceId != parentMaterialResourceId)
		{
			const MaterialResourceId materialResourceId = getId();

			// Unregister from previous parent material resource
			const MaterialResources& materialResources = getResourceManager<MaterialResourceManager>().getMaterialResources();
			if (isInitialized(mParentMaterialResourceId))
			{
				MaterialResource& parentMaterialResource = materialResources.getElementById(mParentMaterialResourceId);
				SortedChildMaterialResourceIds::const_iterator iterator = std::lower_bound(parentMaterialResource.mSortedChildMaterialResourceIds.cbegin(), parentMaterialResource.mSortedChildMaterialResourceIds.cend(), materialResourceId, ::detail::OrderByMaterialResourceId());
				assert(iterator != parentMaterialResource.mSortedChildMaterialResourceIds.end() && *iterator == materialResourceId);
				parentMaterialResource.mSortedChildMaterialResourceIds.erase(iterator);
			}

			// Set new parent material resource ID
			mParentMaterialResourceId = parentMaterialResourceId;
			if (isInitialized(mParentMaterialResourceId))
			{
				// Register to new parent material resource
				assert(materialResources.isElementIdValid(mParentMaterialResourceId));
				MaterialResource& parentMaterialResource = const_cast<MaterialResource&>(materialResources.getElementById(mParentMaterialResourceId));
				assert(parentMaterialResource.getLoadingState() == IResource::LoadingState::LOADED);
				SortedChildMaterialResourceIds::const_iterator iterator = std::lower_bound(parentMaterialResource.mSortedChildMaterialResourceIds.cbegin(), parentMaterialResource.mSortedChildMaterialResourceIds.cend(), materialResourceId, ::detail::OrderByMaterialResourceId());
				assert(iterator == parentMaterialResource.mSortedChildMaterialResourceIds.end() || *iterator != materialResourceId);
				parentMaterialResource.mSortedChildMaterialResourceIds.insert(iterator, materialResourceId);

				// Setup material resource
				setAssetId(parentMaterialResource.getAssetId());
				mSortedMaterialTechniqueVector = parentMaterialResource.mSortedMaterialTechniqueVector;
				for (MaterialTechnique& materialTechnique : mSortedMaterialTechniqueVector)
				{
					materialTechnique.mMaterialResourceId = getId();
				}
				mMaterialProperties = parentMaterialResource.mMaterialProperties;
			}
			else
			{
				// Don't touch the child material resources, but reset everything else
				mSortedMaterialTechniqueVector.clear();
				mMaterialProperties.removeAllProperties();
			}
		}
	}

	MaterialTechnique* MaterialResource::getMaterialTechniqueById(MaterialTechniqueId materialTechniqueId) const
	{
		SortedMaterialTechniqueVector::const_iterator iterator = std::lower_bound(mSortedMaterialTechniqueVector.cbegin(), mSortedMaterialTechniqueVector.cend(), materialTechniqueId, ::detail::OrderByMaterialTechniqueId());
		return (iterator != mSortedMaterialTechniqueVector.end() && iterator._Ptr->getMaterialTechniqueId() == materialTechniqueId) ? iterator._Ptr : nullptr;
	}

	void MaterialResource::releaseTextures()
	{
		// TODO(co) Cleanup
		for (MaterialTechnique& materialTechnique : mSortedMaterialTechniqueVector)
		{
			materialTechnique.mTextures.clear();
		}
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	MaterialResource::~MaterialResource()
	{
		setParentMaterialResourceId(getUninitialized<MaterialResourceId>());

		// Inform child material resources, if required
		if (!mSortedChildMaterialResourceIds.empty())
		{
			const MaterialResources& materialResources = static_cast<MaterialResourceManager&>(getResourceManager()).getMaterialResources();
			for (MaterialResourceId materialResourceId : mSortedChildMaterialResourceIds)
			{
				assert(materialResources.isElementIdValid(materialResourceId));
				materialResources.getElementById(materialResourceId).setParentMaterialResourceId(getUninitialized<MaterialResourceId>());
			}
		}
	}

	bool MaterialResource::setPropertyByIdInternal(MaterialPropertyId materialPropertyId, const MaterialPropertyValue& materialPropertyValue, MaterialProperty::Usage materialPropertyUsage, bool changeOverwrittenState)
	{
		// Call the base implementation
		MaterialProperty* materialProperty = mMaterialProperties.setPropertyById(materialPropertyId, materialPropertyValue, materialPropertyUsage, changeOverwrittenState);
		if (nullptr != materialProperty)
		{
			// Perform derived work, if required to do so
			// TODO(co) "RendererRuntime::MaterialResource::setPropertyByIdInternal()": Implement the other "RendererRuntime::MaterialProperty::Usage" if required
			switch (materialProperty->getUsage())
			{
				case MaterialProperty::Usage::SHADER_UNIFORM:
					// TODO(co)
					break;

				case MaterialProperty::Usage::SHADER_COMBINATION:
					// TODO(co)
					break;

				case MaterialProperty::Usage::RASTERIZER_STATE:
				case MaterialProperty::Usage::DEPTH_STENCIL_STATE:
					// TODO(co)
					break;

				case MaterialProperty::Usage::BLEND_STATE:
					// TODO(co)
					break;

				case MaterialProperty::Usage::TEXTURE_REFERENCE:
					// TODO(co)
					break;

				case MaterialProperty::Usage::UNKNOWN:
				case MaterialProperty::Usage::STATIC:
				case MaterialProperty::Usage::SAMPLER_STATE:
				case MaterialProperty::Usage::COMPOSITOR_TEXTURE_REFERENCE:
				case MaterialProperty::Usage::SHADOW_TEXTURE_REFERENCE:
				case MaterialProperty::Usage::GLOBAL_REFERENCE:
				case MaterialProperty::Usage::UNKNOWN_REFERENCE:
				case MaterialProperty::Usage::PASS_REFERENCE:
				case MaterialProperty::Usage::MATERIAL_REFERENCE:
				case MaterialProperty::Usage::INSTANCE_REFERENCE:
				default:
					// Nothing here
					break;
			}

			// Inform child material resources, if required
			const MaterialResources& materialResources = static_cast<MaterialResourceManager&>(getResourceManager()).getMaterialResources();
			for (MaterialResourceId materialResourceId : mSortedChildMaterialResourceIds)
			{
				assert(materialResources.isElementIdValid(materialResourceId));
				materialResources.getElementById(materialResourceId).setPropertyByIdInternal(materialPropertyId, materialPropertyValue, materialPropertyUsage, false);
			}

			// Material property change detected
			return true;
		}

		// No material property change detected
		return false;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
