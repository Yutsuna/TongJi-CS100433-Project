/**
 * @file RHIGL.hpp
 * @brief OpenGL RHI interface
 */

#pragma once

#include <Core/CoreTypes.hpp>
#include <GL/glcorearb.h>

#include <RHI/RHITypes.hpp>

#define RHI_GL_FUNCTION_LIST( X )                                                                                                \
    X( PFNGLGETSTRINGPROC, "glGetString", GetString )                                                                            \
    X( PFNGLGETINTEGERVPROC, "glGetIntegerv", GetIntegerv )                                                                      \
    X( PFNGLGETINTEGERI_VPROC, "glGetIntegeri_v", GetIntegeriV )                                                                 \
    X( PFNGLENABLEPROC, "glEnable", Enable )                                                                                     \
    X( PFNGLDISABLEPROC, "glDisable", Disable )                                                                                  \
    X( PFNGLVIEWPORTPROC, "glViewport", Viewport )                                                                               \
    X( PFNGLCLEARCOLORPROC, "glClearColor", ClearColor )                                                                         \
    X( PFNGLCLEARPROC, "glClear", Clear )                                                                                        \
    X( PFNGLMEMORYBARRIERPROC, "glMemoryBarrier", MemoryBarrier )                                                                \
    X( PFNGLDEBUGMESSAGECALLBACKPROC, "glDebugMessageCallback", DebugMessageCallback )                                           \
    X( PFNGLOBJECTLABELPROC, "glObjectLabel", ObjectLabel )                                                                      \
    X( PFNGLCREATEBUFFERSPROC, "glCreateBuffers", CreateBuffers )                                                                \
    X( PFNGLDELETEBUFFERSPROC, "glDeleteBuffers", DeleteBuffers )                                                                \
    X( PFNGLNAMEDBUFFERSTORAGEPROC, "glNamedBufferStorage", NamedBufferStorage )                                                 \
    X( PFNGLNAMEDBUFFERSUBDATAPROC, "glNamedBufferSubData", NamedBufferSubData )                                                 \
    X( PFNGLGETNAMEDBUFFERSUBDATAPROC, "glGetNamedBufferSubData", GetNamedBufferSubData )                                        \
    X( PFNGLMAPNAMEDBUFFERRANGEPROC, "glMapNamedBufferRange", MapNamedBufferRange )                                              \
    X( PFNGLUNMAPNAMEDBUFFERPROC, "glUnmapNamedBuffer", UnmapNamedBuffer )                                                       \
    X( PFNGLCOPYNAMEDBUFFERSUBDATAPROC, "glCopyNamedBufferSubData", CopyNamedBufferSubData )                                     \
    X( PFNGLBINDBUFFERBASEPROC, "glBindBufferBase", BindBufferBase )                                                             \
    X( PFNGLCREATETEXTURESPROC, "glCreateTextures", CreateTextures )                                                             \
    X( PFNGLDELETETEXTURESPROC, "glDeleteTextures", DeleteTextures )                                                             \
    X( PFNGLTEXTURESTORAGE2DPROC, "glTextureStorage2D", TextureStorage2D )                                                       \
    X( PFNGLTEXTURESUBIMAGE2DPROC, "glTextureSubImage2D", TextureSubImage2D )                                                    \
    X( PFNGLGETTEXTURESUBIMAGEPROC, "glGetTextureSubImage", GetTextureSubImage )                                                 \
    X( PFNGLTEXTUREPARAMETERIPROC, "glTextureParameteri", TextureParameteri )                                                    \
    X( PFNGLGENERATETEXTUREMIPMAPPROC, "glGenerateTextureMipmap", GenerateTextureMipmap )                                        \
    X( PFNGLBINDTEXTUREUNITPROC, "glBindTextureUnit", BindTextureUnit )                                                          \
    X( PFNGLBINDIMAGETEXTUREPROC, "glBindImageTexture", BindImageTexture )                                                       \
    X( PFNGLCREATESAMPLERSPROC, "glCreateSamplers", CreateSamplers )                                                             \
    X( PFNGLDELETESAMPLERSPROC, "glDeleteSamplers", DeleteSamplers )                                                             \
    X( PFNGLSAMPLERPARAMETERIPROC, "glSamplerParameteri", SamplerParameteri )                                                    \
    X( PFNGLBINDSAMPLERPROC, "glBindSampler", BindSampler )                                                                      \
    X( PFNGLCREATEFRAMEBUFFERSPROC, "glCreateFramebuffers", CreateFramebuffers )                                                 \
    X( PFNGLDELETEFRAMEBUFFERSPROC, "glDeleteFramebuffers", DeleteFramebuffers )                                                 \
    X( PFNGLNAMEDFRAMEBUFFERTEXTUREPROC, "glNamedFramebufferTexture", NamedFramebufferTexture )                                  \
    X( PFNGLNAMEDFRAMEBUFFERDRAWBUFFERSPROC, "glNamedFramebufferDrawBuffers", NamedFramebufferDrawBuffers )                      \
    X( PFNGLCHECKNAMEDFRAMEBUFFERSTATUSPROC, "glCheckNamedFramebufferStatus", CheckNamedFramebufferStatus )                      \
    X( PFNGLBINDFRAMEBUFFERPROC, "glBindFramebuffer", BindFramebuffer )                                                          \
    X( PFNGLCREATEVERTEXARRAYSPROC, "glCreateVertexArrays", CreateVertexArrays )                                                 \
    X( PFNGLDELETEVERTEXARRAYSPROC, "glDeleteVertexArrays", DeleteVertexArrays )                                                 \
    X( PFNGLBINDVERTEXARRAYPROC, "glBindVertexArray", BindVertexArray )                                                          \
    X( PFNGLVERTEXARRAYVERTEXBUFFERPROC, "glVertexArrayVertexBuffer", VertexArrayVertexBuffer )                                  \
    X( PFNGLVERTEXARRAYELEMENTBUFFERPROC, "glVertexArrayElementBuffer", VertexArrayElementBuffer )                               \
    X( PFNGLENABLEVERTEXARRAYATTRIBPROC, "glEnableVertexArrayAttrib", EnableVertexArrayAttrib )                                  \
    X( PFNGLVERTEXARRAYATTRIBFORMATPROC, "glVertexArrayAttribFormat", VertexArrayAttribFormat )                                  \
    X( PFNGLVERTEXARRAYATTRIBIFORMATPROC, "glVertexArrayAttribIFormat", VertexArrayAttribIFormat )                               \
    X( PFNGLVERTEXARRAYATTRIBBINDINGPROC, "glVertexArrayAttribBinding", VertexArrayAttribBinding )                               \
    X( PFNGLCREATESHADERPROC, "glCreateShader", CreateShader )                                                                   \
    X( PFNGLSHADERSOURCEPROC, "glShaderSource", ShaderSource )                                                                   \
    X( PFNGLCOMPILESHADERPROC, "glCompileShader", CompileShader )                                                                \
    X( PFNGLGETSHADERIVPROC, "glGetShaderiv", GetShaderiv )                                                                      \
    X( PFNGLGETSHADERINFOLOGPROC, "glGetShaderInfoLog", GetShaderInfoLog )                                                       \
    X( PFNGLDELETESHADERPROC, "glDeleteShader", DeleteShader )                                                                   \
    X( PFNGLCREATEPROGRAMPROC, "glCreateProgram", CreateProgram )                                                                \
    X( PFNGLATTACHSHADERPROC, "glAttachShader", AttachShader )                                                                   \
    X( PFNGLDETACHSHADERPROC, "glDetachShader", DetachShader )                                                                   \
    X( PFNGLLINKPROGRAMPROC, "glLinkProgram", LinkProgram )                                                                      \
    X( PFNGLGETPROGRAMIVPROC, "glGetProgramiv", GetProgramiv )                                                                   \
    X( PFNGLGETPROGRAMINFOLOGPROC, "glGetProgramInfoLog", GetProgramInfoLog )                                                    \
    X( PFNGLDELETEPROGRAMPROC, "glDeleteProgram", DeleteProgram )                                                                \
    X( PFNGLUSEPROGRAMPROC, "glUseProgram", UseProgram )                                                                         \
    X( PFNGLGETUNIFORMLOCATIONPROC, "glGetUniformLocation", GetUniformLocation )                                                 \
    X( PFNGLPROGRAMUNIFORM1IPROC, "glProgramUniform1i", ProgramUniform1i )                                                       \
    X( PFNGLPROGRAMUNIFORM1UIPROC, "glProgramUniform1ui", ProgramUniform1ui )                                                    \
    X( PFNGLPROGRAMUNIFORM1FPROC, "glProgramUniform1f", ProgramUniform1f )                                                       \
    X( PFNGLPROGRAMUNIFORM2FPROC, "glProgramUniform2f", ProgramUniform2f )                                                       \
    X( PFNGLPROGRAMUNIFORM3FPROC, "glProgramUniform3f", ProgramUniform3f )                                                       \
    X( PFNGLPROGRAMUNIFORM4FPROC, "glProgramUniform4f", ProgramUniform4f )                                                       \
    X( PFNGLPROGRAMUNIFORMMATRIX4FVPROC, "glProgramUniformMatrix4fv", ProgramUniformMatrix4fv )                                  \
    X( PFNGLDRAWARRAYSPROC, "glDrawArrays", DrawArrays )                                                                         \
    X( PFNGLDRAWELEMENTSPROC, "glDrawElements", DrawElements )                                                                   \
    X( PFNGLDISPATCHCOMPUTEPROC, "glDispatchCompute", DispatchCompute )                                                          \
    X( PFNGLCULLFACEPROC, "glCullFace", CullFace )                                                                               \
    X( PFNGLDEPTHFUNCPROC, "glDepthFunc", DepthFunc )                                                                            \
    X( PFNGLDEPTHMASKPROC, "glDepthMask", DepthMask )                                                                            \
    X( PFNGLBLENDFUNCSEPARATEPROC, "glBlendFuncSeparate", BlendFuncSeparate )                                                    \
    X( PFNGLFENCESYNCPROC, "glFenceSync", FenceSync )                                                                            \
    X( PFNGLDELETESYNCPROC, "glDeleteSync", DeleteSync )                                                                         \
    X( PFNGLCLIENTWAITSYNCPROC, "glClientWaitSync", ClientWaitSync )

namespace Lumina
{

namespace RHI
{

    namespace Internal
    {

#define RHI_GL_DECLARE( Type, NativeName, Name ) extern Type Name;
        RHI_GL_FUNCTION_LIST( RHI_GL_DECLARE )
#undef RHI_GL_DECLARE

        /**
         * @brief Loads the OpenGL functions once, publishing the populated table to all later callers.
         * @return Success, or an error if a required OpenGL 4.6 function is unavailable.
         */
        [[nodiscard]] FRHIExpected LoadGLFunctions () noexcept;

    } // namespace Internal

} // namespace RHI

} // namespace Lumina
