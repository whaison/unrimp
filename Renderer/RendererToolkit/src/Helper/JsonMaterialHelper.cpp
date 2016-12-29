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
#include "RendererToolkit/Helper/JsonMaterialHelper.h"
#include "RendererToolkit/Helper/JsonMaterialBlueprintHelper.h"
#include "RendererToolkit/Helper/FileSystemHelper.h"
#include "RendererToolkit/Helper/JsonHelper.h"

#include <RendererRuntime/Resource/MaterialBlueprint/Loader/MaterialBlueprintFileFormat.h>

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4464)	// warning C4464: relative include path contains '..'
	#pragma warning(disable: 4668)	// warning C4668: '__GNUC__' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#pragma warning(disable: 4365)	// warning C4365: '=': conversion from 'int' to 'rapidjson::internal::BigInteger::Type', signed/unsigned mismatch
	#pragma warning(disable: 4625)	// warning C4625: 'rapidjson::GenericMember<Encoding,Allocator>': copy constructor was implicitly defined as deleted
	#include <rapidjson/document.h>
#pragma warning(pop)

#include <fstream>
#include <algorithm>


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
namespace
{
	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Global functions                                      ]
		//[-------------------------------------------------------]
		bool orderByMaterialTechniqueId(const RendererRuntime::v1Material::Technique& left, const RendererRuntime::v1Material::Technique& right)
		{
			return (left.materialTechniqueId < right.materialTechniqueId);
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
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	void JsonMaterialHelper::optionalFillModeProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::FillMode& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::FillMode::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::FillMode::name;

			// Evaluate value
			IF_VALUE(WIREFRAME)
			ELSE_IF_VALUE(SOLID)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalCullModeProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::CullMode& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::CullMode::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::CullMode::name;

			// Evaluate value
			IF_VALUE(NONE)
			ELSE_IF_VALUE(FRONT)
			ELSE_IF_VALUE(BACK)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalConservativeRasterizationModeProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::ConservativeRasterizationMode& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::ConservativeRasterizationMode::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::ConservativeRasterizationMode::name;

			// Evaluate value
			IF_VALUE(OFF)
			ELSE_IF_VALUE(ON)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalDepthWriteMaskProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::DepthWriteMask& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::DepthWriteMask::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::DepthWriteMask::name;

			// Evaluate value
			IF_VALUE(ZERO)
			ELSE_IF_VALUE(ALL)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalStencilOpProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::StencilOp& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::StencilOp::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::StencilOp::name;

			// Evaluate value
			IF_VALUE(KEEP)
			ELSE_IF_VALUE(ZERO)
			ELSE_IF_VALUE(REPLACE)
			ELSE_IF_VALUE(INCR_SAT)
			ELSE_IF_VALUE(DECR_SAT)
			ELSE_IF_VALUE(INVERT)
			ELSE_IF_VALUE(INCREASE)
			ELSE_IF_VALUE(DECREASE)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalBlendProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::Blend& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::Blend::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::Blend::name;

			// Evaluate value
			IF_VALUE(ZERO)
			ELSE_IF_VALUE(ONE)
			ELSE_IF_VALUE(SRC_COLOR)
			ELSE_IF_VALUE(INV_SRC_COLOR)
			ELSE_IF_VALUE(SRC_ALPHA)
			ELSE_IF_VALUE(INV_SRC_ALPHA)
			ELSE_IF_VALUE(DEST_ALPHA)
			ELSE_IF_VALUE(INV_DEST_ALPHA)
			ELSE_IF_VALUE(DEST_COLOR)
			ELSE_IF_VALUE(INV_DEST_COLOR)
			ELSE_IF_VALUE(SRC_ALPHA_SAT)
			ELSE_IF_VALUE(BLEND_FACTOR)
			ELSE_IF_VALUE(INV_BLEND_FACTOR)
			ELSE_IF_VALUE(SRC_1_COLOR)
			ELSE_IF_VALUE(INV_SRC_1_COLOR)
			ELSE_IF_VALUE(SRC_1_ALPHA)
			ELSE_IF_VALUE(INV_SRC_1_ALPHA)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalBlendOpProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::BlendOp& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::BlendOp::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::BlendOp::name;

			// Evaluate value
			IF_VALUE(ADD)
			ELSE_IF_VALUE(SUBTRACT)
			ELSE_IF_VALUE(REV_SUBTRACT)
			ELSE_IF_VALUE(MIN)
			ELSE_IF_VALUE(MAX)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalFilterProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::FilterMode& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::FilterMode::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::FilterMode::name;

			// Evaluate value
			IF_VALUE(MIN_MAG_MIP_POINT)
			ELSE_IF_VALUE(MIN_MAG_POINT_MIP_LINEAR)
			ELSE_IF_VALUE(MIN_POINT_MAG_LINEAR_MIP_POINT)
			ELSE_IF_VALUE(MIN_POINT_MAG_MIP_LINEAR)
			ELSE_IF_VALUE(MIN_LINEAR_MAG_MIP_POINT)
			ELSE_IF_VALUE(MIN_LINEAR_MAG_POINT_MIP_LINEAR)
			ELSE_IF_VALUE(MIN_MAG_LINEAR_MIP_POINT)
			ELSE_IF_VALUE(MIN_MAG_MIP_LINEAR)
			ELSE_IF_VALUE(ANISOTROPIC)
			ELSE_IF_VALUE(COMPARISON_MIN_MAG_MIP_POINT)
			ELSE_IF_VALUE(COMPARISON_MIN_MAG_POINT_MIP_LINEAR)
			ELSE_IF_VALUE(COMPARISON_MIN_POINT_MAG_LINEAR_MIP_POINT)
			ELSE_IF_VALUE(COMPARISON_MIN_POINT_MAG_MIP_LINEAR)
			ELSE_IF_VALUE(COMPARISON_MIN_LINEAR_MAG_MIP_POINT)
			ELSE_IF_VALUE(COMPARISON_MIN_LINEAR_MAG_POINT_MIP_LINEAR)
			ELSE_IF_VALUE(COMPARISON_MIN_MAG_LINEAR_MIP_POINT)
			ELSE_IF_VALUE(COMPARISON_MIN_MAG_MIP_LINEAR)
			ELSE_IF_VALUE(COMPARISON_ANISOTROPIC)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalTextureAddressModeProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::TextureAddressMode& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::TextureAddressMode::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::TextureAddressMode::name;

			// Evaluate value
			IF_VALUE(WRAP)
			ELSE_IF_VALUE(MIRROR)
			ELSE_IF_VALUE(CLAMP)
			ELSE_IF_VALUE(BORDER)
			ELSE_IF_VALUE(MIRROR_ONCE)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::optionalComparisonFuncProperty(const rapidjson::Value& rapidJsonValue, const char* propertyName, Renderer::ComparisonFunc& value)
	{
		if (rapidJsonValue.HasMember(propertyName))
		{
			const rapidjson::Value& rapidJsonValueValueType = rapidJsonValue[propertyName];
			const char* valueAsString = rapidJsonValueValueType.GetString();
			const rapidjson::SizeType valueStringLength = rapidJsonValueValueType.GetStringLength();

			// Define helper macros
			#define IF_VALUE(name)			 if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::ComparisonFunc::name;
			#define ELSE_IF_VALUE(name) else if (strncmp(valueAsString, #name, valueStringLength) == 0) value = Renderer::ComparisonFunc::name;

			// Evaluate value
			IF_VALUE(NEVER)
			ELSE_IF_VALUE(LESS)
			ELSE_IF_VALUE(EQUAL)
			ELSE_IF_VALUE(LESS_EQUAL)
			ELSE_IF_VALUE(GREATER)
			ELSE_IF_VALUE(NOT_EQUAL)
			ELSE_IF_VALUE(GREATER_EQUAL)
			ELSE_IF_VALUE(ALWAYS)
			else
			{
				// TODO(co) Error handling
			}

			// Undefine helper macros
			#undef IF_VALUE
			#undef ELSE_IF_VALUE
		}
	}

	void JsonMaterialHelper::readMaterialPropertyValues(const IAssetCompiler::Input& input, const rapidjson::Value& rapidJsonValueProperties, RendererRuntime::MaterialProperties::SortedPropertyVector& sortedMaterialPropertyVector)
	{
		for (rapidjson::Value::ConstMemberIterator rapidJsonMemberIteratorProperties = rapidJsonValueProperties.MemberBegin(); rapidJsonMemberIteratorProperties != rapidJsonValueProperties.MemberEnd(); ++rapidJsonMemberIteratorProperties)
		{
			// Material property ID
			const char* propertyName = rapidJsonMemberIteratorProperties->name.GetString();
			const RendererRuntime::MaterialPropertyId materialPropertyId(propertyName);

			// Figure out the material property value type by using the material blueprint
			RendererRuntime::MaterialProperties::SortedPropertyVector::iterator iterator = std::lower_bound(sortedMaterialPropertyVector.begin(), sortedMaterialPropertyVector.end(), materialPropertyId, RendererRuntime::detail::OrderByMaterialPropertyId());
			if (iterator != sortedMaterialPropertyVector.end())
			{
				RendererRuntime::MaterialProperty& materialProperty = *iterator;
				if (materialProperty.getMaterialPropertyId() == materialPropertyId)
				{
					// Set the material own property value
					static_cast<RendererRuntime::MaterialPropertyValue&>(materialProperty) = JsonMaterialBlueprintHelper::mandatoryMaterialPropertyValue(input, rapidJsonValueProperties, propertyName, materialProperty.getValueType());
				}
			}
		}
	}

	void JsonMaterialHelper::getTechniquesAndPropertiesByMaterialAssetId(const IAssetCompiler::Input& input, rapidjson::Document& rapidJsonDocument, std::vector<RendererRuntime::v1Material::Technique>& techniques, RendererRuntime::MaterialProperties::SortedPropertyVector& sortedMaterialPropertyVector)
	{
		// Mandatory main sections of the material
		const rapidjson::Value& rapidJsonValueMaterialAsset = rapidJsonDocument["MaterialAsset"];
		const rapidjson::Value& rapidJsonValueTechniques = rapidJsonValueMaterialAsset["Techniques"];
		const rapidjson::Value& rapidJsonValueProperties = rapidJsonValueMaterialAsset["Properties"];

		// Gather the asset IDs of all used material blueprints (one material blueprint per material technique)
		techniques.reserve(rapidJsonValueTechniques.MemberCount());
		std::unordered_map<uint32_t, std::string> materialTechniqueIdToName;	// Key = "RendererRuntime::MaterialTechniqueId"
		for (rapidjson::Value::ConstMemberIterator rapidJsonMemberIteratorTechniques = rapidJsonValueTechniques.MemberBegin(); rapidJsonMemberIteratorTechniques != rapidJsonValueTechniques.MemberEnd(); ++rapidJsonMemberIteratorTechniques)
		{
			// Add technique
			RendererRuntime::v1Material::Technique technique;
			technique.materialTechniqueId	   = RendererRuntime::StringId(rapidJsonMemberIteratorTechniques->name.GetString());
			technique.materialBlueprintAssetId = static_cast<uint32_t>(std::atoi(rapidJsonMemberIteratorTechniques->value.GetString()));
			techniques.push_back(technique);
			materialTechniqueIdToName.emplace(technique.materialTechniqueId, rapidJsonMemberIteratorTechniques->name.GetString());
		}
		std::sort(techniques.begin(), techniques.end(), detail::orderByMaterialTechniqueId);

		// Gather all material blueprint properties of all referenced material blueprints
		for (RendererRuntime::v1Material::Technique& technique : techniques)
		{
			RendererRuntime::MaterialProperties::SortedPropertyVector newSortedMaterialPropertyVector;
			MaterialPropertyIdToName materialPropertyIdToName;
			JsonMaterialBlueprintHelper::getPropertiesByMaterialBlueprintAssetId(input, technique.materialBlueprintAssetId, newSortedMaterialPropertyVector, &materialPropertyIdToName);

			// Add properties and avoid duplicates while doing so
			for (const RendererRuntime::MaterialProperty& materialProperty : newSortedMaterialPropertyVector)
			{
				const RendererRuntime::MaterialPropertyId materialPropertyId = materialProperty.getMaterialPropertyId();
				RendererRuntime::MaterialProperties::SortedPropertyVector::const_iterator iterator = std::lower_bound(sortedMaterialPropertyVector.cbegin(), sortedMaterialPropertyVector.cend(), materialPropertyId, RendererRuntime::detail::OrderByMaterialPropertyId());
				if (iterator == sortedMaterialPropertyVector.end() || iterator->getMaterialPropertyId() != materialPropertyId)
				{
					// Add new material property
					sortedMaterialPropertyVector.insert(iterator, materialProperty);
				}
				else
				{
					// Sanity checks
					const RendererRuntime::MaterialProperty& otherMaterialProperty = *iterator;
					const bool valueTypeMatch = (materialProperty.getValueType() != otherMaterialProperty.getValueType());
					const bool usageMatch = (materialProperty.getUsage() != otherMaterialProperty.getUsage());
					if (valueTypeMatch | usageMatch)
					{
						const std::string mismatch = valueTypeMatch ? "value type" : "usage";
						throw std::runtime_error(std::string("Material blueprint asset ") + std::to_string(technique.materialBlueprintAssetId) + " referenced by material technique \"" + materialTechniqueIdToName[technique.materialTechniqueId] + "\" has material property \"" + materialPropertyIdToName[materialProperty.getMaterialPropertyId()] + "\" which differs in " + mismatch + " to another already registered material property");
					}
				}
			}

			// Transform the source asset ID into a local asset ID
			technique.materialBlueprintAssetId = input.getCompiledAssetIdBySourceAssetId(technique.materialBlueprintAssetId);
		}

		// Properties: Update material property values were required
		JsonMaterialHelper::readMaterialPropertyValues(input, rapidJsonValueProperties, sortedMaterialPropertyVector);
	}

	void JsonMaterialHelper::getPropertiesByMaterialAssetId(const IAssetCompiler::Input& input, RendererRuntime::AssetId materialAssetId, RendererRuntime::MaterialProperties::SortedPropertyVector& sortedMaterialPropertyVector)
	{
		// TODO(co) Error handling and simplification

		// Read material asset compiler configuration
		std::string materialInputFile;
		const std::string absoluteMaterialAssetFilename = JsonHelper::getAbsoluteAssetFilename(input, materialAssetId);
		{
			// Parse material asset JSON
			std::ifstream materialAssetInputFileStream(absoluteMaterialAssetFilename, std::ios::binary);
			rapidjson::Document rapidJsonDocumentMaterialAsset;
			JsonHelper::parseDocumentByInputFileStream(rapidJsonDocumentMaterialAsset, materialAssetInputFileStream, absoluteMaterialAssetFilename, "Asset", "1");
			materialInputFile = rapidJsonDocumentMaterialAsset["Asset"]["MaterialAssetCompiler"]["InputFile"].GetString();
		}

		// Parse material JSON
		const std::string absoluteMaterialFilename = STD_FILESYSTEM_PATH(absoluteMaterialAssetFilename).parent_path().generic_string() + '/' + materialInputFile;
		std::ifstream materialInputFileStream(absoluteMaterialFilename, std::ios::binary);
		rapidjson::Document rapidJsonDocument;
		JsonHelper::parseDocumentByInputFileStream(rapidJsonDocument, materialInputFileStream, absoluteMaterialFilename, "MaterialAsset", "1");
		std::vector<RendererRuntime::v1Material::Technique> techniques;
		getTechniquesAndPropertiesByMaterialAssetId(input, rapidJsonDocument, techniques, sortedMaterialPropertyVector);
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererToolkit
