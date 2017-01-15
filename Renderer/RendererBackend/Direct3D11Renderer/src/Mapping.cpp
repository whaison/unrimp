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
#include "Direct3D11Renderer/Mapping.h"
#include "Direct3D11Renderer/D3D11.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace Direct3D11Renderer
{


	//[-------------------------------------------------------]
	//[ Public static methods                                 ]
	//[-------------------------------------------------------]
	uint32_t Mapping::getDirect3D11Format(Renderer::VertexAttributeFormat vertexAttributeFormat)
	{
		// DXGI_FORMAT
		static const uint32_t MAPPING[] =
		{
			DXGI_FORMAT_R32_FLOAT,			// Renderer::VertexAttributeFormat::FLOAT_1
			DXGI_FORMAT_R32G32_FLOAT,		// Renderer::VertexAttributeFormat::FLOAT_2
			DXGI_FORMAT_R32G32B32_FLOAT,	// Renderer::VertexAttributeFormat::FLOAT_3
			DXGI_FORMAT_R32G32B32A32_FLOAT,	// Renderer::VertexAttributeFormat::FLOAT_4
			DXGI_FORMAT_R8G8B8A8_UNORM,		// Renderer::VertexAttributeFormat::R8G8B8A8_UNORM
			DXGI_FORMAT_R8G8B8A8_UINT,		// Renderer::VertexAttributeFormat::R8G8B8A8_UINT
			DXGI_FORMAT_R16G16_SINT,		// Renderer::VertexAttributeFormat::SHORT_2
			DXGI_FORMAT_R16G16B16A16_SINT	// Renderer::VertexAttributeFormat::SHORT_4
		};
		return MAPPING[static_cast<int>(vertexAttributeFormat)];
	}

	uint32_t Mapping::getDirect3D11UsageAndCPUAccessFlags(Renderer::BufferUsage bufferUsage, uint32_t &cpuAccessFlags)
	{
		// Direct3D 11 only supports a subset of the OpenGL usage indications
		// -> See "D3D11_USAGE enumeration "-documentation at http://msdn.microsoft.com/en-us/library/windows/desktop/ff476259%28v=vs.85%29.aspx
		switch (bufferUsage)
		{
			case Renderer::BufferUsage::STREAM_DRAW:
			case Renderer::BufferUsage::STREAM_COPY:
			case Renderer::BufferUsage::STATIC_DRAW:
			case Renderer::BufferUsage::STATIC_COPY:
				cpuAccessFlags = 0;
				return D3D11_USAGE_IMMUTABLE;

			case Renderer::BufferUsage::STREAM_READ:
			case Renderer::BufferUsage::STATIC_READ:
				cpuAccessFlags = D3D11_CPU_ACCESS_READ;
				return D3D11_USAGE_STAGING;

			case Renderer::BufferUsage::DYNAMIC_DRAW:
			case Renderer::BufferUsage::DYNAMIC_COPY:
				cpuAccessFlags = D3D11_CPU_ACCESS_WRITE;
				return D3D11_USAGE_DYNAMIC;

			default:
			case Renderer::BufferUsage::DYNAMIC_READ:
				cpuAccessFlags = 0;
				return D3D11_USAGE_DEFAULT;
		}
	}

	uint32_t Mapping::getDirect3D11Format(Renderer::IndexBufferFormat::Enum indexBufferFormat)
	{
		// DXGI_FORMAT
		static const uint32_t MAPPING[] =
		{
			DXGI_FORMAT_R32_UINT,	// Renderer::IndexBufferFormat::UNSIGNED_CHAR  - One byte per element, uint8_t (may not be supported by each API) - Not supported by Direct3D 11
			DXGI_FORMAT_R16_UINT,	// Renderer::IndexBufferFormat::UNSIGNED_SHORT - Two bytes per element, uint16_t
			DXGI_FORMAT_R32_UINT	// Renderer::IndexBufferFormat::UNSIGNED_INT   - Four bytes per element, uint32_t (may not be supported by each API)
		};
		return MAPPING[indexBufferFormat];
	}

	uint32_t Mapping::getDirect3D11Format(Renderer::TextureFormat::Enum textureFormat)
	{
		// DXGI_FORMAT
		static const uint32_t MAPPING[] =
		{
			DXGI_FORMAT_A8_UNORM,				// Renderer::TextureFormat::A8            - 8-bit pixel format, all bits alpha
			DXGI_FORMAT_B8G8R8X8_UNORM,			// Renderer::TextureFormat::R8G8B8        - 24-bit pixel format, 8 bits for red, green and blue
			DXGI_FORMAT_R8G8B8A8_UNORM,			// Renderer::TextureFormat::R8G8B8A8      - 32-bit pixel format, 8 bits for red, green, blue and alpha
			DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,	// Renderer::TextureFormat::R8G8B8A8_SRGB - 32-bit pixel format, 8 bits for red, green, blue and alpha; sRGB = RGB hardware gamma correction, the alpha channel always remains linear
			DXGI_FORMAT_R16G16B16A16_FLOAT,		// Renderer::TextureFormat::R16G16B16A16F - 64-bit float format using 16 bits for the each channel (red, green, blue, alpha)
			DXGI_FORMAT_R32G32B32A32_FLOAT,		// Renderer::TextureFormat::R32G32B32A32F - 128-bit float format using 32 bits for the each channel (red, green, blue, alpha)
			DXGI_FORMAT_BC1_UNORM,				// Renderer::TextureFormat::BC1           - DXT1 compression (known as BC1 in DirectX 10, RGB compression: 8:1, 8 bytes per block)
			DXGI_FORMAT_BC1_UNORM_SRGB,			// Renderer::TextureFormat::BC1_SRGB      - DXT1 compression (known as BC1 in DirectX 10, RGB compression: 8:1, 8 bytes per block); sRGB = RGB hardware gamma correction, the alpha channel always remains linear
			DXGI_FORMAT_BC2_UNORM,				// Renderer::TextureFormat::BC2           - DXT3 compression (known as BC2 in DirectX 10, RGBA compression: 4:1, 16 bytes per block)
			DXGI_FORMAT_BC2_UNORM_SRGB,			// Renderer::TextureFormat::BC2_SRGB      - DXT3 compression (known as BC2 in DirectX 10, RGBA compression: 4:1, 16 bytes per block); sRGB = RGB hardware gamma correction, the alpha channel always remains linear
			DXGI_FORMAT_BC3_UNORM,				// Renderer::TextureFormat::BC3           - DXT5 compression (known as BC3 in DirectX 10, RGBA compression: 4:1, 16 bytes per block)
			DXGI_FORMAT_BC3_UNORM_SRGB,			// Renderer::TextureFormat::BC3_SRGB      - DXT5 compression (known as BC3 in DirectX 10, RGBA compression: 4:1, 16 bytes per block); sRGB = RGB hardware gamma correction, the alpha channel always remains linear
			DXGI_FORMAT_BC4_UNORM,				// Renderer::TextureFormat::BC4           - 1 component texture compression (also known as 3DC+/ATI1N, known as BC4 in DirectX 10, 8 bytes per block)
			DXGI_FORMAT_BC5_UNORM,				// Renderer::TextureFormat::BC5           - 2 component texture compression (luminance & alpha compression 4:1 -> normal map compression, also known as 3DC/ATI2N, known as BC5 in DirectX 10, 16 bytes per block)
			DXGI_FORMAT_UNKNOWN,				// Renderer::TextureFormat::ETC1          - 3 component texture compression meant for mobile devices - not supported in Direct3D 11
			DXGI_FORMAT_R32_FLOAT,				// Renderer::TextureFormat::R32_FLOAT     - 32-bit float format
			DXGI_FORMAT_D32_FLOAT,				// Renderer::TextureFormat::D32_FLOAT     - 32-bit float depth format
			DXGI_FORMAT_UNKNOWN					// Renderer::TextureFormat::UNKNOWN       - Unknown
		};
		return MAPPING[textureFormat];
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // Direct3D11Renderer
