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
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	inline MaterialProperty::MaterialProperty() :
		MaterialPropertyValue(fromUnknown()),
		mMaterialPropertyId(0),
		mUsage(Usage::UNKNOWN)
	{
		// Nothing here
	}

	inline MaterialProperty::MaterialProperty(MaterialPropertyId materialPropertyId, Usage usage, const MaterialPropertyValue& materialPropertyValue) :
		MaterialPropertyValue(materialPropertyValue),
		mMaterialPropertyId(materialPropertyId),
		mUsage(usage)
	{
		// Nothing here
	}

	inline MaterialProperty::~MaterialProperty()
	{
		// Nothing here
	}

	inline MaterialPropertyId MaterialProperty::getMaterialPropertyId() const
	{
		return mMaterialPropertyId;
	}

	inline MaterialProperty::Usage MaterialProperty::MaterialProperty::getUsage() const
	{
		return mUsage;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime
