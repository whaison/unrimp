/*********************************************************\
 * Copyright (c) 2012-2014 Christian Ofenberg
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
#include "RendererRuntime/Resource/Texture/DdsTextureResourceSerializer.h"
#include "RendererRuntime/IRendererRuntime.h"

#include <Renderer/Public/Renderer.h>

// Disable warnings in external headers, we can't fix them
#pragma warning(push)
	#pragma warning(disable: 4548)	// warning C4548: expression before comma has no effect; expected expression with side-effect
	#include <fstream>
#pragma warning(pop)


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	namespace detail
	{


		//[-------------------------------------------------------]
		//[ Definitions                                           ]
		//[-------------------------------------------------------]
		static const uint32_t DDSCAPS2_CUBEMAP = 0x00000200;
		static const uint32_t DDS_FOURCC = 0x00000004;
		static const uint32_t DDS_LUMINANCE = 0x00020000;
		static const uint32_t DDS_ALPHAPIXELS = 0x00000001;
		static const uint32_t DDS_LINEARSIZE = 0x00080000;
		static const uint32_t DDS_PITCH = 0x00000008;
		static const uint32_t DDSD_CAPS = 0x00000001;
		static const uint32_t DDSD_PIXELFORMAT = 0x00001000;
		static const uint32_t DDSD_HEIGHT = 0x00000002;
		static const uint32_t DDSD_WIDTH = 0x00000004;
		static const uint32_t DDSD_MIPMAPCOUNT = 0x00020000;
		static const uint32_t DDSD_DEPTH = 0x00800000;
		static const uint32_t DDPF_FOURCC = 0x00000004;
		static const uint32_t DDSCAPS_TEXTURE = 0x00001000;
		static const uint32_t DDSCAPS_MIPMAP = 0x00400000;
		static const uint32_t DDSCAPS_COMPLEX = 0x00000008;
		static const uint32_t DDSCAPS2_VOLUME = 0x00200000;
		static const uint32_t DDSCAPS2_CUBEMAP_POSITIVEX = 0x00000400;
		static const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEX = 0x00000800;
		static const uint32_t DDSCAPS2_CUBEMAP_POSITIVEY = 0x00001000;
		static const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEY = 0x00002000;
		static const uint32_t DDSCAPS2_CUBEMAP_POSITIVEZ = 0x00004000;
		static const uint32_t DDSCAPS2_CUBEMAP_NEGATIVEZ = 0x00008000;
		static const uint32_t DDSCAPS2_CUBEMAP_ALL_FACES = (DDSCAPS2_CUBEMAP_POSITIVEX | DDSCAPS2_CUBEMAP_NEGATIVEX | DDSCAPS2_CUBEMAP_POSITIVEY | DDSCAPS2_CUBEMAP_NEGATIVEY | DDSCAPS2_CUBEMAP_POSITIVEZ | DDSCAPS2_CUBEMAP_NEGATIVEZ);


		//[-------------------------------------------------------]
		//[ Structures                                            ]
		//[-------------------------------------------------------]
		struct DdsHeader
		{
			uint8_t magic[4];
			uint32_t size;
			uint32_t flags;
			uint32_t height;
			uint32_t width;
			uint32_t pitchOrLinearSize;
			uint32_t depth;
			uint32_t mipMapCount;
			uint32_t reserved[11];
			struct
			{
				uint32_t size;
				uint32_t flags;
				uint32_t fourCC;
				uint32_t RGBBitCount;
				uint32_t RBitMask;
				uint32_t GBitMask;
				uint32_t BBitMask;
				uint32_t RGBAlphaBitMask;
			} ddpfPixelFormat;
			struct
			{
				uint32_t caps1;
				uint32_t caps2;
				uint32_t reserved[2];
			} ddsCaps;
			uint32_t reserved2;
		};

		struct DdsHeaderDX10
		{
			uint32_t DXGIFormat; // See http://msdn.microsoft.com/en-us/library/bb173059.aspx
			uint32_t resourceDimension;
			uint32_t miscFlag;
			uint32_t arraySize;
			uint32_t reserved;
		};


	}



	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	// TODO(co) Work-in-progress
	Renderer::ITexture* DdsTextureResourceSerializer::loadDdsTexture(std::istream& istream)
	{
		Renderer::ITexture2D* texture2D = nullptr;

		#define MCHAR4(a, b, c, d) (a | (b << 8) | (c << 16) | (d << 24))

		// Read the header
		detail::DdsHeader ddsHeader;
		istream.read(reinterpret_cast<char*>(&ddsHeader), sizeof(detail::DdsHeader));
		if (ddsHeader.magic[0] == 'D' && ddsHeader.magic[1] == 'D' && ddsHeader.magic[2] == 'S' && ddsHeader.magic[3] == ' ' &&
			// Note that if "size" is "DDS " this is not a valid dds file according
			// to the file spec. Some broken tool out there seems to produce files
			// with this value in the size field, so we support reading them...
			(ddsHeader.size == 124 || ddsHeader.size != MCHAR4('D', 'D', 'S', ' ')))
		{
			// Get the color format and compression
			// TODO(co)
		//	EDataFormat  nDataFormat = DataByte;
		//	EColorFormat nColorFormat;
		//	EColorFormat nInternalColorFormat;
		//	ECompression nCompression = CompressionNone;

			// Get the depth
			const uint32_t depth = ddsHeader.depth ? ddsHeader.depth : 1;

			// Is this image compressed?
			if (ddsHeader.ddpfPixelFormat.flags & detail::DDS_FOURCC)
			{
				// The image is compressed
				if (ddsHeader.ddpfPixelFormat.fourCC == MCHAR4('D', 'X', '1', '0'))
				{
					// Read the DX10 header
					detail::DdsHeaderDX10 ddsHeaderDX10;
					istream.read(reinterpret_cast<char*>(&ddsHeaderDX10), sizeof(detail::DdsHeaderDX10));

					// Get the color format and compression
					switch (ddsHeaderDX10.DXGIFormat)
					{
					// Integer
						// R8 UNORM
						case 61:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							break;

						// RG8 UNORM
						case 49:
							// TODO(co)
							// nInternalColorFormat = (ddsHeader.ddpfPixelFormat.BBitMask == 0xFF) ? ColorBGR : ColorRGB;
							// nColorFormat = ColorRGB;	// Store it as RGB
							break;

						// RGBA8 UNORM
						case 28:
							// TODO(co)
							// nInternalColorFormat = (ddsHeader.ddpfPixelFormat.BBitMask == 0xFF) ? ColorBGRA : ColorRGBA;
							// nColorFormat = ColorRGBA;
							break;

					// 16 bit float
						// R16F
						case 54:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nDataFormat = DataHalf;
							break;

						// RG16F
						case 34:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nDataFormat = DataHalf;
							break;

						// RGBA16F
						case 10:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nDataFormat = DataHalf;
							break;

					// IEEE 32 bit float
						// R32F
						case 41:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nDataFormat = DataFloat;
							break;

						// RG32F
						case 16:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nDataFormat = DataFloat;
							break;

						// RGB32F
						case 6:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGB;
							// nDataFormat = DataFloat;
							break;

						// RGBA32F
						case 2:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nDataFormat = DataFloat;
							break;

					// Compressed
						// DXT1 (BC1 UNORM)
						case 71:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGB;
							// nCompression = CompressionDXT1;
							break;

						// DXT3 (BC2 UNORM)
						case 74:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nCompression = CompressionDXT3;
							break;

						// DXT5 (BC3 UNORM)
						case 77:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nCompression = CompressionDXT5;
							break;

						// LATC1 (BC4 UNORM, previously known as ATI1N)
						case 80:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nCompression = CompressionLATC1;
							break;

						// LATC2 (BC5 UNORM, previously known as ATI2N)
						case 83:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nCompression = CompressionLATC2;
							break;

						default:
							// Error: Unsupported format
							return false;
					}
				}
				else
				{
					switch (ddsHeader.ddpfPixelFormat.fourCC)
					{
					// 16 bit float
						// R16F
						case 111:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nDataFormat = DataHalf;
							break;

						// RG16F
						case 112:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nDataFormat = DataHalf;
							break;

						// RGBA16F
						case 113:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nDataFormat = DataHalf;
							break;

					// IEEE 32 bit float
						// R32F
						case 114:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nDataFormat = DataFloat;
							break;

						// RG32F
						case 115:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nDataFormat = DataFloat;
							break;

						// RGBA32F
						case 116:
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nDataFormat = DataFloat;
							break;

					// Compressed
						// DXT1 (BC1 UNORM)
						case MCHAR4('D', 'X', 'T', '1'):
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGB;
							// nCompression = CompressionDXT1;
							break;

						// DXT3 (BC2 UNORM)
						case MCHAR4('D', 'X', 'T', '3'):
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nCompression = CompressionDXT3;
							break;

						// DXT5 (BC3 UNORM)
						case MCHAR4('D', 'X', 'T', '5'):
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorRGBA;
							// nCompression = CompressionDXT5;
							break;

						// LATC1 (BC4 UNORM, previously known as ATI1N)
						case MCHAR4('A', 'T', 'I', '1'):
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscale;
							// nCompression = CompressionLATC1;
							break;

						// LATC2 (BC5 UNORM, previously known as ATI2N)
						case MCHAR4('A', 'T', 'I', '2'):
							// TODO(co)
							// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
							// nCompression = CompressionLATC2;
							break;

					// Uncompressed
						default:
							switch (ddsHeader.ddpfPixelFormat.RGBBitCount)
							{
								// R8
								case 8:
									// TODO(co)
									// nInternalColorFormat = nColorFormat = ColorGrayscale;
									break;

								// LA8
								case 16:
									if (ddsHeader.ddpfPixelFormat.RGBAlphaBitMask == 0xFF00)
									{
										// TODO(co)
										// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
									}
									else
									{
										// Error: Unsupported format
										return false;
									}
									break;

								// RGB8
								case 24:
									// TODO(co)
									// nInternalColorFormat = (ddsHeader.ddpfPixelFormat.BBitMask == 0xFF) ? ColorBGR : ColorRGB;
									// nColorFormat         = ColorRGB;
									break;

								// RGBA8
								case 32:
									if (ddsHeader.ddpfPixelFormat.RBitMask != 0x3FF00000)
									{
										// TODO(co)
										// nInternalColorFormat = (ddsHeader.ddpfPixelFormat.BBitMask == 0xFF) ? ColorBGRA : ColorRGBA;
										// nColorFormat         = ColorRGBA;
									}
									else
									{
										// Error: Unsupported format
										return false;
									}
									break;

								default:
									// Error: Unsupported format
									return false;
							}
					}
				}
			}
			else
			{
				// The image is not compressed
				if (ddsHeader.ddpfPixelFormat.flags & detail::DDS_LUMINANCE)
				{
					if (ddsHeader.ddpfPixelFormat.flags & detail::DDS_ALPHAPIXELS)
					{
						// TODO(co)
						// nInternalColorFormat = nColorFormat = ColorGrayscaleA;
					}
					else
					{
						// TODO(co)
						// nInternalColorFormat = nColorFormat = ColorGrayscale;
					}
				}
				else
				{
					if (ddsHeader.ddpfPixelFormat.flags & detail::DDS_ALPHAPIXELS)
					{
						// Set color format, please not that all bit mask relevant stuff is done inside "DecompressRGBA()"
						// TODO(co)
						// nInternalColorFormat = nColorFormat = ColorRGBA;
					}
					else
					{
						// Set color format, please not that all bit mask relevant stuff is done inside "DecompressRGBA()"
						// TODO(co)
						// nInternalColorFormat = nColorFormat = ColorRGB;
					}
				}

				// Microsoft bug, they're not following their own documentation
				if (!(ddsHeader.ddpfPixelFormat.flags & (detail::DDS_LINEARSIZE | detail::DDS_PITCH)) || !ddsHeader.pitchOrLinearSize)
				{
					ddsHeader.ddpfPixelFormat.flags |= detail::DDS_LINEARSIZE;
				}
			}

			// Get the number of mipmaps
			const uint32_t numberOfMipmaps = (!ddsHeader.mipMapCount) ? 1 : ddsHeader.mipMapCount;

			// Cube map?
			const uint32_t numberOfFaces = (ddsHeader.ddsCaps.caps2 & detail::DDSCAPS2_CUBEMAP) ? 6u : 1u;

			{ // Loop through all faces
				uint32_t width = ddsHeader.width;
				uint32_t height = ddsHeader.height;
				size_t compressedSize = 0;
				while (width > 1 && height > 1)
				{
					compressedSize += ((width + 3) >> 2) * ((height + 3) >> 2) * 8;

					width /= 2;
					height /= 2;
				}
				compressedSize += ((width + 3) >> 2) * ((height + 3) >> 2) * 8;


				// Avoid slow division by using bit-shift
	//			const size_t compressedSize = ((ddsHeader.width + 3) >> 2) * ((ddsHeader.height + 3) >> 2) * 8;
				// m_nCompressedSize = ((ddsHeader.width + 3) / 4) * ((ddsHeader.height + 3) / 4) * 8;

				// TODO(co)
				// A simple one: Just read in the whole compressed data
				uint8_t* tempDataTest = new uint8_t[compressedSize];
				istream.read(reinterpret_cast<char*>(tempDataTest), compressedSize);

				// Create the texture instance
				texture2D = mRendererRuntime.getRenderer().createTexture2D(ddsHeader.width, ddsHeader.height, Renderer::TextureFormat::BC1, tempDataTest, Renderer::TextureFlag::DATA_CONTAINS_MIPMAPS);
	//			texture2D = renderer.createTexture2D(ddsHeader.width, ddsHeader.height, Renderer::TextureFormat::BC1, tempDataTest, Renderer::TextureFlag::GENERATE_MIPMAPS);
				delete [] tempDataTest;



				uint8_t* tempData = nullptr;	// Used when "DDS_LINEARSIZE" is set
				for (uint32_t face = 0; face < numberOfFaces; ++face)
				{
					// Load in all mipmaps
					for (uint32_t mipmap = 0; mipmap < numberOfMipmaps; ++mipmap)
					{
					}

					// TODO(co)
					/*
					// Create image part with reasonable semantic
					ImagePart *pImagePart = cImage.CreatePart((numberOfFaces == 6) ? (static_cast<uint32_t>(ImagePartCubeSidePosX) + face) : 0);
					if (pImagePart)
					{
						// Load in all mipmaps
						for (uint32_t mipmap = 0; mipmap < numberOfMipmaps; ++mipmap)
						{
							// Create image buffer
							ImageBuffer *pImageBuffer = pImagePart->CreateMipmap();
							pImageBuffer->CreateImage(nDataFormat,
													  nColorFormat,
													  Vector3i(ImageBuffer::GetMipmapSize(ddsHeader.nWidth,  mipmap),
															   ImageBuffer::GetMipmapSize(ddsHeader.nHeight, mipmap),
															   ImageBuffer::GetMipmapSize(nDepth,		   mipmap)),
													  nCompression);

							// Read in compressed data?
							if (nCompression == CompressionNone)
							{
								// If the "DDS_LINEARSIZE" flag is set we need to do some more work...
								if (ddsHeader.ddpfPixelFormat.flags & DDS_LINEARSIZE)
								{
									// Calculate the current linear size
									const uint32_t pitchOrLinearSize = (pImageBuffer->GetSize().x * pImageBuffer->GetSize().y * nDepth * (ddsHeader.ddpfPixelFormat.nRGBBitCount >> 3));

									// Allocate temp data right now? (we can reuse it for the following smaller mipmaps)
									if (!tempData)
									{
										tempData = new uint8_t[pitchOrLinearSize];
									}

									// Read the data
									fread(tempData, 1, pitchOrLinearSize, file);

									// Decompress the image data
									DecompressRGBA(ddsHeader, *pImageBuffer, tempData);
								}
								else
								{
									// A simple one: Just read in the whole uncompressed data
									fread(pImageBuffer->GetData(), 1, pImageBuffer->GetDataSize(), file);
								}
							}
							else
							{
								// A simple one: Just read in the whole compressed data
								fread(pImageBuffer->GetCompressedData(), 1, pImageBuffer->GetCompressedDataSize(), file);
							}
						}
					}
				}

				// Cleanup temp data
				if (tempData)
				{
					delete [] tempData;
				}
				*/
			}

			// TODO(co)
			/*
			// Convert BGR(A) to RGB(A)
			if (nInternalColorFormat == ColorBGR)
			{
				// Loop through all faces
				for (uint32_t face = 0; face < numberOfFaces; ++face)
				{
					// Create image part with reasonable semantic
					ImagePart *pImagePart = cImage.GetPartBySemantics((numberOfFaces == 6) ? (static_cast<uint32_t>(ImagePartCubeSidePosX) + face) : 0);
					if (pImagePart)
					{
						// Load in all mipmaps
						for (uint32_t mipmap = 0; mipmap < numberOfMipmaps; ++mipmap)
						{
							// Get image buffer
							ImageBuffer *pImageBuffer = pImagePart->GetMipmap(mipmap);

							// Swap R/B
							Color3::SwapRB(pImageBuffer->GetData(), pImageBuffer->GetNumOfPixels());
						}
					}
				}
			}
			else if (nInternalColorFormat == ColorBGRA)
			{
				// Loop through all faces
				for (uint32_t face = 0; face < numberOfFaces; ++face)
				{
					// Create image part with reasonable semantic
					ImagePart *pImagePart = cImage.GetPartBySemantics((numberOfFaces == 6) ? (static_cast<uint32_t>(ImagePartCubeSidePosX) + face) : 0);
					if (pImagePart)
					{
						// Load in all mipmaps
						for (uint32_t mipmap = 0; mipmap < numberOfMipmaps; ++mipmap)
						{
							// Get image buffer
							ImageBuffer *pImageBuffer = pImagePart->GetMipmap(mipmap);

							// Swap R/B
							Color4::SwapRB(pImageBuffer->GetData(), pImageBuffer->GetNumOfPixels());
						}
					}
				}
			*/
			}
		}
		else
		{
			// Error: Invalid magic number
		}

		#undef MCHAR4

		return texture2D;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime