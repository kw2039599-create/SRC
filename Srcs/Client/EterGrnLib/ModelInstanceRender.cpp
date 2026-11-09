#include "StdAfx.h"
#include "../eterlib/StateManager.h"
#include "ModelInstance.h"
#include "Model.h"
#include "../SphereLib/Frustum.H"

#ifdef _TEST

#include "../eterlib/GrpScreen.h"

void Granny_RenderBoxBones(const granny_skeleton* pkGrnSkeleton, const granny_world_pose* pkGrnWorldPose, const D3DXMATRIX& matBase)
{
	D3DXMATRIX matWorld;
	CScreen screen;
	for (int iBone = 0; iBone != pkGrnSkeleton->BoneCount; ++iBone)
	{
		const granny_bone& rkGrnBone = pkGrnSkeleton->Bones[iBone];
		const D3DXMATRIX* c_matBone=(const D3DXMATRIX*)GrannyGetWorldPose4x4(pkGrnWorldPose, iBone);

		D3DXMatrixMultiply(&matWorld, c_matBone, &matBase);

		STATEMANAGER.SetTransform(D3DTS_WORLD, &matWorld);
		screen.RenderBox3d(-5.0f, -5.0f, -5.0f, 5.0f, 5.0f, 5.0f);
	}
}

#endif

void CGrannyModelInstance::DeformNoSkin(const D3DXMATRIX * c_pWorldMatrix)
{
	if (IsEmpty())
		return;

	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render
///////////////////////////////////////////////////////////////////////////////////////////////////

// With One Texture
void CGrannyModelInstance::RenderWithOneTexture()
{
	if (IsEmpty())
		return;

#ifdef _TEST
	Granny_RenderBoxBones(GrannyGetSourceSkeleton(m_pgrnModelInstance), m_pgrnWorldPose, TEST_matWorld);
	if (GetAsyncKeyState('P'))
		Tracef("render %x", m_pgrnModelInstance);
	return;
#endif

	const bool bHasDeform = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	const bool bHasRigid = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	if (!bHasDeform && !bHasRigid)
		return;

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(ms_pntVS);
#else
	STATEMANAGER.SetVertexShader(ms_pntVS);
#endif

	if (bHasDeform)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
		if (lpd3dDeformPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		}
	}

	if (bHasRigid)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();
		if (lpd3dRigidPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		}
	}
}

void CGrannyModelInstance::BlendRenderWithOneTexture()
{
	if (IsEmpty())
		return;

	const bool bHasDeform = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);
	const bool bHasRigid = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);
	if (!bHasDeform && !bHasRigid)
		return;

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(ms_pntVS);
#else
	STATEMANAGER.SetVertexShader(ms_pntVS);
#endif

	if (bHasDeform)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
		if (lpd3dDeformPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}

	if (bHasRigid)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();
		if (lpd3dRigidPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithOneTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}
}

// With Two Texture
void CGrannyModelInstance::RenderWithTwoTexture()
{
	if (IsEmpty())
		return;

	const bool bHasDeform = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	const bool bHasRigid = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	if (!bHasDeform && !bHasRigid)
		return;

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(ms_pntVS);
#else
	STATEMANAGER.SetVertexShader(ms_pntVS);
#endif

	if (bHasDeform)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
		if (lpd3dDeformPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		}
	}

	if (bHasRigid)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();
		if (lpd3dRigidPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
		}
	}
}

void CGrannyModelInstance::BlendRenderWithTwoTexture()
{
	if (IsEmpty())
		return;

	const bool bHasDeform = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);
	const bool bHasRigid = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);
	if (!bHasDeform && !bHasRigid)
		return;

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(ms_pntVS);
#else
	STATEMANAGER.SetVertexShader(ms_pntVS);
#endif

	if (bHasDeform)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
		if (lpd3dDeformPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}

	if (bHasRigid)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();
		if (lpd3dRigidPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
			RenderMeshNodeListWithTwoTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}
}

void CGrannyModelInstance::RenderWithoutTexture()
{
	if (IsEmpty())
		return;

#ifdef DIRECTX9
	STATEMANAGER.SetFVF(ms_pntVS);
#else
	STATEMANAGER.SetVertexShader(ms_pntVS);
#endif
	if (Frustum::Instance().graphicsLevel != 0 && Frustum::Instance().shadowTexture)
		STATEMANAGER.SetTexture(0, Frustum::Instance().shadowTexture);
	else
		STATEMANAGER.SetTexture(0, NULL);
	STATEMANAGER.SetTexture(1, NULL);

	const bool bHasDeformDiffuse = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	const bool bHasDeformBlend = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);

	if (bHasDeformDiffuse || bHasDeformBlend)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dDeformPNTVtxBuf = __GetDeformableD3DVertexBufferPtr();
		if (lpd3dDeformPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dDeformPNTVtxBuf, sizeof(TPNTVertex));
			if (bHasDeformDiffuse)
				RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_DIFFUSE_PNT);
			if (bHasDeformBlend)
				RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_DEFORM, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}

	const bool bHasRigidDiffuse = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT) != NULL);
	const bool bHasRigidBlend = (m_pModel->GetMeshNodeList(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT) != NULL);

	if (bHasRigidDiffuse || bHasRigidBlend)
	{
		LPDIRECT3DVERTEXBUFFER8 lpd3dRigidPNTVtxBuf = m_pModel->GetPNTD3DVertexBuffer();
		if (lpd3dRigidPNTVtxBuf)
		{
			STATEMANAGER.SetStreamSource(0, lpd3dRigidPNTVtxBuf, sizeof(TPNTVertex));
			if (bHasRigidDiffuse)
				RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_DIFFUSE_PNT);
			if (bHasRigidBlend)
				RenderMeshNodeListWithoutTexture(CGrannyMesh::TYPE_RIGID, CGrannyMaterial::TYPE_BLEND_PNT);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//// Render Mesh List
///////////////////////////////////////////////////////////////////////////////////////////////////

// With One Texture
void CGrannyModelInstance::RenderMeshNodeListWithOneTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	LPDIRECT3DINDEXBUFFER8 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);
#ifdef DIRECTX9
	STATEMANAGER.SetIndices(lpd3dIdxBuf, 0);
#endif

	const D3DXMATRIX* pLastWorldMat = nullptr;

	while (pMeshNode)
	{
		const CGrannyMesh * pMesh = pMeshNode->pMesh;
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		if (!pTriGroupNode)
		{
			pMeshNode = pMeshNode->pNextMeshNode;
			continue;
		}

		int vtxMeshBasePos = pMesh->GetVertexBasePosition();
#if !defined(DIRECTX9) || defined(__ANDROID__)
		STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
#endif
		const D3DXMATRIX& rMeshMat = m_meshMatrices[pMeshNode->iMesh];
		if (!pLastWorldMat || memcmp(pLastWorldMat, &rMeshMat, sizeof(D3DXMATRIX)) != 0)
		{
			pLastWorldMat = &rMeshMat;
			STATEMANAGER.SetTransform(D3DTS_WORLD, &rMeshMat);
#ifndef __ANDROID__
			D3DXMATRIX matShader;
			D3DXMatrixTranspose(&matShader, &rMeshMat);
			STATEMANAGER.SetVertexShaderConstant(20, &matShader, 4);
#endif
		}

		int vtxCount = pMesh->GetVertexCount();
		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;

			CGrannyMaterial& rkMtrl=m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
			rkMtrl.ApplyRenderState();
#ifdef DIRECTX9
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
#else
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
#endif
			rkMtrl.RestoreRenderState();

			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}

// With Two Texture
void CGrannyModelInstance::RenderMeshNodeListWithTwoTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	LPDIRECT3DINDEXBUFFER8 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);
#ifdef DIRECTX9
	STATEMANAGER.SetIndices(lpd3dIdxBuf, 0);
#endif

	const D3DXMATRIX* pLastWorldMat = nullptr;

	while (pMeshNode)
	{
		const CGrannyMesh * pMesh = pMeshNode->pMesh;
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		if (!pTriGroupNode)
		{
			pMeshNode = pMeshNode->pNextMeshNode;
			continue;
		}

		int vtxMeshBasePos = pMesh->GetVertexBasePosition();
#if !defined(DIRECTX9) || defined(__ANDROID__)
		STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
#endif
		const D3DXMATRIX& rMeshMat = m_meshMatrices[pMeshNode->iMesh];
		if (!pLastWorldMat || memcmp(pLastWorldMat, &rMeshMat, sizeof(D3DXMATRIX)) != 0)
		{
			pLastWorldMat = &rMeshMat;
			STATEMANAGER.SetTransform(D3DTS_WORLD, &rMeshMat);
#ifndef __ANDROID__
			D3DXMATRIX matShader;
			D3DXMatrixTranspose(&matShader, &rMeshMat);
			STATEMANAGER.SetVertexShaderConstant(20, &matShader, 4);
#endif
		}

		int vtxCount = pMesh->GetVertexCount();
		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;

			const CGrannyMaterial& rkMtrl=m_kMtrlPal.GetMaterialRef(pTriGroupNode->mtrlIndex);
			STATEMANAGER.SetTexture(0, rkMtrl.GetD3DTexture(0));
			STATEMANAGER.SetTexture(1, rkMtrl.GetD3DTexture(1));
#ifdef DIRECTX9
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
#else
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
#endif
			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}

// Without Texture
void CGrannyModelInstance::RenderMeshNodeListWithoutTexture(CGrannyMesh::EType eMeshType, CGrannyMaterial::EType eMtrlType)
{
	assert(m_pModel != NULL);

	LPDIRECT3DINDEXBUFFER8 lpd3dIdxBuf = m_pModel->GetD3DIndexBuffer();
	assert(lpd3dIdxBuf != NULL);

	const CGrannyModel::TMeshNode * pMeshNode = m_pModel->GetMeshNodeList(eMeshType, eMtrlType);
#ifdef DIRECTX9
	STATEMANAGER.SetIndices(lpd3dIdxBuf, 0);
#endif

	const D3DXMATRIX* pLastWorldMat = nullptr;

	while (pMeshNode)
	{
		const CGrannyMesh * pMesh = pMeshNode->pMesh;
		const CGrannyMesh::TTriGroupNode* pTriGroupNode = pMesh->GetTriGroupNodeList(eMtrlType);
		if (!pTriGroupNode)
		{
			pMeshNode = pMeshNode->pNextMeshNode;
			continue;
		}

		int vtxMeshBasePos = pMesh->GetVertexBasePosition();
#if !defined(DIRECTX9) || defined(__ANDROID__)
		STATEMANAGER.SetIndices(lpd3dIdxBuf, vtxMeshBasePos);
#endif
		const D3DXMATRIX& rMeshMat = m_meshMatrices[pMeshNode->iMesh];
		if (!pLastWorldMat || memcmp(pLastWorldMat, &rMeshMat, sizeof(D3DXMATRIX)) != 0)
		{
			pLastWorldMat = &rMeshMat;
			STATEMANAGER.SetTransform(D3DTS_WORLD, &rMeshMat);
#ifndef __ANDROID__
			D3DXMATRIX matShader;
			D3DXMatrixTranspose(&matShader, &rMeshMat);
			STATEMANAGER.SetVertexShaderConstant(20, &matShader, 4);
#endif
		}

		int vtxCount = pMesh->GetVertexCount();

		while (pTriGroupNode)
		{
			ms_faceCount += pTriGroupNode->triCount;
#ifdef DIRECTX9
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount, vtxMeshBasePos);
#else
			STATEMANAGER.DrawIndexedPrimitive(D3DPT_TRIANGLELIST, 0, vtxCount, pTriGroupNode->idxPos, pTriGroupNode->triCount);
#endif
			pTriGroupNode = pTriGroupNode->pNextTriGroupNode;
		}

		pMeshNode = pMeshNode->pNextMeshNode;
	}
}