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
#include "OpenGLRenderer/OpenGLRenderer.h"
#include "OpenGLRenderer/OpenGLDebug.h"	// For "OPENGLRENDERER_RENDERERMATCHCHECK_RETURN()"
#include "OpenGLRenderer/Mapping.h"
#include "OpenGLRenderer/Extensions.h"
#include "OpenGLRenderer/RootSignature.h"
#include "OpenGLRenderer/OpenGLRuntimeLinking.h"
#include "OpenGLRenderer/RenderTarget/SwapChain.h"
#include "OpenGLRenderer/RenderTarget/FramebufferDsa.h"
#include "OpenGLRenderer/RenderTarget/FramebufferBind.h"
#include "OpenGLRenderer/Buffer/BufferManager.h"
#include "OpenGLRenderer/Buffer/TextureBuffer.h"
#include "OpenGLRenderer/Buffer/IndexBufferDsa.h"
#include "OpenGLRenderer/Buffer/IndexBufferBind.h"
#include "OpenGLRenderer/Buffer/VertexBufferDsa.h"
#include "OpenGLRenderer/Buffer/VertexBufferBind.h"
#include "OpenGLRenderer/Buffer/UniformBufferDsa.h"
#include "OpenGLRenderer/Buffer/UniformBufferBind.h"
#include "OpenGLRenderer/Buffer/VertexArrayNoVao.h"
#include "OpenGLRenderer/Buffer/VertexArrayVaoDsa.h"
#include "OpenGLRenderer/Buffer/VertexArrayVaoBind.h"
#include "OpenGLRenderer/Buffer/IndirectBuffer.h"
#include "OpenGLRenderer/Texture/TextureManager.h"
#include "OpenGLRenderer/Texture/Texture2D.h"
#include "OpenGLRenderer/Texture/Texture2DArray.h"
#include "OpenGLRenderer/State/SamplerStateSo.h"
#include "OpenGLRenderer/State/SamplerStateDsa.h"
#include "OpenGLRenderer/State/SamplerStateBind.h"
#include "OpenGLRenderer/State/PipelineState.h"
#include "OpenGLRenderer/Shader/Monolithic/ShaderLanguageMonolithic.h"
#include "OpenGLRenderer/Shader/Monolithic/ProgramMonolithic.h"
#include "OpenGLRenderer/Shader/Separate/ShaderLanguageSeparate.h"
#include "OpenGLRenderer/Shader/Separate/ProgramSeparate.h"

#ifdef WIN32
	#include "OpenGLRenderer/Windows/ContextWindows.h"
#elif defined LINUX
	#include "OpenGLRenderer/Linux/ContextLinux.h"
#endif

#include <Renderer/Buffer/CommandBuffer.h>
#include <Renderer/Buffer/IndirectBufferTypes.h>


//[-------------------------------------------------------]
//[ Global functions                                      ]
//[-------------------------------------------------------]
// Export the instance creation function
#ifdef OPENGLRENDERER_EXPORTS
	#define OPENGLRENDERER_API_EXPORT GENERIC_API_EXPORT
#else
	#define OPENGLRENDERER_API_EXPORT
#endif
// TODO(co) We might want to give this function a better name so one knows what it's about
OPENGLRENDERER_API_EXPORT Renderer::IRenderer *createOpenGLRendererInstance2(handle nativeWindowHandle, bool externalContext)
{
	return new OpenGLRenderer::OpenGLRenderer(nativeWindowHandle, externalContext);
}

OPENGLRENDERER_API_EXPORT Renderer::IRenderer *createOpenGLRendererInstance(handle nativeWindowHandle)
{
	return createOpenGLRendererInstance2(nativeWindowHandle, false);
}
#undef OPENGLRENDERER_API_EXPORT


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
		namespace BackendDispatch
		{


			//[-------------------------------------------------------]
			//[ Resource handling                                     ]
			//[-------------------------------------------------------]
			void CopyUniformBufferData(const void* data, Renderer::IRenderer&)
			{
				const Renderer::Command::CopyUniformBufferData* realData = static_cast<const Renderer::Command::CopyUniformBufferData*>(data);
				realData->uniformBuffer->copyDataFrom(realData->numberOfBytes, (nullptr != realData->data) ? realData->data : Renderer::CommandPacketHelper::getAuxiliaryMemory(realData));
			}

			void CopyTextureBufferData(const void* data, Renderer::IRenderer&)
			{
				const Renderer::Command::CopyTextureBufferData* realData = static_cast<const Renderer::Command::CopyTextureBufferData*>(data);
				realData->textureBuffer->copyDataFrom(realData->numberOfBytes, (nullptr != realData->data) ? realData->data : Renderer::CommandPacketHelper::getAuxiliaryMemory(realData));
			}

			//[-------------------------------------------------------]
			//[ Graphics root                                         ]
			//[-------------------------------------------------------]
			void SetGraphicsRootSignature(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetGraphicsRootSignature* realData = static_cast<const Renderer::Command::SetGraphicsRootSignature*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).setGraphicsRootSignature(realData->rootSignature);
			}

			void SetGraphicsRootDescriptorTable(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetGraphicsRootDescriptorTable* realData = static_cast<const Renderer::Command::SetGraphicsRootDescriptorTable*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).setGraphicsRootDescriptorTable(realData->rootParameterIndex, realData->resource);
			}

			//[-------------------------------------------------------]
			//[ States                                                ]
			//[-------------------------------------------------------]
			void SetPipelineState(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetPipelineState* realData = static_cast<const Renderer::Command::SetPipelineState*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).setPipelineState(realData->pipelineState);
			}

			//[-------------------------------------------------------]
			//[ Input-assembler (IA) stage                            ]
			//[-------------------------------------------------------]
			void SetVertexArray(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetVertexArray* realData = static_cast<const Renderer::Command::SetVertexArray*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).iaSetVertexArray(realData->vertexArray);
			}

			void SetPrimitiveTopology(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetPrimitiveTopology* realData = static_cast<const Renderer::Command::SetPrimitiveTopology*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).iaSetPrimitiveTopology(realData->primitiveTopology);
			}

			//[-------------------------------------------------------]
			//[ Rasterizer (RS) stage                                 ]
			//[-------------------------------------------------------]
			void SetViewports(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetViewports* realData = static_cast<const Renderer::Command::SetViewports*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).rsSetViewports(realData->numberOfViewports, (nullptr != realData->viewports) ? realData->viewports : reinterpret_cast<const Renderer::Viewport*>(Renderer::CommandPacketHelper::getAuxiliaryMemory(realData)));
			}

			void SetScissorRectangles(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetScissorRectangles* realData = static_cast<const Renderer::Command::SetScissorRectangles*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).rsSetScissorRectangles(realData->numberOfScissorRectangles, (nullptr != realData->scissorRectangles) ? realData->scissorRectangles : reinterpret_cast<const Renderer::ScissorRectangle*>(Renderer::CommandPacketHelper::getAuxiliaryMemory(realData)));
			}

			//[-------------------------------------------------------]
			//[ Output-merger (OM) stage                              ]
			//[-------------------------------------------------------]
			void SetRenderTarget(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetRenderTarget* realData = static_cast<const Renderer::Command::SetRenderTarget*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).omSetRenderTarget(realData->renderTarget);
			}

			//[-------------------------------------------------------]
			//[ Operations                                            ]
			//[-------------------------------------------------------]
			void Clear(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::Clear* realData = static_cast<const Renderer::Command::Clear*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).clear(realData->flags, realData->color, realData->z, realData->stencil);
			}

			void ResolveMultisampleFramebuffer(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::ResolveMultisampleFramebuffer* realData = static_cast<const Renderer::Command::ResolveMultisampleFramebuffer*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).resolveMultisampleFramebuffer(*realData->destinationRenderTarget, *realData->sourceMultisampleFramebuffer);
			}

			void CopyResource(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::CopyResource* realData = static_cast<const Renderer::Command::CopyResource*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).copyResource(*realData->destinationResource, *realData->sourceResource);
			}

			//[-------------------------------------------------------]
			//[ Draw call                                             ]
			//[-------------------------------------------------------]
			void Draw(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::Draw* realData = static_cast<const Renderer::Command::Draw*>(data);
				if (nullptr != realData->indirectBuffer)
				{
					static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).draw(*realData->indirectBuffer, realData->indirectBufferOffset, realData->numberOfDraws);
				}
				else
				{
					static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).drawEmulated(Renderer::CommandPacketHelper::getAuxiliaryMemory(realData), realData->indirectBufferOffset, realData->numberOfDraws);
				}
			}

			void DrawIndexed(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::Draw* realData = static_cast<const Renderer::Command::Draw*>(data);
				if (nullptr != realData->indirectBuffer)
				{
					static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).drawIndexed(*realData->indirectBuffer, realData->indirectBufferOffset, realData->numberOfDraws);
				}
				else
				{
					static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).drawIndexedEmulated(Renderer::CommandPacketHelper::getAuxiliaryMemory(realData), realData->indirectBufferOffset, realData->numberOfDraws);
				}
			}

			//[-------------------------------------------------------]
			//[ Debug                                                 ]
			//[-------------------------------------------------------]
			void SetDebugMarker(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::SetDebugMarker* realData = static_cast<const Renderer::Command::SetDebugMarker*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).setDebugMarker(realData->name);
			}

			void BeginDebugEvent(const void* data, Renderer::IRenderer& renderer)
			{
				const Renderer::Command::BeginDebugEvent* realData = static_cast<const Renderer::Command::BeginDebugEvent*>(data);
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).beginDebugEvent(realData->name);
			}

			void EndDebugEvent(const void*, Renderer::IRenderer& renderer)
			{
				static_cast<OpenGLRenderer::OpenGLRenderer&>(renderer).endDebugEvent();
			}


		}


		//[-------------------------------------------------------]
		//[ Global definitions                                    ]
		//[-------------------------------------------------------]
		static const Renderer::BackendDispatchFunction DISPATCH_FUNCTIONS[Renderer::CommandDispatchFunctionIndex::NumberOfFunctions] =
		{
			// Resource handling
			&BackendDispatch::CopyUniformBufferData,
			&BackendDispatch::CopyTextureBufferData,
			// Graphics root
			&BackendDispatch::SetGraphicsRootSignature,
			&BackendDispatch::SetGraphicsRootDescriptorTable,
			// States
			&BackendDispatch::SetPipelineState,
			// Input-assembler (IA) stage
			&BackendDispatch::SetVertexArray,
			&BackendDispatch::SetPrimitiveTopology,
			// Rasterizer (RS) stage
			&BackendDispatch::SetViewports,
			&BackendDispatch::SetScissorRectangles,
			// Output-merger (OM) stage
			&BackendDispatch::SetRenderTarget,
			// Operations
			&BackendDispatch::Clear,
			&BackendDispatch::ResolveMultisampleFramebuffer,
			&BackendDispatch::CopyResource,
			// Draw call
			&BackendDispatch::Draw,
			&BackendDispatch::DrawIndexed,
			// Debug
			&BackendDispatch::SetDebugMarker,
			&BackendDispatch::BeginDebugEvent,
			&BackendDispatch::EndDebugEvent
		};


//[-------------------------------------------------------]
//[ Anonymous detail namespace                            ]
//[-------------------------------------------------------]
	} // detail
}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace OpenGLRenderer
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	#ifdef WIN32
	OpenGLRenderer::OpenGLRenderer(handle nativeWindowHandle, bool) :
	#elif defined LINUX
	OpenGLRenderer::OpenGLRenderer(handle nativeWindowHandle, bool useExternalContext) :
	#else
		#error "Unsupported platform"
	#endif
		mOpenGLRuntimeLinking(new OpenGLRuntimeLinking()),
		mContext(nullptr),
		mExtensions(nullptr),
		mShaderLanguage(nullptr),
		mGraphicsRootSignature(nullptr),
		mDefaultSamplerState(nullptr),
		mOpenGLCopyResourceFramebuffer(0),
		mPipelineState(nullptr),
		mVertexArray(nullptr),
		mOpenGLPrimitiveTopology(0xFFFF),	// Unknown default setting
		mMainSwapChain(nullptr),
		mRenderTarget(nullptr)
	{
		// Is OpenGL available?
		if (mOpenGLRuntimeLinking->isOpenGLAvaiable())
		{
			#ifdef WIN32
				// TODO(co) Add external context support
				mContext = new ContextWindows(mOpenGLRuntimeLinking, nativeWindowHandle);
			#elif defined LINUX
				mContext = new ContextLinux(mOpenGLRuntimeLinking, nativeWindowHandle, useExternalContext);
			#else
				#error "Unsupported platform"
			#endif

			// We're using "this" in here, so we are not allowed to write the following within the initializer list
			mExtensions = new Extensions(*mContext);

			// Is the context initialized?
			if (mContext->isInitialized())
			{
				// Initialize the OpenGL extensions
				mExtensions->initialize();

				#ifdef RENDERER_OUTPUT_DEBUG
					// "GL_ARB_debug_output"-extension available?
					if (mExtensions->isGL_ARB_debug_output())
					{
						// Synchronous debug output, please
						// -> Makes it easier to find the place causing the issue
						glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

						// We don't need to configure the debug output by using "glDebugMessageControlARB()",
						// by default all messages are enabled and this is good this way

						// Disable severity​ notifications, most drivers print many things with this severity
						glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DEBUG_SEVERITY_NOTIFICATION, 0, 0, false);

						// Set the debug message callback function
						glDebugMessageCallbackARB(&OpenGLRenderer::debugMessageCallback, nullptr);
					}
				#endif

				// Create the default sampler state
				mDefaultSamplerState = createSamplerState(Renderer::ISamplerState::getDefaultSamplerState());

				// Initialize the capabilities
				initializeCapabilities();

				// Add references to the default sampler state and set it
				if (nullptr != mDefaultSamplerState)
				{
					mDefaultSamplerState->addReference();
					// TODO(co) Set default sampler states
				}

				// Create a main swap chain instance?
				if (NULL_HANDLE != nativeWindowHandle)
				{
					// Create a main swap chain instance
					mMainSwapChain = new SwapChain(*this, nativeWindowHandle, *mContext);
					RENDERER_SET_RESOURCE_DEBUG_NAME(mMainSwapChain, "Main swap chain")
					mMainSwapChain->addReference();	// Internal renderer reference
				}
			}
		}
	}

	OpenGLRenderer::~OpenGLRenderer()
	{
		// Set no pipeline state reference, in case we have one
		if (nullptr != mPipelineState)
		{
			setPipelineState(nullptr);
		}

		// Set no vertex array reference, in case we have one
		if (nullptr != mVertexArray)
		{
			iaSetVertexArray(nullptr);
		}

		// Release instances
		if (nullptr != mMainSwapChain)
		{
			mMainSwapChain->releaseReference();
			mMainSwapChain = nullptr;
		}
		if (nullptr != mRenderTarget)
		{
			mRenderTarget->releaseReference();
			mRenderTarget = nullptr;
		}
		if (nullptr != mDefaultSamplerState)
		{
			mDefaultSamplerState->releaseReference();
			mDefaultSamplerState = nullptr;
		}

		// Destroy the OpenGL framebuffer used by "OpenGLRenderer::OpenGLRenderer::copyResource()" if the "GL_ARB_copy_image"-extension isn't available
		// -> Silently ignores 0's and names that do not correspond to existing buffer objects
		glDeleteFramebuffers(1, &mOpenGLCopyResourceFramebuffer);

		// Release the graphics root signature instance, in case we have one
		if (nullptr != mGraphicsRootSignature)
		{
			mGraphicsRootSignature->releaseReference();
		}

		#ifndef RENDERER_NO_STATISTICS
		{ // For debugging: At this point there should be no resource instances left, validate this!
			// -> Are the currently any resource instances?
			const unsigned long numberOfCurrentResources = getStatistics().getNumberOfCurrentResources();
			if (numberOfCurrentResources > 0)
			{
				// Error!
				if (numberOfCurrentResources > 1)
				{
					RENDERER_OUTPUT_DEBUG_PRINTF("OpenGL error: Renderer is going to be destroyed, but there are still %lu resource instances left (memory leak)\n", numberOfCurrentResources)
				}
				else
				{
					RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Renderer is going to be destroyed, but there is still one resource instance left (memory leak)\n")
				}

				// Use debug output to show the current number of resource instances
				getStatistics().debugOutputCurrentResouces();
			}
		}
		#endif

		// Release the shader language instance, in case we have one
		if (nullptr != mShaderLanguage)
		{
			mShaderLanguage->releaseReference();
		}

		// Destroy the extensions instance
		delete mExtensions;

		// Destroy the OpenGL context instance
		delete mContext;

		// Destroy the OpenGL runtime linking instance
		delete mOpenGLRuntimeLinking;
	}


	//[-------------------------------------------------------]
	//[ States                                                ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::setGraphicsRootSignature(Renderer::IRootSignature* rootSignature)
	{
		if (nullptr != mGraphicsRootSignature)
		{
			mGraphicsRootSignature->releaseReference();
		}
		mGraphicsRootSignature = static_cast<RootSignature*>(rootSignature);
		if (nullptr != mGraphicsRootSignature)
		{
			mGraphicsRootSignature->addReference();

			// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
			OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *rootSignature)
		}
	}

	void OpenGLRenderer::setGraphicsRootDescriptorTable(uint32_t rootParameterIndex, Renderer::IResource* resource)
	{
		// Security checks
		#ifndef OPENGLRENDERER_NO_DEBUG
		{
			if (nullptr == mGraphicsRootSignature)
			{
				RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: No graphics root signature set")
				return;
			}
			const Renderer::RootSignature& rootSignature = mGraphicsRootSignature->getRootSignature();
			if (rootParameterIndex >= rootSignature.numberOfParameters)
			{
				RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Root parameter index is out of bounds")
				return;
			}
			const Renderer::RootParameter& rootParameter = rootSignature.parameters[rootParameterIndex];
			if (Renderer::RootParameterType::DESCRIPTOR_TABLE != rootParameter.parameterType)
			{
				RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Root parameter index doesn't reference a descriptor table")
				return;
			}

			// TODO(co) For now, we only support a single descriptor range
			if (1 != rootParameter.descriptorTable.numberOfDescriptorRanges)
			{
				RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Only a single descriptor range is supported")
				return;
			}
			if (nullptr == reinterpret_cast<const Renderer::DescriptorRange*>(rootParameter.descriptorTable.descriptorRanges))
			{
				RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Descriptor ranges is a null pointer")
				return;
			}
		}
		#endif

		if (nullptr != resource)
		{
			// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
			OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *resource)

			// Get the root signature parameter instance
			const Renderer::RootParameter& rootParameter = mGraphicsRootSignature->getRootSignature().parameters[rootParameterIndex];
			const Renderer::DescriptorRange* descriptorRange = reinterpret_cast<const Renderer::DescriptorRange*>(rootParameter.descriptorTable.descriptorRanges);

			// Check the type of resource to set
			// TODO(co) Some additional resource type root signature security checks in debug build?
			// TODO(co) There's room for binding API call related optimization in here (will certainly be no huge overall efficiency gain)
			const Renderer::ResourceType resourceType = resource->getResourceType();
			switch (resourceType)
			{
				case Renderer::ResourceType::UNIFORM_BUFFER:
					// "GL_ARB_uniform_buffer_object" required
					if (mExtensions->isGL_ARB_uniform_buffer_object())
					{
						// Attach the buffer to the given UBO binding point
						// -> Explicit binding points ("layout(binding = 0)" in GLSL shader) requires OpenGL 4.2 or the "GL_ARB_explicit_uniform_location"-extension
						// -> Direct3D 10 and Direct3D 11 have explicit binding points
						glBindBufferBase(GL_UNIFORM_BUFFER, rootParameterIndex, static_cast<UniformBuffer*>(resource)->getOpenGLUniformBuffer());
					}
					break;

				case Renderer::ResourceType::TEXTURE_BUFFER:
				case Renderer::ResourceType::TEXTURE_2D:
				case Renderer::ResourceType::TEXTURE_2D_ARRAY:
				{
					// In OpenGL, all shaders share the same texture units (= "Renderer::RootParameter::shaderVisibility" stays unused)

					// Is "GL_ARB_direct_state_access" or "GL_EXT_direct_state_access" there?
					if (mExtensions->isGL_ARB_direct_state_access() || mExtensions->isGL_EXT_direct_state_access())
					{
						// Effective direct state access (DSA)
						const bool isArbDsa = mExtensions->isGL_ARB_direct_state_access();

						// "glBindTextureUnit()" unit parameter is zero based so we can simply use the value we received
						const GLuint unit = descriptorRange->baseShaderRegister;

						// TODO(co) Some security checks might be wise *maximum number of texture units*
						// Evaluate the texture type
						switch (resourceType)
						{
							case Renderer::ResourceType::TEXTURE_BUFFER:
								if (isArbDsa)
								{
									glBindTextureUnit(unit, static_cast<TextureBuffer*>(resource)->getOpenGLTexture());
								}
								else
								{
									// "GL_TEXTURE0_ARB" is the first texture unit, while the unit we received is zero based
									glBindMultiTextureEXT(GL_TEXTURE0_ARB + unit, GL_TEXTURE_BUFFER_ARB, static_cast<TextureBuffer*>(resource)->getOpenGLTexture());
								}
								break;

							case Renderer::ResourceType::TEXTURE_2D:
								if (isArbDsa)
								{
									glBindTextureUnit(unit, static_cast<Texture2D*>(resource)->getOpenGLTexture());
								}
								else
								{
									// "GL_TEXTURE0_ARB" is the first texture unit, while the unit we received is zero based
									const Texture2D* texture2D = static_cast<Texture2D*>(resource);
									glBindMultiTextureEXT(GL_TEXTURE0_ARB + unit, static_cast<GLenum>((texture2D->getNumberOfMultisamples() > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), texture2D->getOpenGLTexture());
								}
								break;

							case Renderer::ResourceType::TEXTURE_2D_ARRAY:
								// No texture 2D array extension check required, if we in here we already know it must exist
								if (isArbDsa)
								{
									glBindTextureUnit(unit, static_cast<Texture2DArray*>(resource)->getOpenGLTexture());
								}
								else
								{
									// "GL_TEXTURE0_ARB" is the first texture unit, while the unit we received is zero based
									glBindMultiTextureEXT(GL_TEXTURE0_ARB + unit, GL_TEXTURE_2D_ARRAY_EXT, static_cast<Texture2DArray*>(resource)->getOpenGLTexture());
								}
								break;

							case Renderer::ResourceType::ROOT_SIGNATURE:
							case Renderer::ResourceType::PROGRAM:
							case Renderer::ResourceType::VERTEX_ARRAY:
							case Renderer::ResourceType::SWAP_CHAIN:
							case Renderer::ResourceType::FRAMEBUFFER:
							case Renderer::ResourceType::INDEX_BUFFER:
							case Renderer::ResourceType::VERTEX_BUFFER:
							case Renderer::ResourceType::UNIFORM_BUFFER:
							case Renderer::ResourceType::INDIRECT_BUFFER:
							case Renderer::ResourceType::PIPELINE_STATE:
							case Renderer::ResourceType::SAMPLER_STATE:
							case Renderer::ResourceType::VERTEX_SHADER:
							case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
							case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
							case Renderer::ResourceType::GEOMETRY_SHADER:
							case Renderer::ResourceType::FRAGMENT_SHADER:
								RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Invalid resource type")
								break;
						}

						// Set the OpenGL sampler states, if required (texture buffer has no sampler state)
						if (Renderer::ResourceType::TEXTURE_BUFFER != resourceType)
						{
							const SamplerState* samplerState = mGraphicsRootSignature->getSamplerState(descriptorRange->samplerRootParameterIndex);

							// Is "GL_ARB_sampler_objects" there?
							if (mExtensions->isGL_ARB_sampler_objects())
							{
								// Effective sampler object (SO)
								glBindSampler(descriptorRange->baseShaderRegister, static_cast<const SamplerStateSo*>(samplerState)->getOpenGLSampler());
							}
							else
							{
								#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
									// Backup the currently active OpenGL texture
									GLint openGLActiveTextureBackup = 0;
									glGetIntegerv(GL_ACTIVE_TEXTURE, &openGLActiveTextureBackup);
								#endif

								// TODO(co) Some security checks might be wise *maximum number of texture units*
								// Activate the texture unit we want to manipulate
								// -> "GL_TEXTURE0_ARB" is the first texture unit, while the unit we received is zero based
								glActiveTextureARB(GL_TEXTURE0_ARB + unit);

								// Is "GL_EXT_direct_state_access" there?
								if (mExtensions->isGL_EXT_direct_state_access())
								{
									// Direct state access (DSA) version to emulate a sampler object
									static_cast<const SamplerStateDsa*>(samplerState)->setOpenGLSamplerStates();
								}
								else
								{
									// Traditional bind version to emulate a sampler object
									static_cast<const SamplerStateBind*>(samplerState)->setOpenGLSamplerStates();
								}

								#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
									// Be polite and restore the previous active OpenGL texture
									glActiveTextureARB(static_cast<GLenum>(openGLActiveTextureBackup));
								#endif
							}
						}
					}
					else
					{
						// Traditional bind version

						// "GL_ARB_multitexture" required
						if (mExtensions->isGL_ARB_multitexture())
						{
							#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
								// Backup the currently active OpenGL texture
								GLint openGLActiveTextureBackup = 0;
								glGetIntegerv(GL_ACTIVE_TEXTURE, &openGLActiveTextureBackup);
							#endif

							// TODO(co) Some security checks might be wise *maximum number of texture units*
							// Activate the texture unit we want to manipulate
							// -> "GL_TEXTURE0_ARB" is the first texture unit, while the unit we received is zero based
							glActiveTextureARB(GL_TEXTURE0_ARB + descriptorRange->baseShaderRegister);

							// Evaluate the resource type
							switch (resourceType)
							{
								case Renderer::ResourceType::TEXTURE_BUFFER:
									glBindTexture(GL_TEXTURE_BUFFER_ARB, static_cast<TextureBuffer*>(resource)->getOpenGLTexture());
									break;

								case Renderer::ResourceType::TEXTURE_2D:
								{
									const Texture2D* texture2D = static_cast<Texture2D*>(resource);
									glBindTexture(static_cast<GLenum>((texture2D->getNumberOfMultisamples() > 1) ? GL_TEXTURE_2D_MULTISAMPLE : GL_TEXTURE_2D), static_cast<Texture2D*>(resource)->getOpenGLTexture());
									break;
								}

								case Renderer::ResourceType::TEXTURE_2D_ARRAY:
									// No extension check required, if we in here we already know it must exist
									glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, static_cast<Texture2DArray*>(resource)->getOpenGLTexture());
									break;

								case Renderer::ResourceType::ROOT_SIGNATURE:
								case Renderer::ResourceType::PROGRAM:
								case Renderer::ResourceType::VERTEX_ARRAY:
								case Renderer::ResourceType::SWAP_CHAIN:
								case Renderer::ResourceType::FRAMEBUFFER:
								case Renderer::ResourceType::INDEX_BUFFER:
								case Renderer::ResourceType::VERTEX_BUFFER:
								case Renderer::ResourceType::UNIFORM_BUFFER:
								case Renderer::ResourceType::INDIRECT_BUFFER:
								case Renderer::ResourceType::PIPELINE_STATE:
								case Renderer::ResourceType::SAMPLER_STATE:
								case Renderer::ResourceType::VERTEX_SHADER:
								case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
								case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
								case Renderer::ResourceType::GEOMETRY_SHADER:
								case Renderer::ResourceType::FRAGMENT_SHADER:
									RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Invalid resource type")
									break;
							}

							// Set the OpenGL sampler states, if required (texture buffer has no sampler state)
							if (Renderer::ResourceType::TEXTURE_BUFFER != resourceType)
							{
								const SamplerState* samplerState = mGraphicsRootSignature->getSamplerState(descriptorRange->samplerRootParameterIndex);

								// Is "GL_ARB_sampler_objects" there?
								if (mExtensions->isGL_ARB_sampler_objects())
								{
									// Effective sampler object (SO)
									glBindSampler(descriptorRange->baseShaderRegister, static_cast<const SamplerStateSo*>(samplerState)->getOpenGLSampler());
								}
								// Is "GL_EXT_direct_state_access" there?
								else if (mExtensions->isGL_EXT_direct_state_access() || mExtensions->isGL_ARB_direct_state_access())
								{
									// Direct state access (DSA) version to emulate a sampler object
									static_cast<const SamplerStateDsa*>(samplerState)->setOpenGLSamplerStates();
								}
								else
								{
									// Traditional bind version to emulate a sampler object
									static_cast<const SamplerStateBind*>(samplerState)->setOpenGLSamplerStates();
								}
							}

							#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
								// Be polite and restore the previous active OpenGL texture
								glActiveTextureARB(static_cast<GLenum>(openGLActiveTextureBackup));
							#endif
						}
					}
					break;
				}

				case Renderer::ResourceType::SAMPLER_STATE:
				{
					// Unlike Direct3D >=10, OpenGL directly attaches the sampler settings to the texture (unless the sampler object extension is used)
					mGraphicsRootSignature->setSamplerState(rootParameterIndex, static_cast<SamplerState*>(resource));
					break;
				}

				case Renderer::ResourceType::ROOT_SIGNATURE:
				case Renderer::ResourceType::PROGRAM:
				case Renderer::ResourceType::VERTEX_ARRAY:
				case Renderer::ResourceType::SWAP_CHAIN:
				case Renderer::ResourceType::FRAMEBUFFER:
				case Renderer::ResourceType::INDEX_BUFFER:
				case Renderer::ResourceType::VERTEX_BUFFER:
				case Renderer::ResourceType::INDIRECT_BUFFER:
				case Renderer::ResourceType::PIPELINE_STATE:
				case Renderer::ResourceType::VERTEX_SHADER:
				case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
				case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
				case Renderer::ResourceType::GEOMETRY_SHADER:
				case Renderer::ResourceType::FRAGMENT_SHADER:
					RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Invalid resource type")
					break;
			}
		}
		else
		{
			// TODO(co) Handle this situation?
			/*
			#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
				// Backup the currently active OpenGL texture
				GLint openGLActiveTextureBackup = 0;
				glGetIntegerv(GL_ACTIVE_TEXTURE, &openGLActiveTextureBackup);
			#endif

			// TODO(co) Some security checks might be wise *maximum number of texture units*
			glActiveTexture(GL_TEXTURE0 + unit);

			// Unbind the texture at the given texture unit
			glBindTexture(GL_TEXTURE_2D, 0);

			#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
				// Be polite and restore the previous active OpenGL texture
				glActiveTexture(openGLActiveTextureBackup);
			#endif
			*/
		}
	}

	void OpenGLRenderer::setPipelineState(Renderer::IPipelineState* pipelineState)
	{
		if (mPipelineState != pipelineState)
		{
			if (nullptr != pipelineState)
			{
				// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
				OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *pipelineState)

				// Set new pipeline state and add a reference to it
				if (nullptr != mPipelineState)
				{
					mPipelineState->releaseReference();
				}
				mPipelineState = static_cast<PipelineState*>(pipelineState);
				mPipelineState->addReference();

				// Set pipeline state
				mPipelineState->bindPipelineState();
			}
			else if (nullptr != mPipelineState)
			{
				// TODO(co) Handle this situation by resetting OpenGL states?
				mPipelineState->releaseReference();
				mPipelineState = nullptr;
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Input-assembler (IA) stage                            ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::iaSetVertexArray(Renderer::IVertexArray *vertexArray)
	{
		// New vertex array?
		if (mVertexArray != vertexArray)
		{
			// Set a vertex array?
			if (nullptr != vertexArray)
			{
				// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
				OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *vertexArray)

				// Unset the currently used vertex array
				iaUnsetVertexArray();

				// Set new vertex array and add a reference to it
				mVertexArray = static_cast<VertexArray*>(vertexArray);
				mVertexArray->addReference();

				// Evaluate the internal array type of the new vertex array to set
				switch (static_cast<VertexArray*>(mVertexArray)->getInternalResourceType())
				{
					case VertexArray::InternalResourceType::NO_VAO:
						// Enable OpenGL vertex attribute arrays
						static_cast<VertexArrayNoVao*>(mVertexArray)->enableOpenGLVertexAttribArrays();
						break;

					case VertexArray::InternalResourceType::VAO:
						// Bind OpenGL vertex array
						glBindVertexArray(static_cast<VertexArrayVao*>(mVertexArray)->getOpenGLVertexArray());
						break;
				}
			}
			else
			{
				// Unset the currently used vertex array
				iaUnsetVertexArray();
			}
		}
	}

	void OpenGLRenderer::iaSetPrimitiveTopology(Renderer::PrimitiveTopology primitiveTopology)
	{
		// Tessellation support: Up to 32 vertices per patch are supported "Renderer::PrimitiveTopology::PATCH_LIST_1" ... "Renderer::PrimitiveTopology::PATCH_LIST_32"
		if (primitiveTopology >= Renderer::PrimitiveTopology::PATCH_LIST_1)
		{
			// Use tessellation

			// Get number of vertices that will be used to make up a single patch primitive
			// -> There's no need to check for the "GL_ARB_tessellation_shader" extension, it's there if "Renderer::Capabilities::maximumNumberOfPatchVertices" is not 0
			const GLint numberOfVerticesPerPatch = static_cast<int>(primitiveTopology) - static_cast<int>(Renderer::PrimitiveTopology::PATCH_LIST_1) + 1;
			if (numberOfVerticesPerPatch <= static_cast<GLint>(mCapabilities.maximumNumberOfPatchVertices))
			{
				// Set number of vertices that will be used to make up a single patch primitive
				glPatchParameteri(GL_PATCH_VERTICES, numberOfVerticesPerPatch);

				// Set OpenGL primitive topology
				mOpenGLPrimitiveTopology = GL_PATCHES;
			}
			else
			{
				// Error!
			}
		}
		else
		{
			// Do not use tessellation

			// Map and backup the set OpenGL primitive topology
			mOpenGLPrimitiveTopology = Mapping::getOpenGLType(primitiveTopology);
		}
	}


	//[-------------------------------------------------------]
	//[ Rasterizer (RS) stage                                 ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::rsSetViewports(uint32_t numberOfViewports, const Renderer::Viewport *viewports)
	{
		// Are the given viewports valid?
		if (numberOfViewports > 0 && nullptr != viewports)
		{
			// In OpenGL, the origin of the viewport is left bottom while Direct3D is using a left top origin. To make the
			// Direct3D 11 implementation as efficient as possible the Direct3D convention is used and we have to convert in here.

			// Get the width and height of the current render target
			uint32_t renderTargetWidth =  1;
			uint32_t renderTargetHeight = 1;
			if (nullptr != mRenderTarget)
			{
				mRenderTarget->getWidthAndHeight(renderTargetWidth, renderTargetHeight);
			}

			// Set the OpenGL viewport
			// TODO(co) "GL_ARB_viewport_array" support ("OpenGLRenderer::rsSetViewports()")
			// TODO(co) Check for "numberOfViewports" out of range or are the debug events good enough?
			#ifndef RENDERER_NO_DEBUG
				if (numberOfViewports > 1)
				{
					RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: OpenGL supports only one viewport")
				}
			#endif
			glViewport(static_cast<GLint>(viewports->topLeftX), static_cast<GLint>(renderTargetHeight - viewports->topLeftY - viewports->height), static_cast<GLsizei>(viewports->width), static_cast<GLsizei>(viewports->height));
			glDepthRange(static_cast<GLclampf>(viewports->minDepth), static_cast<GLclampf>(viewports->maxDepth));
		}
	}

	void OpenGLRenderer::rsSetScissorRectangles(uint32_t numberOfScissorRectangles, const Renderer::ScissorRectangle *scissorRectangles)
	{
		// Are the given scissor rectangles valid?
		if (numberOfScissorRectangles > 0 && nullptr != scissorRectangles)
		{
			// In OpenGL, the origin of the scissor rectangle is left bottom while Direct3D is using a left top origin. To make the
			// Direct3D 9 & 10 & 11 implementation as efficient as possible the Direct3D convention is used and we have to convert in here.

			// Get the width and height of the current render target
			uint32_t renderTargetWidth =  1;
			uint32_t renderTargetHeight = 1;
			if (nullptr != mRenderTarget)
			{
				mRenderTarget->getWidthAndHeight(renderTargetWidth, renderTargetHeight);
			}

			// Set the OpenGL scissor rectangle
			// TODO(co) "GL_ARB_viewport_array" support ("OpenGLRenderer::rsSetViewports()")
			// TODO(co) Check for "numberOfViewports" out of range or are the debug events good enough?
			#ifndef RENDERER_NO_DEBUG
				if (numberOfScissorRectangles > 1)
				{
					RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: OpenGL supports only one scissor rectangle")
				}
			#endif
			const GLsizei width  = scissorRectangles->bottomRightX - scissorRectangles->topLeftX;
			const GLsizei height = scissorRectangles->bottomRightY - scissorRectangles->topLeftY;
			glScissor(static_cast<GLint>(scissorRectangles->topLeftX), static_cast<GLint>(renderTargetHeight - scissorRectangles->topLeftY - height), width, height);
		}
	}


	//[-------------------------------------------------------]
	//[ Output-merger (OM) stage                              ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::omSetRenderTarget(Renderer::IRenderTarget *renderTarget)
	{
		// New render target?
		if (mRenderTarget != renderTarget)
		{
			// Set a render target?
			if (nullptr != renderTarget)
			{
				// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
				OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *renderTarget)

				// Release the render target reference, in case we have one
				Framebuffer* framebufferToGenerateMipmapsFor = nullptr;
				if (nullptr != mRenderTarget)
				{
					// Unbind OpenGL framebuffer?
					if (Renderer::ResourceType::FRAMEBUFFER == mRenderTarget->getResourceType() && Renderer::ResourceType::FRAMEBUFFER != renderTarget->getResourceType())
					{
						// Do we need to disable multisample?
						if (static_cast<Framebuffer*>(mRenderTarget)->isMultisampleRenderTarget())
						{
							glDisable(GL_MULTISAMPLE);
						}

						// We do not render into a OpenGL framebuffer
						glBindFramebuffer(GL_FRAMEBUFFER, 0);
					}

					// Generate mipmaps?
					if (Renderer::ResourceType::FRAMEBUFFER == mRenderTarget->getResourceType() && static_cast<Framebuffer*>(mRenderTarget)->getGenerateMipmaps())
					{
						framebufferToGenerateMipmapsFor = static_cast<Framebuffer*>(mRenderTarget);
					}
					else
					{
						// Release
						mRenderTarget->releaseReference();
					}
				}

				// Set new render target and add a reference to it
				mRenderTarget = renderTarget;
				mRenderTarget->addReference();

				// Evaluate the render target type
				switch (mRenderTarget->getResourceType())
				{
					case Renderer::ResourceType::SWAP_CHAIN:
					{
						static_cast<SwapChain*>(mRenderTarget)->getContext().makeCurrent();
						break;
					}

					case Renderer::ResourceType::FRAMEBUFFER:
					{
						// Get the OpenGL framebuffer instance
						Framebuffer *framebuffer = static_cast<Framebuffer*>(mRenderTarget);

						// Bind the OpenGL framebuffer
						glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->getOpenGLFramebuffer());

						// Define the OpenGL buffers to draw into, "GL_ARB_draw_buffers"-extension required
						if (mExtensions->isGL_ARB_draw_buffers())
						{
							// https://www.opengl.org/registry/specs/ARB/draw_buffers.txt - "The draw buffer for output colors beyond <n> is set to NONE."
							// -> Meaning depth only rendering which has no color textures at all will work as well, no need for "glDrawBuffer(GL_NONE)"
							static const GLenum OPENGL_DRAW_BUFFER[16] =
							{
								GL_COLOR_ATTACHMENT0,  GL_COLOR_ATTACHMENT1,  GL_COLOR_ATTACHMENT2,  GL_COLOR_ATTACHMENT3,
								GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5,  GL_COLOR_ATTACHMENT6,  GL_COLOR_ATTACHMENT7,
								GL_COLOR_ATTACHMENT8,  GL_COLOR_ATTACHMENT9,  GL_COLOR_ATTACHMENT10, GL_COLOR_ATTACHMENT11,
								GL_COLOR_ATTACHMENT12, GL_COLOR_ATTACHMENT13, GL_COLOR_ATTACHMENT14, GL_COLOR_ATTACHMENT15
							};
							glDrawBuffersARB(static_cast<GLsizei>(framebuffer->getNumberOfColorTextures()), OPENGL_DRAW_BUFFER);
						}

						// Do we need to enable multisample?
						if (framebuffer->isMultisampleRenderTarget())
						{
							glEnable(GL_MULTISAMPLE);
						}
						break;
					}

					case Renderer::ResourceType::ROOT_SIGNATURE:
					case Renderer::ResourceType::PROGRAM:
					case Renderer::ResourceType::VERTEX_ARRAY:
					case Renderer::ResourceType::INDEX_BUFFER:
					case Renderer::ResourceType::VERTEX_BUFFER:
					case Renderer::ResourceType::UNIFORM_BUFFER:
					case Renderer::ResourceType::TEXTURE_BUFFER:
					case Renderer::ResourceType::INDIRECT_BUFFER:
					case Renderer::ResourceType::TEXTURE_2D:
					case Renderer::ResourceType::TEXTURE_2D_ARRAY:
					case Renderer::ResourceType::PIPELINE_STATE:
					case Renderer::ResourceType::SAMPLER_STATE:
					case Renderer::ResourceType::VERTEX_SHADER:
					case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
					case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
					case Renderer::ResourceType::GEOMETRY_SHADER:
					case Renderer::ResourceType::FRAGMENT_SHADER:
					default:
						// Not handled in here
						break;
				}

				// Generate mipmaps
				if (nullptr != framebufferToGenerateMipmapsFor)
				{
					framebufferToGenerateMipmapsFor->generateMipmaps();
					framebufferToGenerateMipmapsFor->releaseReference();
				}
			}
			else
			{
				// Evaluate the render target type
				if (Renderer::ResourceType::FRAMEBUFFER == mRenderTarget->getResourceType())
				{
					// We do not render into a OpenGL framebuffer
					glBindFramebuffer(GL_FRAMEBUFFER, 0);
				}

				// TODO(co) Set no active render target

				// Release the render target reference, in case we have one
				if (nullptr != mRenderTarget)
				{
					mRenderTarget->releaseReference();
					mRenderTarget = nullptr;
				}
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Operations                                            ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::clear(uint32_t flags, const float color[4], float z, uint32_t stencil)
	{
		// Get API flags
		uint32_t flagsApi = 0;
		if (flags & Renderer::ClearFlag::COLOR)
		{
			flagsApi |= GL_COLOR_BUFFER_BIT;
		}
		if (flags & Renderer::ClearFlag::DEPTH)
		{
			flagsApi |= GL_DEPTH_BUFFER_BIT;
		}
		if (flags & Renderer::ClearFlag::STENCIL)
		{
			flagsApi |= GL_STENCIL_BUFFER_BIT;
		}

		// Are API flags set?
		if (0 != flagsApi)
		{
			// Set clear settings
			if (flags & Renderer::ClearFlag::COLOR)
			{
				glClearColor(color[0], color[1], color[2], color[3]);
			}
			if (flags & Renderer::ClearFlag::DEPTH)
			{
				glClearDepth(z);
				if (nullptr != mPipelineState && Renderer::DepthWriteMask::ALL != mPipelineState->getDepthStencilState().depthWriteMask)
				{
					glDepthMask(GL_TRUE);
				}
			}
			if (flags & Renderer::ClearFlag::STENCIL)
			{
				glClearStencil(static_cast<GLint>(stencil));
			}

			// Unlike OpenGL, when using Direct3D 10 & 11 the scissor rectangle(s) do not affect the clear operation
			// -> We have to compensate the OpenGL behaviour in here

			// Disable OpenGL scissor test, in case it's not disabled, yet
			if (nullptr != mPipelineState && mPipelineState->getRasterizerState().scissorEnable)
			{
				glDisable(GL_SCISSOR_TEST);
			}

			// Clear
			glClear(flagsApi);

			// Restore the previously set OpenGL states
			if (nullptr != mPipelineState && mPipelineState->getRasterizerState().scissorEnable)
			{
				glEnable(GL_SCISSOR_TEST);
			}
			if ((flags & Renderer::ClearFlag::DEPTH) && nullptr != mPipelineState && Renderer::DepthWriteMask::ALL != mPipelineState->getDepthStencilState().depthWriteMask)
			{
				glDepthMask(GL_FALSE);
			}
		}
	}

	void OpenGLRenderer::resolveMultisampleFramebuffer(Renderer::IRenderTarget& destinationRenderTarget, Renderer::IFramebuffer& sourceMultisampleFramebuffer)
	{
		// Security check: Are the given resources owned by this renderer? (calls "return" in case of a mismatch)
		OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, destinationRenderTarget)
		OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, sourceMultisampleFramebuffer)

		// Evaluate the render target type
		switch (destinationRenderTarget.getResourceType())
		{
			case Renderer::ResourceType::SWAP_CHAIN:
			{
				// Get the OpenGL swap chain instance
				// TODO(co) Implement me, not that important in practice so not directly implemented
				// SwapChain& swapChain = static_cast<SwapChain&>(destinationRenderTarget);
				break;
			}

			case Renderer::ResourceType::FRAMEBUFFER:
			{
				// Get the OpenGL framebuffer instances
				const Framebuffer& openGLDestinationFramebuffer = static_cast<const Framebuffer&>(destinationRenderTarget);
				const Framebuffer& openGLSourceMultisampleFramebuffer = static_cast<const Framebuffer&>(sourceMultisampleFramebuffer);

				// Get the width and height of the destination and source framebuffer
				uint32_t destinationWidth = 1;
				uint32_t destinationHeight = 1;
				openGLDestinationFramebuffer.getWidthAndHeight(destinationWidth, destinationHeight);
				uint32_t sourceWidth = 1;
				uint32_t sourceHeight = 1;
				openGLSourceMultisampleFramebuffer.getWidthAndHeight(sourceWidth, sourceHeight);

				// Resolve multisample
				glBindFramebuffer(GL_READ_FRAMEBUFFER, openGLSourceMultisampleFramebuffer.getOpenGLFramebuffer());
				glBindFramebuffer(GL_DRAW_FRAMEBUFFER, openGLDestinationFramebuffer.getOpenGLFramebuffer());
				glBlitFramebuffer(
					0, 0, static_cast<GLint>(sourceWidth), static_cast<GLint>(sourceHeight),			// Source
					0, 0, static_cast<GLint>(destinationWidth), static_cast<GLint>(destinationHeight),	// Destination
					GL_COLOR_BUFFER_BIT, GL_NEAREST);
				break;
			}

			case Renderer::ResourceType::ROOT_SIGNATURE:
			case Renderer::ResourceType::PROGRAM:
			case Renderer::ResourceType::VERTEX_ARRAY:
			case Renderer::ResourceType::INDEX_BUFFER:
			case Renderer::ResourceType::VERTEX_BUFFER:
			case Renderer::ResourceType::UNIFORM_BUFFER:
			case Renderer::ResourceType::TEXTURE_BUFFER:
			case Renderer::ResourceType::INDIRECT_BUFFER:
			case Renderer::ResourceType::TEXTURE_2D:
			case Renderer::ResourceType::TEXTURE_2D_ARRAY:
			case Renderer::ResourceType::PIPELINE_STATE:
			case Renderer::ResourceType::SAMPLER_STATE:
			case Renderer::ResourceType::VERTEX_SHADER:
			case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
			case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
			case Renderer::ResourceType::GEOMETRY_SHADER:
			case Renderer::ResourceType::FRAGMENT_SHADER:
			default:
				// Not handled in here
				break;
		}
	}

	void OpenGLRenderer::copyResource(Renderer::IResource& destinationResource, Renderer::IResource& sourceResource)
	{
		// Security check: Are the given resources owned by this renderer? (calls "return" in case of a mismatch)
		OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, destinationResource)
		OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, sourceResource)

		// Evaluate the render target type
		switch (destinationResource.getResourceType())
		{
			case Renderer::ResourceType::TEXTURE_2D:
				if (sourceResource.getResourceType() == Renderer::ResourceType::TEXTURE_2D)
				{
					// Get the OpenGL texture 2D instances
					const Texture2D& openGlDestinationTexture2D = static_cast<const Texture2D&>(destinationResource);
					const Texture2D& openGlSourceTexture2D = static_cast<const Texture2D&>(sourceResource);
					assert(openGlDestinationTexture2D.getWidth() == openGlSourceTexture2D.getWidth());
					assert(openGlDestinationTexture2D.getHeight() == openGlSourceTexture2D.getHeight());

					// Copy resource
					const GLsizei width = static_cast<GLsizei>(openGlDestinationTexture2D.getWidth());
					const GLsizei height = static_cast<GLsizei>(openGlDestinationTexture2D.getHeight());
					if (mExtensions->isGL_ARB_copy_image())
					{
						glCopyImageSubData(openGlSourceTexture2D.getOpenGLTexture(), GL_TEXTURE_2D, 0, 0, 0, 0,
										   openGlDestinationTexture2D.getOpenGLTexture(), GL_TEXTURE_2D, 0, 0, 0, 0,
										   width, height, 1);
					}
					else
					{
						#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
							// Backup the currently bound OpenGL framebuffer
							GLint openGLFramebufferBackup = 0;
							glGetIntegerv(GL_FRAMEBUFFER_BINDING, &openGLFramebufferBackup);
						#endif

						// Copy resource by using a framebuffer
						if (0 == mOpenGLCopyResourceFramebuffer)
						{
							glGenFramebuffers(1, &mOpenGLCopyResourceFramebuffer);
						}
						glBindFramebuffer(GL_FRAMEBUFFER, mOpenGLCopyResourceFramebuffer);
						glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, openGlSourceTexture2D.getOpenGLTexture(), 0);
						glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, openGlDestinationTexture2D.getOpenGLTexture(), 0);
						static const GLenum OPENGL_DRAW_BUFFER[1] =
						{
							GL_COLOR_ATTACHMENT1
						};
						glDrawBuffersARB(1, OPENGL_DRAW_BUFFER);	// We could use "glDrawBuffer(GL_COLOR_ATTACHMENT1);", but then we would also have to get the "glDrawBuffer()" function pointer, avoid OpenGL function overkill
						glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);

						#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
							// Be polite and restore the previous bound OpenGL framebuffer
							glBindFramebuffer(GL_FRAMEBUFFER, static_cast<GLuint>(openGLFramebufferBackup));
						#endif
					}
				}
				else
				{
					// Error!
					assert(false);
				}
				break;

			case Renderer::ResourceType::ROOT_SIGNATURE:
			case Renderer::ResourceType::PROGRAM:
			case Renderer::ResourceType::VERTEX_ARRAY:
			case Renderer::ResourceType::SWAP_CHAIN:
			case Renderer::ResourceType::FRAMEBUFFER:
			case Renderer::ResourceType::INDEX_BUFFER:
			case Renderer::ResourceType::VERTEX_BUFFER:
			case Renderer::ResourceType::UNIFORM_BUFFER:
			case Renderer::ResourceType::TEXTURE_BUFFER:
			case Renderer::ResourceType::INDIRECT_BUFFER:
			case Renderer::ResourceType::TEXTURE_2D_ARRAY:
			case Renderer::ResourceType::PIPELINE_STATE:
			case Renderer::ResourceType::SAMPLER_STATE:
			case Renderer::ResourceType::VERTEX_SHADER:
			case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
			case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
			case Renderer::ResourceType::GEOMETRY_SHADER:
			case Renderer::ResourceType::FRAGMENT_SHADER:
			default:
				// Not handled in here
				break;
		}
	}


	//[-------------------------------------------------------]
	//[ Draw call                                             ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::draw(const Renderer::IIndirectBuffer& indirectBuffer, uint32_t indirectBufferOffset, uint32_t numberOfDraws)
	{
		// Tessellation support: "glPatchParameteri()" is called within "OpenGLRenderer::iaSetPrimitiveTopology()"

		// Before doing anything else: If there's emulation data, use it (for example "Renderer::IndirectBuffer" might have been used to generate the data)
		const uint8_t* emulationData = indirectBuffer.getEmulationData();
		if (nullptr != emulationData)
		{
			drawEmulated(emulationData, indirectBufferOffset, numberOfDraws);
		}
		else
		{
			// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
			OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, indirectBuffer)

			if (mExtensions->isGL_ARB_draw_indirect())
			{
				if (numberOfDraws == 1)
				{
					// TODO(co) Optimization: Don't bind if it's already the currently bound thing
					glBindBufferARB(GL_DRAW_INDIRECT_BUFFER, static_cast<const IndirectBuffer&>(indirectBuffer).getOpenGLIndirectBuffer());
					glDrawArraysIndirect(mOpenGLPrimitiveTopology, reinterpret_cast<void*>(static_cast<uintptr_t>(indirectBufferOffset)));
				}
				else if (numberOfDraws > 1)
				{
					// TODO(co) Optimization: Don't bind if it's already the currently bound thing
					glBindBufferARB(GL_DRAW_INDIRECT_BUFFER, static_cast<const IndirectBuffer&>(indirectBuffer).getOpenGLIndirectBuffer());
					if (mExtensions->isGL_ARB_multi_draw_indirect())
					{
						glMultiDrawArraysIndirect(mOpenGLPrimitiveTopology, reinterpret_cast<void*>(static_cast<uintptr_t>(indirectBufferOffset)), static_cast<GLsizei>(numberOfDraws), 0);	// 0 = tightly packed
					}
					else
					{
						// TODO(co) Emulation (see specification for examples)
					}
				}
			}
			else
			{
				// TODO(co) Emulation (see specification for examples)
			}
		}
	}

	void OpenGLRenderer::drawEmulated(const uint8_t* emulationData, uint32_t indirectBufferOffset, uint32_t numberOfDraws)
	{
		// Is currently a vertex array set? Do also check whether or not the required "GL_ARB_draw_instanced" extension is there.
		if (numberOfDraws > 0 && nullptr != mVertexArray)
		{
			// TODO(co) Currently no buffer overflow check due to lack of interface provided data
			emulationData += indirectBufferOffset;

			// Emit the draw calls
			for (uint32_t i = 0; i < numberOfDraws; ++i)
			{
				const Renderer::DrawInstancedArguments& drawInstancedArguments = *reinterpret_cast<const Renderer::DrawInstancedArguments*>(emulationData);
				assert(0 == drawInstancedArguments.startInstanceLocation);	// TODO(co) Add support for this

				// Draw and advance
				if (drawInstancedArguments.instanceCount > 1 && mExtensions->isGL_ARB_draw_instanced())
				{
					// With instancing
					glDrawArraysInstancedARB(mOpenGLPrimitiveTopology, static_cast<GLint>(drawInstancedArguments.startVertexLocation), static_cast<GLsizei>(drawInstancedArguments.vertexCountPerInstance), static_cast<GLsizei>(drawInstancedArguments.instanceCount));
				}
				else
				{
					// Without instancing
					assert(drawInstancedArguments.instanceCount <= 1);
					glDrawArrays(mOpenGLPrimitiveTopology, static_cast<GLint>(drawInstancedArguments.startVertexLocation), static_cast<GLsizei>(drawInstancedArguments.vertexCountPerInstance));
				}
				emulationData += sizeof(Renderer::DrawInstancedArguments);
			}
		}
	}

	void OpenGLRenderer::drawIndexed(const Renderer::IIndirectBuffer& indirectBuffer, uint32_t indirectBufferOffset, uint32_t numberOfDraws)
	{
		// Tessellation support: "glPatchParameteri()" is called within "OpenGLRenderer::iaSetPrimitiveTopology()"

		// Before doing anything else: If there's emulation data, use it (for example "Renderer::IndirectBuffer" might have been used to generate the data)
		const uint8_t* emulationData = indirectBuffer.getEmulationData();
		if (nullptr != emulationData)
		{
			drawIndexedEmulated(emulationData, indirectBufferOffset, numberOfDraws);
		}
		else
		{
			// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
			OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, indirectBuffer)

			// Is currently an vertex array set?
			if (nullptr != mVertexArray)
			{
				// Get the used index buffer
				IndexBuffer *indexBuffer = mVertexArray->getIndexBuffer();
				if (nullptr != indexBuffer)
				{
					if (mExtensions->isGL_ARB_draw_indirect())
					{
						if (numberOfDraws == 1)
						{
							// TODO(co) Optimization: Don't bind if it's already the currently bound thing
							glBindBufferARB(GL_DRAW_INDIRECT_BUFFER, static_cast<const IndirectBuffer&>(indirectBuffer).getOpenGLIndirectBuffer());
							glDrawElementsIndirect(mOpenGLPrimitiveTopology, indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(indirectBufferOffset)));
						}
						else if (numberOfDraws > 1)
						{
							// TODO(co) Optimization: Don't bind if it's already the currently bound thing
							glBindBufferARB(GL_DRAW_INDIRECT_BUFFER, static_cast<const IndirectBuffer&>(indirectBuffer).getOpenGLIndirectBuffer());
							if (mExtensions->isGL_ARB_multi_draw_indirect())
							{
								glMultiDrawElementsIndirect(mOpenGLPrimitiveTopology, indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(indirectBufferOffset)), static_cast<GLsizei>(numberOfDraws), 0);	// 0 = tightly packed
							}
							else
							{
								// TODO(co) Emulation (see specification for examples)
							}
						}
					}
					else
					{
						// TODO(co) Emulation (see specification for examples)
					}
				}
			}
		}
	}

	void OpenGLRenderer::drawIndexedEmulated(const uint8_t* emulationData, uint32_t indirectBufferOffset, uint32_t numberOfDraws)
	{
		// Is currently an vertex array set?
		if (numberOfDraws > 0 && nullptr != mVertexArray)
		{
			// Get the used index buffer
			IndexBuffer *indexBuffer = mVertexArray->getIndexBuffer();
			if (nullptr != indexBuffer)
			{
				// TODO(co) Currently no buffer overflow check due to lack of interface provided data
				emulationData += indirectBufferOffset;

				// Emit the draw calls
				for (uint32_t i = 0; i < numberOfDraws; ++i)
				{
					const Renderer::DrawIndexedInstancedArguments& drawIndexedInstancedArguments = *reinterpret_cast<const Renderer::DrawIndexedInstancedArguments*>(emulationData);
					assert(0 == drawIndexedInstancedArguments.startInstanceLocation);	// TODO(co) Add support for this

					// Draw and advance
					if (drawIndexedInstancedArguments.instanceCount > 1 && mExtensions->isGL_ARB_draw_instanced())
					{
						// With instancing

						// Use base vertex location?
						if (drawIndexedInstancedArguments.baseVertexLocation > 0)
						{
							// Is the "GL_ARB_draw_elements_base_vertex" extension there?
							if (mExtensions->isGL_ARB_draw_elements_base_vertex())
							{
								// Draw with base vertex location
								glDrawElementsInstancedBaseVertex(mOpenGLPrimitiveTopology, static_cast<GLsizei>(drawIndexedInstancedArguments.indexCountPerInstance), indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(drawIndexedInstancedArguments.startIndexLocation * indexBuffer->getIndexSizeInBytes())), static_cast<GLsizei>(drawIndexedInstancedArguments.instanceCount), static_cast<GLint>(drawIndexedInstancedArguments.baseVertexLocation));
							}
							else
							{
								// Error!
								assert(false);
							}
						}
						else
						{
							// Draw without base vertex location
							glDrawElementsInstancedARB(mOpenGLPrimitiveTopology, static_cast<GLsizei>(drawIndexedInstancedArguments.indexCountPerInstance), indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(drawIndexedInstancedArguments.startIndexLocation * indexBuffer->getIndexSizeInBytes())), static_cast<GLsizei>(drawIndexedInstancedArguments.instanceCount));
						}
					}
					else
					{
						// Without instancing
						assert(drawIndexedInstancedArguments.instanceCount <= 1);

						// Use base vertex location?
						if (drawIndexedInstancedArguments.baseVertexLocation > 0)
						{
							// Is the "GL_ARB_draw_elements_base_vertex" extension there?
							if (mExtensions->isGL_ARB_draw_elements_base_vertex())
							{
								// Draw with base vertex location
								glDrawElementsBaseVertex(mOpenGLPrimitiveTopology, static_cast<GLsizei>(drawIndexedInstancedArguments.indexCountPerInstance), indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(drawIndexedInstancedArguments.startIndexLocation * indexBuffer->getIndexSizeInBytes())), static_cast<GLint>(drawIndexedInstancedArguments.baseVertexLocation));
							}
							else
							{
								// Error!
							}
						}
						else
						{
							// Draw without base vertex location
							glDrawElements(mOpenGLPrimitiveTopology, static_cast<GLsizei>(drawIndexedInstancedArguments.indexCountPerInstance), indexBuffer->getOpenGLType(), reinterpret_cast<void*>(static_cast<uintptr_t>(drawIndexedInstancedArguments.startIndexLocation * indexBuffer->getIndexSizeInBytes())));
						}
					}
					emulationData += sizeof(Renderer::DrawIndexedInstancedArguments);
				}
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Debug                                                 ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::setDebugMarker(const char *)
	{
		// TODO(co) Implement me. See
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_marker.txt
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_label.txt
		// - https://www.opengl.org/registry/specs/KHR/debug.txt
	}

	void OpenGLRenderer::beginDebugEvent(const char *)
	{
		// TODO(co) Implement me. See
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_marker.txt
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_label.txt
		// - https://www.opengl.org/registry/specs/KHR/debug.txt
	}

	void OpenGLRenderer::endDebugEvent()
	{
		// TODO(co) Implement me. See
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_marker.txt
		// - https://www.opengl.org/registry/specs/EXT/EXT_debug_label.txt
		// - https://www.opengl.org/registry/specs/KHR/debug.txt
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IRenderer methods            ]
	//[-------------------------------------------------------]
	const char *OpenGLRenderer::getName() const
	{
		return "OpenGL";
	}

	bool OpenGLRenderer::isInitialized() const
	{
		// Is the context initialized?
		return (nullptr != mContext && mContext->isInitialized());
	}

	bool OpenGLRenderer::isDebugEnabled()
	{
		// OpenGL has nothing that is similar to the Direct3D 9 PIX functions (D3DPERF_* functions, also works directly within VisualStudio 2012 out-of-the-box)

		// Debug disabled
		return false;
	}

	Renderer::ISwapChain *OpenGLRenderer::getMainSwapChain() const
	{
		return mMainSwapChain;
	}


	//[-------------------------------------------------------]
	//[ Shader language                                       ]
	//[-------------------------------------------------------]
	uint32_t OpenGLRenderer::getNumberOfShaderLanguages() const
	{
		uint32_t numberOfShaderLanguages = 0;

		// "GL_ARB_shader_objects" or "GL_ARB_separate_shader_objects" required
		if (mExtensions->isGL_ARB_shader_objects() || mExtensions->isGL_ARB_separate_shader_objects())
		{
			// GLSL supported
			++numberOfShaderLanguages;
		}

		// Done, return the number of supported shader languages
		return numberOfShaderLanguages;
	}

	const char *OpenGLRenderer::getShaderLanguageName(uint32_t index) const
	{
		uint32_t currentIndex = 0;

		// "GL_ARB_shader_objects" or "GL_ARB_separate_shader_objects" required
		if (mExtensions->isGL_ARB_shader_objects() || mExtensions->isGL_ARB_separate_shader_objects())
		{
			// GLSL supported
			if (currentIndex == index)
			{
				return ShaderLanguageMonolithic::NAME;	// "ShaderLanguageSeparate::NAME" has the same value
			}
			++currentIndex;
		}

		// Error!
		return nullptr;
	}

	Renderer::IShaderLanguage *OpenGLRenderer::getShaderLanguage(const char *shaderLanguageName)
	{
		// "GL_ARB_shader_objects" or "GL_ARB_separate_shader_objects" required
		if (mExtensions->isGL_ARB_shader_objects() || mExtensions->isGL_ARB_separate_shader_objects())
		{
			// In case "shaderLanguage" is a null pointer, use the default shader language
			if (nullptr != shaderLanguageName)
			{
				// Optimization: Check for shader language name pointer match, first
				// -> "ShaderLanguageSeparate::NAME" has the same value
				if (shaderLanguageName == ShaderLanguageMonolithic::NAME || !stricmp(shaderLanguageName, ShaderLanguageMonolithic::NAME))
				{
					// Prefer "GL_ARB_separate_shader_objects" over "GL_ARB_shader_objects"
					if (mExtensions->isGL_ARB_separate_shader_objects())
					{
						// If required, create the separate shader language instance right now
						if (nullptr == mShaderLanguage)
						{
							mShaderLanguage = new ShaderLanguageSeparate(*this);
							mShaderLanguage->addReference();	// Internal renderer reference
						}

						// Return the shader language instance
						return mShaderLanguage;
					}
					else if (mExtensions->isGL_ARB_shader_objects())
					{
						// If required, create the monolithic shader language instance right now
						if (nullptr == mShaderLanguage)
						{
							mShaderLanguage = new ShaderLanguageMonolithic(*this);
							mShaderLanguage->addReference();	// Internal renderer reference
						}

						// Return the shader language instance
						return mShaderLanguage;
					}
				}
			}
			else
			{
				// Return the shader language instance as default
				return getShaderLanguage(ShaderLanguageMonolithic::NAME);
			}
		}

		// Error!
		return nullptr;
	}


	//[-------------------------------------------------------]
	//[ Resource creation                                     ]
	//[-------------------------------------------------------]
	Renderer::ISwapChain *OpenGLRenderer::createSwapChain(handle nativeWindowHandle)
	{
		// The provided native window handle must not be a null handle
		return (NULL_HANDLE != nativeWindowHandle) ? new SwapChain(*this, nativeWindowHandle) : nullptr;
	}

	Renderer::IFramebuffer *OpenGLRenderer::createFramebuffer(uint32_t numberOfColorTextures, Renderer::ITexture **colorTextures, Renderer::ITexture *depthStencilTexture)
	{
		// "GL_ARB_framebuffer_object" required
		if (mExtensions->isGL_ARB_framebuffer_object())
		{
			// Is "GL_EXT_direct_state_access" there?
			if (mExtensions->isGL_EXT_direct_state_access() || mExtensions->isGL_ARB_direct_state_access())
			{
				// Effective direct state access (DSA)
				// -> Validation is done inside the framebuffer implementation
				return new FramebufferDsa(*this, numberOfColorTextures, colorTextures, depthStencilTexture);
			}
			else
			{
				// Traditional bind version
				// -> Validation is done inside the framebuffer implementation
				return new FramebufferBind(*this, numberOfColorTextures, colorTextures, depthStencilTexture);
			}
		}
		else
		{
			// Error!
			return nullptr;
		}
	}

	Renderer::IBufferManager *OpenGLRenderer::createBufferManager()
	{
		return new BufferManager(*this);
	}

	Renderer::ITextureManager *OpenGLRenderer::createTextureManager()
	{
		return new TextureManager(*this);
	}

	Renderer::IRootSignature *OpenGLRenderer::createRootSignature(const Renderer::RootSignature &rootSignature)
	{
		return new RootSignature(*this, rootSignature);
	}

	Renderer::IPipelineState *OpenGLRenderer::createPipelineState(const Renderer::PipelineState & pipelineState)
	{
		return new PipelineState(*this, pipelineState);
	}

	Renderer::ISamplerState *OpenGLRenderer::createSamplerState(const Renderer::SamplerState &samplerState)
	{
		// Is "GL_ARB_sampler_objects" there?
		if (mExtensions->isGL_ARB_sampler_objects())
		{
			// Effective sampler object (SO)
			return new SamplerStateSo(*this, samplerState);
		}
		else
		{
			// Is "GL_EXT_direct_state_access" there?
			if (mExtensions->isGL_EXT_direct_state_access() || mExtensions->isGL_ARB_direct_state_access())
			{
				// Direct state access (DSA) version to emulate a sampler object
				return new SamplerStateDsa(*this, samplerState);
			}
			else
			{
				// Traditional bind version to emulate a sampler object
				return new SamplerStateBind(*this, samplerState);
			}
		}
	}


	//[-------------------------------------------------------]
	//[ Resource handling                                     ]
	//[-------------------------------------------------------]
	bool OpenGLRenderer::map(Renderer::IResource& resource, uint32_t, Renderer::MapType mapType, uint32_t, Renderer::MappedSubresource& mappedSubresource)
	{
		// Evaluate the resource type
		switch (resource.getResourceType())
		{
			case Renderer::ResourceType::INDEX_BUFFER:
			{
				// TODO(co) This buffer update isn't efficient, use e.g. persistent buffer mapping

				// Is "GL_ARB_direct_state_access" there?
				if (mExtensions->isGL_ARB_direct_state_access())
				{
					// Effective direct state access (DSA)
					mappedSubresource.data		 = glMapNamedBuffer(static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer(), Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;
				}
				// Is "GL_EXT_direct_state_access" there?
				else if (mExtensions->isGL_EXT_direct_state_access())
				{
					// Effective direct state access (DSA)
					mappedSubresource.data		 = glMapNamedBufferEXT(static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer(), Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;
				}
				else
				{
					// Traditional bind version

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Backup the currently bound OpenGL array element buffer
						GLint openGLArrayElementBufferBackup = 0;
						glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &openGLArrayElementBufferBackup);
					#endif

					// Bind this OpenGL element buffer and upload the data
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer());

					// Map
					mappedSubresource.data		 = glMapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Be polite and restore the previous bound OpenGL array element buffer
						glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<GLuint>(openGLArrayElementBufferBackup));
					#endif
				}

				// Done
				return true;
			}

			case Renderer::ResourceType::VERTEX_BUFFER:
			{
				// TODO(co) This buffer update isn't efficient, use e.g. persistent buffer mapping

				// Is "GL_ARB_direct_state_access" there?
				if (mExtensions->isGL_ARB_direct_state_access())
				{
					// Effective direct state access (DSA)
					mappedSubresource.data		 = glMapNamedBuffer(static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer(), Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;
				}
				// Is "GL_EXT_direct_state_access" there?
				else if (mExtensions->isGL_EXT_direct_state_access())
				{
					// Effective direct state access (DSA)
					mappedSubresource.data		 = glMapNamedBufferEXT(static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer(), Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;
				}
				else
				{
					// Traditional bind version

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Backup the currently bound OpenGL array buffer
						GLint openGLArrayBufferBackup = 0;
						glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &openGLArrayBufferBackup);
					#endif

					// Bind this OpenGL array buffer and upload the data
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer());

					// Map
					mappedSubresource.data		 = glMapBufferARB(GL_ARRAY_BUFFER_ARB, Mapping::getOpenGLMapType(mapType));
					mappedSubresource.rowPitch   = 0;
					mappedSubresource.depthPitch = 0;

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Be polite and restore the previous bound OpenGL array buffer
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, static_cast<GLuint>(openGLArrayBufferBackup));
					#endif
				}

				// Done
				return true;
			}

			case Renderer::ResourceType::UNIFORM_BUFFER:
				// TODO(co) Implement me
				// return (S_OK == mD3D11DeviceContext->Map(static_cast<UniformBuffer&>(resource).getD3D11Buffer(), subresource, static_cast<D3D11_MAP>(mapType), mapFlags, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubresource)));
				return false;

			case Renderer::ResourceType::TEXTURE_BUFFER:
				// TODO(co) Implement me
				// return (S_OK == mD3D11DeviceContext->Map(static_cast<TextureBuffer&>(resource).getD3D11Buffer(), subresource, static_cast<D3D11_MAP>(mapType), mapFlags, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubresource)));
				return false;

			case Renderer::ResourceType::INDIRECT_BUFFER:
				// TODO(co) Implement me
				// return (S_OK == mD3D11DeviceContext->Map(static_cast<IndirectBuffer&>(resource).getD3D11Buffer(), subresource, static_cast<D3D11_MAP>(mapType), mapFlags, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubresource)));
				return false;

			case Renderer::ResourceType::TEXTURE_2D:
			{
				bool result = false;

				// TODO(co) Implement me
				/*
				// Begin debug event
				RENDERER_BEGIN_DEBUG_EVENT_FUNCTION(this)

				// Get the Direct3D 11 resource instance
				ID3D11Resource *d3d11Resource = nullptr;
				static_cast<Texture2D&>(resource).getD3D11ShaderResourceView()->GetResource(&d3d11Resource);
				if (nullptr != d3d11Resource)
				{
					// Map the Direct3D 11 resource
					result = (S_OK == mD3D11DeviceContext->Map(d3d11Resource, subresource, static_cast<D3D11_MAP>(mapType), mapFlags, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubresource)));

					// Release the Direct3D 11 resource instance
					d3d11Resource->Release();
				}

				// End debug event
				RENDERER_END_DEBUG_EVENT(this)
				*/

				// Done
				return result;
			}

			case Renderer::ResourceType::TEXTURE_2D_ARRAY:
			{
				bool result = false;

				// TODO(co) Implement me
				/*
				// Begin debug event
				RENDERER_BEGIN_DEBUG_EVENT_FUNCTION(this)

				// Get the Direct3D 11 resource instance
				ID3D11Resource *d3d11Resource = nullptr;
				static_cast<Texture2DArray&>(resource).getD3D11ShaderResourceView()->GetResource(&d3d11Resource);
				if (nullptr != d3d11Resource)
				{
					// Map the Direct3D 11 resource
					result = (S_OK == mD3D11DeviceContext->Map(d3d11Resource, subresource, static_cast<D3D11_MAP>(mapType), mapFlags, reinterpret_cast<D3D11_MAPPED_SUBRESOURCE*>(&mappedSubresource)));

					// Release the Direct3D 11 resource instance
					d3d11Resource->Release();
				}

				// End debug event
				RENDERER_END_DEBUG_EVENT(this)
				*/

				// Done
				return result;
			}

			case Renderer::ResourceType::ROOT_SIGNATURE:
			case Renderer::ResourceType::PROGRAM:
			case Renderer::ResourceType::VERTEX_ARRAY:
			case Renderer::ResourceType::SWAP_CHAIN:
			case Renderer::ResourceType::FRAMEBUFFER:
			case Renderer::ResourceType::PIPELINE_STATE:
			case Renderer::ResourceType::SAMPLER_STATE:
			case Renderer::ResourceType::VERTEX_SHADER:
			case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
			case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
			case Renderer::ResourceType::GEOMETRY_SHADER:
			case Renderer::ResourceType::FRAGMENT_SHADER:
			default:
				// Nothing we can map, set known return values
				mappedSubresource.data		 = nullptr;
				mappedSubresource.rowPitch   = 0;
				mappedSubresource.depthPitch = 0;

				// Error!
				return false;
		}
	}

	void OpenGLRenderer::unmap(Renderer::IResource& resource, uint32_t)
	{
		// Evaluate the resource type
		switch (resource.getResourceType())
		{
			case Renderer::ResourceType::INDEX_BUFFER:
			{
				// Is "GL_ARB_direct_state_access" there?
				if (mExtensions->isGL_ARB_direct_state_access())
				{
					// Effective direct state access (DSA)
					glUnmapNamedBuffer(static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer());
				}
				// Is "GL_EXT_direct_state_access" there?
				else if (mExtensions->isGL_EXT_direct_state_access())
				{
					// Effective direct state access (DSA)
					glUnmapNamedBufferEXT(static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer());
				}
				else
				{
					// Traditional bind version

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Backup the currently bound OpenGL array element buffer
						GLint openGLArrayElementBufferBackup = 0;
						glGetIntegerv(GL_ELEMENT_ARRAY_BUFFER_BINDING_ARB, &openGLArrayElementBufferBackup);
					#endif

					// Bind this OpenGL element buffer and upload the data
					glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<IndexBuffer&>(resource).getOpenGLElementArrayBuffer());

					// Map
					glUnmapBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB);

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Be polite and restore the previous bound OpenGL array element buffer
						glBindBufferARB(GL_ELEMENT_ARRAY_BUFFER_ARB, static_cast<GLuint>(openGLArrayElementBufferBackup));
					#endif
				}
				break;
			}

			case Renderer::ResourceType::VERTEX_BUFFER:
			{
				// Is "GL_ARB_direct_state_access" there?
				if (mExtensions->isGL_ARB_direct_state_access())
				{
					// Effective direct state access (DSA)
					glUnmapNamedBuffer(static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer());
				}
				// Is "GL_EXT_direct_state_access" there?
				else if (mExtensions->isGL_EXT_direct_state_access())
				{
					// Effective direct state access (DSA)
					glUnmapNamedBufferEXT(static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer());
				}
				else
				{
					// Traditional bind version

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Backup the currently bound OpenGL array buffer
						GLint openGLArrayBufferBackup = 0;
						glGetIntegerv(GL_ARRAY_BUFFER_BINDING_ARB, &openGLArrayBufferBackup);
					#endif

					// Bind this OpenGL array buffer and upload the data
					glBindBufferARB(GL_ARRAY_BUFFER_ARB, static_cast<VertexBuffer&>(resource).getOpenGLArrayBuffer());

					// Map
					glUnmapBufferARB(GL_ARRAY_BUFFER_ARB);

					#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
						// Be polite and restore the previous bound OpenGL array buffer
						glBindBufferARB(GL_ARRAY_BUFFER_ARB, static_cast<GLuint>(openGLArrayBufferBackup));
					#endif
				}
				break;
			}

			case Renderer::ResourceType::UNIFORM_BUFFER:
				// TODO(co) Implement me
				// mD3D11DeviceContext->Unmap(static_cast<UniformBuffer&>(resource).getD3D11Buffer(), subresource);
				break;

			case Renderer::ResourceType::TEXTURE_BUFFER:
				// TODO(co) Implement me
				// mD3D11DeviceContext->Unmap(static_cast<TextureBuffer&>(resource).getD3D11Buffer(), subresource);
				break;

			case Renderer::ResourceType::INDIRECT_BUFFER:
				// TODO(co) Implement me
				// mD3D11DeviceContext->Unmap(static_cast<IndirectBuffer&>(resource).getD3D11Buffer(), subresource);
				break;

			case Renderer::ResourceType::TEXTURE_2D:
			{
				// TODO(co) Implement me
				/*
				// Get the Direct3D 11 resource instance
				ID3D11Resource *d3d11Resource = nullptr;
				static_cast<Texture2D&>(resource).getD3D11ShaderResourceView()->GetResource(&d3d11Resource);
				if (nullptr != d3d11Resource)
				{
					// Unmap the Direct3D 11 resource
					mD3D11DeviceContext->Unmap(d3d11Resource, subresource);

					// Release the Direct3D 11 resource instance
					d3d11Resource->Release();
				}
				*/
				break;
			}

			case Renderer::ResourceType::TEXTURE_2D_ARRAY:
			{
				// TODO(co) Implement me
				/*
				// Get the Direct3D 11 resource instance
				ID3D11Resource *d3d11Resource = nullptr;
				static_cast<Texture2DArray&>(resource).getD3D11ShaderResourceView()->GetResource(&d3d11Resource);
				if (nullptr != d3d11Resource)
				{
					// Unmap the Direct3D 11 resource
					mD3D11DeviceContext->Unmap(d3d11Resource, subresource);

					// Release the Direct3D 11 resource instance
					d3d11Resource->Release();
				}
				*/
				break;
			}

			case Renderer::ResourceType::ROOT_SIGNATURE:
			case Renderer::ResourceType::PROGRAM:
			case Renderer::ResourceType::VERTEX_ARRAY:
			case Renderer::ResourceType::SWAP_CHAIN:
			case Renderer::ResourceType::FRAMEBUFFER:
			case Renderer::ResourceType::PIPELINE_STATE:
			case Renderer::ResourceType::SAMPLER_STATE:
			case Renderer::ResourceType::VERTEX_SHADER:
			case Renderer::ResourceType::TESSELLATION_CONTROL_SHADER:
			case Renderer::ResourceType::TESSELLATION_EVALUATION_SHADER:
			case Renderer::ResourceType::GEOMETRY_SHADER:
			case Renderer::ResourceType::FRAGMENT_SHADER:
			default:
				// Nothing we can unmap
				break;
		}
	}


	//[-------------------------------------------------------]
	//[ Operations                                            ]
	//[-------------------------------------------------------]
	bool OpenGLRenderer::beginScene()
	{
		// Not required when using OpenGL

		// Done
		return true;
	}

	void OpenGLRenderer::submitCommandBuffer(const Renderer::CommandBuffer& commandBuffer)
	{
		// Loop through all commands
		uint8_t* commandPacketBuffer = const_cast<uint8_t*>(commandBuffer.getCommandPacketBuffer());	// TODO(co) Get rid of the evil const-cast
		Renderer::CommandPacket commandPacket = commandPacketBuffer;
		while (nullptr != commandPacket)
		{
			{ // Submit command packet
				const Renderer::CommandDispatchFunctionIndex commandDispatchFunctionIndex = Renderer::CommandPacketHelper::loadCommandDispatchFunctionIndex(commandPacket);
				const void* command = Renderer::CommandPacketHelper::loadCommand(commandPacket);
				detail::DISPATCH_FUNCTIONS[commandDispatchFunctionIndex](command, *this);
			}

			{ // Next command
				const uint32_t nextCommandPacketByteIndex = Renderer::CommandPacketHelper::getNextCommandPacketByteIndex(commandPacket);
				commandPacket = (~0u != nextCommandPacketByteIndex) ? &commandPacketBuffer[nextCommandPacketByteIndex] : nullptr;
			}
		}
	}

	void OpenGLRenderer::endScene()
	{
		// We need to forget about the currently set render target
		omSetRenderTarget(nullptr);

		// We need to forget about the currently set vertex array
		iaUnsetVertexArray();
	}


	//[-------------------------------------------------------]
	//[ Synchronization                                       ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::flush()
	{
		glFlush();
	}

	void OpenGLRenderer::finish()
	{
		glFinish();
	}


	//[-------------------------------------------------------]
	//[ Private static methods                                ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::debugMessageCallback(uint32_t source, uint32_t type, uint32_t id, uint32_t severity, int, const char *message, const void *)
	{
		// Source to string
		char debugSource[20]{0};
		switch (source)
		{
			case GL_DEBUG_SOURCE_API_ARB:
				strncpy(debugSource, "OpenGL", 20);
				break;

			case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
				strncpy(debugSource, "Windows", 20);
				break;

			case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
				strncpy(debugSource, "Shader compiler", 20);
				break;

			case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
				strncpy(debugSource, "Third party", 20);
				break;

			case GL_DEBUG_SOURCE_APPLICATION_ARB:
				strncpy(debugSource, "Application", 20);
				break;

			case GL_DEBUG_SOURCE_OTHER_ARB:
				strncpy(debugSource, "Other", 20);
				break;

			default:
				strncpy(debugSource, "?", 20);
				break;
		}

		// Debug type to string
		char debugType[25]{0};
		switch (type)
		{
			case GL_DEBUG_TYPE_ERROR_ARB:
				strncpy(debugType, "Error", 25);
				break;

			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
				strncpy(debugType, "Deprecated behavior", 25);
				break;

			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
				strncpy(debugType, "Undefined behavior", 25);
				break;

			case GL_DEBUG_TYPE_PORTABILITY_ARB:
				strncpy(debugType, "Portability", 25);
				break;

			case GL_DEBUG_TYPE_PERFORMANCE_ARB:
				strncpy(debugType, "Performance", 25);
				break;

			case GL_DEBUG_TYPE_OTHER_ARB:
				strncpy(debugType, "Other", 25);
				break;

			default:
				strncpy(debugType, "?", 25);
				break;
		}

		// Debug severity to string
		char debugSeverity[20]{0};
		switch (severity)
		{
			case GL_DEBUG_SEVERITY_HIGH_ARB:
				strncpy(debugSeverity, "High", 20);
				break;

			case GL_DEBUG_SEVERITY_MEDIUM_ARB:
				strncpy(debugSeverity, "Medium", 20);
				break;

			case GL_DEBUG_SEVERITY_LOW_ARB:
				strncpy(debugSeverity, "Low", 20);
				break;

			case GL_DEBUG_SEVERITY_NOTIFICATION:
				strncpy(debugSeverity, "Notification", 20);
				break;

			default:
				strncpy(debugSeverity, "?", 20);
				break;
		}

		// Output the debug message
		#ifdef RENDERER_NO_DEBUG
			// Avoid "warning C4100: '<x>' : unreferenced formal parameter"-warning
			id = id;
			message = message;
		#else
			RENDERER_OUTPUT_DEBUG_PRINTF("OpenGL error: OpenGL debug message\tSource:\"%s\"\tType:\"%s\"\tID:\"%d\"\tSeverity:\"%s\"\tMessage:\"%s\"\n", debugSource, debugType, id, debugSeverity, message)
		#endif
	}


	//[-------------------------------------------------------]
	//[ Private methods                                       ]
	//[-------------------------------------------------------]
	void OpenGLRenderer::initializeCapabilities()
	{
		GLint openGLValue = 0;

		// Maximum number of viewports (always at least 1)
		// TODO(co) "GL_ARB_viewport_array" support ("OpenGLRenderer::rsSetViewports()")
		mCapabilities.maximumNumberOfViewports = 1;	// TODO(co) GL_ARB_viewport_array

		// Maximum number of simultaneous render targets (if <1 render to texture is not supported, "GL_ARB_draw_buffers" required)
		if (mExtensions->isGL_ARB_draw_buffers())
		{
			glGetIntegerv(GL_MAX_DRAW_BUFFERS_ARB, &openGLValue);
			mCapabilities.maximumNumberOfSimultaneousRenderTargets = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			// "GL_ARB_framebuffer_object"-extension for render to texture required
			mCapabilities.maximumNumberOfSimultaneousRenderTargets = static_cast<uint32_t>(mExtensions->isGL_ARB_framebuffer_object());
		}

		// Maximum texture dimension
		glGetIntegerv(GL_MAX_TEXTURE_SIZE, &openGLValue);
		mCapabilities.maximumTextureDimension = static_cast<uint32_t>(openGLValue);

		// Maximum number of 2D texture array slices (usually 512, in case there's no support for 2D texture arrays it's 0)
		if (mExtensions->isGL_EXT_texture_array())
		{
			glGetIntegerv(GL_MAX_ARRAY_TEXTURE_LAYERS_EXT, &openGLValue);
			mCapabilities.maximumNumberOf2DTextureArraySlices = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumNumberOf2DTextureArraySlices = 0;
		}

		// Maximum uniform buffer (UBO) size in bytes (usually at least 4096 * 16 bytes, in case there's no support for uniform buffer it's 0)
		if (mExtensions->isGL_ARB_uniform_buffer_object())
		{
			glGetIntegerv(GL_MAX_UNIFORM_BLOCK_SIZE, &openGLValue);
			mCapabilities.maximumUniformBufferSize = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumUniformBufferSize = 0;
		}

		// Maximum texture buffer (TBO) size in texel (>65536, typically much larger than that of one-dimensional texture, in case there's no support for texture buffer it's 0)
		if (mExtensions->isGL_ARB_texture_buffer_object())
		{
			glGetIntegerv(GL_MAX_TEXTURE_BUFFER_SIZE_EXT, &openGLValue);
			mCapabilities.maximumTextureBufferSize = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumTextureBufferSize = 0;
		}

		// Maximum indirect buffer size in bytes (in case there's no support for indirect buffer it's 0)
		if (mExtensions->isGL_ARB_draw_indirect())
		{
			// TODO(co) How to get the indirect buffer maximum size? Didn't find any information about this.
			mCapabilities.maximumIndirectBufferSize = sizeof(Renderer::DrawIndexedInstancedArguments) * 4096;
		}
		else
		{
			mCapabilities.maximumIndirectBufferSize = sizeof(Renderer::DrawIndexedInstancedArguments) * 4096;	// TODO(co) What is an usually decent emulated indirect buffer size?
		}

		// Maximum number of multisamples (always at least 1, usually 8)
		if (mExtensions->isGL_ARB_texture_multisample())
		{
			glGetIntegerv(GL_MAX_SAMPLES, &openGLValue);
			if (openGLValue > 8)
			{
				// Limit to known maximum we can test, even if e.g. GeForce 980m reports 32 here
				openGLValue = 8;
			}
			mCapabilities.maximumNumberOfMultisamples = static_cast<uint8_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumNumberOfMultisamples = 1;
		}

		// Individual uniforms ("constants" in Direct3D terminology) supported? If not, only uniform buffer objects are supported.
		mCapabilities.individualUniforms = true;

		// Instanced arrays supported? (shader model 3 feature, vertex array element advancing per-instance instead of per-vertex)
		mCapabilities.instancedArrays = true;

		// Instanced arrays supported? (shader model 3 feature, vertex array element advancing per-instance instead of per-vertex, "GL_ARB_instanced_arrays" required)
		mCapabilities.instancedArrays = mExtensions->isGL_ARB_instanced_arrays();

		// Draw instanced supported? (shader model 4 feature, build in shader variable holding the current instance ID, "GL_ARB_draw_instanced" required)
		mCapabilities.drawInstanced = mExtensions->isGL_ARB_draw_instanced();

		// Base vertex supported for draw calls?
		mCapabilities.baseVertex = mExtensions->isGL_ARB_draw_elements_base_vertex();

		// OpenGL has no native multi-threading
		mCapabilities.nativeMultiThreading = false;

		// Is there support for vertex shaders (VS)?
		mCapabilities.vertexShader = mExtensions->isGL_ARB_vertex_shader();

		// Maximum number of vertices per patch (usually 0 for no tessellation support or 32 which is the maximum number of supported vertices per patch)
		if (mExtensions->isGL_ARB_tessellation_shader())
		{
			glGetIntegerv(GL_MAX_PATCH_VERTICES, &openGLValue);
			mCapabilities.maximumNumberOfPatchVertices = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumNumberOfPatchVertices = 0;
		}

		// Maximum number of vertices a geometry shader can emit (usually 0 for no geometry shader support or 1024)
		if (mExtensions->isGL_ARB_geometry_shader4())
		{
			glGetIntegerv(GL_MAX_GEOMETRY_OUTPUT_VERTICES_ARB, &openGLValue);
			mCapabilities.maximumNumberOfGsOutputVertices = static_cast<uint32_t>(openGLValue);
		}
		else
		{
			mCapabilities.maximumNumberOfGsOutputVertices = 0;
		}

		// Is there support for fragment shaders (FS)?
		mCapabilities.fragmentShader = mExtensions->isGL_ARB_fragment_shader();
	}

	void OpenGLRenderer::iaUnsetVertexArray()
	{
		// Release the currently used vertex array reference, in case we have one
		if (nullptr != mVertexArray)
		{
			// Evaluate the internal array type of the currently set vertex array
			switch (static_cast<VertexArray*>(mVertexArray)->getInternalResourceType())
			{
				case VertexArray::InternalResourceType::NO_VAO:
					// Disable OpenGL vertex attribute arrays
					static_cast<VertexArrayNoVao*>(mVertexArray)->disableOpenGLVertexAttribArrays();
					break;

				case VertexArray::InternalResourceType::VAO:
					// Unbind OpenGL vertex array
					// -> No need to check for "GL_ARB_vertex_array_object", in case were in here we know it must exist
					glBindVertexArray(0);
					break;
			}

			// Release reference
			mVertexArray->releaseReference();
			mVertexArray = nullptr;
		}
	}

	void OpenGLRenderer::setProgram(Renderer::IProgram *program)
	{
		// TODO(co) Avoid changing already set program

		if (nullptr != program)
		{
			// Security check: Is the given resource owned by this renderer? (calls "return" in case of a mismatch)
			OPENGLRENDERER_RENDERERMATCHCHECK_RETURN(*this, *program)

			// Prefer "GL_ARB_separate_shader_objects" over "GL_ARB_shader_objects"
			if (mExtensions->isGL_ARB_separate_shader_objects())
			{
				// Bind the program pipeline
				glBindProgramPipeline(static_cast<ProgramSeparate*>(program)->getOpenGLProgramPipeline());
			}
			else if (mExtensions->isGL_ARB_shader_objects())
			{
				// Bind the program
				glUseProgramObjectARB(static_cast<ProgramMonolithic*>(program)->getOpenGLProgram());
			}
		}
		else
		{
			// TODO(co) GLSL buffer settings

			// Prefer "GL_ARB_separate_shader_objects" over "GL_ARB_shader_objects"
			if (mExtensions->isGL_ARB_separate_shader_objects())
			{
				// Unbind the program pipeline
				glBindProgramPipeline(0);
			}
			else if (mExtensions->isGL_ARB_shader_objects())
			{
				// Unbind the program
				glUseProgramObjectARB(0);
			}
		}
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // OpenGLRenderer
