/*********************************************************\
 * Copyright (c) 2012-2017 The Unrimp Team
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
#include "OpenGLRenderer/Buffer/UniformBufferDsa.h"
#include "OpenGLRenderer/Extensions.h"
#include "OpenGLRenderer/OpenGLRenderer.h"
#include "OpenGLRenderer/OpenGLRuntimeLinking.h"


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace OpenGLRenderer
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	UniformBufferDsa::UniformBufferDsa(OpenGLRenderer &openGLRenderer, uint32_t numberOfBytes, const void *data, Renderer::BufferUsage bufferUsage) :
		UniformBuffer(openGLRenderer)
	{
		// TODO(co) Review OpenGL uniform buffer alignment topic

		if (openGLRenderer.getExtensions().isGL_ARB_direct_state_access())
		{
			// Create the OpenGL uniform buffer
			glCreateBuffers(1, &mOpenGLUniformBuffer);

			// Upload the data
			// -> Usage: These constants directly map to "GL_ARB_vertex_buffer_object" and OpenGL ES 2 constants, do not change them
			glNamedBufferData(mOpenGLUniformBuffer, static_cast<GLsizeiptr>(numberOfBytes), data, static_cast<GLenum>(bufferUsage));
		}
		else
		{
			// Create the OpenGL uniform buffer
			glGenBuffersARB(1, &mOpenGLUniformBuffer);

			// Upload the data
			// -> Usage: These constants directly map to "GL_ARB_vertex_buffer_object" and OpenGL ES 2 constants, do not change them
			glNamedBufferDataEXT(mOpenGLUniformBuffer, static_cast<GLsizeiptr>(numberOfBytes), data, static_cast<GLenum>(bufferUsage));
		}
	}

	UniformBufferDsa::~UniformBufferDsa()
	{
		// Nothing here
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IUniformBuffer methods       ]
	//[-------------------------------------------------------]
	void UniformBufferDsa::copyDataFrom(uint32_t numberOfBytes, const void *data)
	{
		if (static_cast<OpenGLRenderer&>(getRenderer()).getExtensions().isGL_ARB_direct_state_access())
		{
			// Upload the data
			// -> Subdata is quite optimized for uniform buffers, see http://on-demand.gputechconf.com/siggraph/2014/presentation/SG4117-OpenGL-Scene-Rendering-Techniques.pdf
			glNamedBufferSubData(mOpenGLUniformBuffer, 0, static_cast<GLsizeiptr>(numberOfBytes), data);
		}
		else
		{
			// Upload the data
			// -> Subdata is quite optimized for uniform buffers, see http://on-demand.gputechconf.com/siggraph/2014/presentation/SG4117-OpenGL-Scene-Rendering-Techniques.pdf
			glNamedBufferSubDataEXT(mOpenGLUniformBuffer, 0, static_cast<GLsizeiptr>(numberOfBytes), data);
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // OpenGLRenderer
