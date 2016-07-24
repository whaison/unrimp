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
#include "OpenGLRenderer/Shader/Separate/ProgramSeparate.h"
#include "OpenGLRenderer/Shader/Separate/VertexShaderSeparate.h"
#include "OpenGLRenderer/Shader/Separate/GeometryShaderSeparate.h"
#include "OpenGLRenderer/Shader/Separate/FragmentShaderSeparate.h"
#include "OpenGLRenderer/Shader/Separate/TessellationControlShaderSeparate.h"
#include "OpenGLRenderer/Shader/Separate/TessellationEvaluationShaderSeparate.h"
#include "OpenGLRenderer/OpenGLRuntimeLinking.h"
#include "OpenGLRenderer/IContext.h"
#include "OpenGLRenderer/Extensions.h"
#include "OpenGLRenderer/RootSignature.h"
#include "OpenGLRenderer/OpenGLRenderer.h"

#include <Renderer/VertexArrayTypes.h>


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
namespace OpenGLRenderer
{


	//[-------------------------------------------------------]
	//[ Public methods                                        ]
	//[-------------------------------------------------------]
	ProgramSeparate::ProgramSeparate(OpenGLRenderer &openGLRenderer, const Renderer::IRootSignature& rootSignature, const Renderer::VertexAttributes& vertexAttributes, VertexShaderSeparate *vertexShaderSeparate, TessellationControlShaderSeparate *tessellationControlShaderSeparate, TessellationEvaluationShaderSeparate *tessellationEvaluationShaderSeparate, GeometryShaderSeparate *geometryShaderSeparate, FragmentShaderSeparate *fragmentShaderSeparate) :
		IProgram(openGLRenderer),
		mOpenGLProgram(0),
		mOpenGLProgramPipeline(0),
		mVertexShaderSeparate(vertexShaderSeparate),
		mTessellationControlShaderSeparate(tessellationControlShaderSeparate),
		mTessellationEvaluationShaderSeparate(tessellationEvaluationShaderSeparate),
		mGeometryShaderSeparate(geometryShaderSeparate),
		mFragmentShaderSeparate(fragmentShaderSeparate),
		mNumberOfRootSignatureParameters(0),
		mRootSignatureParameterIndexToUniformLocation(nullptr)
	{
		// Create the OpenGL program pipeline
		glGenProgramPipelines(1, &mOpenGLProgramPipeline);
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program pipeline
			GLint openGLProgramPipelineBackup = 0;
			glGetIntegerv(GL_PROGRAM_PIPELINE_BINDING, &openGLProgramPipelineBackup);
		#endif
		glBindProgramPipeline(mOpenGLProgramPipeline);

		// Add references to the provided shaders
		if (nullptr != mVertexShaderSeparate)
		{
			mVertexShaderSeparate->addReference();
			glUseProgramStages(mOpenGLProgramPipeline, GL_VERTEX_SHADER_BIT, mVertexShaderSeparate->getOpenGLShaderProgram());

			// TODO(co) Should be done only once inside "VertexShaderSeparate"
			{ // Define the vertex array attribute binding locations ("vertex declaration" in Direct3D 9 terminology, "input layout" in Direct3D 10 & 11 & 12 terminology)
				const GLuint openGLShaderProgram = mVertexShaderSeparate->getOpenGLShaderProgram();
				const uint32_t numberOfVertexAttributes = vertexAttributes.numberOfAttributes;
				for (uint32_t vertexAttribute = 0; vertexAttribute < numberOfVertexAttributes; ++vertexAttribute)
				{
					glBindAttribLocationARB(openGLShaderProgram, vertexAttribute, vertexAttributes.attributes[vertexAttribute].name);
				}
			}
		}
		if (nullptr != mTessellationControlShaderSeparate)
		{
			mTessellationControlShaderSeparate->addReference();
			glUseProgramStages(mOpenGLProgramPipeline, GL_TESS_CONTROL_SHADER_BIT, mTessellationControlShaderSeparate->getOpenGLShaderProgram());
		}
		if (nullptr != mTessellationEvaluationShaderSeparate)
		{
			mTessellationEvaluationShaderSeparate->addReference();
			glUseProgramStages(mOpenGLProgramPipeline, GL_TESS_EVALUATION_SHADER_BIT, mTessellationEvaluationShaderSeparate->getOpenGLShaderProgram());
		}
		if (nullptr != mGeometryShaderSeparate)
		{
			mGeometryShaderSeparate->addReference();
			glUseProgramStages(mOpenGLProgramPipeline, GL_GEOMETRY_SHADER_BIT, mGeometryShaderSeparate->getOpenGLShaderProgram());
		}
		if (nullptr != mFragmentShaderSeparate)
		{
			mFragmentShaderSeparate->addReference();
			glUseProgramStages(mOpenGLProgramPipeline, GL_FRAGMENT_SHADER_BIT, mFragmentShaderSeparate->getOpenGLShaderProgram());
		}

		// Validate program pipeline
		glValidateProgramPipeline(mOpenGLProgramPipeline);
		GLint validateStatus = 0;
		glGetProgramPipelineiv(mOpenGLProgramPipeline, GL_VALIDATE_STATUS, &validateStatus);
		if (GL_TRUE == validateStatus)
		{
			// We're not using "glBindFragDataLocation()", else the user would have to provide us with additional OpenGL-only specific information
			// -> Use modern GLSL:
			//    "layout(location = 0) out vec4 ColorOutput0;"
			//    "layout(location = 1) out vec4 ColorOutput1;"
			// -> Use legacy GLSL if necessary:
			//    "gl_FragData[0] = vec4(1.0f, 0.0f, 0.0f, 0.0f);"
			//    "gl_FragData[1] = vec4(0.0f, 0.0f, 1.0f, 0.0f);"

			// The actual locations assigned to uniform variables are not known until the program object is linked successfully
			// -> So we have to build a root signature parameter index -> uniform location mapping here
			const Renderer::RootSignature& rootSignatureData = static_cast<const RootSignature&>(rootSignature).getRootSignature();
			const uint32_t numberOfParameters = rootSignatureData.numberOfParameters;
			if (numberOfParameters > 0)
			{
				mRootSignatureParameterIndexToUniformLocation = new int32_t[numberOfParameters];
				memset(mRootSignatureParameterIndexToUniformLocation, -1, sizeof(int32_t) * numberOfParameters);
				for (uint32_t parameterIndex = 0; parameterIndex < numberOfParameters; ++parameterIndex)
				{
					const Renderer::RootParameter& rootParameter = rootSignatureData.parameters[parameterIndex];
					if (Renderer::RootParameterType::DESCRIPTOR_TABLE == rootParameter.parameterType)
					{
						// TODO(co) For now, we only support a single descriptor range
						if (1 != rootParameter.descriptorTable.numberOfDescriptorRanges)
						{
							RENDERER_OUTPUT_DEBUG_STRING("OpenGL error: Only a single descriptor range is supported")
						}
						else
						{
							const Renderer::DescriptorRange* descriptorRange = rootParameter.descriptorTable.descriptorRanges;

							// Ignore sampler range types in here (OpenGL handles samplers in a different way then Direct3D 10>=)
							if (Renderer::DescriptorRangeType::UBV == descriptorRange->rangeType)
							{
								// TODO(co) Bindings
								/*
								// Explicit binding points ("layout(binding = 0)" in GLSL shader) requires OpenGL 4.2 or the "GL_ARB_explicit_uniform_location"-extension,
								// for backward compatibility, ask for the uniform block index
								const GLuint uniformBlockIndex = glGetUniformBlockIndex(mOpenGLProgram, descriptorRange->baseShaderRegisterName);
								if (GL_INVALID_INDEX != uniformBlockIndex)
								{
									mRootSignatureParameterIndexToUniformLocation[parameterIndex] = static_cast<int32_t>(uniformBlockIndex);

									// Associate the uniform block with the given binding point
									glUniformBlockBinding(mOpenGLProgram, uniformBlockIndex, parameterIndex);
								}
								*/
							}
							else if (Renderer::DescriptorRangeType::SAMPLER != descriptorRange->rangeType)
							{
								// TODO(co) Bindings
								/*
								const GLint uniformLocation = glGetUniformLocationARB(mOpenGLProgram, descriptorRange->baseShaderRegisterName);
								if (uniformLocation >= 0)
								{
									mRootSignatureParameterIndexToUniformLocation[parameterIndex] = uniformLocation;

									// OpenGL/GLSL is not automatically assigning texture units to samplers, so, we have to take over this job
									// -> When using OpenGL or OpenGL ES 2 this is required
									// -> OpenGL 4.2 or the "GL_ARB_explicit_uniform_location"-extension supports explicit binding points ("layout(binding = 0)"
									//    in GLSL shader) , for backward compatibility we don't use it in here
									// -> When using Direct3D 9, 10, 11 or 12, the texture unit
									//    to use is usually defined directly within the shader by using the "register"-keyword
									// TODO(co) There's room for binding API call related optimization in here (will certainly be no huge overall efficiency gain)
									#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
										// Backup the currently used OpenGL program
										GLint openGLProgramBackup = 0;
										glGetIntegerv(GL_CURRENT_PROGRAM, &openGLProgramBackup);
										if (openGLProgramBackup == mOpenGLProgram)
										{
											// Set uniform, please note that for this our program must be the currently used one
											glUniform1iARB(uniformLocation, static_cast<GLint>(descriptorRange->baseShaderRegister));
										}
										else
										{
											// Set uniform, please note that for this our program must be the currently used one
											glUseProgramObjectARB(mOpenGLProgram);
											glUniform1iARB(uniformLocation, static_cast<GLint>(descriptorRange->baseShaderRegister));

											// Be polite and restore the previous used OpenGL program
											glUseProgramObjectARB(openGLProgramBackup);
										}
									#else
										// Set uniform, please note that for this our program must be the currently used one
										glUseProgramObjectARB(mOpenGLProgram);
										glUniform1iARB(uniformLocation, static_cast<GLint>(descriptorRange->baseShaderRegister));
									#endif
								}
								*/
							}
						}
					}
				}
			}
		}
		else
		{
			// Error, program pipeline validation failed!
			#ifdef RENDERER_OUTPUT_DEBUG
				// Get the length of the information (including a null termination)
				GLint informationLength = 0;
				glGetProgramPipelineiv(mOpenGLProgramPipeline, GL_INFO_LOG_LENGTH, &informationLength);
				if (informationLength > 1)
				{
					// Allocate memory for the information
					char *informationLog = new char[static_cast<uint32_t>(informationLength)];

					// Get the information
					glGetProgramPipelineInfoLog(mOpenGLProgramPipeline, informationLength, nullptr, informationLog);

					// Output the debug string
					RENDERER_OUTPUT_DEBUG_STRING(informationLog)

					// Cleanup information memory
					delete [] informationLog;
				}
			#endif
		}

		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Be polite and restore the previous used OpenGL program pipeline
			glBindProgramPipeline(static_cast<GLuint>(openGLProgramPipelineBackup));
		#endif
	}

	ProgramSeparate::~ProgramSeparate()
	{
		// Destroy the OpenGL program pipeline
		glDeleteProgramPipelines(1, &mOpenGLProgramPipeline);

		// Release the shader references
		if (nullptr != mVertexShaderSeparate)
		{
			mVertexShaderSeparate->release();
		}
		if (nullptr != mTessellationControlShaderSeparate)
		{
			mTessellationControlShaderSeparate->release();
		}
		if (nullptr != mTessellationEvaluationShaderSeparate)
		{
			mTessellationEvaluationShaderSeparate->release();
		}
		if (nullptr != mGeometryShaderSeparate)
		{
			mGeometryShaderSeparate->release();
		}
		if (nullptr != mFragmentShaderSeparate)
		{
			mFragmentShaderSeparate->release();
		}

		// Destroy root signature parameter index to OpenGL uniform location mapping, if required
		if (nullptr != mRootSignatureParameterIndexToUniformLocation)
		{
			delete [] mRootSignatureParameterIndexToUniformLocation;
		}
	}


	//[-------------------------------------------------------]
	//[ Public virtual Renderer::IProgram methods             ]
	//[-------------------------------------------------------]
	handle ProgramSeparate::getUniformHandle(const char *uniformName)
	{
		return static_cast<handle>(glGetUniformLocationARB(mOpenGLProgram, uniformName));
	}

	void ProgramSeparate::setUniform1i(handle uniformHandle, int value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniform1iARB(static_cast<GLint>(uniformHandle), value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniform1iARB(static_cast<GLint>(uniformHandle), value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniform1iARB(static_cast<GLint>(uniformHandle), value);
		#endif
	}

	void ProgramSeparate::setUniform1f(handle uniformHandle, float value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniform1fARB(static_cast<GLint>(uniformHandle), value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniform1fARB(static_cast<GLint>(uniformHandle), value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniform1fARB(static_cast<GLint>(uniformHandle), value);
		#endif
	}

	void ProgramSeparate::setUniform2fv(handle uniformHandle, const float *value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniform2fvARB(static_cast<GLint>(uniformHandle), 1, value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniform2fvARB(static_cast<GLint>(uniformHandle), 1, value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniform2fvARB(static_cast<GLint>(uniformHandle), 1, value);
		#endif
	}

	void ProgramSeparate::setUniform3fv(handle uniformHandle, const float *value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniform3fvARB(static_cast<GLint>(uniformHandle), 1, value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniform3fvARB(static_cast<GLint>(uniformHandle), 1, value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniform3fvARB(static_cast<GLint>(uniformHandle), 1, value);
		#endif
	}

	void ProgramSeparate::setUniform4fv(handle uniformHandle, const float *value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniform4fvARB(static_cast<GLint>(uniformHandle), 1, value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniform4fvARB(static_cast<GLint>(uniformHandle), 1, value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniform4fvARB(static_cast<GLint>(uniformHandle), 1, value);
		#endif
	}

	void ProgramSeparate::setUniformMatrix3fv(handle uniformHandle, const float *value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniformMatrix3fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniformMatrix3fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniformMatrix3fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);
		#endif
	}

	void ProgramSeparate::setUniformMatrix4fv(handle uniformHandle, const float *value)
	{
		#ifndef OPENGLRENDERER_NO_STATE_CLEANUP
			// Backup the currently used OpenGL program
			const GLint openGLProgramBackup = glGetHandleARB(GL_PROGRAM_OBJECT_ARB);
			if (openGLProgramBackup == mOpenGLProgram)
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUniformMatrix4fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);
			}
			else
			{
				// Set uniform, please note that for this our program must be the currently used one
				glUseProgramObjectARB(mOpenGLProgram);
				glUniformMatrix4fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);

				// Be polite and restore the previous used OpenGL program
				glUseProgramObjectARB(openGLProgramBackup);
			}
		#else
			// Set uniform, please note that for this our program must be the currently used one
			glUseProgramObjectARB(mOpenGLProgram);
			glUniformMatrix4fvARB(static_cast<GLint>(uniformHandle), 1, GL_FALSE, value);
		#endif
	}


//[-------------------------------------------------------]
//[ Namespace                                             ]
//[-------------------------------------------------------]
} // OpenGLRenderer
