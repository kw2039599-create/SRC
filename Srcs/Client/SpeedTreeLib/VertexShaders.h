///////////////////////////////////////////////////////////////////////  
//	SpeedTreeRT4.1 DirectX9 Example
//
//	(c) 2006 IDV, Inc.
//
//	This example demonstrates how to render trees using SpeedTreeRT
//	and DirectX.  Techniques illustrated include ".spt" file parsing,
//	static lighting, dynamic lighting, LOD implementation, cloning,
//	instancing, and dynamic wind effects.
//
//
//	*** INTERACTIVE DATA VISUALIZATION (IDV) PROPRIETARY INFORMATION ***
//
//	This software is supplied under the terms of a license agreement or
//	nondisclosure agreement with Interactive Data Visualization and may
//	not be copied or disclosed except in accordance with the terms of
//	that agreement.
//
//      Copyright (c) 2001-2006 IDV, Inc.
//      All Rights Reserved.
//
//		IDV, Inc.
//		1233 Washington St. Suite 610
//		Columbia, SC 29201
//		Voice: (803) 799-1699
//		Fax:   (803) 931-0320
//		Web:   http://www.idvinc.com


///////////////////////////////////////////////////////////////////////  
//	Includes

#pragma once
#include <MicrosoftDirectX/d3d9.h>
#include <MicrosoftDirectX/d3dx9.h>
#include "SpeedTreeConfig.h"
#include <map>
#include <string>


static DWORD D3DFVF_SPEEDTREE_BRANCH_VERTEX =
D3DFVF_XYZ |
#ifdef JTX_TREE_DYNAMIC_LIGHT
D3DFVF_NORMAL |
#else
D3DFVF_DIFFUSE |
#endif
D3DFVF_TEXCOORDSIZE2(0) |
D3DFVF_TEXCOORDSIZE2(1) |
D3DFVF_TEXCOORDSIZE2(2)
#ifdef JTX_GPU_TREE_WIND
#ifdef WRAPPER_RENDER_SELF_SHADOWS
| D3DFVF_TEX3;
#else
| D3DFVF_TEX2;
#endif
#else
#ifdef WRAPPER_RENDER_SELF_SHADOWS
| D3DFVF_TEX2;
#else
| D3DFVF_TEX1;
#endif
#endif


///////////////////////////////////////////////////////////////////////
// FVF Branch Vertex Structure

struct SFVFBranchVertex
{
	D3DXVECTOR3		m_vPosition;			// Always Used							
#ifdef JTX_TREE_DYNAMIC_LIGHT			
	D3DXVECTOR3		m_vNormal;				// Dynamic Lighting Only			
#else										     
	DWORD			m_dwDiffuseColor;		// Static Lighting Only	
#endif	
	FLOAT			m_fTexCoords[2];		// Always Used
#ifdef JTX_GPU_TREE_WIND		
	FLOAT			m_fWindIndex;			// GPU Only
	FLOAT			m_fWindWeight;
#endif
#ifdef WRAPPER_RENDER_SELF_SHADOWS
	FLOAT			m_fShadowCoords[2];		// Texture coordinates for the shadows
#endif
};


///////////////////////////////////////////////////////////////////////  
//	Branch/Frond Vertex Program

static const char g_achSimpleVertexProgram[] =
{
		"vs.1.1\n"											// identity shader version

		"dcl_position		v0\n"							// declare the data coming in
	#ifdef JTX_TREE_DYNAMIC_LIGHT	
		"dcl_normal			v3\n"
	#else
		"dcl_color			v5\n"
	#endif
		"dcl_texcoord0		v7\n"
		"dcl_texcoord1		v8\n"
	#ifdef WRAPPER_RENDER_SELF_SHADOWS
		"dcl_texcoord2		v9\n"
	#endif

		"mov		oT0.xy,	v7\n"							// always pass texcoord0 through

	#ifdef WRAPPER_RENDER_SELF_SHADOWS
		"mov		oT1.xy, v9\n"							// pass shadow texcoords through
	#endif

		// retrieve and convert wind matrix index
		"mov		a0.x,	v8.x\n"

	// perform wind interpolation
	"m4x4		r1,		v0,			c[54+a0.x]\n"		// compute full wind effect
	"sub		r2,		r1,			v0\n"				// compute difference between full wind and none
	"mov		r3.x,	v8.y\n"							// mad can't access two v's at once, use r3.x as tmp
	"mad		r1,		r2,			r3.x,		v0\n"	// perform interpolation

	"add		r2,		c[52],		r1\n"				// translate to tree's position
	"m4x4		r4,		r2,			c[0]\n"				// project to screen
	"mov		oPos,	r4\n"     //// use of temp register
	"mov		oT1,    r4\n"   //// pass Z value as additional output

#ifdef WRAPPER_USE_FOG
	"dp4		r1,			r2,			c[2]\n"			// compute distance
	"sub		r2.x,		c[85].y,	r1.z\n"			// linear fog
	"mul		oFog,		r2.x,		c[85].z\n"		// write outputs
#endif

#ifdef WRAPPER_USE_STATIC_LIGHTING
	"mov		oD0,	v5\n"							// pass color through
#else
	"mov		r1,		c[74]\n"						// can only use one const register per instruction
	"mul		r5,		c[73],		r1\n"				// diffuse values

	"mov		r1,		c[75]\n"						// can only use one const register per instruction
	"mul		r4,		c[72],		r1\n"				// ambient values

	"dp3		r2,		v3,			-c[71]\n"				// dot light direction with normal
	"max		r2.x,		r2.x,		c[38].x\n"			// limit it
	"mad		oD0,	r2.x,		r5,			r4\n"	// compute the final color
#endif
};


///////////////////////////////////////////////////////////////////////  
//	LoadBranchShader

inline LPDIRECT3DVERTEXSHADER9 LoadBranchShader(LPDIRECT3DDEVICE9 pDx)
{
#ifndef JTX_GPU_TREE_WIND
	//lint -e522
	UNREFERENCED_PARAMETER(pDx);
	return NULL;
#else
	// assemble shader
	LPDIRECT3DVERTEXSHADER9 dwShader;
	LPD3DXBUFFER pCode, pError;
	if (D3DXAssembleShader(g_achSimpleVertexProgram, sizeof(g_achSimpleVertexProgram) - 1, 0, NULL, 0, &pCode, &pError) == D3D_OK)
	{
		if (pDx->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &dwShader) != D3D_OK)
		{
			char szError[1024];
			sprintf(szError, "Failed to create branch vertex shader.");
			MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);
		}
	}
	else
	{
		char szError[1024];
		sprintf(szError, "Failed to assemble branch vertex shader.\nThe error reported is [ %s ].\n", pError->GetBufferPointer());
		MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	if (pCode)
		pCode->Release();

	return dwShader;
#endif
}


///////////////////////////////////////////////////////////////////////  
//	Leaf Vertex Formats

static DWORD D3DFVF_SPEEDTREE_LEAF_VERTEX =
D3DFVF_XYZ |							// always have the position
#ifdef JTX_TREE_DYNAMIC_LIGHT			// precomputed colors or geometric normals
D3DFVF_NORMAL |
#else
D3DFVF_DIFFUSE |
#endif
D3DFVF_TEXCOORDSIZE2(0) |
D3DFVF_TEXCOORDSIZE2(1) |
D3DFVF_TEXCOORDSIZE2(2) |
#if defined JTX_GPU_TREE_WIND || defined JTX_GPU_LEAF_PLACEMENT
D3DFVF_TEX3;
#else
D3DFVF_TEX1;
#endif


///////////////////////////////////////////////////////////////////////
// FVF Leaf Vertex Structure

struct SFVFLeafVertex
{
	D3DXVECTOR3		m_vPosition;			// Always Used							
#ifdef JTX_TREE_DYNAMIC_LIGHT			
	D3DXVECTOR3		m_vNormal;				// Dynamic Lighting Only			
#else										     
	DWORD			m_dwDiffuseColor;		// Static Lighting Only	
#endif											
	FLOAT			m_fTexCoords[2];		// Always Used
#if defined JTX_GPU_TREE_WIND || defined JTX_GPU_LEAF_PLACEMENT
	FLOAT			m_fWindIndex;			// Only used when GPU is involved
	FLOAT			m_fWindWeight;
	FLOAT			m_fLeafPlacementIndex;
	FLOAT			m_fLeafScalarValue;
#endif
};


///////////////////////////////////////////////////////////////////////  
//	Leaf Vertex Program

static const char g_achLeafVertexProgram[] =
{
		"vs.1.1\n"											// identity shader version

		"dcl_position		v0\n"							// declare the data coming in
	#ifdef JTX_TREE_DYNAMIC_LIGHT	
		"dcl_normal			v3\n"
	#else
		"dcl_color			v5\n"
	#endif
		"dcl_texcoord0		v7\n"
		"dcl_texcoord1		v8\n"
		"dcl_texcoord2		v9\n"

		"mov		oT0.xy,	v7\n"							// always pass texcoord0 through

	#ifdef JTX_GPU_TREE_WIND
		// retrieve and convert wind matrix index
		"mov		a0.x,	v8.x\n"

	// perform wind interpolation
	"m4x4		r1,		v0,			c[54+a0.x]\n"		// compute full wind effect
	"sub		r2,		r1,			v0\n"				// compute difference between full wind and none
	"mov		r3.x,	v8.y\n"							// mad can't access two v's at once, use r3.x as tmp
	"mad		r0,		r2,			r3.x,		v0\n"	// perform interpolation
#else
	"mov		r0,		v0\n"							// wind already handled, pass the vertex through
#endif

#ifdef JTX_GPU_LEAF_PLACEMENT
	"mov		a0.x,	v9.x\n"							// place the leaves
	"mul		r1,		c[a0.x],	v9.y\n"
	"add		r0,		r1,			r0\n"
#endif

	"add		r0,		c[52],		r0\n"				// translate to tree's position
	"m4x4		r4,	r0,			c[0]\n"				// project to screen
	"mov		oPos,	r4\n"     //// use of temp register
	"mov		oT1,    r4\n"   //// pass Z value as additional output
	//"mov		oT1,    c0\n"  

#ifdef WRAPPER_USE_FOG
	"dp4		r1,			r0,			c[2]\n"			// compute distance
	"sub		r2.x,		c[85].y,	r1.z\n"			// linear fog
	"mul		oFog,		r2.x,		c[85].z\n"		// write outputs
#endif

#ifdef WRAPPER_USE_STATIC_LIGHTING
	"mov		oD0,	v5\n"							// pass color through
#else
	"mov		r1,		c[74]\n"						// can only use one const register per instruction
	"mul		r5,		c[73],		r1\n"				// diffuse values

	"mov		r1,		c[75]\n"						// can only use one const register per instruction
	"mul		r4,		c[72],		r1\n"				// ambient values

	"dp3		r2.x,   v3,			-c[71]\n"			// dot light direction with normal
	"max		r2.x,	r2.x,		c[38].x\n"			// limit it
	"mad		oD0,	r2.x,		r5,			r4\n"	// compute the final color
#endif
};


///////////////////////////////////////////////////////////////////////  
//	LoadLeafShader

inline LPDIRECT3DVERTEXSHADER9 LoadLeafShader(LPDIRECT3DDEVICE9 pDx)
{
	LPDIRECT3DVERTEXSHADER9 dwShader = NULL;

#if defined JTX_GPU_LEAF_PLACEMENT || defined JTX_GPU_TREE_WIND

	// assemble shader
	LPD3DXBUFFER pCode, pError;

	if (D3DXAssembleShader(g_achLeafVertexProgram, sizeof(g_achLeafVertexProgram) - 1, 0, NULL, 0, &pCode, &pError) == D3D_OK)
	{
		if (pDx->CreateVertexShader((DWORD*)pCode->GetBufferPointer(), &dwShader) != D3D_OK)
		{
			char szError[1024];
			sprintf(szError, "Failed to create leaf vertex shader.");
			(void)MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);
		}
	}
	else
	{
		char szError[1024];
		sprintf(szError, "Failed to assemble leaf vertex shader. The error reported is [ %s ].\n", (char*)pError->GetBufferPointer());
		(void)MessageBox(NULL, szError, "Vertex Shader Error", MB_ICONSTOP);
	}

	if (pCode)
		pCode->Release();

#endif

	return dwShader;
}

static DWORD D3DFVF_SPEEDTREE_BILLBOARD_VERTEX = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;


///////////////////////////////////////////////////////////////////////  
//	Branch & Frond Vertex Formats


//martysama0134's fb0d8d5d0566583187a94f9a823b2075
