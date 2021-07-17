// TODO: Use OPENGL_GL_INCLUDED and declare gl types and function prototypes if necessary

#ifndef OPENGL_H
#define OPENGL_H

#include "types.h"
#include "config.h"

#include "opengl_internal.h"

#ifndef APIENTRY
#define APIENTRY GLAPIENTRY
#endif
#ifndef APIENTRYP
#define APIENTRYP APIENTRY*
#endif
#ifndef GLAPI
#define GLAPI extern
#endif
#ifndef GLAPIENTRY
#define GLAPIENTRY
#endif
#ifndef GLAPIENTRYP
#define GLAPIENTRYP GLAPIENTRY*
#endif

#ifndef OPENGL_GL_INCLUDED
// TODO: Define normal GL types and declare GL function prototypes here!
#endif //OPENGL_GL_INCLUDED

// Extended types
typedef char GLchar;
typedef i32  GLfixed;
typedef iptr GLintptr;
typedef iptr GLsizeiptr;
typedef i64  GLint64;
typedef u16  GLhalf;
typedef u64  GLuint64;

// Extension function list, define DEFEXT(_type, _name, ...) before using this macro
#define OPENGL_EXTFUNC_LIST												\
	DEFEXT(GLuint,			CreateShader,				GLenum)			\
	DEFEXT(void,			ShaderSource,				GLuint, GLsizei, const GLchar**, const GLint*) \
	DEFEXT(void,			CompileShader,				GLuint)			\
	DEFEXT(void,			GetShaderiv,				GLuint, GLenum, GLint*)	\
	DEFEXT(void,			GetShaderInfoLog,			GLuint, GLsizei, GLsizei*, GLchar*)	\
	DEFEXT(GLuint,			CreateProgram,				void)			\
	DEFEXT(void,			AttachShader,				GLuint, GLuint)	\
	DEFEXT(void,			LinkProgram,				GLuint)			\
	DEFEXT(void,			GetProgramiv,				GLuint, GLenum, GLint*)	\
	DEFEXT(void,			GetProgramInfoLog,			GLuint, GLsizei, GLsizei*, GLchar*)	\
	DEFEXT(void,			DeleteShader,				GLuint)			\
	DEFEXT(void,			DetachShader,				GLuint, GLuint)	\
	DEFEXT(void,			DeleteProgram,				GLuint)			\
	DEFEXT(void,			BindAttribLocation,			GLuint, GLuint, const GLchar*) \
	DEFEXT(void,			GenVertexArrays,			GLsizei, GLuint*) \
	DEFEXT(void,			GenBuffers,					GLsizei, GLuint*) \
	DEFEXT(void,			BindBuffer,					GLenum, GLuint)	\
	DEFEXT(void,			BindVertexArray,			GLuint)			\
	DEFEXT(void,			BufferData,					GLenum, GLsizeiptr, const GLvoid*, GLenum) \
	DEFEXT(void,			VertexAttribPointer,		GLuint, GLint, GLenum, GLboolean, GLsizei, const GLvoid*) \
	DEFEXT(void,			EnableVertexAttribArray,	GLuint)			\
	DEFEXT(void,			DisableVertexAttribArray,	GLuint)			\
	DEFEXT(void,			DeleteBuffers,				GLsizei, const GLuint*)	\
	DEFEXT(void,			DeleteVertexArrays,			GLsizei, const GLuint*)	\
	DEFEXT(void,			UseProgram,					GLuint)			\
	DEFEXT(void,			ActiveTexture,				GLenum)			\
	DEFEXT(void,			GenerateMipmap,				GLenum)			\
	DEFEXT(void,			BufferSubData,				GLenum, GLintptr, GLsizeiptr, const GLvoid*) \
	DEFEXT(void,			BindBufferRange,			GLenum, GLuint, GLuint, GLintptr, GLsizeiptr) \
	DEFEXT(GLuint,			GetUniformBlockIndex,		GLuint, const GLchar*) \
	DEFEXT(void,			UniformBlockBinding,		GLuint, GLuint, GLuint)	\
	DEFEXT(const GLubyte*,	GetStringi,                 GLenum, GLuint)	\
	DEFEXT(GLint,			GetUniformLocation,			GLuint, const GLchar*) \
	DEFEXT(void,			Uniform1i,					GLint, GLint)	\
	DEFEXT(void,            Uniform2f,                  GLint, GLfloat, GLfloat) \
	DEFEXT(void,			GetBufferSubData,			GLenum, GLintptr, GLsizeiptr, void*) \
	DEFEXT(void*,			MapBuffer,					GLenum, GLenum)	\
	DEFEXT(GLboolean,		UnmapBuffer,				GLenum)

namespace GL {
	// ******************
	//  GL enum values
	// ******************
	enum enumVal {
		///////////////////
		// Boolean values
		FALSE	= 0,
		ZERO	= 0,
		TRUE	= 1,
		ONE		= 1,
		
		//////////////
		// Types
		BYTE			= 0x1400,
		UNSIGNED_BYTE	= 0x1401,
		SHORT			= 0x1402,
		UNSIGNED_SHORT	= 0x1403,
		INT				= 0x1404,
		UNSIGNED_INT	= 0x1405,
		FLOAT			= 0x1406,
		DOUBLE			= 0x140a,
		HALF_FLOAT		= 0x140b,

		////////////////////////
		// OpenGL Capabilities
		BLEND = 0x0be2,

		////////////////////////////
		// Blend function factors
		SRC_ALPHA           = 0x0302,
		ONE_MINUS_SRC_ALPHA = 0x0303,
		
		//////////////////////
		// Pixel buffer bits
		COLOR_BUFFER_BIT	= 0x4000,
		DEPTH_BUFFER_BIT	= 0x0100,
		STENCIL_BUFFER_BIT	= 0x0400,
		
		////////////////////
		// Buffer targets
		ARRAY_BUFFER				= 0x8892,
		ATOMIC_COUNTER_BUFFER		= 0x92c0,
		COPY_READ_BUFFER			= 0x8f36,
		COPY_WRITE_BUFFER			= 0x8f37,
		DISPATCH_INDIRECT_BUFFER	= 0x90ee,
		DRAW_INDIRECT_BUFFER		= 0x8f3f,
		ELEMENT_ARRAY_BUFFER		= 0x8893,
		PIXEL_PACK_BUFFER			= 0x88eb,
		PIXEL_UNPACK_BUFFER			= 0x88ec,
		QUERY_BUFFER				= 0x9192,
		SHADER_STORAGE_BUFFER		= 0x90d2,
		TEXTURE_BUFFER				= 0x8c2a,
		TRANSFORM_FEEDBACK_BUFFER	= 0x8c8e,
		UNIFORM_BUFFER				= 0x8a11,
		
		///////////////////////
		// Buffer data usages
		STATIC_DRAW		= 0x88e4,
		STATIC_READ		= 0x88e5,
		STATIC_COPY		= 0x88e6,
		DYNAMIC_DRAW	= 0x88e8,
		DYNAMIC_READ	= 0x88e9,
		DYNAMIC_COPY	= 0x88ea,
		STREAM_DRAW		= 0x88e0,
		STREAM_READ		= 0x88e1,
		STREAM_COPY		= 0x88e2,
		
		////////////////////
		// Primitives
		POINTS						= 0x0000,
		LINE_STRIP					= 0x0003,
		LINE_LOOP					= 0x0002,
		LINES						= 0x0001,
		LINE_STRIP_ADJACENCY		= 0x000b,
		LINES_ADJACENCY				= 0x000a,
		TRIANGLE_STRIP				= 0x0005,
		TRIANGLE_FAN				= 0x0006,
		TRIANGLES					= 0x0004,
		TRIANGLE_STRIP_ADJACENCY	= 0x000d,
		TRIANGLES_ADJACENCY			= 0x000c,
		PATCHES						= 0x000e,
		
		//////////////////////////
		// Comparison functions
		LEQUAL		= 0x0203,
		GEQUAL		= 0x0206,
		LESS		= 0x0201,
		GREATER		= 0x0204,
		EQUAL		= 0x0202,
		NOTEQUAL	= 0x0205,
		ALWAYS		= 0x0207,
		NEVER		= 0x0200,
		
		///////////////////
		// Shader types
		GEOMETRY_SHADER	= 0x8dd9,
		FRAGMENT_SHADER	= 0x8b30,
		VERTEX_SHADER	= 0x8b31,
		
		//////////////////////////
		// Shader parameters
		SHADER_TYPE				= 0x8b4f,
		COMPILE_STATUS			= 0x8b81,
		DELETE_STATUS			= 0x8b80,
		INFO_LOG_LENGTH			= 0x8b84,
		SHADER_SOURCE_LENGTH	= 0x8b88,
		
		///////////////////////////////
		// Shader program parameters
//		DELETE_STATUS							= 0x8b80,
		LINK_STATUS								= 0x8b82,
		VALIDATE_STATUS							= 0x8b83,
//		INFO_LOG_LENGTH							= 0x8b84,
		ATTACHED_SHADERS						= 0x8b85,
		ACTIVE_ATOMIC_COUNTER_BUFFERS			= 0x92d9,
		ACTIVE_ATTRIBUTES						= 0x8b89,
		ACTIVE_ATTRIBUTE_MAX_LENGTH				= 0x8b8a,
		ACTIVE_UNIFORMS							= 0x8b86,
		ACTIVE_UNIFORM_BLOCKS					= 0x8a36,
		ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH	= 0x8a35,
		ACTIVE_UNIFORM_MAX_LENGTH				= 0x8b87,
		COMPUTE_WORK_GROUP_SIZE					= 0x8267,
		PROGRAM_BINARY_LENGTH					= 0x8741,
		TRANSFORM_FEEDBACK_BUFFER_MODE			= 0x8c7f,
		TRANSFORM_FEEDBACK_VARYINGS				= 0x8c83,
		TRANSFORM_FEEDBACK_VARYING_MAX_LENGTH	= 0x8c76,
		GEOMETRY_VERTICES_OUT					= 0x8916,
		GEOMETRY_INPUT_TYPE						= 0x8917,
		GEOMETRY_OUTPUT_TYPE					= 0x8918,

		/////////////////////////////////
		// GetString parameters
		VENDOR                   = 0x1f00,
		RENDERER                 = 0x1f01,
		VERSION                  = 0x1f02,
		SHADING_LANGUAGE_VERSION = 0x8b8c,

		////////////////////////////////
		// GetIntegerv parameters

		// Buffer bindings
		ARRAY_BUFFER_BINDING         = 0x8894,
		ELEMENT_ARRAY_BUFFER_BINDING = 0x8895,
		PIXEL_UNPACK_BUFFER_BINDING  = 0x88ef,
		UNIFORM_BUFFER_BINDING       = 0x8a28,
		
		//////////////////////////////////
		// 2-dimensional texture targets
		TEXTURE_2D					= 0x0de1,
		PROXY_TEXTURE_2D			= 0x8064,
		TEXTURE_1D_ARRAY			= 0x8c18,
		PROXY_TEXTURE_1D_ARRAY		= 0x8c19,
		TEXTURE_RECTANGLE			= 0x84f5,
		PROXY_TEXTURE_RECTANGLE		= 0x84f7,
		TEXTURE_CUBE_MAP_POSITIVE_X = 0x8515,
		TEXTURE_CUBE_MAP_NEGATIVE_X	= 0x8516,
		TEXTURE_CUBE_MAP_POSITIVE_Y = 0x8517,
		TEXTURE_CUBE_MAP_NEGATIVE_Y = 0x8518,
		TEXTURE_CUBE_MAP_POSITIVE_Z	= 0x8519,
		TEXTURE_CUBE_MAP_NEGATIVE_Z	= 0x851a,
		PROXY_TEXTURE_CUBE_MAP		= 0x851b,
		
		////////////////////////////////
		// Texture parameters
		DEPTH_STENCIL_TEXTURE_MODE		= 0x90ea,
		TEXTURE_MAG_FILTER				= 0x2800,
		TEXTURE_MIN_FILTER				= 0x2801,
		TEXTURE_MIN_LOD					= 0x813a,
		TEXTURE_MAX_LOD					= 0x813b,
		TEXTURE_BASE_LEVEL				= 0x813c,
		TEXTURE_MAX_LEVEL				= 0x813d,
		TEXTURE_SWIZZLE_R				= 0x8e42,
		TEXTURE_SWIZZLE_G				= 0x8e43,
		TEXTURE_SWIZZLE_B				= 0x8e44,
		TEXTURE_SWIZZLE_A				= 0x8e45,
		TEXTURE_SWIZZLE_RGBA			= 0x8e46,
		TEXTURE_WRAP_S					= 0x2802,
		TEXTURE_WRAP_T					= 0x2803,
		TEXTURE_WRAP_R					= 0x8072,
		TEXTURE_BORDER_COLOR			= 0x1004,
		TEXTURE_COMPARE_MODE			= 0x884c,
		TEXTURE_COMPARE_FUNC			= 0x884d,
		TEXTURE_VIEW_MIN_LEVEL			= 0x82db,
		TEXTURE_VIEW_NUM_LEVELS			= 0x82dc,
		TEXTURE_VIEW_MIN_LAYER			= 0x82dd,
		TEXTURE_VIEW_NJM_LAYERS			= 0x82de,
		TEXTURE_IMMUTABLE_LEVELS		= 0x82df,
		IMAGE_FORMAT_COMPATIBILITY_TYPE	= 0x90c7,
		TEXTURE_IMMUTABLE_FORMAT		= 0x912f,
		TEXTURE_TARGET					= 0x1006,
		TEXTURE_LOD_BIAS				= 0x8501,
		
		//////////////////////////////
		// Texture parameter values
		DEPTH_COMPONENT	= 0x1902,
		STENCIL_INDEX	= 0x1901,
		
		COMPARE_REF_TO_TEXTURE	= 0x884e,
		
		MAX_TEXTURE_LOD_BIAS	= 0x84fd,
		
		NEAREST					= 0x2600,
		LINEAR					= 0x2601,
		NEAREST_MIPMAP_NEAREST	= 0x2700,
		NEAREST_MIPMAP_LINEAR	= 0x2702,
		LINEAR_MIPMAP_NEAREST	= 0x2701,
		LINEAR_MIPMAP_LINEAR	= 0x2703,
		
		RED		= 0x1903,
		GREEN	= 0x1904,
		BLUE	= 0x1905,
		ALPHA	= 0x1906,
		
		CLAMP_TO_EDGE			= 0x812f,
		CLAMP_TO_BORDER			= 0x812d,
		MIRRORED_REPEAT			= 0x8370,
		REPEAT					= 0x2901,
		MIRROR_CLAMP_TO_EDGE	= 0x8743,
		
		/////////////////////
		// Texel formats
//		RED				= 0x1903,
		RG				= 0x8227,
		RGB				= 0x1907,
		BGR				= 0x80e0,
		RGBA			= 0x1908,
		BGRA			= 0x80e1,
		RED_INTEGER		= 0x8d94,
		RG_INTEGER		= 0x8228,
		RGB_INTEGER		= 0x8d98,
		BGR_INTEGER		= 0x8d9a,
		RGBA_INTEGER	= 0x8d99,
		BGRA_INTEGER	= 0x8d9b,
//		STENCIL_INDEX	= 0x1901,
//		DEPTH_COMPONENT	= 0x1902,
		DEPTH_STENCIL	= 0x84f9,
		
		/////////////////////////////////
		// Sized internal texel formats
		R8				= 0x8229,
		R8_SNORM		= 0x8f94,
		R16				= 0x822a,
		R16_SNORM		= 0x8f98,
		R3_G3_B2		= 0x2a10,
		RGB4			= 0x804f,
		RGB5			= 0x8050,
		RGB8			= 0x8051,
		RGB8_SNORM		= 0x8f96,
		RGB10			= 0x8052,
		RGB12			= 0x8053,
		RGB16_SNORM		= 0x8f9a,
		RGBA2			= 0x8055,
		RGBA4			= 0x8056,
		RGB5_A1			= 0x8057,
		RGBA8			= 0x8058,
		RGBA8_SNORM		= 0x8f97,
		RGB10_A2		= 0x8059,
		RGB10_A2UI		= 0x906f,
		RGBA12			= 0x805a,
		RGBA16			= 0x805b,
		SRGB8			= 0x8c41,
		SRGB8_ALPHA8	= 0x8c43,
		R16F			= 0x822d,
		RG16F			= 0x822f,
		RGB16F			= 0x881b,
		RGBA16F			= 0x881a,
		R32F			= 0x822e,
		RG32F			= 0x8230,
		RGB32F			= 0x8815,
		RGBA32F			= 0x8814,
		R11F_G11F_B10F	= 0x8c3a,
		RGB9_E5			= 0x8c3d,
		R8I				= 0x8231,
		R8UI			= 0x8232,
		R16I			= 0x8233,
		R16UI			= 0x8234,
		R32I			= 0x8235,
		R32UI			= 0x8236,
		RG8I			= 0x8237,
		RG8UI			= 0x8238,
		RG16I			= 0x8239,
		RG16UI			= 0x823a,
		RG32I			= 0x823b,
		RG32UI			= 0x823c,
		RGB8I			= 0x8d8f,
		RGB8UI			= 0x8d7d,
		RGB16I			= 0x8d89,
		RGB16UI			= 0x8d77,
		RGB32I			= 0x8d83,
		RGB32UI			= 0x8d71,
		RGBA8I			= 0x8d8e,
		RGBA8UI			= 0x8d7c,
		RGBA16I			= 0x8d88,
		RGBA16UI		= 0x8d76,
		RGBA32I			= 0x8d82,
		RGBA32UI		= 0x8d70,
		
		////////////////////////////////////////
		// Compressed internal texel formats
		COMPRESSED_RED						= 0x8225,
		COMPRESSED_RG						= 0x8226,
		COMPRESSED_RGB						= 0x84ed,
		COMPRESSED_RGBA						= 0x84ee,
		COMPRESSED_SRGB						= 0x8c48,
		COMPRESSED_SRGB_ALPHA				= 0x8c49,
		COMPRESSED_RED_RGTC1				= 0x8dbb,
		COMPRESSED_SIGNED_RED_RGTC1			= 0x8dbc,
		COMPRESSED_RG_RGTC2					= 0x8dbd,
		COMPRESSED_SIGNED_RG_RGTC2			= 0x8dbe,
		COMPRESSED_RGBA_BPTC_UNORM			= 0x8e8c,
		COMPRESSED_SRGB_ALPHA_BPTC_UNORM	= 0x8e8d,
		COMPRESSED_RGB_BPTC_SIGNED_FLOAT	= 0x8e8e,
		COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT	= 0x8e8f,
		
		////////////////////////////
		// Texel types
		UNSIGNED_BYTE_3_3_2			= 0x8032,
		UNSIGNED_BYTE_2_3_3_REV		= 0x8362,
		UNSIGNED_SHORT_5_6_5		= 0x8363,
		UNSIGNED_SHORT_5_6_5_REV	= 0x8364,
		UNSIGNED_SHORT_4_4_4_4		= 0x8033,
		UNSIGNED_SHORT_4_4_4_4_REV	= 0x8365,
		UNSIGNED_SHORT_5_5_5_1		= 0x8034,
		UNSIGNED_SHORT_1_5_5_5_REV	= 0x8366,
		UNSIGNED_INT_8_8_8_8		= 0x8035,
		UNSIGNED_INT_8_8_8_8_REV	= 0x8367,
		UNSIGNED_INT_10_10_10_2		= 0x8036,
		UNSIGNED_INT_2_10_10_10_REV	= 0x8368,
		
		/////////////////
		// Error codes
		NO_ERROR						= 0,
		INVALID_ENUM					= 0x500,
		INVALID_VALUE					= 0x501,
		INVALID_OPERATION				= 0x502,
		STACK_OVERFLOW					= 0x503,
		STACK_UNDERFLOW					= 0x504,
		OUT_OF_MEMORY					= 0x505,
		INVALID_FRAMEBUFFER_OPERATION	= 0x506,
		CONTEXT_LOST					= 0x507,

		/////////////////////
		// Error values
		INVALID_INDEX = 0xffffffff, // returned by GL::GetUniformBlockIndex as an error
		
		//////////////////////
		// Misc
		NONE	= 0,
	};
	
	// *****************************
	//  Library function wrappers
	// *****************************
#	define DEFNLIB0(type, name)																				\
		static FINLINE type name() {																		\
			return gl ## name();																			\
		}
#	define DEFNLIB1(type, name, a1t)																		\
		static FINLINE type name(a1t a1) {																	\
			return gl ## name(a1);																			\
		}
#	define DEFNLIB2(type, name, a1t, a2t)																	\
		static FINLINE type name(a1t a1, a2t a2) {															\
			return gl ## name(a1, a2);																		\
		}
#	define DEFNLIB3(type, name, a1t, a2t, a3t)																\
		static FINLINE type name(a1t a1, a2t a2, a3t a3) {													\
			return gl ## name(a1, a2, a3);																	\
		}
#	define DEFNLIB4(type, name, a1t, a2t, a3t, a4t)															\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4) {											\
			return gl ## name(a1, a2, a3, a4);																\
		}
#	define DEFNLIB5(type, name, a1t, a2t, a3t, a4t, a5t)													\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5) {									\
			return gl ## name(a1, a2, a3, a4, a5);															\
		}
#	define DEFNLIB6(type, name, a1t, a2t, a3t, a4t, a5t, a6t)												\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6) {							\
			return gl ## name(a1, a2, a3, a4, a5, a6);														\
		}
#	define DEFNLIB7(type, name, a1t, a2t, a3t, a4t, a5t, a6t, a7t)											\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7) {					\
			return gl ## name(a1, a2, a3, a4, a5, a6, a7);													\
		}
#	define DEFNLIB8(type, name, a1t, a2t, a3t, a4t, a5t, a6t, a7t, a8t)										\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7, a8t a8) {			\
			return gl ## name(a1, a2, a3, a4, a5, a6, a7, a8);												\
		}
#	define DEFNLIB9(type, name, a1t, a2t, a3t, a4t, a5t, a6t, a7t, a8t, a9t)								\
		static FINLINE type name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7, a8t a8, a9t a9) {	\
			return gl ## name(a1, a2, a3, a4, a5, a6, a7, a8, a9);											\
		}

#	define VOIDLIB0(name)																					\
		static FINLINE void name() {																		\
			gl ## name();																					\
		}
#	define VOIDLIB1(name, a1t)																				\
		static FINLINE void name(a1t a1) {																	\
			gl ## name(a1);																					\
		}
#	define VOIDLIB2(name, a1t, a2t)																			\
		static FINLINE void name(a1t a1, a2t a2) {															\
			gl ## name(a1, a2);																				\
		}
#	define VOIDLIB3(name, a1t, a2t, a3t)																	\
		static FINLINE void name(a1t a1, a2t a2, a3t a3) {													\
			gl ## name(a1, a2, a3);																			\
		}
#	define VOIDLIB4(name, a1t, a2t, a3t, a4t)																\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4) {											\
			gl ## name(a1, a2, a3, a4);																		\
		}
#	define VOIDLIB5(name, a1t, a2t, a3t, a4t, a5t)															\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5) {									\
			gl ## name(a1, a2, a3, a4, a5);																	\
		}
#	define VOIDLIB6(name, a1t, a2t, a3t, a4t, a5t, a6t)														\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6) {							\
			gl ## name(a1, a2, a3, a4, a5, a6);																\
		}
#	define VOIDLIB7(name, a1t, a2t, a3t, a4t, a5t, a6t, a7t)												\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7) {					\
			gl ## name(a1, a2, a3, a4, a5, a6, a7);															\
		}
#	define VOIDLIB8(name, a1t, a2t, a3t, a4t, a5t, a6t, a7t, a8t)											\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7, a8t a8) {			\
			gl ## name(a1, a2, a3, a4, a5, a6, a7, a8);														\
		}
#	define VOIDLIB9(name, a1t, a2t, a3t, a4t, a5t, a6t, a7t, a8t, a9t)										\
		static FINLINE void name(a1t a1, a2t a2, a3t a3, a4t a4, a5t a5, a6t a6, a7t a7, a8t a8, a9t a9) {	\
			gl ## name(a1, a2, a3, a4, a5, a6, a7, a8, a9);													\
		}
	
	VOIDLIB2(BindTexture,		GLenum, GLuint)
	VOIDLIB1(Clear,				GLbitfield)
	VOIDLIB4(ClearColor,		GLfloat, GLfloat, GLfloat, GLfloat)
	VOIDLIB2(DeleteTextures,	GLsizei, const GLuint*)
	VOIDLIB3(DrawArrays,		GLenum, GLint, GLsizei)
	VOIDLIB2(GenTextures,		GLsizei, GLuint*)
	VOIDLIB9(TexImage2D,		GLenum, GLint, GLint, GLsizei, GLsizei, GLint, GLenum, GLenum, const void*)
	VOIDLIB9(TexSubImage2D,     GLenum, GLint, GLint, GLint, GLsizei, GLsizei, GLenum, GLenum, const void*)
	VOIDLIB3(TexParameteri,		GLenum, GLenum, GLint)
	VOIDLIB4(Viewport,			GLint, GLint, GLsizei, GLsizei)
	VOIDLIB4(DrawElements,      GLenum, GLsizei, GLenum, const void*)
	VOIDLIB1(Enable,            GLenum)
	VOIDLIB2(BlendFunc,         GLenum, GLenum)
	VOIDLIB2(GetIntegerv,       GLenum, GLint*)

	DEFNLIB0(GLenum,         GetError)
	DEFNLIB1(const GLubyte*, GetString, GLenum)

#	undef DEFNLIB0
#	undef DEFNLIB1
#	undef DEFNLIB2
#	undef DEFNLIB3
#	undef DEFNLIB4
#	undef DEFNLIB5
#	undef DEFNLIB6
#	undef DEFNLIB7
#	undef DEFNLIB8
#	undef DEFNLIB9
#	undef VOIDLIB0
#	undef VOIDLIB1
#	undef VOIDLIB2
#	undef VOIDLIB3
#	undef VOIDLIB4
#	undef VOIDLIB5
#	undef VOIDLIB6
#	undef VOIDLIB7
#	undef VOIDLIB8
#	undef VOIDLIB9
	
	// *********************
	//  Extension functions
	// *********************
#	define DEFEXT(_type, _name, ...)						\
		typedef _type (APIENTRYP _name ## _t)(__VA_ARGS__);	\
		extern _name ## _t _name;
	
	OPENGL_EXTFUNC_LIST
	
#	undef DEFEXT
	
	// Extension loading function, platform dependent
	// Returns false if functions failed to load
	// TODO: I should look into putting the extension functions
	//       into a structure, instead of leaving them global.
	ubool loadExt();
}

#endif //OPENGL_H
