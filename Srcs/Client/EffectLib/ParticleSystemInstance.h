#pragma once

#include "EffectElementBaseInstance.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "../eterlib/GrpScreen.h"
#include "../eterlib/StateManager.h"
#include "../eterLib/GrpImageInstance.h"
#include "EmitterProperty.h"

class CParticleSystemInstance : public CEffectElementBaseInstance
{
	public:
		static void DestroySystem();

		static CParticleSystemInstance* New();
		static void Delete(CParticleSystemInstance* pkData);

		static CDynamicPool<CParticleSystemInstance>	ms_kPool;

	public:
#ifdef __ANDROID__
		struct SAndroidBatchParticleVertex
		{
			D3DXVECTOR3 pos;
			DWORD color;
			TTextureCoordinate tex;
		};

		static inline void AppendQuadToAndroidBatch(std::vector<SAndroidBatchParticleVertex>& rBatch, const TPTVertex* m, DWORD color)
		{
			const size_t base = rBatch.size();
			rBatch.resize(base + 6);
			SAndroidBatchParticleVertex* dst = &rBatch[base];
			dst[0] = { m[0].position, color, m[0].texCoord };
			dst[1] = { m[1].position, color, m[1].texCoord };
			dst[2] = { m[2].position, color, m[2].texCoord };
			dst[3] = { m[2].position, color, m[2].texCoord };
			dst[4] = { m[1].position, color, m[1].texCoord };
			dst[5] = { m[3].position, color, m[3].texCoord };
		}
#endif

		template <typename T>
		inline void ForEachParticleRendering(T && FunObj)
		{
			DWORD dwFrameIndex;
#ifdef __ANDROID__
			const D3DXMATRIX* pLocalMat = m_pParticleProperty->m_bAttachFlag ? mc_pmatLocal : NULL;
			const BYTE byBillboard = m_pParticleProperty->m_byBillboardType;
			static std::vector<SAndroidBatchParticleVertex> s_batchVtx;

			STATEMANAGER.SaveRenderState(D3DRS_LIGHTING, FALSE);
			STATEMANAGER.SaveRenderState(D3DRS_COLORVERTEX, TRUE);
			STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SaveTextureStageState(0, D3DTSS_COLORARG2, D3DTA_TEXTURE);
			STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
			STATEMANAGER.SaveTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_TEXTURE);
#ifdef DIRECTX9
			STATEMANAGER.SaveFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#else
			STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1);
#endif

			for (dwFrameIndex = 0; dwFrameIndex < m_kVct_pkImgInst.size(); ++dwFrameIndex)
			{
				TParticleInstanceList& rList = m_ParticleInstanceListVector[dwFrameIndex];
				if (rList.empty())
					continue;

				STATEMANAGER.SetTexture(0, m_kVct_pkImgInst[dwFrameIndex]->GetTextureReference().GetD3DTexture());
				s_batchVtx.clear();
				if (s_batchVtx.capacity() < rList.size() * 6)
					s_batchVtx.reserve(rList.size() * 6);

				for (TParticleInstanceList::iterator itor = rList.begin(); itor != rList.end(); ++itor)
				{
					CParticleInstance* pInst = *itor;
					if (byBillboard < BILLBOARD_TYPE_2FACE)
					{
						pInst->Transform(pLocalMat);
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), STATEMANAGER.GetRenderState(D3DRS_TEXTUREFACTOR));
					}
					else if (byBillboard == BILLBOARD_TYPE_2FACE)
					{
						pInst->Transform(pLocalMat, D3DXToRadian(-30.0f));
						DWORD col = STATEMANAGER.GetRenderState(D3DRS_TEXTUREFACTOR);
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), col);
						pInst->Transform(pLocalMat, D3DXToRadian(+30.0f));
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), col);
					}
					else
					{
						pInst->Transform(pLocalMat);
						DWORD col = STATEMANAGER.GetRenderState(D3DRS_TEXTUREFACTOR);
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), col);
						pInst->Transform(pLocalMat, D3DXToRadian(-60.0f));
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), col);
						pInst->Transform(pLocalMat, D3DXToRadian(+60.0f));
						AppendQuadToAndroidBatch(s_batchVtx, pInst->GetParticleMeshPointer(), col);
					}
				}

				if (!s_batchVtx.empty())
				{
					STATEMANAGER.DrawPrimitiveUP(D3DPT_TRIANGLELIST, (UINT)(s_batchVtx.size() / 3), s_batchVtx.data(), sizeof(SAndroidBatchParticleVertex));
				}
			}

#ifdef DIRECTX9
			STATEMANAGER.RestoreFVF();
#else
			STATEMANAGER.SetVertexShader(D3DFVF_XYZ | D3DFVF_TEX1);
#endif
			STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG2);
			STATEMANAGER.RestoreTextureStageState(0, D3DTSS_ALPHAARG1);
			STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG2);
			STATEMANAGER.RestoreTextureStageState(0, D3DTSS_COLORARG1);
			STATEMANAGER.RestoreRenderState(D3DRS_COLORVERTEX);
			STATEMANAGER.RestoreRenderState(D3DRS_LIGHTING);
			return;
#else
			for(dwFrameIndex=0; dwFrameIndex<m_kVct_pkImgInst.size(); dwFrameIndex++)
			{
				STATEMANAGER.SetTexture(0, m_kVct_pkImgInst[dwFrameIndex]->GetTextureReference().GetD3DTexture());
				TParticleInstanceList::iterator itor = m_ParticleInstanceListVector[dwFrameIndex].begin();
				for (; itor != m_ParticleInstanceListVector[dwFrameIndex].end(); ++itor)
				{
					if (!InFrustum(*itor) && !m_ignoreFrustum)
						return;
					FunObj(*itor);
				}
			}
#endif
		}

		CParticleSystemInstance();
		virtual ~CParticleSystemInstance();

		void OnSetDataPointer(CEffectElementBase * pElement);

		void CreateParticles(float fElapsedTime);

		inline bool InFrustum(CParticleInstance * pInstance)
		{
			if (m_pParticleProperty->m_bAttachFlag)
				return CScreen::GetFrustum().ViewVolumeTestEffects(Vector3d(
					pInstance->m_v3Position.x + mc_pmatLocal->_41,
					pInstance->m_v3Position.y + mc_pmatLocal->_42,
					pInstance->m_v3Position.z + mc_pmatLocal->_43
					),pInstance->GetRadiusApproximation())!=VS_OUTSIDE;
			else
				return CScreen::GetFrustum().ViewVolumeTestEffects(Vector3d(pInstance->m_v3Position.x,pInstance->m_v3Position.y,pInstance->m_v3Position.z),pInstance->GetRadiusApproximation())!=VS_OUTSIDE;
		}

		DWORD GetEmissionCount();

	protected:
		void OnInitialize();
		void OnDestroy();

		bool OnUpdate(float fElapsedTime);
		void OnRender();

	protected:
		float m_fEmissionResidue;

		DWORD m_dwCurrentEmissionCount;
		int	m_iLoopCount;

		typedef std::list<CParticleInstance*> TParticleInstanceList;
		typedef std::vector<TParticleInstanceList> TParticleInstanceListVector;
		TParticleInstanceListVector m_ParticleInstanceListVector;

		typedef std::vector<CGraphicImageInstance*> TImageInstanceVector;
		TImageInstanceVector m_kVct_pkImgInst;

		CParticleSystemData * m_pData;

		CParticleProperty * m_pParticleProperty;
		CEmitterProperty * m_pEmitterProperty;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
