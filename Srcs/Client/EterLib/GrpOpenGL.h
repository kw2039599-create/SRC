#pragma once

#ifdef ENABLE_OPENGL
// Block both DirectX 8 and DirectX 9 headers (uppercase and lowercase guards)
#define _D3D_H_
#define _d3d_H_
#define _D3D8_H_
#define _d3d8_H_
#define _D3D8TYPES_H_
#define _d3d8TYPES_H_
#define _D3D8CAPS_H_
#define _D3D8CAPS_H
#define _d3d8CAPS_H_
#define _d3d8CAPS_H
#define __D3DX8_H__
#define __D3DX8MATH_H__
#define __D3DX8MATH_INL__
#define __D3DX8CORE_H__
#define __D3DX8TEX_H__
#define __D3DX8MESH_H__
#define __D3DX8EFFECT_H__
#define __D3DX8SHAPES_H__
#define __D3DX8ASM_H__

#define _D3D9_H_
#define _d3d9_H_
#define _D3D9TYPES_H_
#define _d3d9TYPES_H_
#define _D3D9CAPS_H_
#define _D3D9CAPS_H
#define _d3d9CAPS_H_
#define _d3d9CAPS_H
#define __D3DX9_H__
#define __D3DX9MATH_H__
#define __D3DX9MATH_H___
#define __D3DX9MATH_INL__
#define __D3DX9CORE_H__
#define __D3DX9TEX_H__
#define __D3DX9MESH_H__
#define __D3DX9EFFECT_H__
#define __D3DX9SHAPES_H__
#define __D3DX9ASM_H__
#define __D3DX9ANIM_H__
#define __D3DX9SHADER_H__
#define __D3DX9XOF_H__
#endif

#if defined(ANDROID) || defined(__ANDROID__) || defined(__APPLE__)
	#ifndef ANDROID
		#define ANDROID 1
	#endif
	#ifndef __ANDROID__
		#define __ANDROID__ 1
	#endif
	#ifndef __declspec
		#define __declspec(x)
	#endif
	#include <windows.h>
	#ifdef __APPLE__
		#include <OpenGLES/ES3/gl.h>
		#include <OpenGLES/ES3/glext.h>
	#else
		#include <GLES3/gl3.h>
		#include <GLES2/gl2ext.h>
	#endif
	#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__) || defined(__arm64__)
		#include <arm_neon.h>
	#endif
	void NotifyAndroidGLBufferDeleted(GLuint vbo, GLuint ibo);
#else
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#include <windows.h>
	#ifndef DIRECTINPUT_VERSION
		#define DIRECTINPUT_VERSION 0x0800
	#endif
	#include <dinput.h>
	#ifndef WINGDIAPI
		#define WINGDIAPI __declspec(dllimport)
	#endif
	#ifndef APIENTRY
		#define APIENTRY __stdcall
	#endif
	#include <GL/gl.h>
	#include <il/il.h>
	#ifndef GL_BGRA_EXT
		#define GL_BGRA_EXT 0x80E1
	#endif
	#ifndef IL_ORIGIN_SET
		#define IL_ORIGIN_SET 0x0600
		#define IL_ORIGIN_UPPER_LEFT 0x0602
	#endif
	#include <stddef.h>
	#pragma comment(lib, "opengl32.lib")

	typedef ptrdiff_t GLsizeiptr;
	typedef ptrdiff_t GLintptr;

	#ifndef GL_ARRAY_BUFFER
		#define GL_ARRAY_BUFFER 0x8892
	#endif
	#ifndef GL_ELEMENT_ARRAY_BUFFER
		#define GL_ELEMENT_ARRAY_BUFFER 0x8893
	#endif
	#ifndef GL_STATIC_DRAW
		#define GL_STATIC_DRAW 0x88E4
	#endif
	#ifndef GL_DYNAMIC_DRAW
		#define GL_DYNAMIC_DRAW 0x88E8
	#endif
	#ifndef GL_TEXTURE0
		#define GL_TEXTURE0 0x84C0
	#endif
	#ifndef GL_TEXTURE1
		#define GL_TEXTURE1 0x84C1
	#endif
	#ifndef GL_CLAMP_TO_EDGE
		#define GL_CLAMP_TO_EDGE 0x812F
	#endif
	#ifndef GL_GENERATE_MIPMAP
		#define GL_GENERATE_MIPMAP 0x8191
	#endif
	#ifndef GL_TEXTURE_MAX_ANISOTROPY_EXT
		#define GL_TEXTURE_MAX_ANISOTROPY_EXT 0x84FE
	#endif
	#ifndef GL_COMBINE
		#define GL_COMBINE 0x8570
		#define GL_COMBINE_RGB 0x8571
		#define GL_COMBINE_ALPHA 0x8572
		#define GL_RGB_SCALE 0x8573
		#define GL_ADD_SIGNED 0x8574
		#define GL_INTERPOLATE 0x8575
		#define GL_CONSTANT 0x8576
		#define GL_PRIMARY_COLOR 0x8577
		#define GL_PREVIOUS 0x8578
		#define GL_SOURCE0_RGB 0x8580
		#define GL_SOURCE1_RGB 0x8581
		#define GL_SOURCE2_RGB 0x8582
		#define GL_SOURCE0_ALPHA 0x8588
		#define GL_SOURCE1_ALPHA 0x8589
		#define GL_OPERAND0_RGB 0x8590
		#define GL_OPERAND1_RGB 0x8591
		#define GL_OPERAND0_ALPHA 0x8598
		#define GL_OPERAND1_ALPHA 0x8599
	#endif

	typedef void (APIENTRY* PFNGLGENBUFFERSPROC)(GLsizei n, GLuint* buffers);
	typedef void (APIENTRY* PFNGLBINDBUFFERPROC)(GLenum target, GLuint buffer);
	typedef void (APIENTRY* PFNGLBUFFERDATAPROC)(GLenum target, GLsizeiptr size, const void* data, GLenum usage);
	typedef void (APIENTRY* PFNGLDELETEBUFFERSPROC)(GLsizei n, const GLuint* buffers);
	typedef void (APIENTRY* PFNGLACTIVETEXTUREPROC)(GLenum texture);
	typedef void (APIENTRY* PFNGLCLIENTACTIVETEXTUREPROC)(GLenum texture);

	inline void glGenBuffers(GLsizei n, GLuint* b) { static PFNGLGENBUFFERSPROC p = NULL; if (!p) p = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers"); if (p) p(n, b); }
	inline void glBindBuffer(GLenum t, GLuint b) { static PFNGLBINDBUFFERPROC p = NULL; if (!p) p = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer"); if (p) p(t, b); }
	inline void glBufferData(GLenum t, GLsizeiptr s, const void* d, GLenum u) { static PFNGLBUFFERDATAPROC p = NULL; if (!p) p = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData"); if (p) p(t, s, d, u); }
	inline void glDeleteBuffers(GLsizei n, const GLuint* b) { static PFNGLDELETEBUFFERSPROC p = NULL; if (!p) p = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers"); if (p) p(n, b); }
	inline void glActiveTexture(GLenum t) { static PFNGLACTIVETEXTUREPROC p = NULL; if (!p) p = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture"); if (p) p(t); }
	inline void glClientActiveTexture(GLenum t) { static PFNGLCLIENTACTIVETEXTUREPROC p = NULL; if (!p) p = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture"); if (p) p(t); }
#endif

#include <math.h>
#include <string.h>
#include <stdlib.h>
#include <vector>

#ifdef ANDROID
typedef const void* LPCVOID;
typedef void* LPVOID;
typedef const char* LPCSTR;
typedef uint32_t ULONG;
typedef char* PCHAR;
typedef char* LPSTR;
typedef uint8_t* PBYTE;
typedef uint8_t* LPBYTE;
#ifndef VOID
#define VOID void
#endif
#ifndef CONST
#define CONST const
#endif
#ifndef IN
#define IN
#endif
#ifndef OUT
#define OUT
#endif
#ifndef OPTIONAL
#define OPTIONAL
#endif
#ifndef FAR
#define FAR
#endif
#ifndef NEAR
#define NEAR
#endif
#ifndef PASCAL
#define PASCAL
#endif
#ifndef CDECL
#define CDECL
#endif
typedef char CHAR;
typedef unsigned char UCHAR;
typedef unsigned short USHORT;
typedef char TCHAR;
typedef const char* LPCTSTR;
typedef char* LPTSTR;

#ifndef WINAPI
#define WINAPI
#endif
#ifndef CALLBACK
#define CALLBACK
#endif
#ifndef APIENTRY
#define APIENTRY
#endif
#ifndef __forceinline
#define __forceinline inline
#endif


#ifndef _GUID_DEFINED
#define _GUID_DEFINED
typedef struct _GUID {
	uint32_t Data1;
	uint16_t Data2;
	uint16_t Data3;
	uint8_t Data4[8];
} GUID;
#endif

#define DIRECTINPUT_VERSION 0x0800
#endif

#ifndef S_OK
#define S_OK ((HRESULT)0L)
#endif
#ifndef FAILED
#define FAILED(hr) (((HRESULT)(hr)) < 0)
#endif
#ifndef SUCCEEDED
#define SUCCEEDED(hr) (((HRESULT)(hr)) >= 0)
#endif
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) { if (p) { (p)->Release(); (p) = NULL; } }
#endif

#ifdef ENABLE_OPENGL

typedef DWORD D3DFORMAT;
typedef DWORD D3DPRIMITIVETYPE;
typedef DWORD D3DDEVTYPE;
typedef DWORD D3DTEXTUREOP;
typedef DWORD D3DRENDERSTATETYPE;
typedef DWORD D3DTEXTURESTAGESTATETYPE;
typedef DWORD D3DBLEND;
typedef DWORD D3DCMPFUNC;
typedef DWORD D3DCULL;
typedef DWORD D3DLIGHTSTATETYPE;
typedef DWORD D3DLIGHTTYPE;
typedef DWORD D3DFILLMODE;
typedef DWORD D3DMULTISAMPLE_TYPE;
typedef DWORD D3DTRANSFORMSTATETYPE;
typedef DWORD D3DBACKBUFFER_TYPE;
typedef DWORD D3DPOOL;
typedef RECT D3DRECT;

#ifndef D3DCOLOR_DEFINED
typedef DWORD D3DCOLOR;
#define D3DCOLOR_DEFINED
#endif

#ifndef D3DCOLOR_ARGB
#define D3DCOLOR_ARGB(a,r,g,b) ((D3DCOLOR)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif
#ifndef D3DCOLOR_RGBA
#define D3DCOLOR_RGBA(r,g,b,a) D3DCOLOR_ARGB(a,r,g,b)
#endif
#ifndef D3DCOLOR_XRGB
#define D3DCOLOR_XRGB(r,g,b)   D3DCOLOR_ARGB(0xff,r,g,b)
#endif
#ifndef D3DCOLOR_COLORVALUE
#define D3DCOLOR_COLORVALUE(r,g,b,a) D3DCOLOR_RGBA((DWORD)((r)*255.f),(DWORD)((g)*255.f),(DWORD)((b)*255.f),(DWORD)((a)*255.f))
#endif

#define D3DX_PI 3.141592654f
#define D3DXToRadian(degree) ((degree) * (D3DX_PI / 180.0f))
#define D3DXToDegree(radian) ((radian) * (180.0f / D3DX_PI))

#define D3DX_DEFAULT ((UINT)-1)
#define D3DX_DEFAULT_NONPOW2 ((UINT)-2)
#define D3DX_FILTER_NONE 1
#define D3DX_FILTER_POINT 2
#define D3DX_FILTER_LINEAR 3
#define D3DX_FILTER_TRIANGLE 4
#define D3DX_FILTER_BOX 5
#define D3DX_FILTER_MIRROR 0x00010000
#define D3DX_FILTER_DITHER 0x00080000

#define D3DTS_WORLD 1
#define D3DTS_VIEW 2
#define D3DTS_PROJECTION 3
#define D3DTS_TEXTURE0 16
#define D3DTS_TEXTURE1 17
#define D3DTS_TEXTURE2 18
#define D3DTS_TEXTURE3 19
#define D3DTS_TEXTURE4 20
#define D3DTS_TEXTURE5 21
#define D3DTS_TEXTURE6 22
#define D3DTS_TEXTURE7 23
#define D3DTS_WORLDMATRIX(index) (D3DTRANSFORMSTATETYPE)(index + 256)

#define D3DCLEAR_TARGET 1
#define D3DCLEAR_ZBUFFER 2
#define D3DCLEAR_STENCIL 4

#define D3DFVF_XYZ 0x002
#define D3DFVF_XYZRHW 0x004
#define D3DFVF_XYZB1 0x006
#define D3DFVF_XYZB2 0x008
#define D3DFVF_XYZB3 0x00a
#define D3DFVF_XYZB4 0x00c
#define D3DFVF_XYZB5 0x00e
#define D3DFVF_NORMAL 0x010
#define D3DFVF_PSIZE 0x020
#define D3DFVF_DIFFUSE 0x040
#define D3DFVF_SPECULAR 0x080
#define D3DFVF_TEX0 0x000
#define D3DFVF_TEX1 0x100
#define D3DFVF_TEX2 0x200
#define D3DFVF_TEX3 0x300
#define D3DFVF_TEX4 0x400
#define D3DFVF_TEXCOUNT_MASK 0xf00
#define D3DFVF_TEXCOUNT_SHIFT 8

#define D3DFVF_TEXTUREFORMAT1 3
#define D3DFVF_TEXTUREFORMAT2 0
#define D3DFVF_TEXTUREFORMAT3 1
#define D3DFVF_TEXTUREFORMAT4 2
#define D3DFVF_TEXCOORDSIZE1(CoordIndex) (D3DFVF_TEXTUREFORMAT1 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE2(CoordIndex) (D3DFVF_TEXTUREFORMAT2)
#define D3DFVF_TEXCOORDSIZE3(CoordIndex) (D3DFVF_TEXTUREFORMAT3 << (CoordIndex*2 + 16))
#define D3DFVF_TEXCOORDSIZE4(CoordIndex) (D3DFVF_TEXTUREFORMAT4 << (CoordIndex*2 + 16))

#define D3DRS_ZENABLE 7
#define D3DRS_FILLMODE 8
#define D3DRS_SHADEMODE 9
#define D3DRS_LINEPATTERN 10
#define D3DRS_ZWRITEENABLE 14
#define D3DRS_ALPHATESTENABLE 15
#define D3DRS_LASTPIXEL 16
#define D3DRS_SRCBLEND 19
#define D3DRS_DESTBLEND 20
#define D3DRS_CULLMODE 22
#define D3DRS_ZFUNC 23
#define D3DRS_ALPHAREF 24
#define D3DRS_ALPHAFUNC 25
#define D3DRS_DITHERENABLE 26
#define D3DRS_ALPHABLENDENABLE 27
#define D3DRS_FOGENABLE 28
#define D3DRS_SPECULARENABLE 29
#define D3DRS_ZVISIBLE 30
#define D3DRS_FOGCOLOR 34
#define D3DRS_FOGTABLEMODE 35
#define D3DRS_FOGSTART 36
#define D3DRS_FOGEND 37
#define D3DRS_FOGDENSITY 38
#define D3DRS_EDGEANTIALIAS 40
#define D3DRS_ZBIAS 47
#define D3DRS_RANGEFOGENABLE 48
#define D3DRS_STENCILENABLE 52
#define D3DRS_STENCILFAIL 53
#define D3DRS_STENCILZFAIL 54
#define D3DRS_STENCILPASS 55
#define D3DRS_STENCILFUNC 56
#define D3DRS_STENCILREF 57
#define D3DRS_STENCILMASK 58
#define D3DRS_STENCILWRITEMASK 59
#define D3DRS_TEXTUREFACTOR 60
#define D3DRS_WRAP0 128
#define D3DRS_WRAP1 129
#define D3DRS_WRAP2 130
#define D3DRS_WRAP3 131
#define D3DRS_WRAP4 132
#define D3DRS_WRAP5 133
#define D3DRS_WRAP6 134
#define D3DRS_WRAP7 135
#define D3DRS_CLIPPING 136
#define D3DRS_LIGHTING 137
#define D3DRS_AMBIENT 139
#define D3DRS_FOGVERTEXMODE 140
#define D3DRS_COLORVERTEX 141
#define D3DRS_LOCALVIEWER 142
#define D3DRS_NORMALIZENORMALS 143
#define D3DRS_DIFFUSEMATERIALSOURCE 145
#define D3DRS_SPECULARMATERIALSOURCE 146
#define D3DRS_AMBIENTMATERIALSOURCE 147
#define D3DRS_EMISSIVEMATERIALSOURCE 148
#define D3DRS_VERTEXBLEND 151
#define D3DRS_CLIPPLANEENABLE 152
#define D3DRS_SOFTWAREVERTEXPROCESSING 153
#define D3DRS_POINTSIZE 154
#define D3DRS_POINTSIZE_MIN 155
#define D3DRS_POINTSPRITEENABLE 156
#define D3DRS_POINTSCALEENABLE 157
#define D3DRS_POINTSCALE_A 158
#define D3DRS_POINTSCALE_B 159
#define D3DRS_POINTSCALE_C 160
#define D3DRS_MULTISAMPLEANTIALIAS 161
#define D3DRS_MULTISAMPLEMASK 162
#define D3DRS_INDEXEDVERTEXBLENDENABLE 167
#define D3DRS_COLORWRITEENABLE 168
#define D3DRS_TWEENFACTOR 170
#define D3DRS_BLENDOP 171
#define D3DRS_SLOPESCALEDEPTHBIAS 175
#define D3DRS_DEPTHBIAS 195
#define D3DFMT_FROM_FILE ((D3DFORMAT)-3)

#define D3DTSS_COLOROP 1
#define D3DTSS_COLORARG1 2
#define D3DTSS_COLORARG2 3
#define D3DTSS_ALPHAOP 4
#define D3DTSS_ALPHAARG1 5
#define D3DTSS_ALPHAARG2 6
#define D3DTSS_BUMPENVMAT00 7
#define D3DTSS_BUMPENVMAT01 8
#define D3DTSS_BUMPENVMAT10 9
#define D3DTSS_BUMPENVMAT11 10
#define D3DTSS_TEXCOORDINDEX 11
#define D3DTSS_ADDRESSU 13
#define D3DTSS_ADDRESSV 14
#define D3DTSS_BORDERCOLOR 15
#define D3DTSS_MAGFILTER 16
#define D3DTSS_MINFILTER 17
#define D3DTSS_MIPFILTER 18
#define D3DTSS_MIPMAPLODBIAS 19
#define D3DTSS_MAXMIPLEVEL 20
#define D3DTSS_MAXANISOTROPY 21
#define D3DTSS_BUMPENVLSCALE 22
#define D3DTSS_BUMPENVLOFFSET 23
#define D3DTSS_TEXTURETRANSFORMFLAGS 24
#define D3DTSS_ADDRESSW 25
#define D3DTSS_COLORARG0 26
#define D3DTSS_ALPHAARG0 27
#define D3DTSS_RESULTARG 28

#define D3DTSS_TCI_PASSTHRU 0x00000000
#define D3DTSS_TCI_CAMERASPACENORMAL 0x00010000
#define D3DTSS_TCI_CAMERASPACEPOSITION 0x00020000
#define D3DTSS_TCI_CAMERASPACEREFLECTIONVECTOR 0x00030000

#define D3DTTFF_DISABLE 0
#define D3DTTFF_COUNT1 1
#define D3DTTFF_COUNT2 2
#define D3DTTFF_COUNT3 3
#define D3DTTFF_COUNT4 4
#define D3DTTFF_PROJECTED 256

#define D3DTEXF_NONE 0
#define D3DTEXF_POINT 1
#define D3DTEXF_LINEAR 2
#define D3DTEXF_ANISOTROPIC 3
#define D3DPTFILTERCAPS_MINFANISOTROPIC 0x00000400
#define D3DPTFILTERCAPS_MAGFANISOTROPIC 0x04000000

#define D3DMCS_MATERIAL 0
#define D3DMCS_COLOR1 1
#define D3DMCS_COLOR2 2

#define D3DVBF_DISABLE 0
#define D3DVBF_1WEIGHTS 1
#define D3DVBF_2WEIGHTS 2
#define D3DVBF_3WEIGHTS 3
#define D3DVBF_TWEENING 255
#define D3DVBF_0WEIGHTS 256

#define D3DBLENDOP_ADD 1
#define D3DBLENDOP_SUBTRACT 2
#define D3DBLENDOP_REVSUBTRACT 3
#define D3DBLENDOP_MIN 4
#define D3DBLENDOP_MAX 5

#define D3DFOG_NONE 0
#define D3DFOG_EXP 1
#define D3DFOG_EXP2 2
#define D3DFOG_LINEAR 3

#define D3DPT_POINTLIST 1
#define D3DPT_LINELIST 2
#define D3DPT_LINESTRIP 3
#define D3DPT_TRIANGLELIST 4
#define D3DPT_TRIANGLESTRIP 5
#define D3DPT_TRIANGLEFAN 6

#define D3DFMT_UNKNOWN 0
#define D3DFMT_R8G8B8 20
#define D3DFMT_A8R8G8B8 21
#define D3DFMT_X8R8G8B8 22
#define D3DFMT_R5G6B5 23
#define D3DFMT_X1R5G5B5 24
#define D3DFMT_A1R5G5B5 25
#define D3DFMT_A4R4G4B4 26
#define D3DFMT_R3G3B2 27
#define D3DFMT_A8 28
#define D3DFMT_A8R3G3B2 29
#define D3DFMT_X4R4G4B4 30
#define D3DFMT_A2B10G10R10 31
#define D3DFMT_P8 41
#define D3DFMT_L8 50
#define D3DFMT_A8L8 51
#define D3DFMT_D16_LOCKABLE 70
#define D3DFMT_D32 71
#define D3DFMT_D15S1 73
#define D3DFMT_D24S8 75
#define D3DFMT_D16 80
#define D3DFMT_D24X8 77
#define D3DFMT_D24X4S4 79
#define D3DFMT_INDEX16 101
#define D3DFMT_INDEX32 102
#define D3DFMT_DXT1 827611204
#define D3DFMT_DXT2 844388420
#define D3DFMT_DXT3 861165636
#define D3DFMT_DXT4 877942852
#define D3DFMT_DXT5 894720068
#define D3DFMT_R16F 111
#define D3DFMT_G16R16F 112
#define D3DFMT_A16B16G16R16F 113
#define D3DFMT_R32F 114
#define D3DFMT_G32R32F 115
#define D3DFMT_A32B32G32R32F 116
#define D3DBACKBUFFER_TYPE_MONO 0
#define D3DBACKBUFFER_TYPE_LEFT 1
#define D3DBACKBUFFER_TYPE_RIGHT 2

#define D3DSGR_NO_CALIBRATION 0x00000000L
#define D3DSGR_CALIBRATE 0x00000001L
#define D3DCAPS2_FULLSCREENGAMMA 0x00020000L
#define D3DCAPS2_CANRENDERWINDOWED 0x00080000L

#define D3DTA_SELECTMASK 0x0000000f
#define D3DTA_DIFFUSE 0x00000000
#define D3DTA_CURRENT 0x00000001
#define D3DTA_TEXTURE 0x00000002
#define D3DTA_TFACTOR 0x00000003
#define D3DTA_SPECULAR 0x00000004
#define D3DTA_TEMP 0x00000005
#define D3DTA_COMPLEMENT 0x00000010
#define D3DTA_ALPHAREPLICATE 0x00000020

#define D3DTOP_DISABLE 1
#define D3DTOP_SELECTARG1 2
#define D3DTOP_SELECTARG2 3
#define D3DTOP_MODULATE 4
#define D3DTOP_MODULATE2X 5
#define D3DTOP_MODULATE4X 6
#define D3DTOP_ADD 7
#define D3DTOP_ADDSIGNED 8
#define D3DTOP_ADDSIGNED2X 9
#define D3DTOP_SUBTRACT 10
#define D3DTOP_ADDSMOOTH 11
#define D3DTOP_BLENDDIFFUSEALPHA 12
#define D3DTOP_BLENDTEXTUREALPHA 13
#define D3DTOP_BLENDFACTORALPHA 14
#define D3DTOP_BLENDTEXTUREALPHAPM 15
#define D3DTOP_BLENDCURRENTALPHA 16
#define D3DTOP_PREMODULATE 17
#define D3DTOP_MODULATEALPHA_ADDCOLOR 18
#define D3DTOP_MODULATECOLOR_ADDALPHA 19
#define D3DTOP_MODULATEINVALPHA_ADDCOLOR 20
#define D3DTOP_MODULATEINVCOLOR_ADDALPHA 21
#define D3DTOP_BUMPENVMAP 22
#define D3DTOP_BUMPENVMAPLUMINANCE 23
#define D3DTOP_DOTPRODUCT3 24
#define D3DTOP_MULTIPLYADD 25
#define D3DTOP_LERP 26

#define D3DBLEND_ZERO 1
#define D3DBLEND_ONE 2
#define D3DBLEND_SRCCOLOR 3
#define D3DBLEND_INVSRCCOLOR 4
#define D3DBLEND_SRCALPHA 5
#define D3DBLEND_INVSRCALPHA 6
#define D3DBLEND_DESTALPHA 7
#define D3DBLEND_INVDESTALPHA 8
#define D3DBLEND_DESTCOLOR 9
#define D3DBLEND_INVDESTCOLOR 10
#define D3DBLEND_SRCALPHASAT 11
#define D3DBLEND_BOTHSRCALPHA 12
#define D3DBLEND_BOTHINVSRCALPHA 13

#define D3DCULL_NONE 1
#define D3DCULL_CW 2
#define D3DCULL_CCW 3

#define D3DFILL_POINT 1
#define D3DFILL_WIREFRAME 2
#define D3DFILL_SOLID 3

#define D3DLIGHT_POINT 1
#define D3DLIGHT_SPOT 2
#define D3DLIGHT_DIRECTIONAL 3
#define D3DSHADE_FLAT 1
#define D3DSHADE_GOURAUD 2
#define D3DSHADE_PHONG 3

#define D3DCMP_NEVER 1
#define D3DCMP_LESS 2
#define D3DCMP_EQUAL 3
#define D3DCMP_LESSEQUAL 4
#define D3DCMP_GREATER 5
#define D3DCMP_NOTEQUAL 6
#define D3DCMP_GREATEREQUAL 7
#define D3DCMP_ALWAYS 8

#define D3DTADDRESS_WRAP 1
#define D3DTADDRESS_MIRROR 2
#define D3DTADDRESS_CLAMP 3
#define D3DTADDRESS_BORDER 4
#define D3DTADDRESS_MIRRORONCE 5

#define D3DPOOL_DEFAULT 0
#define D3DPOOL_MANAGED 1
#define D3DPOOL_SYSTEMMEM 2
#define D3DPOOL_SCRATCH 3

#define D3DLOCK_READONLY 0x0010
#define D3DLOCK_DISCARD 0x2000
#define D3DLOCK_NOOVERWRITE 0x1000
#define D3DLOCK_NOSYSLOCK 0x0800
#define D3DLOCK_NO_DIRTY_UPDATE 0x8000

#define D3DUSAGE_RENDERTARGET 0x0001
#define D3DUSAGE_DEPTHSTENCIL 0x0002
#define D3DUSAGE_WRITEONLY 0x0008
#define D3DUSAGE_SOFTWAREPROCESSING 0x0010
#define D3DUSAGE_DONOTCLIP 0x0020
#define D3DUSAGE_POINTS 0x0040
#define D3DUSAGE_RTPATCHES 0x0080
#define D3DUSAGE_NPATCHES 0x0100
#define D3DUSAGE_DYNAMIC 0x0200

#define D3D_OK 0
#define D3DERR_DEVICELOST 0x88760868
#define D3DERR_DEVICENOTRESET 0x88760869
#define D3DERR_INVALIDCALL 0x8876086C
#define D3DERR_NOTAVAILABLE 0x8876086A
#define D3DERR_OUTOFVIDEOMEMORY 0x8876017C

#define D3DADAPTER_DEFAULT 0
#define D3DSWAPEFFECT_DISCARD 1
#define D3DSWAPEFFECT_FLIP 2
#define D3DSWAPEFFECT_COPY 3
#define D3DSWAPEFFECT_COPY_VSYNC 4
#define D3DPRESENT_INTERVAL_DEFAULT 0
#define D3DPRESENT_INTERVAL_ONE 1
#define D3DPRESENT_INTERVAL_IMMEDIATE 0x80000000
#define D3DPRESENTFLAG_LOCKABLE_BACKBUFFER 0x00000001
#define D3DENUM_NO_WHQL_LEVEL 0x00000002

#define D3DCREATE_SOFTWARE_VERTEXPROCESSING 0x00000020
#define D3DCREATE_MIXED_VERTEXPROCESSING 0x00000080
#define D3DCREATE_HARDWARE_VERTEXPROCESSING 0x00000040
#define D3DCREATE_PUREDEVICE 0x00000010
#define D3DCREATE_MULTITHREADED 0x00000004
#define D3DCREATE_FPU_PRESERVE 0x00000002

#define D3DRTYPE_SURFACE 1
#define D3DRTYPE_VOLUME 2
#define D3DRTYPE_TEXTURE 3
#define D3DRTYPE_VOLUMETEXTURE 4
#define D3DRTYPE_CUBETEXTURE 5
#define D3DRTYPE_VERTEXBUFFER 6
#define D3DRTYPE_INDEXBUFFER 7
#define D3DMULTISAMPLE_NONE 0

#define D3DDEVTYPE_HAL 1
#define D3DDEVTYPE_REF 2
#define D3DDEVTYPE_SW 3

#define D3DPTADDRESSCAPS_WRAP 0x00000001
#define D3DPTADDRESSCAPS_MIRROR 0x00000002
#define D3DPTADDRESSCAPS_CLAMP 0x00000004
#define D3DPTADDRESSCAPS_BORDER 0x00000008
#define D3DDEVCAPS_EXECUTESYSTEMMEMORY 0x00000010
#define D3DDEVCAPS_TLVERTEXSYSTEMMEMORY 0x00000040
#define D3DDEVCAPS_TLVERTEXVIDEOMEMORY 0x00000080
#define D3DDEVCAPS_TEXTURESYSTEMMEMORY 0x00000100
#define D3DDEVCAPS_TEXTUREVIDEOMEMORY 0x00000200
#define D3DDEVCAPS_DRAWPRIMTLVERTEX 0x00000400
#define D3DDEVCAPS_CANRENDERAFTERFLIP 0x00000800
#define D3DDEVCAPS_TEXTURENONLOCALVIDMEM 0x00001000
#define D3DDEVCAPS_SEPARATETEXTUREMEMORIES 0x00004000
#define D3DDEVCAPS_HWTRANSFORMANDLIGHT 0x00010000
#define D3DDEVCAPS_CANBLTSYSTONONLOCAL 0x00020000
#define D3DDEVCAPS_HWRASTERIZATION 0x00080000
#define D3DDEVCAPS_PUREDEVICE 0x00100000
#define D3DPMISCCAPS_MASKZ 0x00000002
#define D3DPMISCCAPS_CULLNONE 0x00000010
#define D3DPMISCCAPS_CULLCW 0x00000020
#define D3DPMISCCAPS_CULLCCW 0x00000040
#define D3DPMISCCAPS_CLIPTLVERTS 0x00000200
#define D3DVTXPCAPS_TEXGEN 0x00000001
#define D3DVTXPCAPS_MATERIALSOURCE7 0x00000002
#define D3DVTXPCAPS_DIRECTIONALLIGHTS 0x00000008
#define D3DVTXPCAPS_POSITIONALLIGHTS 0x00000010
#define D3DVTXPCAPS_LOCALVIEWER 0x00000020
#define D3DVS_VERSION(major, minor) (0xFFFE0000 | ((major) << 8) | (minor))
#define D3DPS_VERSION(major, minor) (0xFFFF0000 | ((major) << 8) | (minor))
#define D3D_SDK_VERSION 220

#define D3DVSD_STREAM(s) (s)
#define D3DVSD_REG(r, t) (r)
#define D3DVSD_END() 0
#define D3DVSDT_FLOAT1 0
#define D3DVSDT_FLOAT2 0
#define D3DVSDT_FLOAT3 0
#define D3DVSDT_FLOAT4 0
#define D3DVSDT_D3DCOLOR 0
#define D3DVSDT_UBYTE4 0
#define D3DVSDT_SHORT2 0
#define D3DVSDT_SHORT4 0
#define D3DVSDE_POSITION 0
#define D3DVSDE_BLENDWEIGHT 0
#define D3DVSDE_BLENDINDICES 0
#define D3DVSDE_NORMAL 0
#define D3DVSDE_PSIZE 0
#define D3DVSDE_DIFFUSE 0
#define D3DVSDE_SPECULAR 0
#define D3DVSDE_TEXCOORD0 0
#define D3DVSDE_TEXCOORD1 0
#define D3DVSDE_POSITION2 0
#define D3DVSDE_NORMAL2 0

struct GLMATRIX;
inline GLMATRIX* D3DXMatrixMultiply(GLMATRIX* pOut, const GLMATRIX* pM1, const GLMATRIX* pM2);

struct GLMATRIX {
	union {
		struct {
			float _11, _12, _13, _14;
			float _21, _22, _23, _24;
			float _31, _32, _33, _34;
			float _41, _42, _43, _44;
		};
		float m[4][4];
	};
	GLMATRIX() { memset(m, 0, sizeof(m)); m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f; }
	GLMATRIX(float f11, float f12, float f13, float f14, float f21, float f22, float f23, float f24, float f31, float f32, float f33, float f34, float f41, float f42, float f43, float f44) {
		_11 = f11; _12 = f12; _13 = f13; _14 = f14; _21 = f21; _22 = f22; _23 = f23; _24 = f24; _31 = f31; _32 = f32; _33 = f33; _34 = f34; _41 = f41; _42 = f42; _43 = f43; _44 = f44;
	}
	float& operator()(int r, int c) { return m[r][c]; }
	const float& operator()(int r, int c) const { return m[r][c]; }
	GLMATRIX operator*(const GLMATRIX& rhs) const { GLMATRIX out; D3DXMatrixMultiply(&out, this, &rhs); return out; }
	GLMATRIX& operator*=(const GLMATRIX& rhs) { D3DXMatrixMultiply(this, this, &rhs); return *this; }
	bool operator==(const GLMATRIX& rhs) const { return memcmp(m, rhs.m, sizeof(m)) == 0; }
	bool operator!=(const GLMATRIX& rhs) const { return memcmp(m, rhs.m, sizeof(m)) != 0; }
	bool operator!() const { return false; }
};

struct GLVECTOR3 {
	float x, y, z;
	GLVECTOR3() : x(0.0f), y(0.0f), z(0.0f) {}
	GLVECTOR3(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f) {}
	GLVECTOR3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
	GLVECTOR3 operator+(const GLVECTOR3& r) const { return GLVECTOR3(x + r.x, y + r.y, z + r.z); }
	GLVECTOR3 operator-(const GLVECTOR3& r) const { return GLVECTOR3(x - r.x, y - r.y, z - r.z); }
	GLVECTOR3 operator*(float s) const { return GLVECTOR3(x * s, y * s, z * s); }
	GLVECTOR3 operator/(float s) const { return GLVECTOR3(x / s, y / s, z / s); }
	GLVECTOR3 operator+() const { return *this; }
	GLVECTOR3 operator-() const { return GLVECTOR3(-x, -y, -z); }
	GLVECTOR3& operator+=(const GLVECTOR3& r) { x += r.x; y += r.y; z += r.z; return *this; }
	GLVECTOR3& operator-=(const GLVECTOR3& r) { x -= r.x; y -= r.y; z -= r.z; return *this; }
	GLVECTOR3& operator*=(float s) { x *= s; y *= s; z *= s; return *this; }
	GLVECTOR3& operator/=(float s) { x /= s; y /= s; z /= s; return *this; }
	bool operator==(const GLVECTOR3& r) const { return x == r.x && y == r.y && z == r.z; }
	bool operator!=(const GLVECTOR3& r) const { return !(*this == r); }
	operator float*() { return &x; }
	operator const float*() const { return &x; }
};
inline GLVECTOR3 operator*(float s, const GLVECTOR3& v) { return GLVECTOR3(v.x * s, v.y * s, v.z * s); }

struct GLVECTOR2 {
	float x, y;
	GLVECTOR2() : x(0.0f), y(0.0f) {}
	explicit GLVECTOR2(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f) {}
	GLVECTOR2(float _x, float _y) : x(_x), y(_y) {}
	GLVECTOR2 operator+(const GLVECTOR2& r) const { return GLVECTOR2(x + r.x, y + r.y); }
	GLVECTOR2 operator-(const GLVECTOR2& r) const { return GLVECTOR2(x - r.x, y - r.y); }
	GLVECTOR2 operator*(float s) const { return GLVECTOR2(x * s, y * s); }
	GLVECTOR2 operator/(float s) const { return GLVECTOR2(x / s, y / s); }
	GLVECTOR2 operator+() const { return *this; }
	GLVECTOR2 operator-() const { return GLVECTOR2(-x, -y); }
	GLVECTOR2& operator+=(const GLVECTOR2& r) { x += r.x; y += r.y; return *this; }
	GLVECTOR2& operator-=(const GLVECTOR2& r) { x -= r.x; y -= r.y; return *this; }
	GLVECTOR2& operator*=(float s) { x *= s; y *= s; return *this; }
	GLVECTOR2& operator/=(float s) { x /= s; y /= s; return *this; }
	bool operator==(const GLVECTOR2& r) const { return x == r.x && y == r.y; }
	bool operator!=(const GLVECTOR2& r) const { return !(*this == r); }
	operator float*() { return &x; }
	operator const float*() const { return &x; }
};
inline GLVECTOR2 operator*(float s, const GLVECTOR2& v) { return GLVECTOR2(v.x * s, v.y * s); }

struct GLVECTOR4 {
	float x, y, z, w;
	GLVECTOR4() : x(0.0f), y(0.0f), z(0.0f), w(0.0f) {}
	explicit GLVECTOR4(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f), w(pf ? pf[3] : 0.0f) {}
	GLVECTOR4(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	operator float*() { return &x; }
	operator const float*() const { return &x; }
};

struct GLQUATERNION {
	float x, y, z, w;
	GLQUATERNION() : x(0.0f), y(0.0f), z(0.0f), w(1.0f) {}
	explicit GLQUATERNION(const float* pf) : x(pf ? pf[0] : 0.0f), y(pf ? pf[1] : 0.0f), z(pf ? pf[2] : 0.0f), w(pf ? pf[3] : 1.0f) {}
	GLQUATERNION(float _x, float _y, float _z, float _w) : x(_x), y(_y), z(_z), w(_w) {}
	GLQUATERNION operator+(const GLQUATERNION& r) const { return GLQUATERNION(x + r.x, y + r.y, z + r.z, w + r.w); }
	GLQUATERNION operator-(const GLQUATERNION& r) const { return GLQUATERNION(x - r.x, y - r.y, z - r.z, w - r.w); }
	GLQUATERNION operator*(float s) const { return GLQUATERNION(x * s, y * s, z * s, w * s); }
	GLQUATERNION operator/(float s) const { return GLQUATERNION(x / s, y / s, z / s, w / s); }
	operator float*() { return &x; }
	operator const float*() const { return &x; }
};

struct GLPLANE {
	float a, b, c, d;
	GLPLANE() : a(0.0f), b(0.0f), c(0.0f), d(0.0f) {}
	explicit GLPLANE(const float* pf) : a(pf ? pf[0] : 0.0f), b(pf ? pf[1] : 0.0f), c(pf ? pf[2] : 0.0f), d(pf ? pf[3] : 0.0f) {}
	GLPLANE(float _a, float _b, float _c, float _d) : a(_a), b(_b), c(_c), d(_d) {}
	operator float*() { return &a; }
	operator const float*() const { return &a; }
};

struct D3DCOLORVALUE {
	float r, g, b, a;
};

struct GLCOLOR : public D3DCOLORVALUE {
	GLCOLOR() { r = 0.0f; g = 0.0f; b = 0.0f; a = 1.0f; }
	GLCOLOR(float _r, float _g, float _b, float _a) { r = _r; g = _g; b = _b; a = _a; }
	GLCOLOR(const D3DCOLORVALUE& c) { r = c.r; g = c.g; b = c.b; a = c.a; }
	GLCOLOR(DWORD dw) {
		a = ((dw >> 24) & 0xFF) / 255.0f;
		r = ((dw >> 16) & 0xFF) / 255.0f;
		g = ((dw >> 8) & 0xFF) / 255.0f;
		b = (dw & 0xFF) / 255.0f;
	}
	operator DWORD() const {
		float ca = (a < 0.0f) ? 0.0f : ((a > 1.0f) ? 1.0f : a);
		float cr = (r < 0.0f) ? 0.0f : ((r > 1.0f) ? 1.0f : r);
		float cg = (g < 0.0f) ? 0.0f : ((g > 1.0f) ? 1.0f : g);
		float cb = (b < 0.0f) ? 0.0f : ((b > 1.0f) ? 1.0f : b);
		return (((DWORD)(ca * 255.0f + 0.5f) & 0xFF) << 24) |
		       (((DWORD)(cr * 255.0f + 0.5f) & 0xFF) << 16) |
		       (((DWORD)(cg * 255.0f + 0.5f) & 0xFF) << 8)  |
		       ((DWORD)(cb * 255.0f + 0.5f) & 0xFF);
	}
	operator float*() { return &r; }
	operator const float*() const { return &r; }
	GLCOLOR operator*(float s) const { return GLCOLOR(r * s, g * s, b * s, a * s); }
	GLCOLOR operator+(const GLCOLOR& rhs) const { return GLCOLOR(r + rhs.r, g + rhs.g, b + rhs.b, a + rhs.a); }
	GLCOLOR operator-(const GLCOLOR& rhs) const { return GLCOLOR(r - rhs.r, g - rhs.g, b - rhs.b, a - rhs.a); }
	GLCOLOR& operator*=(float s) { r *= s; g *= s; b *= s; a *= s; return *this; }
	GLCOLOR& operator+=(const GLCOLOR& rhs) { r += rhs.r; g += rhs.g; b += rhs.b; a += rhs.a; return *this; }
};

#define D3DXMATRIX GLMATRIX
#define D3DMATRIX GLMATRIX
#define D3DXVECTOR3 GLVECTOR3
#define D3DVECTOR GLVECTOR3
#define D3DXVECTOR2 GLVECTOR2
#define D3DXVECTOR4 GLVECTOR4
#define D3DXQUATERNION GLQUATERNION
#define D3DXPLANE GLPLANE
#define D3DXCOLOR GLCOLOR

struct GL_LIGHT {
	int Type;
	GLCOLOR Diffuse, Specular, Ambient;
	GLVECTOR3 Position, Direction;
	float Range, Falloff, Attenuation0, Attenuation1, Attenuation2, Theta, Phi;
};
typedef GL_LIGHT D3DLIGHT8;

struct GL_MATERIAL {
	GLCOLOR Diffuse, Ambient, Specular, Emissive;
	float Power;
};
typedef GL_MATERIAL D3DMATERIAL8;

#ifndef ANDROID
struct D3DXIMAGE_INFO { UINT Width, Height, Depth, MipLevels; D3DFORMAT Format; int ResourceType, ImageFileFormat; };
struct D3DVIEWPORT8 { DWORD X, Y, Width, Height; float MinZ, MaxZ; };
#endif
struct D3DSURFACE_DESC { D3DFORMAT Format; int Type; DWORD Usage; D3DPOOL Pool; UINT Size; D3DMULTISAMPLE_TYPE MultiSampleType; DWORD MultiSampleQuality; UINT Width, Height; };
struct D3DGAMMARAMP { WORD red[256], green[256], blue[256]; };

struct D3DCAPS8 {
	D3DDEVTYPE DeviceType; UINT AdapterOrdinal; DWORD Caps, Caps2, Caps3, PresentationIntervals, CursorCaps, DevCaps, PrimitiveMiscCaps, RasterCaps, ZCmpCaps, SrcBlendCaps, DestBlendCaps, AlphaCmpCaps, ShadeCaps, TextureCaps, TextureFilterCaps, CubeTextureFilterCaps, VolumeTextureFilterCaps, TextureAddressCaps, VolumeTextureAddressCaps, LineCaps;
	DWORD MaxTextureWidth, MaxTextureHeight, MaxVolumeExtent, MaxTextureRepeat, MaxTextureAspectRatio, MaxAnisotropy;
	float MaxVertexW, GuardBandLeft, GuardBandTop, GuardBandRight, GuardBandBottom, ExtentsAdjust;
	DWORD StencilCaps, FVFCaps, TextureOpCaps, MaxTextureBlendStages, MaxSimultaneousTextures, VertexProcessingCaps, MaxActiveLights, MaxUserClipPlanes, MaxVertexBlendMatrices, MaxVertexBlendMatrixIndex;
	float MaxPointSize; DWORD MaxPrimitiveCount, MaxVertexIndex, MaxStreams, MaxStreamStride, VertexShaderVersion, MaxVertexShaderConst, PixelShaderVersion; float MaxPixelShaderValue;
};

struct D3DPRESENT_PARAMETERS {
	UINT BackBufferWidth, BackBufferHeight; D3DFORMAT BackBufferFormat; UINT BackBufferCount; D3DMULTISAMPLE_TYPE MultiSampleType;
	DWORD SwapEffect; HWND hDeviceWindow; BOOL Windowed, EnableAutoDepthStencil; D3DFORMAT AutoDepthStencilFormat; DWORD Flags; UINT FullScreen_RefreshRateInHz, FullScreen_PresentationInterval;
};

struct D3DADAPTER_IDENTIFIER8 { char Driver[512], Description[512]; LARGE_INTEGER DriverVersion; DWORD VendorId, DeviceId, SubSysId, Revision; GUID DeviceIdentifier; DWORD WHQLLevel; };
struct D3DDISPLAYMODE { UINT Width, Height, RefreshRate; D3DFORMAT Format; };
struct D3DLOCKED_RECT { int Pitch; void* pBits; };

struct IDirect3D8; struct IDirect3DDevice8; struct IDirect3DBaseTexture8; struct IDirect3DTexture8;
struct IDirect3DSurface8; struct IDirect3DVertexBuffer8; struct IDirect3DIndexBuffer8;
struct ID3DXBuffer; struct ID3DXMesh; struct ID3DXMatrixStack;

typedef IDirect3D8* LPDIRECT3D8; typedef IDirect3DDevice8* LPDIRECT3DDEVICE8;
typedef IDirect3DBaseTexture8* LPDIRECT3DBaseTexture8; typedef IDirect3DBaseTexture8* LPDIRECT3DBASETEXTURE8;
typedef IDirect3DTexture8* LPDIRECT3DTEXTURE8; typedef IDirect3DSurface8* LPDIRECT3DSURFACE8;
typedef IDirect3DVertexBuffer8* LPDIRECT3DVERTEXBUFFER8; typedef IDirect3DIndexBuffer8* LPDIRECT3DINDEXBUFFER8;
typedef ID3DXBuffer* LPD3DXBUFFER; typedef ID3DXMesh* LPD3DXMESH; typedef ID3DXMatrixStack* LPD3DXMATRIXSTACK;

inline float D3DXVec2Length(const GLVECTOR2* pV) { return sqrtf(pV->x * pV->x + pV->y * pV->y); }
inline float D3DXVec2LengthSq(const GLVECTOR2* pV) { return pV->x * pV->x + pV->y * pV->y; }
inline float D3DXVec2Dot(const GLVECTOR2* pV1, const GLVECTOR2* pV2) { return pV1->x * pV2->x + pV1->y * pV2->y; }
inline float D3DXVec2CCW(const GLVECTOR2* pV1, const GLVECTOR2* pV2) { return pV1->x * pV2->y - pV1->y * pV2->x; }
inline GLVECTOR2* D3DXVec2Add(GLVECTOR2* pOut, const GLVECTOR2* pV1, const GLVECTOR2* pV2) { pOut->x = pV1->x + pV2->x; pOut->y = pV1->y + pV2->y; return pOut; }
inline GLVECTOR2* D3DXVec2Subtract(GLVECTOR2* pOut, const GLVECTOR2* pV1, const GLVECTOR2* pV2) { pOut->x = pV1->x - pV2->x; pOut->y = pV1->y - pV2->y; return pOut; }
inline GLVECTOR2* D3DXVec2Scale(GLVECTOR2* pOut, const GLVECTOR2* pV, float s) { pOut->x = pV->x * s; pOut->y = pV->y * s; return pOut; }
inline GLVECTOR2* D3DXVec2Normalize(GLVECTOR2* pOut, const GLVECTOR2* pV) { float l = D3DXVec2Length(pV); if (l > 1e-6f) { pOut->x = pV->x / l; pOut->y = pV->y / l; } else { pOut->x = pOut->y = 0.0f; } return pOut; }

inline float D3DXVec3LengthSq(const GLVECTOR3* pV) { return pV->x * pV->x + pV->y * pV->y + pV->z * pV->z; }
inline float D3DXVec3Length(const GLVECTOR3* pV) { return sqrtf(D3DXVec3LengthSq(pV)); }
inline float D3DXVec3Dot(const GLVECTOR3* pV1, const GLVECTOR3* pV2) { return pV1->x * pV2->x + pV1->y * pV2->y + pV1->z * pV2->z; }
inline GLVECTOR3* D3DXVec3Cross(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { GLVECTOR3 v(pV1->y * pV2->z - pV1->z * pV2->y, pV1->z * pV2->x - pV1->x * pV2->z, pV1->x * pV2->y - pV1->y * pV2->x); *pOut = v; return pOut; }
inline GLVECTOR3* D3DXVec3Add(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { pOut->x = pV1->x + pV2->x; pOut->y = pV1->y + pV2->y; pOut->z = pV1->z + pV2->z; return pOut; }
inline GLVECTOR3* D3DXVec3Subtract(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { pOut->x = pV1->x - pV2->x; pOut->y = pV1->y - pV2->y; pOut->z = pV1->z - pV2->z; return pOut; }
inline GLVECTOR3* D3DXVec3Scale(GLVECTOR3* pOut, const GLVECTOR3* pV, float s) { pOut->x = pV->x * s; pOut->y = pV->y * s; pOut->z = pV->z * s; return pOut; }
inline GLVECTOR3* D3DXVec3Normalize(GLVECTOR3* pOut, const GLVECTOR3* pV) { float l = D3DXVec3Length(pV); if (l > 1e-6f) { pOut->x = pV->x / l; pOut->y = pV->y / l; pOut->z = pV->z / l; } else { pOut->x = pOut->y = pOut->z = 0.0f; } return pOut; }
inline GLVECTOR3* D3DXVec3Minimize(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { pOut->x = (pV1->x < pV2->x) ? pV1->x : pV2->x; pOut->y = (pV1->y < pV2->y) ? pV1->y : pV2->y; pOut->z = (pV1->z < pV2->z) ? pV1->z : pV2->z; return pOut; }
inline GLVECTOR3* D3DXVec3Maximize(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { pOut->x = (pV1->x > pV2->x) ? pV1->x : pV2->x; pOut->y = (pV1->y > pV2->y) ? pV1->y : pV2->y; pOut->z = (pV1->z > pV2->z) ? pV1->z : pV2->z; return pOut; }
inline GLVECTOR3* D3DXVec3Lerp(GLVECTOR3* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2, float s) { pOut->x = pV1->x + s * (pV2->x - pV1->x); pOut->y = pV1->y + s * (pV2->y - pV1->y); pOut->z = pV1->z + s * (pV2->z - pV1->z); return pOut; }

inline GLVECTOR3* D3DXVec3TransformCoord(GLVECTOR3* pOut, const GLVECTOR3* pV, const GLMATRIX* pM) {
	GLVECTOR3 v;
	float norm = pM->m[0][3] * pV->x + pM->m[1][3] * pV->y + pM->m[2][3] * pV->z + pM->m[3][3];
	v.x = pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z + pM->m[3][0];
	v.y = pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z + pM->m[3][1];
	v.z = pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z + pM->m[3][2];
	if (fabsf(norm) > 1e-6f) { v.x /= norm; v.y /= norm; v.z /= norm; }
	*pOut = v; return pOut;
}
inline GLVECTOR3* D3DXVec3TransformNormal(GLVECTOR3* pOut, const GLVECTOR3* pV, const GLMATRIX* pM) {
	GLVECTOR3 v(pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z, pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z, pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z);
	*pOut = v; return pOut;
}
inline GLVECTOR4* D3DXVec4Transform(GLVECTOR4* pOut, const GLVECTOR4* pV, const GLMATRIX* pM) {
	GLVECTOR4 v;
	v.x = pM->m[0][0] * pV->x + pM->m[1][0] * pV->y + pM->m[2][0] * pV->z + pM->m[3][0] * pV->w;
	v.y = pM->m[0][1] * pV->x + pM->m[1][1] * pV->y + pM->m[2][1] * pV->z + pM->m[3][1] * pV->w;
	v.z = pM->m[0][2] * pV->x + pM->m[1][2] * pV->y + pM->m[2][2] * pV->z + pM->m[3][2] * pV->w;
	v.w = pM->m[0][3] * pV->x + pM->m[1][3] * pV->y + pM->m[2][3] * pV->z + pM->m[3][3] * pV->w;
	*pOut = v; return pOut;
}
inline GLVECTOR4* D3DXVec3Transform(GLVECTOR4* pOut, const GLVECTOR3* pV, const GLMATRIX* pM) { GLVECTOR4 in(pV->x, pV->y, pV->z, 1.0f); return D3DXVec4Transform(pOut, &in, pM); }

inline GLMATRIX* D3DXMatrixIdentity(GLMATRIX* pOut) { memset(pOut->m, 0, sizeof(pOut->m)); pOut->m[0][0] = pOut->m[1][1] = pOut->m[2][2] = pOut->m[3][3] = 1.0f; return pOut; }
inline BOOL D3DXMatrixIsIdentity(const GLMATRIX* pM) { GLMATRIX id; return (*pM == id) ? TRUE : FALSE; }
inline GLMATRIX* D3DXMatrixMultiply(GLMATRIX* pOut, const GLMATRIX* pM1, const GLMATRIX* pM2) {
#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
	float32x4_t r0 = vld1q_f32(&pM2->m[0][0]);
	float32x4_t r1 = vld1q_f32(&pM2->m[1][0]);
	float32x4_t r2 = vld1q_f32(&pM2->m[2][0]);
	float32x4_t r3 = vld1q_f32(&pM2->m[3][0]);
	GLMATRIX out;
	for (int i = 0; i < 4; ++i) {
		float32x4_t acc = vmulq_n_f32(r0, pM1->m[i][0]);
		acc = vmlaq_n_f32(acc, r1, pM1->m[i][1]);
		acc = vmlaq_n_f32(acc, r2, pM1->m[i][2]);
		acc = vmlaq_n_f32(acc, r3, pM1->m[i][3]);
		vst1q_f32(&out.m[i][0], acc);
	}
	*pOut = out;
	return pOut;
#else
	GLMATRIX out;
	for (int i = 0; i < 4; i++) {
		const float a0 = pM1->m[i][0], a1 = pM1->m[i][1], a2 = pM1->m[i][2], a3 = pM1->m[i][3];
		out.m[i][0] = a0 * pM2->m[0][0] + a1 * pM2->m[1][0] + a2 * pM2->m[2][0] + a3 * pM2->m[3][0];
		out.m[i][1] = a0 * pM2->m[0][1] + a1 * pM2->m[1][1] + a2 * pM2->m[2][1] + a3 * pM2->m[3][1];
		out.m[i][2] = a0 * pM2->m[0][2] + a1 * pM2->m[1][2] + a2 * pM2->m[2][2] + a3 * pM2->m[3][2];
		out.m[i][3] = a0 * pM2->m[0][3] + a1 * pM2->m[1][3] + a2 * pM2->m[2][3] + a3 * pM2->m[3][3];
	}
	*pOut = out;
	return pOut;
#endif
}
inline GLMATRIX* D3DXMatrixTranspose(GLMATRIX* pOut, const GLMATRIX* pM) { GLMATRIX out; for (int i = 0; i < 4; i++) for (int j = 0; j < 4; j++) out.m[i][j] = pM->m[j][i]; *pOut = out; return pOut; }
inline GLMATRIX* D3DXMatrixInverse(GLMATRIX* pOut, float* pDeterminant, const GLMATRIX* pM) {
	const float* m = &pM->m[0][0]; float inv[16], det;
	inv[0] = m[5]*m[10]*m[15] - m[5]*m[11]*m[14] - m[9]*m[6]*m[15] + m[9]*m[7]*m[14] + m[13]*m[6]*m[11] - m[13]*m[7]*m[10];
	inv[4] = -m[4]*m[10]*m[15] + m[4]*m[11]*m[14] + m[8]*m[6]*m[15] - m[8]*m[7]*m[14] - m[12]*m[6]*m[11] + m[12]*m[7]*m[10];
	inv[8] = m[4]*m[9]*m[15] - m[4]*m[11]*m[13] - m[8]*m[5]*m[15] + m[8]*m[7]*m[13] + m[12]*m[5]*m[11] - m[12]*m[7]*m[9];
	inv[12] = -m[4]*m[9]*m[14] + m[4]*m[10]*m[13] + m[8]*m[5]*m[14] - m[8]*m[6]*m[13] - m[12]*m[5]*m[10] + m[12]*m[6]*m[9];
	inv[1] = -m[1]*m[10]*m[15] + m[1]*m[11]*m[14] + m[9]*m[2]*m[15] - m[9]*m[3]*m[14] - m[13]*m[2]*m[11] + m[13]*m[3]*m[10];
	inv[5] = m[0]*m[10]*m[15] - m[0]*m[11]*m[14] - m[8]*m[2]*m[15] + m[8]*m[3]*m[14] + m[12]*m[2]*m[11] - m[12]*m[3]*m[10];
	inv[9] = -m[0]*m[9]*m[15] + m[0]*m[11]*m[13] + m[8]*m[1]*m[15] - m[8]*m[3]*m[13] - m[12]*m[1]*m[11] + m[12]*m[3]*m[9];
	inv[13] = m[0]*m[9]*m[14] - m[0]*m[10]*m[13] - m[8]*m[1]*m[14] + m[8]*m[2]*m[13] + m[12]*m[1]*m[10] - m[12]*m[2]*m[9];
	inv[2] = m[1]*m[6]*m[15] - m[1]*m[7]*m[14] - m[5]*m[2]*m[15] + m[5]*m[3]*m[14] + m[13]*m[2]*m[7] - m[13]*m[3]*m[6];
	inv[6] = -m[0]*m[6]*m[15] + m[0]*m[7]*m[14] + m[4]*m[2]*m[15] - m[4]*m[3]*m[14] - m[12]*m[2]*m[7] + m[12]*m[3]*m[6];
	inv[10] = m[0]*m[5]*m[15] - m[0]*m[7]*m[13] - m[4]*m[1]*m[15] + m[4]*m[3]*m[13] + m[12]*m[1]*m[7] - m[12]*m[3]*m[5];
	inv[14] = -m[0]*m[5]*m[14] + m[0]*m[6]*m[13] + m[4]*m[1]*m[14] - m[4]*m[2]*m[13] - m[12]*m[1]*m[6] + m[12]*m[2]*m[5];
	inv[3] = -m[1]*m[6]*m[11] + m[1]*m[7]*m[10] + m[5]*m[2]*m[11] - m[5]*m[3]*m[10] - m[9]*m[2]*m[7] + m[9]*m[3]*m[6];
	inv[7] = m[0]*m[6]*m[11] - m[0]*m[7]*m[10] - m[4]*m[2]*m[11] + m[4]*m[3]*m[10] + m[8]*m[2]*m[7] - m[8]*m[3]*m[6];
	inv[11] = -m[0]*m[5]*m[11] + m[0]*m[7]*m[9] + m[4]*m[1]*m[11] - m[4]*m[3]*m[9] - m[8]*m[1]*m[7] + m[8]*m[3]*m[5];
	inv[15] = m[0]*m[5]*m[10] - m[0]*m[6]*m[9] - m[4]*m[1]*m[10] + m[4]*m[2]*m[9] + m[8]*m[1]*m[6] - m[8]*m[2]*m[5];
	det = m[0]*inv[0] + m[1]*inv[4] + m[2]*inv[8] + m[3]*inv[12];
	if (pDeterminant) *pDeterminant = det;
	if (fabsf(det) < 1e-8f) { *pOut = *pM; return NULL; }
	det = 1.0f / det; float* out = &pOut->m[0][0]; for (int i = 0; i < 16; i++) out[i] = inv[i] * det; return pOut;
}
inline GLMATRIX* D3DXMatrixTranslation(GLMATRIX* pOut, float x, float y, float z) { D3DXMatrixIdentity(pOut); pOut->m[3][0] = x; pOut->m[3][1] = y; pOut->m[3][2] = z; return pOut; }
inline GLMATRIX* D3DXMatrixScaling(GLMATRIX* pOut, float x, float y, float z) { D3DXMatrixIdentity(pOut); pOut->m[0][0] = x; pOut->m[1][1] = y; pOut->m[2][2] = z; return pOut; }
inline GLMATRIX* D3DXMatrixRotationX(GLMATRIX* pOut, float a) { float s = sinf(a), c = cosf(a); D3DXMatrixIdentity(pOut); pOut->m[1][1] = c; pOut->m[1][2] = s; pOut->m[2][1] = -s; pOut->m[2][2] = c; return pOut; }
inline GLMATRIX* D3DXMatrixRotationY(GLMATRIX* pOut, float a) { float s = sinf(a), c = cosf(a); D3DXMatrixIdentity(pOut); pOut->m[0][0] = c; pOut->m[0][2] = -s; pOut->m[2][0] = s; pOut->m[2][2] = c; return pOut; }
inline GLMATRIX* D3DXMatrixRotationZ(GLMATRIX* pOut, float a) { float s = sinf(a), c = cosf(a); D3DXMatrixIdentity(pOut); pOut->m[0][0] = c; pOut->m[0][1] = s; pOut->m[1][0] = -s; pOut->m[1][1] = c; return pOut; }
inline GLMATRIX* D3DXMatrixRotationAxis(GLMATRIX* pOut, const GLVECTOR3* pV, float a) {
	GLVECTOR3 v; D3DXVec3Normalize(&v, pV); float s = sinf(a), c = cosf(a), t = 1.0f - c; D3DXMatrixIdentity(pOut);
	pOut->m[0][0] = t*v.x*v.x + c; pOut->m[0][1] = t*v.x*v.y + s*v.z; pOut->m[0][2] = t*v.x*v.z - s*v.y;
	pOut->m[1][0] = t*v.x*v.y - s*v.z; pOut->m[1][1] = t*v.y*v.y + c; pOut->m[1][2] = t*v.y*v.z + s*v.x;
	pOut->m[2][0] = t*v.x*v.z + s*v.y; pOut->m[2][1] = t*v.y*v.z - s*v.x; pOut->m[2][2] = t*v.z*v.z + c; return pOut;
}
inline GLMATRIX* D3DXMatrixRotationYawPitchRoll(GLMATRIX* pOut, float y, float p, float r) {
	GLMATRIX my, mp, mr; D3DXMatrixRotationY(&my, y); D3DXMatrixRotationX(&mp, p); D3DXMatrixRotationZ(&mr, r);
	D3DXMatrixMultiply(pOut, &mr, &mp); D3DXMatrixMultiply(pOut, pOut, &my); return pOut;
}
inline GLMATRIX* D3DXMatrixRotationQuaternion(GLMATRIX* pOut, const GLQUATERNION* pQ) {
	D3DXMatrixIdentity(pOut);
	float xx = pQ->x*pQ->x, yy = pQ->y*pQ->y, zz = pQ->z*pQ->z, xy = pQ->x*pQ->y, xz = pQ->x*pQ->z, yz = pQ->y*pQ->z, wx = pQ->w*pQ->x, wy = pQ->w*pQ->y, wz = pQ->w*pQ->z;
	pOut->m[0][0] = 1.0f - 2.0f*(yy + zz); pOut->m[0][1] = 2.0f*(xy + wz); pOut->m[0][2] = 2.0f*(xz - wy);
	pOut->m[1][0] = 2.0f*(xy - wz); pOut->m[1][1] = 1.0f - 2.0f*(xx + zz); pOut->m[1][2] = 2.0f*(yz + wx);
	pOut->m[2][0] = 2.0f*(xz + wy); pOut->m[2][1] = 2.0f*(yz - wx); pOut->m[2][2] = 1.0f - 2.0f*(xx + yy); return pOut;
}

inline GLQUATERNION* D3DXQuaternionMultiply(GLQUATERNION* pOut, const GLQUATERNION* pQ1, const GLQUATERNION* pQ2) {
	GLQUATERNION q(pQ2->w*pQ1->x + pQ2->x*pQ1->w + pQ2->y*pQ1->z - pQ2->z*pQ1->y, pQ2->w*pQ1->y - pQ2->x*pQ1->z + pQ2->y*pQ1->w + pQ2->z*pQ1->x, pQ2->w*pQ1->z + pQ2->x*pQ1->y - pQ2->y*pQ1->x + pQ2->z*pQ1->w, pQ2->w*pQ1->w - pQ2->x*pQ1->x - pQ2->y*pQ1->y - pQ2->z*pQ1->z);
	*pOut = q; return pOut;
}
inline GLQUATERNION* D3DXQuaternionConjugate(GLQUATERNION* pOut, const GLQUATERNION* pQ) { pOut->x = -pQ->x; pOut->y = -pQ->y; pOut->z = -pQ->z; pOut->w = pQ->w; return pOut; }
inline GLQUATERNION* D3DXQuaternionRotationAxis(GLQUATERNION* pOut, const GLVECTOR3* pV, float a) { GLVECTOR3 v; D3DXVec3Normalize(&v, pV); float s = sinf(a * 0.5f); pOut->x = v.x * s; pOut->y = v.y * s; pOut->z = v.z * s; pOut->w = cosf(a * 0.5f); return pOut; }
inline GLQUATERNION* D3DXQuaternionRotationYawPitchRoll(GLQUATERNION* pOut, float y, float p, float r) {
	float sy = sinf(y*0.5f), cy = cosf(y*0.5f), sp = sinf(p*0.5f), cp = cosf(p*0.5f), sr = sinf(r*0.5f), cr = cosf(r*0.5f);
	pOut->x = cy*sp*cr + sy*cp*sr; pOut->y = sy*cp*cr - cy*sp*sr; pOut->z = cy*cp*sr - sy*sp*cr; pOut->w = cy*cp*cr + sy*sp*sr; return pOut;
}
inline GLQUATERNION* D3DXQuaternionRotationMatrix(GLQUATERNION* pOut, const GLMATRIX* pM) { pOut->x = pOut->y = pOut->z = 0.0f; pOut->w = 1.0f; return pOut; }
inline GLQUATERNION* D3DXQuaternionSlerp(GLQUATERNION* pOut, const GLQUATERNION* pQ1, const GLQUATERNION* pQ2, float t) { pOut->x = pQ1->x + t*(pQ2->x - pQ1->x); pOut->y = pQ1->y + t*(pQ2->y - pQ1->y); pOut->z = pQ1->z + t*(pQ2->z - pQ1->z); pOut->w = pQ1->w + t*(pQ2->w - pQ1->w); return pOut; }

inline float D3DXPlaneDotCoord(const GLPLANE* pP, const GLVECTOR3* pV) { return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z + pP->d; }
inline float D3DXPlaneDotNormal(const GLPLANE* pP, const GLVECTOR3* pV) { return pP->a * pV->x + pP->b * pV->y + pP->c * pV->z; }
inline GLPLANE* D3DXPlaneNormalize(GLPLANE* pOut, const GLPLANE* pP) { float l = sqrtf(pP->a*pP->a + pP->b*pP->b + pP->c*pP->c); if (l > 1e-6f) { pOut->a = pP->a/l; pOut->b = pP->b/l; pOut->c = pP->c/l; pOut->d = pP->d/l; } else *pOut = *pP; return pOut; }
inline GLPLANE* D3DXPlaneFromPointNormal(GLPLANE* pOut, const GLVECTOR3* pPt, const GLVECTOR3* pN) { pOut->a = pN->x; pOut->b = pN->y; pOut->c = pN->z; pOut->d = -D3DXVec3Dot(pPt, pN); return pOut; }
inline GLPLANE* D3DXPlaneFromPoints(GLPLANE* pOut, const GLVECTOR3* pV1, const GLVECTOR3* pV2, const GLVECTOR3* pV3) { GLVECTOR3 e1 = *pV2 - *pV1, e2 = *pV3 - *pV1, n; D3DXVec3Cross(&n, &e1, &e2); D3DXVec3Normalize(&n, &n); return D3DXPlaneFromPointNormal(pOut, pV1, &n); }
inline GLVECTOR3* D3DXPlaneIntersectLine(GLVECTOR3* pOut, const GLPLANE* pP, const GLVECTOR3* pV1, const GLVECTOR3* pV2) { GLVECTOR3 d = *pV2 - *pV1; float denom = pP->a*d.x + pP->b*d.y + pP->c*d.z; if (fabsf(denom) < 1e-6f) return NULL; float u = -(pP->a*pV1->x + pP->b*pV1->y + pP->c*pV1->z + pP->d) / denom; *pOut = *pV1 + d * u; return pOut; }

inline float D3DXMatrixfDeterminant(const GLMATRIX* pM) { float det = 1.0f; GLMATRIX tmp; D3DXMatrixInverse(&tmp, &det, pM); return det; }
inline GLMATRIX* D3DXMatrixLookAtRH(GLMATRIX* pOut, const GLVECTOR3* pEye, const GLVECTOR3* pAt, const GLVECTOR3* pUp) {
	GLVECTOR3 f, s, u, tmp(pAt->x - pEye->x, pAt->y - pEye->y, pAt->z - pEye->z);
	D3DXVec3Normalize(&f, &tmp); D3DXVec3Cross(&s, &f, pUp); D3DXVec3Normalize(&s, &s); D3DXVec3Cross(&u, &s, &f);
	D3DXMatrixIdentity(pOut);
	pOut->m[0][0] = s.x; pOut->m[1][0] = s.y; pOut->m[2][0] = s.z;
	pOut->m[0][1] = u.x; pOut->m[1][1] = u.y; pOut->m[2][1] = u.z;
	pOut->m[0][2] = -f.x; pOut->m[1][2] = -f.y; pOut->m[2][2] = -f.z;
	pOut->m[3][0] = -D3DXVec3Dot(&s, pEye); pOut->m[3][1] = -D3DXVec3Dot(&u, pEye); pOut->m[3][2] = D3DXVec3Dot(&f, pEye); return pOut;
}
inline GLMATRIX* D3DXMatrixPerspectiveFovRH(GLMATRIX* pOut, float fovy, float Aspect, float zn, float zf) {
	float h = 1.0f / tanf(fovy / 2.0f), w = h / Aspect; D3DXMatrixIdentity(pOut);
	pOut->m[0][0] = w; pOut->m[1][1] = h; pOut->m[2][2] = zf / (zn - zf); pOut->m[2][3] = -1.0f; pOut->m[3][2] = (zn * zf) / (zn - zf); pOut->m[3][3] = 0.0f; return pOut;
}
inline GLMATRIX* D3DXMatrixOrthoRH(GLMATRIX* pOut, float w, float h, float zn, float zf) {
	D3DXMatrixIdentity(pOut); pOut->m[0][0] = 2.0f / w; pOut->m[1][1] = 2.0f / h; pOut->m[2][2] = 1.0f / (zn - zf); pOut->m[3][2] = zn / (zn - zf); return pOut;
}
inline GLMATRIX* D3DXMatrixOrthoOffCenterRH(GLMATRIX* pOut, float l, float r, float b, float t, float zn, float zf) {
	D3DXMatrixIdentity(pOut);
	if (l == 0.0f && t == 0.0f && r > 0.0f && b > 0.0f) {
		l -= 0.5f; r -= 0.5f;
		t -= 0.5f; b -= 0.5f;
	}
	if (r != l) { pOut->m[0][0] = 2.0f / (r - l); pOut->m[3][0] = (l + r) / (l - r); }
	if (t != b) { pOut->m[1][1] = 2.0f / (t - b); pOut->m[3][1] = (t + b) / (b - t); }
	if (zn != zf) { pOut->m[2][2] = 1.0f / (zn - zf); pOut->m[3][2] = zn / (zn - zf); }
	return pOut;
}
inline GLVECTOR3* D3DXVec3Project(GLVECTOR3* pOut, const GLVECTOR3* pV, const D3DVIEWPORT8* pVp, const GLMATRIX* pProj, const GLMATRIX* pView, const GLMATRIX* pWorld) {
	GLMATRIX mat; D3DXMatrixMultiply(&mat, pWorld, pView); D3DXMatrixMultiply(&mat, &mat, pProj);
	GLVECTOR3 v; D3DXVec3TransformCoord(&v, pV, &mat);
	pOut->x = pVp->X + (1.0f + v.x) * pVp->Width * 0.5f; pOut->y = pVp->Y + (1.0f - v.y) * pVp->Height * 0.5f; pOut->z = pVp->MinZ + v.z * (pVp->MaxZ - pVp->MinZ); return pOut;
}
inline GLVECTOR3* D3DXVec3Unproject(GLVECTOR3* pOut, const GLVECTOR3* pV, const D3DVIEWPORT8* pVp, const GLMATRIX* pProj, const GLMATRIX* pView, const GLMATRIX* pWorld) {
	GLMATRIX mat, inv; D3DXMatrixMultiply(&mat, pWorld, pView); D3DXMatrixMultiply(&mat, &mat, pProj); D3DXMatrixInverse(&inv, NULL, &mat);
	GLVECTOR3 v(((pV->x - pVp->X) / pVp->Width) * 2.0f - 1.0f, 1.0f - ((pV->y - pVp->Y) / pVp->Height) * 2.0f, (pV->z - pVp->MinZ) / (pVp->MaxZ - pVp->MinZ));
	return D3DXVec3TransformCoord(pOut, &v, &inv);
}

inline BYTE* GetAndroidSafeScratchBuffer(size_t minBytes) {
	static std::vector<BYTE> s_scratch(65536, 0);
	if (minBytes > s_scratch.size()) s_scratch.resize(minBytes, 0);
	return s_scratch.data();
}

struct IDirect3DSurface8 {
	ULONG m_refCount;
	int width, height;
	void* pLockedData;
	IDirect3DTexture8* pParentTex;
	IDirect3DSurface8(int w = 1024, int h = 1024, IDirect3DTexture8* parent = NULL)
		: m_refCount(1), width(w > 0 ? w : 1024), height(h > 0 ? h : 1024), pLockedData(NULL), pParentTex(parent) {}
	~IDirect3DSurface8() { if (pLockedData) free(pLockedData); }
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
	HRESULT LockRect(D3DLOCKED_RECT* p, const RECT*, DWORD) {
		if (!pLockedData) pLockedData = calloc((size_t)width * (size_t)height, 4);
		if (!pLockedData) {
			if (p) { p->pBits = GetAndroidSafeScratchBuffer((size_t)width * 4); p->Pitch = width * 4; }
			return E_OUTOFMEMORY;
		}
		if (p) { p->pBits = pLockedData; p->Pitch = width * 4; }
		return S_OK;
	}
	HRESULT UnlockRect() { return S_OK; }
	HRESULT GetDesc(D3DSURFACE_DESC* p) {
		if (p) { memset(p, 0, sizeof(D3DSURFACE_DESC)); p->Width = width; p->Height = height; p->Format = D3DFMT_A8R8G8B8; }
		return S_OK;
	}
};

struct IDirect3DBaseTexture8 {
	ULONG m_refCount;
	IDirect3DBaseTexture8() : m_refCount(1) {}
	virtual ~IDirect3DBaseTexture8() {}
	virtual ULONG AddRef() { return ++m_refCount; }
	virtual ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
	virtual DWORD GetLevelCount() { return 1; }
};

inline IDirect3DDevice8*& GetCurrentGLDevice() {
	static IDirect3DDevice8* s_pDev = NULL;
	return s_pDev;
}

#ifdef __ANDROID__
inline void SwizzleBGRAtoRGBAInPlace(DWORD* pix, int total, bool forceOpaque) {
	int i = 0;
#if defined(__ARM_NEON) || defined(__ARM_NEON__) || defined(__aarch64__)
	const uint8x8_t fullAlpha = vdup_n_u8(0xFF);
	for (; i + 8 <= total; i += 8) {
		uint8x8x4_t v = vld4_u8((const uint8_t*)(pix + i));
		uint8x8_t tmp = v.val[0];
		v.val[0] = v.val[2];
		v.val[2] = tmp;
		if (forceOpaque) v.val[3] = fullAlpha;
		vst4_u8((uint8_t*)(pix + i), v);
	}
#endif
	for (; i < total; ++i) {
		DWORD c = pix[i];
		DWORD a = forceOpaque ? 0xFF000000u : (c & 0xFF000000u);
		pix[i] = a | ((c & 0xFFu) << 16) | (c & 0x0000FF00u) | ((c >> 16) & 0xFFu);
	}
}
#endif

struct IDirect3DTexture8 : public IDirect3DBaseTexture8 {
	GLuint glId, glFbo, glDepthRbo; int width, height; D3DFORMAT format; void* pLockedData;
	GLint lastWrapS, lastWrapT;
	IDirect3DTexture8() : glId(0), glFbo(0), glDepthRbo(0), width(256), height(256), format(D3DFMT_A8R8G8B8), pLockedData(NULL), lastWrapS(GL_REPEAT), lastWrapT(GL_REPEAT) {}
	virtual ~IDirect3DTexture8();
	HRESULT LockRect(UINT, D3DLOCKED_RECT* p, const RECT*, DWORD) {
		int w = (width > 0 && width <= 8192) ? width : 256;
		int h = (height > 0 && height <= 8192) ? height : 256;
		int bpp = (format == D3DFMT_A4R4G4B4 || format == D3DFMT_A1R5G5B5 || format == D3DFMT_R5G6B5) ? 2 : 4;
		if (!pLockedData) pLockedData = calloc((size_t)w * (size_t)h, 4);
		if (!pLockedData) {
			if (p) { p->pBits = GetAndroidSafeScratchBuffer((size_t)w * (size_t)bpp); p->Pitch = w * bpp; }
			return E_OUTOFMEMORY;
		}
		if (p) { p->pBits = pLockedData; p->Pitch = w * bpp; }
		return S_OK;
	}
	HRESULT UnlockRect(UINT Level);
	DWORD GetLevelCount() override { return 1; }
	HRESULT GetSurfaceLevel(UINT, LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(width, height, this); return S_OK; }
	HRESULT GetLevelDesc(UINT, D3DSURFACE_DESC* p) { if (p) { memset(p, 0, sizeof(D3DSURFACE_DESC)); p->Width = width; p->Height = height; p->Format = format; } return S_OK; }
};

struct IDirect3DVertexBuffer8 {
	ULONG m_refCount;
	GLuint glVbo; UINT length; DWORD usage; DWORD lastLockFlags; bool bDirty; void* pData;
	IDirect3DVertexBuffer8(UINT len, DWORD dwUsage = 0)
		: m_refCount(1), glVbo(0), length(len > 0 ? len : 4), usage(dwUsage), lastLockFlags(0), bDirty(true), pData(NULL) {
		pData = calloc(this->length, 1);
	}
	~IDirect3DVertexBuffer8();
	HRESULT Lock(UINT Offset, UINT Size, BYTE** ppbData, DWORD Flags) {
		if (!pData) pData = calloc(this->length, 1);
		if (!pData) {
			if (ppbData) *ppbData = GetAndroidSafeScratchBuffer(Size > 0 ? Size : 4096);
			return E_OUTOFMEMORY;
		}
		lastLockFlags = Flags;
		if ((Flags & D3DLOCK_READONLY) == 0) bDirty = true;
		if (ppbData) *ppbData = (Offset < length) ? ((BYTE*)pData + Offset) : (BYTE*)pData;
		return S_OK;
	}
	HRESULT Lock(UINT Offset, UINT Size, void** ppvData, DWORD Flags) {
		return Lock(Offset, Size, (BYTE**)ppvData, Flags);
	}
	HRESULT Unlock() {
		if ((lastLockFlags & D3DLOCK_READONLY) == 0) bDirty = true;
		return S_OK;
	}
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
};

struct IDirect3DIndexBuffer8 {
	ULONG m_refCount;
	GLuint glIbo; UINT length; DWORD usage; DWORD lastLockFlags; bool bDirty; void* pData;
	IDirect3DIndexBuffer8(UINT len, DWORD dwUsage = 0)
		: m_refCount(1), glIbo(0), length(len > 0 ? len : 4), usage(dwUsage), lastLockFlags(0), bDirty(true), pData(NULL) {
		pData = calloc(this->length, 1);
	}
	~IDirect3DIndexBuffer8();
	HRESULT Lock(UINT Offset, UINT Size, BYTE** ppbData, DWORD Flags) {
		if (!pData) pData = calloc(this->length, 1);
		if (!pData) {
			if (ppbData) *ppbData = GetAndroidSafeScratchBuffer(Size > 0 ? Size : 4096);
			return E_OUTOFMEMORY;
		}
		lastLockFlags = Flags;
		if ((Flags & D3DLOCK_READONLY) == 0) bDirty = true;
		if (ppbData) *ppbData = (Offset < length) ? ((BYTE*)pData + Offset) : (BYTE*)pData;
		return S_OK;
	}
	HRESULT Lock(UINT Offset, UINT Size, void** ppvData, DWORD Flags) {
		return Lock(Offset, Size, (BYTE**)ppvData, Flags);
	}
	HRESULT Unlock() {
		if ((lastLockFlags & D3DLOCK_READONLY) == 0) bDirty = true;
		return S_OK;
	}
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
};

enum EAndroidPixelShaderType {
	PS_TYPE_NONE = 0,
	PS_TYPE_SHADOW_DEPTH = 1,
	PS_TYPE_TERRAIN_BASE = 2,
	PS_TYPE_TERRAIN_SHADOW = 3,
	PS_TYPE_SHADOW_MAIN = 4,
	PS_TYPE_WATER = 5,
	PS_TYPE_GRASS = 6,
	PS_TYPE_SHADOW_OBJECTS = 7
};

struct IDirect3DVertexShader9 { ULONG m_refCount = 1; int shaderType = 0; ULONG AddRef() { return ++m_refCount; } ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; } };
struct IDirect3DPixelShader9 { ULONG m_refCount = 1; int shaderType = 0; ULONG AddRef() { return ++m_refCount; } ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; } };
struct IDirect3DVertexDeclaration9 { ULONG m_refCount = 1; ULONG AddRef() { return ++m_refCount; } ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; } };

typedef IDirect3DVertexShader9* LPDIRECT3DVERTEXSHADER9;
typedef IDirect3DPixelShader9* LPDIRECT3DPIXELSHADER9;
typedef IDirect3DVertexDeclaration9* LPDIRECT3DVERTEXDECLARATION9;
typedef IDirect3DPixelShader9* LPDIRECT3DPIXELSHADER8;
typedef IDirect3DVertexShader9* LPDIRECT3DVERTEXSHADER8;

#define D3DQUERYTYPE_OCCLUSION 9
#define D3DISSUE_END (1 << 0)
#define D3DISSUE_BEGIN (1 << 1)
#define D3DGETDATA_FLUSH (1 << 0)
typedef int D3DQUERYTYPE;

inline DWORD& GetAndroidQueryDrawCounter() {
	static DWORD s_dwCount = 0;
	return s_dwCount;
}

struct IDirect3DQuery9 {
	ULONG m_refCount = 1;
	DWORD m_dwStartCount = 0;
	DWORD m_dwLastResult = 0;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
	HRESULT Issue(DWORD dwIssueFlags) {
		if (dwIssueFlags == D3DISSUE_BEGIN)
			m_dwStartCount = GetAndroidQueryDrawCounter();
		else if (dwIssueFlags == D3DISSUE_END)
			m_dwLastResult = GetAndroidQueryDrawCounter() - m_dwStartCount;
		return S_OK;
	}
	HRESULT GetData(void* pData, DWORD dwSize, DWORD) {
		if (pData && dwSize >= sizeof(DWORD)) *(DWORD*)pData = m_dwLastResult;
		return S_OK;
	}
};
typedef IDirect3DQuery9* LPDIRECT3DQUERY9;

#define D3DSBT_ALL 1
#define D3DSBT_PIXELSTATE 2
#define D3DSBT_VERTEXSTATE 3
typedef int D3DSTATEBLOCKTYPE;

struct IDirect3DStateBlock9 {
	ULONG m_refCount = 1;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
	HRESULT Capture() { return S_OK; }
	HRESULT Apply() { return S_OK; }
};
typedef IDirect3DStateBlock9* LPDIRECT3DSTATEBLOCK9;

typedef const char* D3DXHANDLE;
struct ID3DXConstantTable {
	ULONG m_refCount = 1;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
	HRESULT SetDefaults(LPDIRECT3DDEVICE8) { return S_OK; }
	D3DXHANDLE GetConstantByName(D3DXHANDLE, LPCSTR pName) { return pName; }
	UINT GetSamplerIndex(D3DXHANDLE) { return 0; }
	HRESULT SetValue(LPDIRECT3DDEVICE8, D3DXHANDLE, LPCVOID, UINT) { return S_OK; }
	HRESULT SetBool(LPDIRECT3DDEVICE8, D3DXHANDLE, BOOL) { return S_OK; }
	HRESULT SetInt(LPDIRECT3DDEVICE8, D3DXHANDLE, int) { return S_OK; }
	HRESULT SetFloat(LPDIRECT3DDEVICE8, D3DXHANDLE, float) { return S_OK; }
	HRESULT SetFloatArray(LPDIRECT3DDEVICE8, D3DXHANDLE, const float*, UINT) { return S_OK; }
	HRESULT SetVector(LPDIRECT3DDEVICE8, D3DXHANDLE, const GLVECTOR4*) { return S_OK; }
	HRESULT SetVectorArray(LPDIRECT3DDEVICE8, D3DXHANDLE, const GLVECTOR4*, UINT) { return S_OK; }
	HRESULT SetMatrix(LPDIRECT3DDEVICE8, D3DXHANDLE, const GLMATRIX*) { return S_OK; }
	HRESULT SetMatrixArray(LPDIRECT3DDEVICE8, D3DXHANDLE, const GLMATRIX*, UINT) { return S_OK; }
};
typedef ID3DXConstantTable* LPD3DXCONSTANTTABLE;

inline UINT D3DXGetFVFVertexSize(DWORD FVF);

#ifdef __ANDROID__
extern "C" int g_AndroidPhysWidth;
extern "C" int g_AndroidPhysHeight;
#endif

struct IDirect3DDevice8 {
#ifndef ANDROID
	HDC m_hDC;
	HWND m_hWnd;
#endif
	DWORD m_dwFVF;
	LPDIRECT3DVERTEXBUFFER8 m_pStreamSource;
	UINT m_StreamStride;
	LPDIRECT3DINDEXBUFFER8 m_pIndexBuffer;
	UINT m_BaseVertexIndex;
	GLMATRIX m_matWorld, m_matView, m_matProj;
	GLMATRIX m_matTex[8];
	DWORD m_TexCoordIndex[8];
	DWORD m_TexTransformFlags[8];
	DWORD m_ColorOp[8], m_ColorArg1[8], m_ColorArg2[8];
	DWORD m_AlphaOp[8], m_AlphaArg1[8], m_AlphaArg2[8];
	DWORD m_AddressU[8], m_AddressV[8];
	LPDIRECT3DBaseTexture8 m_pBoundTextures[16];
	GLuint m_BoundGLTexId[8];
	DWORD m_RenderStates[256];
	int m_ActivePixelShaderType;
	float m_vsConsts[256][4];
	float m_psConsts[256][4];
	float m_ClipPlane0[4];
	DWORD m_ClipPlaneEnable;
	D3DVIEWPORT8 m_ScreenViewport;
	IDirect3DTexture8* m_pCurrentRenderTargetTex;
	D3DVIEWPORT8 m_Viewport;
	GLenum m_SrcBlend, m_DstBlend;
	float m_FogStart, m_FogEnd;
	DWORD m_AlphaTestEnable;
	DWORD m_AlphaRef;
	DWORD m_TextureFactor;
	DWORD m_LightingEnable;
	DWORD m_Light0Enabled;
	D3DLIGHT8 m_Light0;
	D3DMATERIAL8 m_Material;
	DWORD m_AmbientColor;
	DWORD m_ColorVertex;
	DWORD m_DiffuseMaterialSource;
	DWORD m_FogEnable;
	DWORD m_FogColor;
	DWORD m_DirtyFlags;

	IDirect3DDevice8() : m_dwFVF(0), m_pStreamSource(NULL), m_StreamStride(0), m_pIndexBuffer(NULL), m_BaseVertexIndex(0), m_pCurrentRenderTargetTex(NULL), m_SrcBlend(GL_SRC_ALPHA), m_DstBlend(GL_ONE_MINUS_SRC_ALPHA), m_FogStart(2000.0f), m_FogEnd(45000.0f), m_AlphaTestEnable(0), m_AlphaRef(1), m_TextureFactor(0xFFFFFFFF) {
#ifndef ANDROID
		m_hDC = NULL;
		m_hWnd = NULL;
#endif
		for (int i = 0; i < 8; ++i) {
			m_TexCoordIndex[i] = i;
			m_TexTransformFlags[i] = D3DTTFF_DISABLE;
			m_ColorOp[i] = (i == 0) ? D3DTOP_MODULATE : D3DTOP_DISABLE;
			m_ColorArg1[i] = D3DTA_TEXTURE;
			m_ColorArg2[i] = D3DTA_CURRENT;
			m_AlphaOp[i] = (i == 0) ? D3DTOP_SELECTARG1 : D3DTOP_DISABLE;
			m_AlphaArg1[i] = D3DTA_TEXTURE;
			m_AlphaArg2[i] = D3DTA_CURRENT;
			m_pBoundTextures[i] = NULL;
			m_AddressU[i] = D3DTADDRESS_WRAP;
			m_AddressV[i] = D3DTADDRESS_WRAP;
		}
		m_Viewport.X = 0; m_Viewport.Y = 0; m_Viewport.Width = 1024; m_Viewport.Height = 768; m_Viewport.MinZ = 0.0f; m_Viewport.MaxZ = 1.0f;
		m_LightingEnable = 0;
		m_Light0Enabled = 0;
		memset(&m_Light0, 0, sizeof(m_Light0));
		m_Light0.Direction = GLVECTOR3(0.0f, 0.0f, -1.0f);
		m_Light0.Diffuse = GLCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		m_Light0.Ambient = GLCOLOR(0.5f, 0.5f, 0.5f, 1.0f);
		memset(&m_Material, 0, sizeof(m_Material));
		m_Material.Diffuse = GLCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		m_Material.Ambient = GLCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
		m_AmbientColor = 0x00000000;
		m_ColorVertex = 1;
		m_DiffuseMaterialSource = D3DMCS_COLOR1;
		m_FogEnable = 0;
		m_FogColor = 0xFF000000;
		for (int i = 0; i < 16; ++i) m_pBoundTextures[i] = NULL;
		for (int i = 0; i < 8; ++i) m_BoundGLTexId[i] = 0xFFFFFFFFu;
		for (int i = 0; i < 256; ++i) m_RenderStates[i] = 0xDEADBEEFu;
		m_ActivePixelShaderType = 0;
		memset(m_vsConsts, 0, sizeof(m_vsConsts));
		memset(m_psConsts, 0, sizeof(m_psConsts));
		memset(m_ClipPlane0, 0, sizeof(m_ClipPlane0));
		m_ClipPlaneEnable = 0;
		m_ScreenViewport = m_Viewport;
		m_DirtyFlags = 0xFFFFFFFFu;
		GetCurrentGLDevice() = this;
	}
	~IDirect3DDevice8() {
		if (GetCurrentGLDevice() == this) GetCurrentGLDevice() = NULL;
	}
	HRESULT CreateTexture(UINT W, UINT H, UINT, DWORD Usage, D3DFORMAT Fmt, D3DPOOL, LPDIRECT3DTEXTURE8* ppTex) {
		if (W == 0 || W > 8192) W = 256;
		if (H == 0 || H > 8192) H = 256;
		IDirect3DTexture8* tex = new IDirect3DTexture8(); tex->width = W; tex->height = H; tex->format = Fmt;
		glActiveTexture(GL_TEXTURE0);
		GLuint prevTex = m_pBoundTextures[0] ? ((IDirect3DTexture8*)m_pBoundTextures[0])->glId : 0;
		glGenTextures(1, &tex->glId); glBindTexture(GL_TEXTURE_2D, tex->glId);
		GLint filter = (Fmt == D3DFMT_R32F) ? GL_NEAREST : GL_LINEAR;
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
#ifdef __ANDROID__
		if (Usage & D3DUSAGE_RENDERTARGET) {
			std::vector<DWORD> whitePixels((size_t)W * (size_t)H, 0xFFFFFFFF);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixels.data());
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		}
#else
		if (Usage & D3DUSAGE_RENDERTARGET) {
			std::vector<DWORD> whitePixels((size_t)W * (size_t)H, 0xFFFFFFFF);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, whitePixels.data());
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, W, H, 0, GL_BGRA_EXT, GL_UNSIGNED_BYTE, NULL);
		}
#endif
		glBindTexture(GL_TEXTURE_2D, prevTex);
		*ppTex = tex; return S_OK;
	}
	HRESULT CreateImageSurface(UINT W, UINT H, D3DFORMAT, LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(W, H); return S_OK; }
	HRESULT CopyRects(LPDIRECT3DSURFACE8, const RECT*, UINT, LPDIRECT3DSURFACE8, const POINT*) { return S_OK; }
	HRESULT UpdateTexture(LPDIRECT3DBaseTexture8, LPDIRECT3DBaseTexture8) { return S_OK; }
	HRESULT GetVertexShader(DWORD* p) { if (p) *p = m_dwFVF; return S_OK; }
	HRESULT SetVertexShader(DWORD s) { if (s > 1 && m_dwFVF != s) { m_dwFVF = s; m_DirtyFlags |= 4; } return S_OK; }
	HRESULT CreateVertexShader(const DWORD*, const DWORD*, DWORD* p, DWORD) { if (p) *p = 1; return S_OK; }
	HRESULT DeleteVertexShader(DWORD) { return S_OK; }
	HRESULT CreatePixelShader(const DWORD*, DWORD* p) { if (p) *p = 2; return S_OK; }
	HRESULT DeletePixelShader(DWORD) { return S_OK; }
	HRESULT SetPixelShader(DWORD s) { if (s == 0 && m_ActivePixelShaderType != 0) { m_ActivePixelShaderType = 0; m_DirtyFlags |= 12; } return S_OK; }
	HRESULT SetVertexShaderConstant(DWORD reg, const void* data, DWORD count) {
		return SetVertexShaderConstantF(reg, (const float*)data, count);
	}
	HRESULT SetPixelShaderConstant(DWORD reg, const void* data, DWORD count) {
		return SetPixelShaderConstantF(reg, (const float*)data, count);
	}
	HRESULT SetFVF(DWORD dwFVF) { if (dwFVF > 1 && m_dwFVF != dwFVF) { m_dwFVF = dwFVF; m_DirtyFlags |= 4; } return S_OK; }
	HRESULT GetFVF(DWORD* pdwFVF) { if (pdwFVF) *pdwFVF = m_dwFVF; return S_OK; }
	HRESULT CreateVertexShader(const DWORD*, IDirect3DVertexShader9** ppShader) { if (ppShader) *ppShader = new IDirect3DVertexShader9(); return S_OK; }
	HRESULT SetVertexShader(IDirect3DVertexShader9*) { return S_OK; }
	HRESULT GetVertexShader(IDirect3DVertexShader9** ppShader) { if (ppShader) *ppShader = NULL; return S_OK; }
	HRESULT CreatePixelShader(const DWORD* pCode, IDirect3DPixelShader9** ppShader) {
		if (ppShader) {
			IDirect3DPixelShader9* ps = new IDirect3DPixelShader9();
			if (pCode) ps->shaderType = (int)pCode[0];
			*ppShader = ps;
		}
		return S_OK;
	}
	HRESULT SetPixelShader(IDirect3DPixelShader9* pShader) {
    int prev = m_ActivePixelShaderType;
    if (!pShader) {
        m_ActivePixelShaderType = 0;
    } else if (m_pCurrentRenderTargetTex && m_ActivePixelShaderType == PS_TYPE_SHADOW_DEPTH) {
    } else {
        m_ActivePixelShaderType = pShader->shaderType;
    }
    if (m_ActivePixelShaderType != prev) m_DirtyFlags |= 12;
    return S_OK;
}
	HRESULT GetPixelShader(IDirect3DPixelShader9** ppShader) { if (ppShader) *ppShader = NULL; return S_OK; }
	HRESULT CreateVertexDeclaration(const void*, IDirect3DVertexDeclaration9** ppDecl) { if (ppDecl) *ppDecl = new IDirect3DVertexDeclaration9(); return S_OK; }
	HRESULT SetVertexDeclaration(IDirect3DVertexDeclaration9*) { return S_OK; }
	HRESULT GetVertexDeclaration(IDirect3DVertexDeclaration9** ppDecl) { if (ppDecl) *ppDecl = NULL; return S_OK; }
	HRESULT SetVertexShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
		if (pConstantData && StartRegister < 256) {
			UINT maxCount = (StartRegister + Vector4fCount <= 256) ? Vector4fCount : (256 - StartRegister);
			size_t bytes = maxCount * 4 * sizeof(float);
			if (memcmp(&m_vsConsts[StartRegister][0], pConstantData, bytes) != 0) {
				memcpy(&m_vsConsts[StartRegister][0], pConstantData, bytes);
				m_DirtyFlags |= 8;
			}
		}
		return S_OK;
	}
	HRESULT SetPixelShaderConstantF(UINT StartRegister, const float* pConstantData, UINT Vector4fCount) {
		if (pConstantData && StartRegister < 256) {
			UINT maxCount = (StartRegister + Vector4fCount <= 256) ? Vector4fCount : (256 - StartRegister);
			size_t bytes = maxCount * 4 * sizeof(float);
			if (memcmp(&m_psConsts[StartRegister][0], pConstantData, bytes) != 0) {
				memcpy(&m_psConsts[StartRegister][0], pConstantData, bytes);
				m_DirtyFlags |= 8;
			}
		}
		return S_OK;
	}
	HRESULT SetSamplerState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) { return SetTextureStageState(stage, type, value); }
	HRESULT GetSamplerState(DWORD, D3DTEXTURESTAGESTATETYPE, DWORD* pVal) { if (pVal) *pVal = 0; return S_OK; }
	HRESULT SetClipPlane(DWORD Index, const float* pPlane) {
		if (Index == 0 && pPlane) { memcpy(m_ClipPlane0, pPlane, 4 * sizeof(float)); m_DirtyFlags |= 4; }
		return S_OK;
	}
	HRESULT TestCooperativeLevel() { return S_OK; }
	HRESULT CreateDepthStencilSurface(UINT W, UINT H, D3DFORMAT, D3DMULTISAMPLE_TYPE, LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(W, H); return S_OK; }
	HRESULT GetRenderTarget(LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(m_Viewport.Width, m_Viewport.Height); return S_OK; }
	HRESULT GetDepthStencilSurface(LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(m_Viewport.Width, m_Viewport.Height); return S_OK; }
	HRESULT GetViewport(D3DVIEWPORT8* pVp) { if (pVp) *pVp = m_Viewport; return S_OK; }
	#ifdef ANDROID
	HRESULT SetRenderTarget(LPDIRECT3DSURFACE8 pRenderTarget, LPDIRECT3DSURFACE8 pNewZStencil);
#else
	HRESULT SetRenderTarget(LPDIRECT3DSURFACE8 pRenderTarget, LPDIRECT3DSURFACE8) {
#ifndef ANDROID
		if (m_pCurrentRenderTargetTex && (!pRenderTarget || !pRenderTarget->pParentTex)) {
			int w = m_pCurrentRenderTargetTex->width;
			int h = m_pCurrentRenderTargetTex->height;
			if (w > 0 && h > 0 && w <= 2048 && h <= 2048) {
				std::vector<DWORD> pixels((size_t)w * (size_t)h);
				std::vector<DWORD> flipped((size_t)w * (size_t)h);
				glReadPixels(0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pixels.data());
				for (int y = 0; y < h; ++y) {
					memcpy(&flipped[(size_t)y * (size_t)w], &pixels[(size_t)(h - 1 - y) * (size_t)w], (size_t)w * 4);
				}
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, m_pCurrentRenderTargetTex->glId);
				glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, flipped.data());
			}
			m_pCurrentRenderTargetTex = NULL;
		}
		if (pRenderTarget && pRenderTarget->pParentTex) {
			m_pCurrentRenderTargetTex = pRenderTarget->pParentTex;
		}
#endif
		return S_OK;
	}
#endif
	HRESULT SetViewport(const D3DVIEWPORT8* v) {
		if (v && v->Width > 0 && v->Height > 0) {
			if (m_pCurrentRenderTargetTex) {
				glViewport(v->X, v->Y, v->Width, v->Height);
			} else {
				m_Viewport = *v;
				m_ScreenViewport = *v;
#ifdef __ANDROID__
				glViewport(0, 0,
					(g_AndroidPhysWidth > 0) ? g_AndroidPhysWidth : v->Width,
					(g_AndroidPhysHeight > 0) ? g_AndroidPhysHeight : v->Height);
#else
				glViewport(v->X, v->Y, v->Width, v->Height);
#endif
			}
		}
		return S_OK;
	}
	HRESULT GetDeviceCaps(D3DCAPS8* p) {
		if (p) {
			memset(p, 0, sizeof(D3DCAPS8));
			p->MaxTextureWidth = 8192;
			p->MaxTextureHeight = 8192;
			p->MaxSimultaneousTextures = 8;
			p->MaxTextureBlendStages = 8;
			p->MaxActiveLights = 8;
			p->MaxStreams = 16;
			p->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
			p->Caps2 = D3DCAPS2_CANRENDERWINDOWED | D3DCAPS2_FULLSCREENGAMMA;
			p->VertexProcessingCaps = D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_TEXGEN;
			p->PrimitiveMiscCaps = D3DPMISCCAPS_CLIPTLVERTS;
			p->TextureAddressCaps = D3DPTADDRESSCAPS_BORDER | D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_CLAMP;
			p->VertexShaderVersion = D3DVS_VERSION(2, 0);
			p->PixelShaderVersion = D3DPS_VERSION(2, 0);
		}
		return S_OK;
	}
	void SetGammaRamp(DWORD, const D3DGAMMARAMP*) {}
	void SetGammaRamp(DWORD, DWORD, const void*) {}
	HRESULT GetBackBuffer(UINT, D3DBACKBUFFER_TYPE, LPDIRECT3DSURFACE8* pp) { if (pp) *pp = new IDirect3DSurface8(m_Viewport.Width, m_Viewport.Height); return S_OK; }
	UINT GetAvailableTextureMem() { return 512 * 1024 * 1024; }
	HRESULT CreateVertexBuffer(UINT Len, DWORD Usage, DWORD, D3DPOOL, LPDIRECT3DVERTEXBUFFER8* pp) { *pp = new IDirect3DVertexBuffer8(Len, Usage); return S_OK; }
	HRESULT CreateTexture(UINT W, UINT H, UINT Levels, DWORD Usage, D3DFORMAT Fmt, D3DPOOL Pool, LPDIRECT3DTEXTURE8* ppTex, void*) {
		return CreateTexture(W, H, Levels, Usage, Fmt, Pool, ppTex);
	}
	HRESULT CreateVertexBuffer(UINT Len, DWORD Usage, DWORD FVF, D3DPOOL Pool, LPDIRECT3DVERTEXBUFFER8* pp, void*) {
		return CreateVertexBuffer(Len, Usage, FVF, Pool, pp);
	}
	HRESULT CreateIndexBuffer(UINT Len, DWORD Usage, D3DFORMAT Fmt, D3DPOOL Pool, LPDIRECT3DINDEXBUFFER8* pp, void*) {
		return CreateIndexBuffer(Len, Usage, Fmt, Pool, pp);
	}
	HRESULT CreateDepthStencilSurface(UINT W, UINT H, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, LPDIRECT3DSURFACE8* pp, void* = NULL) {
		if (pp) *pp = new IDirect3DSurface8(W, H);
		return S_OK;
	}
	HRESULT CreateRenderTarget(UINT W, UINT H, D3DFORMAT, D3DMULTISAMPLE_TYPE, DWORD, BOOL, LPDIRECT3DSURFACE8* pp, void* = NULL) {
		if (pp) *pp = new IDirect3DSurface8(W, H);
		return S_OK;
	}
	HRESULT GetBackBuffer(UINT, UINT, D3DBACKBUFFER_TYPE, LPDIRECT3DSURFACE8* pp) {
		if (pp) *pp = new IDirect3DSurface8(m_Viewport.Width, m_Viewport.Height);
		return S_OK;
	}
	HRESULT GetRenderTarget(DWORD, LPDIRECT3DSURFACE8* pp) {
		if (pp) *pp = new IDirect3DSurface8(m_Viewport.Width, m_Viewport.Height);
		return S_OK;
	}
	HRESULT SetRenderTarget(int, LPDIRECT3DSURFACE8 pRT) { return SetRenderTarget(pRT, NULL); }
	HRESULT SetRenderTarget(DWORD, LPDIRECT3DSURFACE8 pRT) { return SetRenderTarget(pRT, NULL); }
	HRESULT SetDepthStencilSurface(LPDIRECT3DSURFACE8) { return S_OK; }
	HRESULT StretchRect(LPDIRECT3DSURFACE8, const RECT*, LPDIRECT3DSURFACE8, const RECT*, int) { return S_OK; }
	HRESULT CreateQuery(D3DQUERYTYPE, LPDIRECT3DQUERY9* ppQuery) {
		if (ppQuery) *ppQuery = new IDirect3DQuery9();
		return S_OK;
	}
	HRESULT CreateStateBlock(D3DSTATEBLOCKTYPE, IDirect3DStateBlock9** ppSB) {
		if (ppSB) *ppSB = new IDirect3DStateBlock9();
		return S_OK;
	}
	HRESULT BeginStateBlock() { return S_OK; }
	HRESULT EndStateBlock(IDirect3DStateBlock9** ppSB) {
		if (ppSB) *ppSB = new IDirect3DStateBlock9();
		return S_OK;
	}
	HRESULT SetLight(DWORD Index, const void* pLight) {
		if (Index == 0 && pLight) { m_Light0 = *(const D3DLIGHT8*)pLight; m_DirtyFlags |= 4; }
		return S_OK;
	}
	HRESULT LightEnable(DWORD Index, BOOL bEnable) {
		if (Index == 0) { m_Light0Enabled = bEnable ? 1 : 0; m_DirtyFlags |= 4; }
		return S_OK;
	}
	HRESULT SetMaterial(const D3DMATERIAL8* pMat) {
		if (pMat) { m_Material = *pMat; m_DirtyFlags |= 4; }
		return S_OK;
	}
	HRESULT SetStreamSource(UINT StreamNum, LPDIRECT3DVERTEXBUFFER8 pData, UINT, UINT Stride) { return SetStreamSource(StreamNum, pData, Stride); }
	HRESULT SetStreamSourceFreq(UINT, UINT) { return S_OK; }
	HRESULT SetIndices(LPDIRECT3DINDEXBUFFER8 pIdx) { return SetIndices(pIdx, 0); }
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, int BaseVertexIdx, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount) {
		UINT prevBase = m_BaseVertexIndex;
		if (BaseVertexIdx >= 0) m_BaseVertexIndex = (UINT)BaseVertexIdx;
		HRESULT hr = DrawIndexedPrimitive(Type, MinIndex, NumVertices, StartIndex, PrimitiveCount);
		m_BaseVertexIndex = prevBase;
		return hr;
	}
	HRESULT CreateIndexBuffer(UINT Len, DWORD Usage, D3DFORMAT, D3DPOOL, LPDIRECT3DINDEXBUFFER8* pp) { *pp = new IDirect3DIndexBuffer8(Len, Usage); return S_OK; }
#ifndef ANDROID
	static GLenum MapCombineSrc(DWORD d3dArg, DWORD stage) {
		switch (d3dArg & D3DTA_SELECTMASK) {
		case D3DTA_TEXTURE: return GL_TEXTURE;
		case D3DTA_DIFFUSE: return GL_PRIMARY_COLOR;
		case D3DTA_TFACTOR: return GL_CONSTANT;
		case D3DTA_CURRENT: return (stage == 0) ? GL_PRIMARY_COLOR : GL_PREVIOUS;
		default: return GL_TEXTURE;
		}
	}
	void ApplyStageCombine(DWORD stage) {
		if (stage >= 8) return;
		glActiveTexture(GL_TEXTURE0 + stage);
		DWORD cop = m_ColorOp[stage];
		DWORD aop = m_AlphaOp[stage];
		if (cop == D3DTOP_DISABLE || !m_pBoundTextures[stage]) {
			glDisable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			return;
		}
		glEnable(GL_TEXTURE_2D);
		glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
		glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 1);

		if (cop == D3DTOP_SELECTARG1) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, MapCombineSrc(m_ColorArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, (m_ColorArg1[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
		} else if (cop == D3DTOP_SELECTARG2) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, MapCombineSrc(m_ColorArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, (m_ColorArg2[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
		} else if (cop == D3DTOP_ADD) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, MapCombineSrc(m_ColorArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, MapCombineSrc(m_ColorArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		} else if (cop == D3DTOP_ADDSIGNED) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_ADD_SIGNED);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, MapCombineSrc(m_ColorArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, MapCombineSrc(m_ColorArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);
		} else {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_RGB, MapCombineSrc(m_ColorArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, (m_ColorArg1[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_RGB, MapCombineSrc(m_ColorArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, (m_ColorArg2[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_COLOR : GL_SRC_COLOR);
			if (cop == D3DTOP_MODULATE2X) glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 2);
			else if (cop == D3DTOP_MODULATE4X) glTexEnvi(GL_TEXTURE_ENV, GL_RGB_SCALE, 4);
		}

		if (aop == D3DTOP_DISABLE) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, (stage == 0) ? GL_PRIMARY_COLOR : GL_PREVIOUS);
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
		} else if (aop == D3DTOP_SELECTARG1) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, MapCombineSrc(m_AlphaArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, (m_AlphaArg1[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
		} else if (aop == D3DTOP_SELECTARG2) {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_REPLACE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, MapCombineSrc(m_AlphaArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, (m_AlphaArg2[stage] & D3DTA_COMPLEMENT) ? GL_ONE_MINUS_SRC_ALPHA : GL_SRC_ALPHA);
		} else {
			glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE0_ALPHA, MapCombineSrc(m_AlphaArg1[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);
			glTexEnvi(GL_TEXTURE_ENV, GL_SOURCE1_ALPHA, MapCombineSrc(m_AlphaArg2[stage], stage));
			glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);
		}
		glActiveTexture(GL_TEXTURE0);
	}
	void UpdateTexGen(DWORD stage) {
		if (stage >= 8) return;
		glActiveTexture(GL_TEXTURE0 + stage);
		if ((m_TexCoordIndex[stage] & 0xFFFF0000) == D3DTSS_TCI_CAMERASPACEPOSITION) {
			static const GLfloat sPlane[4] = { 1.0f, 0.0f, 0.0f, 0.0f };
			static const GLfloat tPlane[4] = { 0.0f, 1.0f, 0.0f, 0.0f };
			static const GLfloat rPlane[4] = { 0.0f, 0.0f, 1.0f, 0.0f };
			static const GLfloat qPlane[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
			glMatrixMode(GL_MODELVIEW);
			glPushMatrix();
			glLoadIdentity();
			glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenfv(GL_S, GL_EYE_PLANE, sPlane);
			glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenfv(GL_T, GL_EYE_PLANE, tPlane);
			glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenfv(GL_R, GL_EYE_PLANE, rPlane);
			glTexGeni(GL_Q, GL_TEXTURE_GEN_MODE, GL_EYE_LINEAR);
			glTexGenfv(GL_Q, GL_EYE_PLANE, qPlane);
			glPopMatrix();
			glEnable(GL_TEXTURE_GEN_S);
			glEnable(GL_TEXTURE_GEN_T);
			glEnable(GL_TEXTURE_GEN_R);
			glEnable(GL_TEXTURE_GEN_Q);
		} else {
			glDisable(GL_TEXTURE_GEN_S);
			glDisable(GL_TEXTURE_GEN_T);
			glDisable(GL_TEXTURE_GEN_R);
			glDisable(GL_TEXTURE_GEN_Q);
		}
		glMatrixMode(GL_TEXTURE);
		if (m_TexTransformFlags[stage] != D3DTTFF_DISABLE)
			glLoadMatrixf(&m_matTex[stage].m[0][0]);
		else
			glLoadIdentity();
		glMatrixMode(GL_MODELVIEW);
		glActiveTexture(GL_TEXTURE0);
	}
	void BindVertexAttributes(const BYTE* basePtr, UINT stride) {
		DWORD fvf = m_dwFVF;
		if (D3DXGetFVFVertexSize(fvf) != stride) {
			switch (stride) {
			case 12: fvf = D3DFVF_XYZ; break;
			case 16: fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE; break;
			case 20: fvf = D3DFVF_XYZ | D3DFVF_TEX1; break;
			case 24: fvf = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1; break;
			case 32: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1; break;
			case 36: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_DIFFUSE | D3DFVF_TEX1; break;
			case 40: fvf = D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX2; break;
			default: break;
			}
		}
		glEnableClientState(GL_VERTEX_ARRAY);
		int offset = 0;
		glVertexPointer(3, GL_FLOAT, (GLsizei)stride, basePtr + offset);
		offset += ((fvf & 0x00E) == D3DFVF_XYZRHW) ? 16 : 12;

		if ((fvf & D3DFVF_NORMAL) && (offset + 12 <= (int)stride)) {
			glEnableClientState(GL_NORMAL_ARRAY);
			glNormalPointer(GL_FLOAT, (GLsizei)stride, basePtr + offset);
			offset += 12;
		} else {
			glDisableClientState(GL_NORMAL_ARRAY);
		}

		if ((fvf & D3DFVF_PSIZE) && (offset + 4 <= (int)stride))
			offset += 4;

		if ((fvf & D3DFVF_DIFFUSE) && (offset + 4 <= (int)stride)) {
			glEnableClientState(GL_COLOR_ARRAY);
			glColorPointer(GL_BGRA_EXT, GL_UNSIGNED_BYTE, (GLsizei)stride, basePtr + offset);
			offset += 4;
		} else {
			glDisableClientState(GL_COLOR_ARRAY);
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
		}

		if ((fvf & D3DFVF_SPECULAR) && (offset + 4 <= (int)stride))
			offset += 4;

		int numTex = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
		glClientActiveTexture(GL_TEXTURE0);
		if ((numTex >= 1 || (fvf & D3DFVF_TEX1)) && (offset + 8 <= (int)stride)) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, (GLsizei)stride, basePtr + offset);
			offset += 8;
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}

		glClientActiveTexture(GL_TEXTURE1);
		if ((numTex >= 2 || (fvf & D3DFVF_TEX2)) && (offset + 8 <= (int)stride)) {
			glEnableClientState(GL_TEXTURE_COORD_ARRAY);
			glTexCoordPointer(2, GL_FLOAT, (GLsizei)stride, basePtr + offset);
		} else {
			glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		}
		glClientActiveTexture(GL_TEXTURE0);
	}
	void UnbindVertexAttributes() {
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glClientActiveTexture(GL_TEXTURE1);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glClientActiveTexture(GL_TEXTURE0);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	}
#endif
	HRESULT SetTransform(DWORD type, const GLMATRIX* mat) {
		if (!mat) return S_OK;
		if (type == D3DTS_WORLD) {
			m_matWorld = *mat;
			m_DirtyFlags |= 1;
		} else if (type == D3DTS_VIEW) {
			if (memcmp(&m_matView, mat, sizeof(GLMATRIX)) != 0) {
				m_matView = *mat;
				m_DirtyFlags |= 1;
			}
		} else if (type == D3DTS_PROJECTION) {
			if (memcmp(&m_matProj, mat, sizeof(GLMATRIX)) != 0) {
				m_matProj = *mat;
				m_DirtyFlags |= 2;
			}
		} else if (type >= D3DTS_TEXTURE0 && type <= D3DTS_TEXTURE7) {
			DWORD idx = type - D3DTS_TEXTURE0;
			if (memcmp(&m_matTex[idx], mat, sizeof(GLMATRIX)) != 0) {
				m_matTex[idx] = *mat;
				if (idx < 2) m_DirtyFlags |= 4;
			}
		}
#ifndef ANDROID
		if (type == D3DTS_PROJECTION) { glMatrixMode(GL_PROJECTION); glLoadMatrixf(&mat->m[0][0]); }
		else if (type == D3DTS_WORLD || type == D3DTS_VIEW) { GLMATRIX mv; D3DXMatrixMultiply(&mv, &m_matWorld, &m_matView); glMatrixMode(GL_MODELVIEW); glLoadMatrixf(&mv.m[0][0]); }
		else if (type >= D3DTS_TEXTURE0 && type <= D3DTS_TEXTURE7) { UpdateTexGen(type - D3DTS_TEXTURE0); }
#endif
		return S_OK;
	}
	HRESULT GetTransform(DWORD type, GLMATRIX* mat) {
		if (!mat) return S_OK;
		if (type == D3DTS_WORLD) *mat = m_matWorld;
		else if (type == D3DTS_VIEW) *mat = m_matView;
		else if (type == D3DTS_PROJECTION) *mat = m_matProj;
		else if (type >= D3DTS_TEXTURE0 && type <= D3DTS_TEXTURE7) *mat = m_matTex[type - D3DTS_TEXTURE0];
		return S_OK;
	}
	HRESULT SetTextureStageState(DWORD stage, D3DTEXTURESTAGESTATETYPE type, DWORD value) {
		if (stage >= 8) return S_OK;
#ifndef ANDROID
		glActiveTexture(GL_TEXTURE0 + stage);
#endif
		switch (type) {
		case D3DTSS_ADDRESSU:
			if (m_AddressU[stage] == value) return S_OK;
			m_AddressU[stage] = value;
			return S_OK;
		case D3DTSS_ADDRESSV:
			if (m_AddressV[stage] == value) return S_OK;
			m_AddressV[stage] = value;
			return S_OK;
		case D3DTSS_COLOROP:
			if (m_ColorOp[stage] == value) return S_OK;
			m_ColorOp[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_COLORARG1:
			if (m_ColorArg1[stage] == value) return S_OK;
			m_ColorArg1[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_COLORARG2:
			if (m_ColorArg2[stage] == value) return S_OK;
			m_ColorArg2[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_ALPHAOP:
			if (m_AlphaOp[stage] == value) return S_OK;
			m_AlphaOp[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_ALPHAARG1:
			if (m_AlphaArg1[stage] == value) return S_OK;
			m_AlphaArg1[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_ALPHAARG2:
			if (m_AlphaArg2[stage] == value) return S_OK;
			m_AlphaArg2[stage] = value;
#ifndef ANDROID
			ApplyStageCombine(stage);
#endif
			break;
		case D3DTSS_TEXCOORDINDEX:
			if (m_TexCoordIndex[stage] == value) return S_OK;
			m_TexCoordIndex[stage] = value;
#ifndef ANDROID
			UpdateTexGen(stage);
#endif
			break;
		case D3DTSS_TEXTURETRANSFORMFLAGS:
			if (m_TexTransformFlags[stage] == value) return S_OK;
			m_TexTransformFlags[stage] = value;
#ifndef ANDROID
			UpdateTexGen(stage);
#endif
			break;
		default:
			return S_OK;
		}
#ifndef ANDROID
		glActiveTexture(GL_TEXTURE0);
#endif
		if (stage < 2) m_DirtyFlags |= 4;
		return S_OK;
	}
	static GLenum ConvertBlend(DWORD d3dBlend) {
		switch (d3dBlend) {
		case D3DBLEND_ZERO: return GL_ZERO;
		case D3DBLEND_ONE: return GL_ONE;
		case D3DBLEND_SRCCOLOR: return GL_SRC_COLOR;
		case D3DBLEND_INVSRCCOLOR: return GL_ONE_MINUS_SRC_COLOR;
		case D3DBLEND_SRCALPHA: return GL_SRC_ALPHA;
		case D3DBLEND_INVSRCALPHA: return GL_ONE_MINUS_SRC_ALPHA;
#ifdef __ANDROID__
		case D3DBLEND_DESTALPHA: return GL_ONE;
		case D3DBLEND_INVDESTALPHA: return GL_SRC_ALPHA;
#else
		case D3DBLEND_DESTALPHA: return GL_DST_ALPHA;
		case D3DBLEND_INVDESTALPHA: return GL_ONE_MINUS_DST_ALPHA;
#endif
		case D3DBLEND_DESTCOLOR: return GL_DST_COLOR;
		case D3DBLEND_INVDESTCOLOR: return GL_ONE_MINUS_DST_COLOR;
		case D3DBLEND_SRCALPHASAT: return GL_SRC_ALPHA_SATURATE;
		default: return GL_ONE;
		}
	}
	HRESULT SetRenderState(DWORD Type, DWORD Value) {
		if (Type < 256) {
			if (m_RenderStates[Type] == Value) return S_OK;
			m_RenderStates[Type] = Value;
		}
		switch (Type) {
		case D3DRS_ZENABLE: if (Value) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST); return S_OK;
		case D3DRS_ZFUNC: glDepthFunc(GL_LEQUAL); return S_OK;
		case D3DRS_ALPHABLENDENABLE:
			if (Value) { glEnable(GL_BLEND); glBlendFunc(m_SrcBlend, m_DstBlend); }
			else glDisable(GL_BLEND);
			return S_OK;
		case D3DRS_SRCBLEND:
			if (Value == D3DBLEND_BOTHSRCALPHA) {
				m_SrcBlend = GL_SRC_ALPHA; m_DstBlend = GL_ONE_MINUS_SRC_ALPHA;
			} else if (Value == D3DBLEND_BOTHINVSRCALPHA) {
				m_SrcBlend = GL_ONE_MINUS_SRC_ALPHA; m_DstBlend = GL_SRC_ALPHA;
			} else {
				m_SrcBlend = ConvertBlend(Value);
			}
			glBlendFunc(m_SrcBlend, m_DstBlend);
			return S_OK;
		case D3DRS_DESTBLEND:
			m_DstBlend = ConvertBlend(Value);
			glBlendFunc(m_SrcBlend, m_DstBlend);
			return S_OK;
		case D3DRS_ZWRITEENABLE: glDepthMask(Value ? GL_TRUE : GL_FALSE); return S_OK;
		case D3DRS_ALPHATESTENABLE:
			m_AlphaTestEnable = Value;
#ifndef ANDROID
			if (Value) glEnable(GL_ALPHA_TEST); else glDisable(GL_ALPHA_TEST);
#endif
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_ALPHAFUNC:
#ifndef ANDROID
			glAlphaFunc(GL_GREATER, 0.1f);
#endif
			return S_OK;
		case D3DRS_CULLMODE:
			glDisable(GL_CULL_FACE);
			return S_OK;
		case D3DRS_LIGHTING:
			m_LightingEnable = Value ? 1 : 0;
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_AMBIENT:
			m_AmbientColor = Value;
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_COLORVERTEX:
			m_ColorVertex = Value ? 1 : 0;
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_DIFFUSEMATERIALSOURCE:
			m_DiffuseMaterialSource = Value;
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_FOGENABLE:
			m_FogEnable = Value ? 1 : 0;
#ifndef ANDROID
			if (Value) { glEnable(GL_FOG); glFogi(GL_FOG_MODE, GL_LINEAR); }
			else glDisable(GL_FOG);
#endif
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_FOGCOLOR:
			m_FogColor = Value;
#ifndef ANDROID
			{
				GLCOLOR c(Value);
				GLfloat fc[4] = { c.r, c.g, c.b, c.a };
				glFogfv(GL_FOG_COLOR, fc);
			}
#endif
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_FOGSTART:
			m_FogStart = *((float*)&Value);
#ifndef ANDROID
			glFogf(GL_FOG_START, m_FogStart);
#endif
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_FOGEND:
			m_FogEnd = *((float*)&Value);
#ifndef ANDROID
			glFogf(GL_FOG_END, m_FogEnd);
#endif
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_CLIPPLANEENABLE:
			m_ClipPlaneEnable = Value;
			m_DirtyFlags |= 4;
			return S_OK;
		case D3DRS_TEXTUREFACTOR:
			m_TextureFactor = Value;
#ifndef ANDROID
			{
				GLCOLOR tf(Value);
				GLfloat envCol[4] = { tf.r, tf.g, tf.b, tf.a };
				glActiveTexture(GL_TEXTURE0);
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envCol);
				glActiveTexture(GL_TEXTURE1);
				glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envCol);
				glActiveTexture(GL_TEXTURE0);
			}
#endif
			return S_OK;
		case D3DRS_ALPHAREF:
			m_AlphaRef = Value & 0xFF;
			return S_OK;
		case D3DRS_BLENDOP:
			if (Value == D3DBLENDOP_SUBTRACT) glBlendEquation(GL_FUNC_SUBTRACT);
			else if (Value == D3DBLENDOP_REVSUBTRACT) glBlendEquation(GL_FUNC_REVERSE_SUBTRACT);
			else glBlendEquation(GL_FUNC_ADD);
			return S_OK;
		default:
			return S_OK;
		}
	}
#ifdef ANDROID
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE Type, UINT StartVertex, UINT PrimitiveCount);
	HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE Type, UINT PrimitiveCount, const void* pData, UINT Stride);
	HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE Type, UINT MinVertexIndex, UINT NumVertexIndices, UINT PrimitiveCount, const void* pIdx, D3DFORMAT IdxFmt, const void* pData, UINT Stride);
	HRESULT Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil);
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount);
	HRESULT Reset(D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT BeginScene();
	HRESULT EndScene();
	HRESULT Present(const RECT* pSourceRect, const RECT* pDestRect, HWND hDestWindowOverride, const void* pDirtyRegion);
	HRESULT SetTexture(DWORD Stage, LPDIRECT3DBaseTexture8 pTexture);
	HRESULT SetStreamSource(UINT StreamNumber, LPDIRECT3DVERTEXBUFFER8 pStreamData, UINT Stride);
	HRESULT SetIndices(LPDIRECT3DINDEXBUFFER8 pIndexBuffer, UINT BaseVertexIndex);
#else
	HRESULT DrawPrimitive(D3DPRIMITIVETYPE Type, UINT StartVertex, UINT PrimitiveCount) {
		if (!m_pStreamSource || !m_pStreamSource->pData || m_StreamStride == 0 || PrimitiveCount == 0)
			return S_OK;
		GLenum mode = GL_TRIANGLES; GLsizei count = 0;
		switch (Type) {
		case D3DPT_POINTLIST: mode = GL_POINTS; count = PrimitiveCount; break;
		case D3DPT_LINELIST: mode = GL_LINES; count = PrimitiveCount * 2; break;
		case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = PrimitiveCount + 1; break;
		case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = PrimitiveCount * 3; break;
		case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = PrimitiveCount + 2; break;
		case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = PrimitiveCount + 2; break;
		default: return S_OK;
		}
		if ((size_t)(StartVertex + count) * (size_t)m_StreamStride > (size_t)m_pStreamSource->length)
			return S_OK;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		BindVertexAttributes((const BYTE*)m_pStreamSource->pData, m_StreamStride);
		glDrawArrays(mode, StartVertex, count);
		UnbindVertexAttributes();
		return S_OK;
	}
	HRESULT DrawPrimitiveUP(D3DPRIMITIVETYPE Type, UINT PrimitiveCount, const void* pData, UINT Stride) {
		if (!pData || Stride == 0 || PrimitiveCount == 0) return S_OK;
		GLenum mode = GL_TRIANGLES; GLsizei count = 0;
		switch (Type) {
		case D3DPT_POINTLIST: mode = GL_POINTS; count = PrimitiveCount; break;
		case D3DPT_LINELIST: mode = GL_LINES; count = PrimitiveCount * 2; break;
		case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = PrimitiveCount + 1; break;
		case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = PrimitiveCount * 3; break;
		case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = PrimitiveCount + 2; break;
		case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = PrimitiveCount + 2; break;
		default: return S_OK;
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		BindVertexAttributes((const BYTE*)pData, Stride);
		glDrawArrays(mode, 0, count);
		UnbindVertexAttributes();
		return S_OK;
	}
	HRESULT DrawIndexedPrimitiveUP(D3DPRIMITIVETYPE Type, UINT, UINT, UINT PrimitiveCount, const void* pIdx, D3DFORMAT IdxFmt, const void* pData, UINT Stride) {
		if (!pData || !pIdx || Stride == 0 || PrimitiveCount == 0) return S_OK;
		GLenum mode = GL_TRIANGLES; GLsizei count = 0;
		switch (Type) {
		case D3DPT_POINTLIST: mode = GL_POINTS; count = PrimitiveCount; break;
		case D3DPT_LINELIST: mode = GL_LINES; count = PrimitiveCount * 2; break;
		case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = PrimitiveCount + 1; break;
		case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = PrimitiveCount * 3; break;
		case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = PrimitiveCount + 2; break;
		case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = PrimitiveCount + 2; break;
		default: return S_OK;
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		BindVertexAttributes((const BYTE*)pData, Stride);
		glDrawElements(mode, count, (IdxFmt == D3DFMT_INDEX16 ? GL_UNSIGNED_SHORT : GL_UNSIGNED_INT), pIdx);
		UnbindVertexAttributes();
		return S_OK;
	}

	HRESULT Clear(DWORD Count, const D3DRECT* pRects, DWORD Flags, D3DCOLOR Color, float Z, DWORD Stencil) {
		GLbitfield mask = 0;
		if (Flags & D3DCLEAR_TARGET) {
			GLCOLOR col(Color);
			glClearColor(col.r, col.g, col.b, col.a);
			mask |= GL_COLOR_BUFFER_BIT;
		}
		if (Flags & D3DCLEAR_ZBUFFER) {
			glClearDepth(Z);
			mask |= GL_DEPTH_BUFFER_BIT;
		}
		if (mask) glClear(mask);
		return S_OK;
	}
	HRESULT DrawIndexedPrimitive(D3DPRIMITIVETYPE Type, UINT MinIndex, UINT NumVertices, UINT StartIndex, UINT PrimitiveCount) {
		if (!m_pStreamSource || !m_pIndexBuffer || !m_pStreamSource->pData || !m_pIndexBuffer->pData || m_StreamStride == 0 || PrimitiveCount == 0)
			return S_OK;
		GLenum mode = GL_TRIANGLES;
		GLsizei count = 0;
		switch (Type) {
		case D3DPT_POINTLIST: mode = GL_POINTS; count = (GLsizei)PrimitiveCount; break;
		case D3DPT_LINELIST: mode = GL_LINES; count = (GLsizei)PrimitiveCount * 2; break;
		case D3DPT_LINESTRIP: mode = GL_LINE_STRIP; count = (GLsizei)PrimitiveCount + 1; break;
		case D3DPT_TRIANGLELIST: mode = GL_TRIANGLES; count = (GLsizei)PrimitiveCount * 3; break;
		case D3DPT_TRIANGLESTRIP: mode = GL_TRIANGLE_STRIP; count = (GLsizei)PrimitiveCount + 2; break;
		case D3DPT_TRIANGLEFAN: mode = GL_TRIANGLE_FAN; count = (GLsizei)PrimitiveCount + 2; break;
		default: return S_OK;
		}
		size_t vOffsetBytes = (size_t)m_BaseVertexIndex * (size_t)m_StreamStride;
		if (vOffsetBytes >= (size_t)m_pStreamSource->length)
			return S_OK;
		if (((size_t)StartIndex + (size_t)count) * sizeof(WORD) > (size_t)m_pIndexBuffer->length)
			return S_OK;
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		BindVertexAttributes((const BYTE*)m_pStreamSource->pData + vOffsetBytes, m_StreamStride);
		const WORD* idxPtr = (const WORD*)m_pIndexBuffer->pData + StartIndex;
		glDrawElements(mode, count, GL_UNSIGNED_SHORT, idxPtr);
		UnbindVertexAttributes();
		return S_OK;
	}
	HRESULT Reset(D3DPRESENT_PARAMETERS* pPP) {
		if (pPP && pPP->BackBufferWidth > 0 && pPP->BackBufferHeight > 0) {
			m_Viewport.X = 0; m_Viewport.Y = 0;
			m_Viewport.Width = pPP->BackBufferWidth;
			m_Viewport.Height = pPP->BackBufferHeight;
			glViewport(0, 0, m_Viewport.Width, m_Viewport.Height);
		}
		return S_OK;
	}
	HRESULT BeginScene() {
		if (m_hWnd && !m_pCurrentRenderTargetTex) {
			RECT rc;
			if (GetClientRect(m_hWnd, &rc) && (rc.right - rc.left) > 0 && (rc.bottom - rc.top) > 0) {
				m_Viewport.X = 0; m_Viewport.Y = 0;
				m_Viewport.Width = rc.right - rc.left;
				m_Viewport.Height = rc.bottom - rc.top;
				glViewport(0, 0, m_Viewport.Width, m_Viewport.Height);
			}
		}
		return S_OK;
	}
	HRESULT EndScene() { return S_OK; }
	HRESULT Present(const RECT*, const RECT*, HWND, const void*) {
		if (m_hDC) SwapBuffers(m_hDC);
		return S_OK;
	}
	HRESULT SetTexture(DWORD Stage, LPDIRECT3DBaseTexture8 pTexture) {
		if (Stage >= 8) return S_OK;
		m_pBoundTextures[Stage] = pTexture;
		glActiveTexture(GL_TEXTURE0 + Stage);
		if (pTexture) {
			IDirect3DTexture8* tex = (IDirect3DTexture8*)pTexture;
			glBindTexture(GL_TEXTURE_2D, tex->glId);
			ApplyStageCombine(Stage);
		} else {
			glBindTexture(GL_TEXTURE_2D, 0);
			glDisable(GL_TEXTURE_2D);
		}
		glActiveTexture(GL_TEXTURE0);
		return S_OK;
	}
	HRESULT SetStreamSource(UINT, LPDIRECT3DVERTEXBUFFER8 pStreamData, UINT Stride) {
		m_pStreamSource = pStreamData;
		m_StreamStride = Stride;
		return S_OK;
	}
	HRESULT SetIndices(LPDIRECT3DINDEXBUFFER8 pIndexBuffer, UINT BaseVertexIndex) {
		m_pIndexBuffer = pIndexBuffer;
		m_BaseVertexIndex = BaseVertexIndex;
		return S_OK;
	}
#endif
	ULONG AddRef() { return 1; }
	ULONG Release() { delete this; return 0; }
};

inline HRESULT IDirect3DTexture8::UnlockRect(UINT Level) {
	if (glId && pLockedData && Level == 0) {
		int w = (width > 0 && width <= 8192) ? width : 256;
		int h = (height > 0 && height <= 8192) ? height : 256;
		glActiveTexture(GL_TEXTURE0);
		GLuint prevTex = (GetCurrentGLDevice() && GetCurrentGLDevice()->m_pBoundTextures[0])
			? ((IDirect3DTexture8*)GetCurrentGLDevice()->m_pBoundTextures[0])->glId : 0;
		glBindTexture(GL_TEXTURE_2D, glId);
		glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
		if (format == D3DFMT_A4R4G4B4) {
			std::vector<DWORD> rgba((size_t)w * (size_t)h);
			const WORD* src = (const WORD*)pLockedData;
			for (int i = 0; i < w * h; ++i) {
				WORD c = src[i];
				DWORD a = ((c >> 12) & 0xF) * 17;
				DWORD r = ((c >> 8) & 0xF) * 17;
				DWORD g = ((c >> 4) & 0xF) * 17;
				DWORD b = (c & 0xF) * 17;
#ifdef __ANDROID__
				rgba[i] = (a << 24) | (b << 16) | (g << 8) | r;
#else
				rgba[i] = (a << 24) | (r << 16) | (g << 8) | b;
#endif
			}
#ifdef __ANDROID__
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
#else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, rgba.data());
#endif
		} else if (format == D3DFMT_R5G6B5 || format == D3DFMT_X1R5G5B5 || format == D3DFMT_A1R5G5B5) {
			std::vector<DWORD> rgba((size_t)w * (size_t)h);
			const WORD* src = (const WORD*)pLockedData;
			for (int i = 0; i < w * h; ++i) {
				WORD c = src[i];
				DWORD r = ((c >> 11) & 0x1F) * 255 / 31;
				DWORD g = ((c >> 5) & 0x3F) * 255 / 63;
				DWORD b = (c & 0x1F) * 255 / 31;
#ifdef __ANDROID__
				rgba[i] = 0xFF000000 | (b << 16) | (g << 8) | r;
#else
				rgba[i] = 0xFF000000 | (r << 16) | (g << 8) | b;
#endif
			}
#ifdef __ANDROID__
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, rgba.data());
#else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, rgba.data());
#endif
		} else {
#ifdef __ANDROID__
			DWORD* pix = (DWORD*)pLockedData;
			const bool forceOpaque = (format == D3DFMT_X8R8G8B8 || format == D3DFMT_R8G8B8);
			SwizzleBGRAtoRGBAInPlace(pix, w * h, forceOpaque);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_RGBA, GL_UNSIGNED_BYTE, pix);
#else
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA_EXT, GL_UNSIGNED_BYTE, pLockedData);
#endif
		}
#ifdef __ANDROID__
		if (w >= 64 && h >= 64 && (w & (w - 1)) == 0 && (h & (h - 1)) == 0) {
			glGenerateMipmap(GL_TEXTURE_2D);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		}
		free(pLockedData);
		pLockedData = NULL;
#endif
		glBindTexture(GL_TEXTURE_2D, prevTex);
	}
	return S_OK;
}
inline IDirect3DTexture8::~IDirect3DTexture8() {
	if (GetCurrentGLDevice()) {
		if (GetCurrentGLDevice()->m_pCurrentRenderTargetTex == this)
			GetCurrentGLDevice()->m_pCurrentRenderTargetTex = NULL;
		for (int i = 0; i < 16; ++i) {
			if (GetCurrentGLDevice()->m_pBoundTextures[i] == this) {
				GetCurrentGLDevice()->m_pBoundTextures[i] = NULL;
				if (i < 8) GetCurrentGLDevice()->m_BoundGLTexId[i] = 0xFFFFFFFFu;
			}
		}
	}
#ifdef ANDROID
	if (glFbo) glDeleteFramebuffers(1, &glFbo);
	if (glDepthRbo) glDeleteRenderbuffers(1, &glDepthRbo);
#endif
	if (glId) glDeleteTextures(1, &glId);
	if (pLockedData) free(pLockedData);
}

inline IDirect3DVertexBuffer8::~IDirect3DVertexBuffer8() {
	if (GetCurrentGLDevice() && GetCurrentGLDevice()->m_pStreamSource == this)
		GetCurrentGLDevice()->m_pStreamSource = NULL;
#ifdef ANDROID
	if (glVbo) NotifyAndroidGLBufferDeleted(glVbo, 0);
#endif
	if (glVbo) glDeleteBuffers(1, &glVbo);
	if (pData) free(pData);
}

inline IDirect3DIndexBuffer8::~IDirect3DIndexBuffer8() {
	if (GetCurrentGLDevice() && GetCurrentGLDevice()->m_pIndexBuffer == this)
		GetCurrentGLDevice()->m_pIndexBuffer = NULL;
#ifdef ANDROID
	if (glIbo) NotifyAndroidGLBufferDeleted(0, glIbo);
#endif
	if (glIbo) glDeleteBuffers(1, &glIbo);
	if (pData) free(pData);
}

struct IDirect3D8 {
	UINT GetAdapterCount() { return 1; }
	HRESULT GetAdapterIdentifier(UINT, DWORD, D3DADAPTER_IDENTIFIER8* p) {
		if (p) {
			memset(p, 0, sizeof(D3DADAPTER_IDENTIFIER8));
			strcpy(p->Driver, "OpenGL Generic");
			strcpy(p->Description, "OpenGL Port");
		}
		return S_OK;
	}
	UINT GetAdapterModeCount(UINT) { return 9; }
	HRESULT EnumAdapterModes(UINT, UINT Mode, D3DDISPLAYMODE* p) {
		static const UINT res[9][2] = {
			{800, 600}, {1024, 768}, {1152, 864}, {1280, 720},
			{1280, 800}, {1280, 1024}, {1366, 768}, {1600, 900}, {1920, 1080}
		};
		if (p) {
			UINT idx = (Mode < 9) ? Mode : 0;
			p->Width = res[idx][0];
			p->Height = res[idx][1];
			p->RefreshRate = 60;
			p->Format = D3DFMT_X8R8G8B8;
		}
		return S_OK;
	}
	HRESULT GetAdapterDisplayMode(UINT, D3DDISPLAYMODE* p) {
		if (p) {
			p->Width = 1024;
			p->Height = 768;
			p->RefreshRate = 60;
			p->Format = D3DFMT_X8R8G8B8;
		}
		return S_OK;
	}
	HRESULT CheckDeviceType(UINT, D3DDEVTYPE, D3DFORMAT, D3DFORMAT, BOOL) { return S_OK; }
	HRESULT CheckDeviceFormat(UINT, int, int, DWORD, int, int) { return S_OK; }
	HRESULT CheckDepthStencilMatch(UINT, int, int, int, int) { return S_OK; }
	HRESULT GetDeviceCaps(UINT, int, D3DCAPS8* p) {
		if (p) {
			memset(p, 0, sizeof(D3DCAPS8));
			p->MaxTextureWidth = 8192;
			p->MaxTextureHeight = 8192;
			p->MaxSimultaneousTextures = 8;
			p->MaxTextureBlendStages = 8;
			p->MaxActiveLights = 8;
			p->MaxStreams = 16;
			p->DevCaps = D3DDEVCAPS_HWTRANSFORMANDLIGHT;
			p->Caps2 = D3DCAPS2_CANRENDERWINDOWED | D3DCAPS2_FULLSCREENGAMMA;
			p->VertexProcessingCaps = D3DVTXPCAPS_DIRECTIONALLIGHTS | D3DVTXPCAPS_POSITIONALLIGHTS | D3DVTXPCAPS_TEXGEN;
			p->PrimitiveMiscCaps = D3DPMISCCAPS_CLIPTLVERTS;
			p->TextureAddressCaps = D3DPTADDRESSCAPS_BORDER | D3DPTADDRESSCAPS_WRAP | D3DPTADDRESSCAPS_CLAMP;
			p->VertexShaderVersion = D3DVS_VERSION(2, 0);
			p->PixelShaderVersion = D3DPS_VERSION(2, 0);
		}
		return S_OK;
	}
#ifdef ANDROID
	HRESULT CreateDevice(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice8** ppReturnedDeviceInterface);
#else
	HRESULT CreateDevice(UINT, D3DDEVTYPE, HWND hFocusWindow, DWORD, D3DPRESENT_PARAMETERS* pPP, IDirect3DDevice8** ppReturnedDeviceInterface) {
		IDirect3DDevice8* dev = new IDirect3DDevice8();
		*ppReturnedDeviceInterface = dev;
		HDC hDC = GetDC(hFocusWindow);
		static PIXELFORMATDESCRIPTOR pfd = {
			sizeof(PIXELFORMATDESCRIPTOR), 1,
			PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
			PFD_TYPE_RGBA, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 24, 8, 0,
			PFD_MAIN_PLANE, 0, 0, 0, 0
		};
		int pixelFormat = ChoosePixelFormat(hDC, &pfd);
		SetPixelFormat(hDC, pixelFormat, &pfd);
		HGLRC hRC = wglCreateContext(hDC);
		wglMakeCurrent(hDC, hRC);
		dev->m_hDC = hDC;
		dev->m_hWnd = hFocusWindow;
		if (pPP && pPP->BackBufferWidth > 0 && pPP->BackBufferHeight > 0) {
			dev->m_Viewport.Width = pPP->BackBufferWidth;
			dev->m_Viewport.Height = pPP->BackBufferHeight;
		}
		glViewport(0, 0, dev->m_Viewport.Width, dev->m_Viewport.Height);
		return S_OK;
	}
#endif
	ULONG AddRef() { return 1; }
	ULONG Release() { delete this; return 0; }
};

#ifdef ANDROID
IDirect3D8* Direct3DCreate8(UINT SDKVersion);
#else
inline IDirect3D8* Direct3DCreate8(UINT) { return new IDirect3D8(); }
#endif

struct ID3DXMatrixStack {
	std::vector<GLMATRIX> m_stack;
	ID3DXMatrixStack() { m_stack.push_back(GLMATRIX()); }
	HRESULT Push() { m_stack.push_back(m_stack.back()); return S_OK; }
	HRESULT Pop() { if (m_stack.size() > 1) m_stack.pop_back(); return S_OK; }
	HRESULT LoadIdentity() { D3DXMatrixIdentity(&m_stack.back()); return S_OK; }
	HRESULT LoadMatrix(const GLMATRIX* pMat) { m_stack.back() = *pMat; return S_OK; }
	HRESULT MultMatrix(const GLMATRIX* pMat) { D3DXMatrixMultiply(&m_stack.back(), &m_stack.back(), pMat); return S_OK; }
	HRESULT MultMatrixLocal(const GLMATRIX* pMat) { D3DXMatrixMultiply(&m_stack.back(), pMat, &m_stack.back()); return S_OK; }
	HRESULT Scale(float x, float y, float z) { GLMATRIX s; D3DXMatrixScaling(&s, x, y, z); return MultMatrix(&s); }
	HRESULT ScaleLocal(float x, float y, float z) { GLMATRIX s; D3DXMatrixScaling(&s, x, y, z); return MultMatrixLocal(&s); }
	HRESULT Translate(float x, float y, float z) { GLMATRIX t; D3DXMatrixTranslation(&t, x, y, z); return MultMatrix(&t); }
	HRESULT TranslateLocal(float x, float y, float z) { GLMATRIX t; D3DXMatrixTranslation(&t, x, y, z); return MultMatrixLocal(&t); }
	HRESULT RotateAxis(const GLVECTOR3* pV, float a) { GLMATRIX r; D3DXMatrixRotationAxis(&r, pV, a); return MultMatrix(&r); }
	HRESULT RotateAxisLocal(const GLVECTOR3* pV, float a) { GLMATRIX r; D3DXMatrixRotationAxis(&r, pV, a); return MultMatrixLocal(&r); }
	HRESULT RotateYawPitchRoll(float y, float p, float r) { GLMATRIX m; D3DXMatrixRotationYawPitchRoll(&m, y, p, r); return MultMatrix(&m); }
	HRESULT RotateYawPitchRollLocal(float y, float p, float r) { GLMATRIX m; D3DXMatrixRotationYawPitchRoll(&m, y, p, r); return MultMatrixLocal(&m); }
	GLMATRIX* GetTop() { return &m_stack.back(); }
	ULONG m_refCount = 1;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
};

struct ID3DXBuffer {
	DWORD dummy[16];
	ID3DXBuffer() { memset(dummy, 0, sizeof(dummy)); }
	void* GetBufferPointer() { return dummy; }
	DWORD GetBufferSize() { return sizeof(dummy); }
	ULONG m_refCount = 1;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; }
};
struct ID3DXMesh { HRESULT GetIndexBuffer(LPDIRECT3DINDEXBUFFER8*) { return S_OK; } HRESULT GetVertexBuffer(LPDIRECT3DVERTEXBUFFER8*) { return S_OK; } DWORD GetFVF() { return 0; } DWORD GetNumVertices() { return 0; } DWORD GetNumFaces() { return 0; } HRESULT DrawSubset(DWORD) { return S_OK; } ULONG m_refCount = 1;
	ULONG AddRef() { return ++m_refCount; }
	ULONG Release() { if (--m_refCount == 0) { delete this; return 0; } return m_refCount; } };

inline HRESULT D3DXCreateMatrixStack(DWORD, LPD3DXMATRIXSTACK* pp) { if (pp) *pp = new ID3DXMatrixStack(); return S_OK; }
inline HRESULT D3DXCreateSphere(LPDIRECT3DDEVICE8, float, UINT, UINT, LPD3DXMESH* pp, LPD3DXBUFFER*) { if (pp) *pp = new ID3DXMesh(); return S_OK; }
inline HRESULT D3DXCreateCylinder(LPDIRECT3DDEVICE8, float, float, float, UINT, UINT, LPD3DXMESH* pp, LPD3DXBUFFER*) { if (pp) *pp = new ID3DXMesh(); return S_OK; }
inline HRESULT D3DXCreateBox(LPDIRECT3DDEVICE8, float, float, float, LPD3DXMESH* pp, LPD3DXBUFFER*) { if (pp) *pp = new ID3DXMesh(); return S_OK; }
inline HRESULT D3DXAssembleShaderFromFile(LPCSTR, DWORD, LPD3DXBUFFER*, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE) { if (ppS) *ppS = new ID3DXBuffer(); if (ppE) *ppE = new ID3DXBuffer(); return S_OK; }
inline HRESULT D3DXAssembleShader(LPCSTR, UINT, DWORD, LPD3DXBUFFER*, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE) { if (ppS) *ppS = new ID3DXBuffer(); if (ppE) *ppE = new ID3DXBuffer(); return S_OK; }
#ifdef __ANDROID__
extern "C" bool AndroidDecodeImageFromMemory(const void* data, unsigned int size, unsigned int* outW, unsigned int* outH, std::vector<uint32_t>& outBGRA);
#endif
inline WORD ReadU16Unaligned(const BYTE* p) { WORD v; memcpy(&v, p, sizeof(WORD)); return v; }
inline DWORD ReadU32Unaligned(const BYTE* p) { DWORD v; memcpy(&v, p, sizeof(DWORD)); return v; }
inline int ReadI32Unaligned(const BYTE* p) { int v; memcpy(&v, p, sizeof(int)); return v; }

inline bool DecodeTGAorBMPFromMemory(const BYTE* data, UINT size, UINT& outW, UINT& outH, std::vector<DWORD>& outBGRA) {
	if (!data || size < 18) return false;

	// 1. Check BMP ('BM')
	if (data[0] == 'B' && data[1] == 'M' && size >= 54) {
		DWORD pixelOffset = ReadU32Unaligned(data + 10);
		int w = ReadI32Unaligned(data + 18);
		int h = ReadI32Unaligned(data + 22);
		WORD bpp = ReadU16Unaligned(data + 28);
		bool flipY = (h > 0);
		if (h < 0) h = -h;
		if (w > 0 && h > 0 && w <= 4096 && h <= 4096 && (bpp == 24 || bpp == 32) && pixelOffset < size) {
			outW = (UINT)w; outH = (UINT)h;
			outBGRA.resize((size_t)w * (size_t)h, 0xFFFFFFFF);
			int rowStride = ((w * (bpp / 8)) + 3) & ~3;
			for (int y = 0; y < h; ++y) {
				int srcY = flipY ? (h - 1 - y) : y;
				size_t rowOff = (size_t)pixelOffset + (size_t)srcY * (size_t)rowStride;
				if (rowOff + (size_t)w * (bpp / 8) > size) break;
				const BYTE* row = data + rowOff;
				for (int x = 0; x < w; ++x) {
					BYTE b = row[x * (bpp / 8) + 0];
					BYTE g = row[x * (bpp / 8) + 1];
					BYTE r = row[x * (bpp / 8) + 2];
					BYTE a = (bpp == 32) ? row[x * 4 + 3] : 0xFF;
					outBGRA[(size_t)y * (size_t)w + (size_t)x] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
				}
			}
			return true;
		}
	}

	// 2. Check DDS ('DDS ') - Uncompressed (16/24/32-bit) & DXT1/DXT3/DXT5
	if (size >= 128 && data[0] == 'D' && data[1] == 'D' && data[2] == 'S' && data[3] == ' ') {
		UINT h = ReadU32Unaligned(data + 12);
		UINT w = ReadU32Unaligned(data + 16);
		UINT fourCC = ReadU32Unaligned(data + 84);
		UINT bpp = ReadU32Unaligned(data + 88);
		UINT rMask = ReadU32Unaligned(data + 92);
		UINT gMask = ReadU32Unaligned(data + 96);
		UINT bMask = ReadU32Unaligned(data + 100);
		UINT aMask = ReadU32Unaligned(data + 104);
		if (w > 0 && h > 0 && w <= 4096 && h <= 4096) {
			const bool isDXT1 = (fourCC == 0x31545844u); // 'DXT1'
			const bool isDXT3 = (fourCC == 0x33545844u); // 'DXT3'
			const bool isDXT5 = (fourCC == 0x35545844u); // 'DXT5'
			if (isDXT1 || isDXT3 || isDXT5) {
				outW = w; outH = h;
				outBGRA.assign((size_t)w * (size_t)h, 0);
				UINT bw = (w + 3) / 4, bh = (h + 3) / 4;
				size_t blockSize = isDXT1 ? 8 : 16;
				size_t offset = 128;
				for (UINT by = 0; by < bh; ++by) {
					for (UINT bx = 0; bx < bw; ++bx) {
						if (offset + blockSize > size) break;
						const BYTE* blk = data + offset;
						offset += blockSize;
						BYTE alpha[16];
						const BYTE* cBlk = blk;
						if (isDXT3) {
							for (int i = 0; i < 8; ++i) {
								alpha[i * 2 + 0] = (blk[i] & 0x0F) * 17;
								alpha[i * 2 + 1] = (blk[i] >> 4) * 17;
							}
							cBlk = blk + 8;
						} else if (isDXT5) {
							BYTE a0 = blk[0], a1 = blk[1];
							uint64_t aBits = 0;
							for (int i = 0; i < 6; ++i) aBits |= ((uint64_t)blk[2 + i]) << (8 * i);
							BYTE aLut[8];
							aLut[0] = a0; aLut[1] = a1;
							if (a0 > a1) {
								for (int i = 1; i <= 6; ++i) aLut[i + 1] = (BYTE)(((7 - i) * a0 + i * a1) / 7);
							} else {
								for (int i = 1; i <= 4; ++i) aLut[i + 1] = (BYTE)(((5 - i) * a0 + i * a1) / 5);
								aLut[6] = 0; aLut[7] = 255;
							}
							for (int i = 0; i < 16; ++i) alpha[i] = aLut[(aBits >> (i * 3)) & 0x07];
							cBlk = blk + 8;
						}
						WORD c0 = ReadU16Unaligned(cBlk + 0);
						WORD c1 = ReadU16Unaligned(cBlk + 2);
						DWORD bits = ReadU32Unaligned(cBlk + 4);
						BYTE r[4], g[4], b[4], a[4] = { 255, 255, 255, 255 };
						r[0] = ((c0 >> 11) & 31) * 255 / 31; g[0] = ((c0 >> 5) & 63) * 255 / 63; b[0] = (c0 & 31) * 255 / 31;
						r[1] = ((c1 >> 11) & 31) * 255 / 31; g[1] = ((c1 >> 5) & 63) * 255 / 63; b[1] = (c1 & 31) * 255 / 31;
						if (!isDXT1 || c0 > c1) {
							r[2] = (2 * r[0] + r[1]) / 3; g[2] = (2 * g[0] + g[1]) / 3; b[2] = (2 * b[0] + b[1]) / 3;
							r[3] = (r[0] + 2 * r[1]) / 3; g[3] = (g[0] + 2 * g[1]) / 3; b[3] = (b[0] + 2 * b[1]) / 3;
						} else {
							r[2] = (r[0] + r[1]) / 2; g[2] = (g[0] + g[1]) / 2; b[2] = (b[0] + b[1]) / 2;
							r[3] = 0; g[3] = 0; b[3] = 0; a[3] = 0;
						}
						for (UINT py = 0; py < 4; ++py) {
							UINT yy = by * 4 + py;
							if (yy >= h) continue;
							for (UINT px = 0; px < 4; ++px) {
								UINT xx = bx * 4 + px;
								if (xx >= w) continue;
								int idx = py * 4 + px;
								int ci = (bits >> (idx * 2)) & 3;
								BYTE pa = isDXT1 ? a[ci] : alpha[idx];
								outBGRA[(size_t)yy * (size_t)w + (size_t)xx] = ((DWORD)pa << 24) | ((DWORD)r[ci] << 16) | ((DWORD)g[ci] << 8) | (DWORD)b[ci];
							}
						}
					}
				}
				return true;
			}
			if (bpp == 24 || bpp == 32) {
				outW = w; outH = h;
				outBGRA.resize((size_t)w * (size_t)h, 0xFFFFFFFF);
				const BYTE* src = data + 128;
				size_t bytesPerPixel = bpp / 8;
				size_t totalPixels = (size_t)w * (size_t)h;
				for (size_t i = 0; i < totalPixels && (128 + (i + 1) * bytesPerPixel <= size); ++i) {
					BYTE b = src[i * bytesPerPixel + 0];
					BYTE g = src[i * bytesPerPixel + 1];
					BYTE r = src[i * bytesPerPixel + 2];
					BYTE a = (bytesPerPixel == 4 && aMask != 0) ? src[i * 4 + 3] : 0xFF;
					outBGRA[i] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
				}
				return true;
			}
			if (bpp == 16) {
				outW = w; outH = h;
				outBGRA.resize((size_t)w * (size_t)h, 0xFFFFFFFF);
				const BYTE* src = data + 128;
				size_t totalPixels = (size_t)w * (size_t)h;
				for (size_t i = 0; i < totalPixels && (128 + (i + 1) * 2 <= size); ++i) {
					WORD c = ReadU16Unaligned(src + i * 2);
					BYTE r, g, b, a = 0xFF;
					if (aMask == 0xF000 || rMask == 0x0F00) { // A4R4G4B4
						a = (aMask ? ((c >> 12) & 0xF) * 17 : 0xFF);
						r = ((c >> 8) & 0xF) * 17; g = ((c >> 4) & 0xF) * 17; b = (c & 0xF) * 17;
					} else if (gMask == 0x03E0) { // A1R5G5B5 / X1R5G5B5
						a = (aMask ? ((c & 0x8000) ? 0xFF : 0x00) : 0xFF);
						r = ((c >> 10) & 31) * 255 / 31; g = ((c >> 5) & 31) * 255 / 31; b = (c & 31) * 255 / 31;
					} else { // R5G6B5
						r = ((c >> 11) & 31) * 255 / 31; g = ((c >> 5) & 63) * 255 / 63; b = (c & 31) * 255 / 31;
					}
					outBGRA[i] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
				}
				return true;
			}
		}
	}

	// 3. Check Truevision TGA (Type 2 Uncompressed True-Color or Type 10 RLE True-Color)
	BYTE idLen = data[0];
	BYTE colorMapType = data[1];
	BYTE imageType = data[2];
	WORD w = ReadU16Unaligned(data + 12);
	WORD h = ReadU16Unaligned(data + 14);
	BYTE bpp = data[16];
	BYTE desc = data[17];

	if (colorMapType == 0 && (imageType == 2 || imageType == 10) && w > 0 && h > 0 && w <= 4096 && h <= 4096 && (bpp == 24 || bpp == 32)) {
		size_t offset = 18 + (size_t)idLen;
		if (offset >= size) return false;
		outW = w; outH = h;
		size_t totalPixels = (size_t)w * (size_t)h;
		std::vector<DWORD> rawPixels(totalPixels, 0xFFFFFFFF);
		size_t bytesPerPixel = bpp / 8;

		if (imageType == 2) {
			for (size_t i = 0; i < totalPixels && (offset + bytesPerPixel <= size); ++i) {
				BYTE b = data[offset + 0];
				BYTE g = data[offset + 1];
				BYTE r = data[offset + 2];
				BYTE a = (bytesPerPixel == 4) ? data[offset + 3] : 0xFF;
				rawPixels[i] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
				offset += bytesPerPixel;
			}
		} else if (imageType == 10) {
			size_t pixIdx = 0;
			while (pixIdx < totalPixels && offset < size) {
				BYTE packetHdr = data[offset++];
				size_t count = (packetHdr & 0x7F) + 1;
				if (packetHdr & 0x80) {
					if (offset + bytesPerPixel > size) break;
					BYTE b = data[offset + 0];
					BYTE g = data[offset + 1];
					BYTE r = data[offset + 2];
					BYTE a = (bytesPerPixel == 4) ? data[offset + 3] : 0xFF;
					offset += bytesPerPixel;
					DWORD col = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
					for (size_t k = 0; k < count && pixIdx < totalPixels; ++k)
						rawPixels[pixIdx++] = col;
				} else {
					for (size_t k = 0; k < count && pixIdx < totalPixels; ++k) {
						if (offset + bytesPerPixel > size) break;
						BYTE b = data[offset + 0];
						BYTE g = data[offset + 1];
						BYTE r = data[offset + 2];
						BYTE a = (bytesPerPixel == 4) ? data[offset + 3] : 0xFF;
						offset += bytesPerPixel;
						rawPixels[pixIdx++] = ((DWORD)a << 24) | ((DWORD)r << 16) | ((DWORD)g << 8) | (DWORD)b;
					}
				}
			}
		}

		bool topOrigin = (desc & 0x20) != 0;
		if (topOrigin) {
			outBGRA.swap(rawPixels);
		} else {
			outBGRA.resize(totalPixels);
			for (UINT y = 0; y < h; ++y) {
				memcpy(&outBGRA[(size_t)y * (size_t)w], &rawPixels[(size_t)(h - 1 - y) * (size_t)w], (size_t)w * sizeof(DWORD));
			}
		}
		return true;
	}

#ifdef __ANDROID__
	// 4. Fallback to Android BitmapFactory for PNG, JPG, and WebP
	if (AndroidDecodeImageFromMemory(data, size, &outW, &outH, outBGRA)) {
		return true;
	}
#endif
	return false;
}
inline HRESULT D3DXGetImageInfoFromFileInMemory(LPCVOID pSrcData, UINT SrcDataSize, D3DXIMAGE_INFO* p) {
#ifndef ANDROID
	if (pSrcData && SrcDataSize > 0 && p) {
		ILuint imgId = 0;
		ilGenImages(1, &imgId);
		ilBindImage(imgId);
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		if (ilLoadL(IL_TYPE_UNKNOWN, pSrcData, SrcDataSize)) {
			p->Width = (UINT)ilGetInteger(IL_IMAGE_WIDTH);
			p->Height = (UINT)ilGetInteger(IL_IMAGE_HEIGHT);
			p->Depth = 1; p->MipLevels = 1; p->Format = D3DFMT_A8R8G8B8;
			ilDeleteImages(1, &imgId);
			return S_OK;
		}
		ilDeleteImages(1, &imgId);
	}
#else
	if (pSrcData && SrcDataSize > 0 && p) {
		UINT w = 0, h = 0;
		std::vector<DWORD> dummy;
		if (DecodeTGAorBMPFromMemory((const BYTE*)pSrcData, SrcDataSize, w, h, dummy)) {
			p->Width = w; p->Height = h; p->Depth = 1; p->MipLevels = 1; p->Format = D3DFMT_A8R8G8B8;
			return S_OK;
		}
	}
#endif
	if (p) { p->Width = 256; p->Height = 256; p->Depth = 1; p->MipLevels = 1; p->Format = D3DFMT_A8R8G8B8; }
	return S_OK;
}
inline HRESULT D3DXCreateTexture(LPDIRECT3DDEVICE8 pDev, UINT W, UINT H, UINT Mip, DWORD Usage, D3DFORMAT Fmt, D3DPOOL Pool, LPDIRECT3DTEXTURE8* ppTex) { return pDev->CreateTexture(W, H, Mip, Usage, Fmt, Pool, ppTex); }
inline HRESULT D3DXCreateTextureFromFileInMemoryEx(LPDIRECT3DDEVICE8 pDev, LPCVOID pSrcData, UINT SrcDataSize, UINT W, UINT H, UINT, DWORD Usage, D3DFORMAT, D3DPOOL Pool, DWORD, DWORD, D3DCOLOR ColorKey, D3DXIMAGE_INFO* pInfo, PALETTEENTRY*, LPDIRECT3DTEXTURE8* ppTex) {
#ifndef ANDROID
	if (pSrcData && SrcDataSize > 0) {
		ILuint imgId = 0;
		ilGenImages(1, &imgId);
		ilBindImage(imgId);
		ilEnable(IL_ORIGIN_SET);
		ilOriginFunc(IL_ORIGIN_UPPER_LEFT);
		if (ilLoadL(IL_TYPE_UNKNOWN, pSrcData, SrcDataSize)) {
			ilConvertImage(IL_BGRA, IL_UNSIGNED_BYTE);
			UINT w = (UINT)ilGetInteger(IL_IMAGE_WIDTH);
			UINT h = (UINT)ilGetInteger(IL_IMAGE_HEIGHT);
			if (w > 0 && h > 0) {
				if (pInfo) { pInfo->Width = w; pInfo->Height = h; pInfo->Depth = 1; pInfo->MipLevels = 1; pInfo->Format = D3DFMT_A8R8G8B8; }
				HRESULT hr = pDev->CreateTexture(w, h, 1, Usage, D3DFMT_A8R8G8B8, Pool, ppTex);
				if (SUCCEEDED(hr) && ppTex && *ppTex) {
					D3DLOCKED_RECT lr;
					if (SUCCEEDED((*ppTex)->LockRect(0, &lr, NULL, 0))) {
						memcpy(lr.pBits, ilGetData(), (size_t)w * (size_t)h * 4);
						(*ppTex)->UnlockRect(0);
					}
				}
				ilDeleteImages(1, &imgId);
				return hr;
			}
		}
		ilDeleteImages(1, &imgId);
	}
#else
	if (pSrcData && SrcDataSize > 0) {
		UINT decW = 0, decH = 0;
		std::vector<DWORD> bgra;
		if (DecodeTGAorBMPFromMemory((const BYTE*)pSrcData, SrcDataSize, decW, decH, bgra)) {
			if (ColorKey != 0) {
				DWORD ckRGB = ColorKey & 0x00FFFFFFu;
				for (size_t i = 0; i < bgra.size(); ++i) {
					if ((bgra[i] & 0x00FFFFFFu) == ckRGB)
						bgra[i] = 0x00000000u;
				}
			}
			if (pInfo) { pInfo->Width = decW; pInfo->Height = decH; pInfo->Depth = 1; pInfo->MipLevels = 1; pInfo->Format = D3DFMT_A8R8G8B8; }
			SwizzleBGRAtoRGBAInPlace(bgra.data(), (int)bgra.size(), false);
			IDirect3DTexture8* tex = new IDirect3DTexture8();
			tex->width = decW; tex->height = decH; tex->format = D3DFMT_A8R8G8B8;
			glActiveTexture(GL_TEXTURE0);
			GLuint prevTex = pDev->m_pBoundTextures[0] ? ((IDirect3DTexture8*)pDev->m_pBoundTextures[0])->glId : 0;
			glGenTextures(1, &tex->glId);
			glBindTexture(GL_TEXTURE_2D, tex->glId);
			glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, decW, decH, 0, GL_RGBA, GL_UNSIGNED_BYTE, bgra.data());
			if (decW >= 64 && decH >= 64 && (decW & (decW - 1)) == 0 && (decH & (decH - 1)) == 0) {
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			} else {
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			}
			glBindTexture(GL_TEXTURE_2D, prevTex);
			*ppTex = tex;
			return S_OK;
		}
	}
#endif
	UINT w = (W == D3DX_DEFAULT || W == D3DX_DEFAULT_NONPOW2 || W == 0) ? 256 : W;
	UINT h = (H == D3DX_DEFAULT || H == D3DX_DEFAULT_NONPOW2 || H == 0) ? 256 : H;
	if (pInfo) { pInfo->Width = w; pInfo->Height = h; pInfo->MipLevels = 1; pInfo->Format = D3DFMT_A8R8G8B8; }
	return pDev->CreateTexture(w, h, 1, Usage, D3DFMT_A8R8G8B8, Pool, ppTex);
}
inline HRESULT D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE8, const PALETTEENTRY*, const RECT*, LPDIRECT3DSURFACE8, const PALETTEENTRY*, const RECT*, DWORD, D3DCOLOR) { return S_OK; }
inline HRESULT D3DXLoadSurfaceFromMemory(LPDIRECT3DSURFACE8, const PALETTEENTRY*, const RECT*, LPCVOID, D3DFORMAT, UINT, const PALETTEENTRY*, const RECT*, DWORD, D3DCOLOR) { return S_OK; }

inline UINT D3DXGetFVFVertexSize(DWORD FVF) {
	UINT size = 0;
	DWORD pos = FVF & 0x00E;
	if (pos == D3DFVF_XYZ) size += 12;
	else if (pos == D3DFVF_XYZRHW) size += 16;
	else if (pos == D3DFVF_XYZB1) size += 16;
	else if (pos == D3DFVF_XYZB2) size += 20;
	else if (pos == D3DFVF_XYZB3) size += 24;
	else if (pos == D3DFVF_XYZB4) size += 28;
	else if (pos == D3DFVF_XYZB5) size += 32;

	if (FVF & D3DFVF_NORMAL) size += 12;
	if (FVF & D3DFVF_PSIZE) size += 4;
	if (FVF & D3DFVF_DIFFUSE) size += 4;
	if (FVF & D3DFVF_SPECULAR) size += 4;

	int numTex = (FVF & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	size += numTex * 8;
	return size;
}

inline GLCOLOR* D3DXColorModulate(GLCOLOR* pOut, const GLCOLOR* pC1, const GLCOLOR* pC2) {
	pOut->r = pC1->r * pC2->r;
	pOut->g = pC1->g * pC2->g;
	pOut->b = pC1->b * pC2->b;
	pOut->a = pC1->a * pC2->a;
	return pOut;
}

inline GLCOLOR* D3DXColorLerp(GLCOLOR* pOut, const GLCOLOR* pC1, const GLCOLOR* pC2, float s) {
	pOut->r = pC1->r + s * (pC2->r - pC1->r);
	pOut->g = pC1->g + s * (pC2->g - pC1->g);
	pOut->b = pC1->b + s * (pC2->b - pC1->b);
	pOut->a = pC1->a + s * (pC2->a - pC1->a);
	return pOut;
}

inline GLCOLOR* D3DXColorScale(GLCOLOR* pOut, const GLCOLOR* pC, float s) {
	pOut->r = pC->r * s;
	pOut->g = pC->g * s;
	pOut->b = pC->b * s;
	pOut->a = pC->a * s;
	return pOut;
}

inline GLCOLOR* D3DXColorAdd(GLCOLOR* pOut, const GLCOLOR* pC1, const GLCOLOR* pC2) {
	pOut->r = pC1->r + pC2->r;
	pOut->g = pC1->g + pC2->g;
	pOut->b = pC1->b + pC2->b;
	pOut->a = pC1->a + pC2->a;
	return pOut;
}

inline GLCOLOR* D3DXColorSubtract(GLCOLOR* pOut, const GLCOLOR* pC1, const GLCOLOR* pC2) {
	pOut->r = pC1->r - pC2->r;
	pOut->g = pC1->g - pC2->g;
	pOut->b = pC1->b - pC2->b;
	pOut->a = pC1->a - pC2->a;
	return pOut;
}

// DirectX 9 to OpenGL Shim Compatibility Aliases
typedef IDirect3D8 IDirect3D9;
typedef IDirect3DDevice8 IDirect3DDevice9;
typedef IDirect3DTexture8 IDirect3DTexture9;
typedef IDirect3DBaseTexture8 IDirect3DBaseTexture9;
typedef IDirect3DSurface8 IDirect3DSurface9;
typedef IDirect3DVertexBuffer8 IDirect3DVertexBuffer9;
typedef IDirect3DIndexBuffer8 IDirect3DIndexBuffer9;
typedef LPDIRECT3D8 LPDIRECT3D9;
typedef LPDIRECT3DDEVICE8 LPDIRECT3DDEVICE9;
typedef LPDIRECT3DTEXTURE8 LPDIRECT3DTEXTURE9;
typedef LPDIRECT3DBASETEXTURE8 LPDIRECT3DBASETEXTURE9;
typedef LPDIRECT3DSURFACE8 LPDIRECT3DSURFACE9;
typedef LPDIRECT3DVERTEXBUFFER8 LPDIRECT3DVERTEXBUFFER9;
typedef LPDIRECT3DINDEXBUFFER8 LPDIRECT3DINDEXBUFFER9;
typedef D3DVIEWPORT8 D3DVIEWPORT9;
typedef D3DCAPS8 D3DCAPS9;
typedef D3DLIGHT8 D3DLIGHT9;
typedef D3DMATERIAL8 D3DMATERIAL9;
typedef D3DADAPTER_IDENTIFIER8 D3DADAPTER_IDENTIFIER9;
#define Direct3DCreate9 Direct3DCreate8

typedef int D3DSAMPLERSTATETYPE;
#define D3DSAMP_ADDRESSU D3DTSS_ADDRESSU
#define D3DSAMP_ADDRESSV D3DTSS_ADDRESSV
#define D3DSAMP_ADDRESSW D3DTSS_ADDRESSW
#define D3DSAMP_BORDERCOLOR D3DTSS_BORDERCOLOR
#define D3DSAMP_MAGFILTER D3DTSS_MAGFILTER
#define D3DSAMP_MINFILTER D3DTSS_MINFILTER
#define D3DSAMP_MIPFILTER D3DTSS_MIPFILTER
#define D3DSAMP_MIPMAPLODBIAS D3DTSS_MIPMAPLODBIAS
#define D3DSAMP_MAXMIPLEVEL D3DTSS_MAXMIPLEVEL
#define D3DSAMP_MAXANISOTROPY D3DTSS_MAXANISOTROPY

inline GLMATRIX* D3DXMatrixMultiplyTranspose(GLMATRIX* pOut, const GLMATRIX* pM1, const GLMATRIX* pM2) {
	GLMATRIX temp;
	D3DXMatrixMultiply(&temp, pM1, pM2);
	return D3DXMatrixTranspose(pOut, &temp);
}

inline GLPLANE* D3DXPlaneTransform(GLPLANE* pOut, const GLPLANE* pP, const GLMATRIX* pM) {
	GLPLANE out;
	out.a = pM->m[0][0] * pP->a + pM->m[1][0] * pP->b + pM->m[2][0] * pP->c + pM->m[3][0] * pP->d;
	out.b = pM->m[0][1] * pP->a + pM->m[1][1] * pP->b + pM->m[2][1] * pP->c + pM->m[3][1] * pP->d;
	out.c = pM->m[0][2] * pP->a + pM->m[1][2] * pP->b + pM->m[2][2] * pP->c + pM->m[3][2] * pP->d;
	out.d = pM->m[0][3] * pP->a + pM->m[1][3] * pP->b + pM->m[2][3] * pP->c + pM->m[3][3] * pP->d;
	*pOut = out;
	return pOut;
}

inline int DetectAndroidShaderType(LPCSTR src) {
	if (!src) return PS_TYPE_NONE;
	if (strstr(src, "DepthPosition") || strstr(src, "ps.1.4")) return PS_TYPE_SHADOW_DEPTH;
	if (strstr(src, "SplatsTexture") || strstr(src, "AlphaOverride")) return PS_TYPE_TERRAIN_BASE;
	if (strstr(src, "terrainShadow") || (strstr(src, "ShadowMap1") && !strstr(src, "DiffuseTexture") && !strstr(src, "water"))) return PS_TYPE_TERRAIN_SHADOW;
	if (strstr(src, "waves_timer") || strstr(src, "waterOpacity")) return PS_TYPE_WATER;
	if (strstr(src, "windStrength") || strstr(src, "grass")) return PS_TYPE_GRASS;
	if (strstr(src, "ShadowMap1") && strstr(src, "DiffuseTexture")) return PS_TYPE_SHADOW_MAIN;
	return PS_TYPE_NONE;
}

inline HRESULT D3DXAssembleShader(LPCSTR pSrc, UINT, const void*, void*, DWORD, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE) {
	if (ppS) { *ppS = new ID3DXBuffer(); (*ppS)->dummy[0] = (DWORD)DetectAndroidShaderType(pSrc); }
	if (ppE) *ppE = new ID3DXBuffer();
	return S_OK;
}
inline HRESULT D3DXAssembleShaderFromFile(LPCSTR, const void*, void*, DWORD, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE) {
	if (ppS) *ppS = new ID3DXBuffer();
	if (ppE) *ppE = new ID3DXBuffer();
	return S_OK;
}
inline HRESULT D3DXCompileShader(LPCSTR pSrc, UINT, const void*, void*, LPCSTR, LPCSTR, DWORD, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE, LPD3DXCONSTANTTABLE* ppC) {
	if (ppS) { *ppS = new ID3DXBuffer(); (*ppS)->dummy[0] = (DWORD)DetectAndroidShaderType(pSrc); }
	if (ppE) *ppE = new ID3DXBuffer();
	if (ppC) *ppC = new ID3DXConstantTable();
	return S_OK;
}
inline HRESULT D3DXCompileShaderFromFile(LPCSTR, const void*, void*, LPCSTR, LPCSTR, DWORD, LPD3DXBUFFER* ppS, LPD3DXBUFFER* ppE, LPD3DXCONSTANTTABLE* ppC) {
	if (ppS) *ppS = new ID3DXBuffer();
	if (ppE) *ppE = new ID3DXBuffer();
	if (ppC) *ppC = new ID3DXConstantTable();
	return S_OK;
}
inline HRESULT D3DXCreateTextureFromFile(LPDIRECT3DDEVICE8 pDev, LPCSTR, LPDIRECT3DTEXTURE8* ppTex) {
	return pDev->CreateTexture(256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, ppTex);
}
inline HRESULT D3DXCreateTextureFromFileEx(LPDIRECT3DDEVICE8 pDev, LPCSTR, UINT W, UINT H, UINT, DWORD Usage, D3DFORMAT, D3DPOOL Pool, DWORD, DWORD, D3DCOLOR, D3DXIMAGE_INFO* pInfo, PALETTEENTRY*, LPDIRECT3DTEXTURE8* ppTex) {
	UINT w = (W == D3DX_DEFAULT || W == D3DX_DEFAULT_NONPOW2 || W == 0) ? 256 : W;
	UINT h = (H == D3DX_DEFAULT || H == D3DX_DEFAULT_NONPOW2 || H == 0) ? 256 : H;
	if (pInfo) { pInfo->Width = w; pInfo->Height = h; pInfo->MipLevels = 1; pInfo->Format = D3DFMT_A8R8G8B8; }
	return pDev->CreateTexture(w, h, 1, Usage, D3DFMT_A8R8G8B8, Pool, ppTex);
}
inline HRESULT D3DXCreateTextureFromFileInMemory(LPDIRECT3DDEVICE8 pDev, LPCVOID, UINT, LPDIRECT3DTEXTURE8* ppTex) {
	return pDev->CreateTexture(256, 256, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, ppTex);
}

#endif // ENABLE_OPENGL