#include "StdAfx.h"
#include "../eterbase/Debug.h"
#include "ModelInstance.h"
#include "Model.h"

#ifdef __ANDROID__
#include <unordered_map>
struct SAndroidDeformState {
	DWORD lastUpdateFrame = 0xFFFFFFFF;
	DWORD lastWorldPoseFrame = 0xFFFFFFFF;
	int warmupCount = 0;
	D3DXMATRIX lastWorldMat;
};
static std::unordered_map<const CGrannyModelInstance*, SAndroidDeformState> s_androidDeformCache;
#endif

void CGrannyModelInstance::Update(DWORD dwAniFPS)
{
	if (!dwAniFPS)
		return;

	const DWORD c_dwCurUpdateFrame = (DWORD) (GetLocalTime() * ANIFPS_MAX);
	const DWORD ANIFPS_STEP = ANIFPS_MAX / dwAniFPS;
	if (c_dwCurUpdateFrame > ANIFPS_STEP && c_dwCurUpdateFrame / ANIFPS_STEP == m_dwOldUpdateFrame / ANIFPS_STEP)
		return;

	m_dwOldUpdateFrame = c_dwCurUpdateFrame;

	GrannyFreeCompletedModelControls(m_pgrnModelInstance); //@fixme029
	GrannySetModelClock(m_pgrnModelInstance, GetLocalTime());

#ifdef __ANDROID__
	if (m_pgrnModelInstance && (!m_ppkSkeletonInst || *m_ppkSkeletonInst == this))
	{
		UpdateWorldPose();
		s_androidDeformCache[this].lastWorldPoseFrame = m_dwOldUpdateFrame;
	}
#endif
}

void CGrannyModelInstance::UpdateLocalTime(float fElapsedTime)
{
	m_fSecondsElapsed = fElapsedTime;
	m_fLocalTime += fElapsedTime;
}

void CGrannyModelInstance::UpdateTransform(D3DXMATRIX * pMatrix, float fSecondsElapsed)
{
	if (!m_pgrnModelInstance)
	{
		TraceError("CGrannyModelIstance::UpdateTransform - m_pgrnModelInstance = NULL");
		return;
	}
	GrannySetModelClock(m_pgrnModelInstance, GetLocalTime());
#if GrannyProductMinorVersion==4
	GrannyUpdateModelMatrix(m_pgrnModelInstance, fSecondsElapsed, (const float *) pMatrix, (float *) pMatrix);
#elif GrannyProductMinorVersion==11 || GrannyProductMinorVersion==9 || GrannyProductMinorVersion==8 || GrannyProductMinorVersion==7
	GrannyUpdateModelMatrix(m_pgrnModelInstance, fSecondsElapsed, (const float *) pMatrix, (float *) pMatrix, false);
#else
#error "unknown granny version"
#endif
}

void CGrannyModelInstance::Deform(const D3DXMATRIX * c_pWorldMatrix)
{
	if (IsEmpty())
		return;

#ifdef __ANDROID__
	SAndroidDeformState& st = s_androidDeformCache[this];
	DWORD effectiveFrame = m_dwOldUpdateFrame;
	if (m_ppkSkeletonInst && *m_ppkSkeletonInst)
		effectiveFrame = (*m_ppkSkeletonInst)->m_dwOldUpdateFrame;
	else if (mc_pParentInstance)
		effectiveFrame = mc_pParentInstance->m_dwOldUpdateFrame;

	const bool bNeedPose = (st.warmupCount < 4) ||
	                       (st.lastUpdateFrame != effectiveFrame);
	if (!bNeedPose)
	{
		if (c_pWorldMatrix && memcmp(&st.lastWorldMat, c_pWorldMatrix, sizeof(D3DXMATRIX)) != 0)
		{
			st.lastWorldMat = *c_pWorldMatrix;
			UpdateWorldMatrices(c_pWorldMatrix);
		}
		return;
	}
	if (st.warmupCount < 10) ++st.warmupCount;
	st.lastUpdateFrame = effectiveFrame;
	if (c_pWorldMatrix) st.lastWorldMat = *c_pWorldMatrix;

	if (!m_ppkSkeletonInst || *m_ppkSkeletonInst == this)
	{
		if (st.lastWorldPoseFrame != effectiveFrame || st.warmupCount <= 4)
		{
			if (m_pgrnModelInstance)
				GrannySetModelClock(m_pgrnModelInstance, GetLocalTime());
			UpdateWorldPose();
			st.lastWorldPoseFrame = effectiveFrame;
		}
	}
	UpdateWorldMatrices(c_pWorldMatrix);
#else
	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);
#endif

	if (m_pModel->CanDeformPNTVertices())
	{
		// WORK
		CGraphicVertexBuffer& rkDeformableVertexBuffer = __GetDeformableVertexBufferRef();
		TPNTVertex* pntVertices;
		if (rkDeformableVertexBuffer.LockRange(m_pModel->GetDeformVertexCount(), (void **)&pntVertices))
		{
			DeformPNTVertices(pntVertices);
			rkDeformableVertexBuffer.Unlock();
		}
		else
		{
			TraceError("GRANNY DEFORM DYNAMIC BUFFER LOCK ERROR");
		}
		// END_OF_WORK
	}
}

//////////////////////////////////////////////////////
class CGrannyLocalPose
{
	public:
		CGrannyLocalPose()
		{
			m_pgrnLocalPose = NULL;
			m_boneCount = 0;
		}

		virtual ~CGrannyLocalPose()
		{
			if (m_pgrnLocalPose)
				GrannyFreeLocalPose(m_pgrnLocalPose);
		}

		granny_local_pose * Get(int boneCount)
		{
			if (m_pgrnLocalPose)
			{
				if (m_boneCount >= boneCount)
					return m_pgrnLocalPose;

				GrannyFreeLocalPose(m_pgrnLocalPose);
			}

			m_boneCount = boneCount;
			m_pgrnLocalPose = GrannyNewLocalPose(m_boneCount);
			return m_pgrnLocalPose;
		}

	private:
		granny_local_pose *	m_pgrnLocalPose;
		int					m_boneCount;
};
//////////////////////////////////////////////////////

void CGrannyModelInstance::UpdateSkeleton(const D3DXMATRIX * c_pWorldMatrix, float /*fLocalTime*/)
{
	UpdateWorldPose();
	UpdateWorldMatrices(c_pWorldMatrix);
}

void CGrannyModelInstance::UpdateWorldPose()
{
	if (m_ppkSkeletonInst)
		if (*m_ppkSkeletonInst!=this)
			return;

	static CGrannyLocalPose s_SharedLocalPose;

	granny_skeleton * pgrnSkeleton = GrannyGetSourceSkeleton(m_pgrnModelInstance);
	granny_local_pose * pgrnLocalPose = s_SharedLocalPose.Get(pgrnSkeleton->BoneCount);

	const float * pAttachBoneMatrix = (mc_pParentInstance) ? mc_pParentInstance->GetBoneMatrixPointer(m_iParentBoneIndex) : NULL;

	GrannySampleModelAnimationsAccelerated(m_pgrnModelInstance, pgrnSkeleton->BoneCount, pAttachBoneMatrix, pgrnLocalPose, __GetWorldPosePtr());
}

void CGrannyModelInstance::UpdateWorldMatrices(const D3DXMATRIX* c_pWorldMatrix)
{
	if (!m_meshMatrices)
		return;

	assert(m_pModel != NULL);
	assert(ms_lpd3dMatStack != NULL);

	int meshCount = m_pModel->GetMeshCount();

	granny_matrix_4x4 * pgrnMatCompositeBuffer = GrannyGetWorldPoseComposite4x4Array(__GetWorldPosePtr());
	D3DXMATRIX * boneMatrices = (D3DXMATRIX *) pgrnMatCompositeBuffer;

	for (int i = 0; i < meshCount; ++i)
	{
		D3DXMATRIX & rWorldMatrix = m_meshMatrices[i];

		const CGrannyMesh * pMesh = m_pModel->GetMeshPointer(i);

#if GrannyProductMinorVersion==4
		int * boneIndices = __GetMeshBoneIndices(i);
#elif GrannyProductMinorVersion==11 || GrannyProductMinorVersion==9 || GrannyProductMinorVersion==8 || GrannyProductMinorVersion==7
		const granny_int32x * boneIndices = __GetMeshBoneIndices(i);
#else
#error "unknown granny version"
#endif

		if (pMesh->CanDeformPNTVertices())
		{
			rWorldMatrix = *c_pWorldMatrix;
		}
		else
		{
#if GrannyProductMinorVersion==4
			int iBone = *boneIndices;
#elif GrannyProductMinorVersion==11 || GrannyProductMinorVersion==9 || GrannyProductMinorVersion==8 || GrannyProductMinorVersion==7
			const granny_int32x iBone = *boneIndices;
#else
#error "unknown granny version"
#endif
			D3DXMatrixMultiply(&rWorldMatrix, &boneMatrices[iBone], c_pWorldMatrix);
		}
	}

#ifdef _TEST
	TEST_matWorld = *c_pWorldMatrix;
#endif
}

void CGrannyModelInstance::DeformPNTVertices(void * pvDest)
{
	assert(m_pModel != NULL);
	assert(m_pModel->CanDeformPNTVertices());

	m_pModel->DeformPNTVertices(pvDest, (D3DXMATRIX *) GrannyGetWorldPoseComposite4x4Array(__GetWorldPosePtr()), m_vct_pgrnMeshBinding);
}