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
#ifndef __DIRECT3D12RENDERER_GEOMETRYSHADER_HLSL_H__
#define __DIRECT3D12RENDERER_GEOMETRYSHADER_HLSL_H__


//[-------------------------------------------------------]
//[ Includes                                              ]
//[-------------------------------------------------------]
#include "Direct3D12Renderer/GeometryShader.h"


//[-------------------------------------------------------]
//[ Forward declarations                                  ]
//[-------------------------------------------------------]
struct ID3D12GeometryShader;


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace Direct3D12Renderer
{


	//[-------------------------------------------------------]
	//[ Classes                                               ]
	//[-------------------------------------------------------]
	/**
	*  @brief
	*    HLSL geometry shader class
	*/
	class GeometryShaderHlsl : public GeometryShader
	{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	public:
		/**
		*  @brief
		*    Constructor for creating a geometry shader from shader bytecode
		*
		*  @param[in] direct3D12Renderer
		*    Owner Direct3D 12 renderer instance
		*  @param[in] bytecode
		*    Shader bytecode, must be valid
		*  @param[in] numberOfBytes
		*    Number of bytes in the bytecode
		*/
		GeometryShaderHlsl(Direct3D12Renderer &direct3D12Renderer, const uint8_t *bytecode, uint32_t numberOfBytes);

		/**
		*  @brief
		*    Constructor for creating a geometry shader from shader source code
		*
		*  @param[in] direct3D12Renderer
		*    Owner Direct3D 12 renderer instance
		*  @param[in] sourceCode
		*    Shader ASCII source code, must be valid
		*/
		GeometryShaderHlsl(Direct3D12Renderer &direct3D12Renderer, const char *sourceCode);

		/**
		*  @brief
		*    Destructor
		*/
		virtual ~GeometryShaderHlsl();

		/**
		*  @brief
		*    Return the Direct3D 12 geometry shader
		*
		*  @return
		*    Direct3D 12 geometry shader, can be a null pointer on error, do not release the returned instance unless you added an own reference to it
		*/
		inline ID3D12GeometryShader *getD3D12GeometryShader() const;


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IResource methods            ]
	//[-------------------------------------------------------]
	public:
		virtual void setDebugName(const char *name) override;


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IShader methods              ]
	//[-------------------------------------------------------]
	public:
		virtual const char *getShaderLanguageName() const override;


	//[-------------------------------------------------------]
	//[ Private data                                          ]
	//[-------------------------------------------------------]
	private:
		ID3D12GeometryShader *mD3D12GeometryShader;	///< Direct3D 12 geometry shader, can be a null pointer


	};


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // Direct3D12Renderer


//[-------------------------------------------------------]
//[ Implementation                                        ]
//[-------------------------------------------------------]
#include "Direct3D12Renderer/GeometryShaderHlsl.inl"


//[-------------------------------------------------------]
//[ Header guard                                          ]
//[-------------------------------------------------------]
#endif // __DIRECT3D12RENDERER_GEOMETRYSHADER_HLSL_H__
