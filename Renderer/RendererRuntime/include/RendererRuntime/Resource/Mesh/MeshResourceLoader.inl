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
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace RendererRuntime
{


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	inline MeshResourceLoader::MeshResourceLoader(IResourceManager& resourceManager, IRendererRuntime& rendererRuntime) :
		IResourceLoader(resourceManager),
		mRendererRuntime(rendererRuntime),
		mMeshResource(nullptr),
		mProgram(nullptr),
		mNumberOfVertexBufferDataBytes(0),
		mNumberOfUsedVertexBufferDataBytes(0),
		mVertexBufferData(nullptr),
		mNumberOfIndexBufferDataBytes(0),
		mNumberOfUsedIndexBufferDataBytes(0),
		mIndexBufferData(nullptr),
		mIndexBufferFormat(0),
		mNumberOfVertexArrayAttributes(0),
		mNumberOfUsedVertexArrayAttributes(0),
		mVertexArrayFromFile(nullptr),
		mVertexArray(nullptr)
	{
		// Nothing here
	}

	inline void MeshResourceLoader::initialize(const Asset& asset, MeshResource& meshResource, Renderer::IProgram& program)
	{
		mAsset		  = asset;
		mMeshResource = &meshResource;
		mProgram	  = &program;
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // RendererRuntime