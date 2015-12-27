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
//[ Header guard                                          ]
//[-------------------------------------------------------]
#pragma once


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include <RendererRuntime/Core/NonCopyable.h>

#include <Renderer/Public/Renderer.h>

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4127)	// warning C4127: conditional expression is constant
	#pragma warning(disable: 4244)	// warning C4244: 'argument': conversion from '<x>' to '<y>', possible loss of data
	#pragma warning(disable: 4266)	// warning C4266: '<x>': no override available for virtual member function from base '<y>'; function is hidden
	#pragma warning(disable: 4365)	// warning C4365: 'return': conversion from '<x>' to '<y>', signed/unsigned mismatch
	#pragma warning(disable: 4548)	// warning C4548: expression before comma has no effect; expected expression with side-effect
	#pragma warning(disable: 4571)	// warning C4571: Informational: catch(...) semantics changed since Visual C++ 7.1; structured exceptions (SEH) are no longer caught
	#pragma warning(disable: 4619)	// warning C4619: #pragma warning: there is no warning number '<x>'
	#pragma warning(disable: 4668)	// warning C4668: '<x>' is not defined as a preprocessor macro, replacing with '0' for '#if/#elif'
	#define POCO_NO_UNWINDOWS
	#include <Poco/JSON/Parser.h>
#pragma warning(pop)


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererToolkit
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	class JsonMaterialHelper : public RendererRuntime::NonCopyable
	{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	public:
		static void optionalFillModeProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::FillMode& value);
		static void optionalCullModeProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::CullMode& value);
		static void optionalConservativeRasterizationModeProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::ConservativeRasterizationMode& value);
		static void optionalDepthWriteMaskProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::DepthWriteMask& value);
		static void optionalStencilOpProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::StencilOp& value);
		static void optionalBlendProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::Blend& value);
		static void optionalBlendOpProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::BlendOp& value);
		static void optionalFilterProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::FilterMode& value);
		static void optionalTextureAddressModeProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::TextureAddressMode& value);
		static void optionalComparisonFuncProperty(Poco::JSON::Object::Ptr jsonObject, const std::string& propertyName, Renderer::ComparisonFunc& value);


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	private:
		JsonMaterialHelper(const JsonMaterialHelper&) = delete;
		JsonMaterialHelper& operator=(const JsonMaterialHelper&) = delete;


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererToolkit
