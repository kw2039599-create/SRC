#include "StdAfx.h"
#include "ParticleInstance.h"
#include "ParticleProperty.h"

#include "../eterBase/Random.h"
#include "../eterLib/Camera.h"
#include "../eterLib/StateManager.h"

CDynamicPool<CParticleInstance> CParticleInstance::ms_kPool;

using namespace NEffectUpdateDecorator;

void CParticleInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CParticleInstance* CParticleInstance::New()
{
	return ms_kPool.Alloc();
}

void CParticleInstance::DeleteThis()
{
	Destroy();

	ms_kPool.Free(this);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//CDynamicPool<CRayParticleInstance> CRayParticleInstance::ms_kPool;

/*void CRayParticleInstance::DestroySystem()
{
	ms_kPool.Destroy();
}

CRayParticleInstance* CRayParticleInstance::New()
{
	return ms_kPool.Alloc();
}

void CRayParticleInstance::DeleteThis()
{
#ifdef RAY_TO_AFTERIMAGE
	m_PositionList.clear();
#else
	m_bStart = false;
#endif
	ms_kPool.Free(this);
}
*/
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

float CParticleInstance::GetRadiusApproximation()
{
	return m_v2HalfSize.y*m_v2Scale.y + m_v2HalfSize.x*m_v2Scale.x;
}
/*
D3DXVECTOR3 CBaseParticleInstance::GetCenterApproximation()
{
	return D3DXVECTOR3(
		m_v3Position.x + mc_pmatLocal._41,
		m_v3Position.y + mc_pmatLocal._42,
		m_v3Position.z + mc_pmatLocal._43
		);
}*/

BOOL CParticleInstance::Update(float fElapsedTime, float fAngle)
{
	m_fLastLifeTime -= fElapsedTime;
	if (m_fLastLifeTime < 0.0f)
		return FALSE;

	float fLifePercentage = (m_fLifeTime - m_fLastLifeTime) / m_fLifeTime;

	m_pDecorator->Excute(CDecoratorData(fLifePercentage,fElapsedTime,this));

	m_v3LastPosition = m_v3Position;
	m_v3Position += m_v3Velocity * fElapsedTime;

	if (fAngle)
	{
		if (m_pParticleProperty->m_bAttachFlag)
		{
			float fCos, fSin;
			fAngle = D3DXToRadian(fAngle);
			fCos = cos(fAngle);
			fSin = sin(fAngle);

			float rx = m_v3Position.x - m_v3StartPosition.x;
			float ry = m_v3Position.y - m_v3StartPosition.y;

			m_v3Position.x =   fCos * rx + fSin * ry + m_v3StartPosition.x;
			m_v3Position.y = - fSin * rx + fCos * ry + m_v3StartPosition.y;
		}
		else
		{
			D3DXQUATERNION q,qc;
			D3DXQuaternionRotationAxis(&q,&m_pParticleProperty->m_v3ZAxis,D3DXToRadian(fAngle));
			D3DXQuaternionConjugate(&qc,&q);

			D3DXQUATERNION qr(
				m_v3Position.x-m_v3StartPosition.x,
				m_v3Position.y-m_v3StartPosition.y,
				m_v3Position.z-m_v3StartPosition.z,
				0.0f);
			D3DXQuaternionMultiply(&qr,&q,&qr);
			D3DXQuaternionMultiply(&qr,&qr,&qc);

			m_v3Position.x = qr.x;
			m_v3Position.y = qr.y;
			m_v3Position.z = qr.z;

			m_v3Position += m_v3StartPosition;
		}
	}

	return TRUE;
}

void CParticleInstance::Transform(const D3DXMATRIX * c_matLocal)
{
#ifdef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);
#else
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_dcColor);
#endif

	D3DXVECTOR3 v3Up;
	D3DXVECTOR3 v3Cross;

	CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!m_pParticleProperty->m_bStretchFlag)
	{
		const D3DXVECTOR3 & c_rv3Up = pCurrentCamera->GetUp();
		const D3DXVECTOR3 & c_rv3Cross = pCurrentCamera->GetCross();

		switch (m_pParticleProperty->m_byBillboardType)
		{
		case BILLBOARD_TYPE_LIE:
			{
				const float fRad = D3DXToRadian(m_fRotation);
				const float fCos = cosf(fRad);
				const float fSin = sinf(fRad);
				v3Up.x = fCos;
				v3Up.y = -fSin;
				v3Up.z = 0.0f;
				v3Cross.x = fSin;
				v3Cross.y = fCos;
				v3Cross.z = 0.0f;
			}
			break;
		case BILLBOARD_TYPE_2FACE:
		case BILLBOARD_TYPE_3FACE:
		case BILLBOARD_TYPE_Y:
			{
				v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				const D3DXVECTOR3 & c_rv3View = pCurrentCamera->GetView();
				static D3DXVECTOR3 s_lastView(0.0f, 0.0f, 0.0f);
				static D3DXVECTOR3 s_cachedCross(1.0f, 0.0f, 0.0f);
				if (c_rv3View.x != s_lastView.x || c_rv3View.y != s_lastView.y)
				{
					s_lastView = c_rv3View;
					D3DXVECTOR3 val(c_rv3View.x, c_rv3View.y, 0.0f);
					D3DXVec3Cross(&s_cachedCross, &v3Up, &val);
					D3DXVec3Normalize(&s_cachedCross, &s_cachedCross);
				}
				v3Cross = s_cachedCross;

				if (m_fRotation)
				{
					const float fRad = D3DXToRadian(m_fRotation);
					const float fCos = -sinf(fRad);
					const float fSin = cosf(fRad);

					const D3DXVECTOR3 v3Temp = v3Up * fCos - v3Cross * fSin;
					v3Cross = v3Cross * fCos + v3Up * fSin;
					v3Up = v3Temp;
				}
			}
			break;
		case BILLBOARD_TYPE_ALL:
		default:
			{
				if (m_fRotation == 0.0f)
				{
					v3Up = -c_rv3Cross;
					v3Cross = c_rv3Up;
				}
				else
				{
					// Fast 2D orthonormal plane rotation around c_rv3View (replaces 4 quaternion multiplies)
					const float fRad = D3DXToRadian(m_fRotation);
					const float fCos = cosf(fRad);
					const float fSin = sinf(fRad);
					v3Up = -c_rv3Cross * fCos + c_rv3Up * fSin;
					v3Cross = c_rv3Up * fCos + c_rv3Cross * fSin;
				}
			}
			break;
		}
	}
	else
	{
		v3Up = m_v3Position - m_v3LastPosition;

		if (c_matLocal)
		{
			D3DXVec3TransformNormal(&v3Up, &v3Up, c_matLocal);
		}

		const float length = D3DXVec3Length(&v3Up);
		if (length == 0.0f)
			v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		else
			v3Up *= (1.0f + logf(1.0f + length)) / length;

		const D3DXVECTOR3 & c_rv3View = pCurrentCamera->GetView();
		D3DXVec3Cross(&v3Cross, &v3Up, &c_rv3View);
		D3DXVec3Normalize(&v3Cross, &v3Cross);
	}

	v3Cross = -(m_v2HalfSize.x * m_v2Scale.x) * v3Cross;
	v3Up = (m_v2HalfSize.y * m_v2Scale.y) * v3Up;

	if (c_matLocal && m_pParticleProperty->m_bAttachFlag)
	{
		D3DXVECTOR3 v3Position;
		D3DXVec3TransformCoord(&v3Position, &m_v3Position, c_matLocal);
		m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
	}
	else
	{
		m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
	}
}

void CParticleInstance::Transform(const D3DXMATRIX * c_matLocal, const float c_fZRotation)
{
#ifdef WORLD_EDITOR
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, m_Color);
#else
	STATEMANAGER.SetRenderState(D3DRS_TEXTUREFACTOR, (DWORD)m_dcColor);
#endif

	D3DXVECTOR3 v3Up;
	D3DXVECTOR3 v3Cross;

	CCamera * pCurrentCamera = CCameraManager::Instance().GetCurrentCamera();

	if (!m_pParticleProperty->m_bStretchFlag)
	{
		const D3DXVECTOR3 & c_rv3Up = pCurrentCamera->GetUp();
		const D3DXVECTOR3 & c_rv3Cross = pCurrentCamera->GetCross();

		switch (m_pParticleProperty->m_byBillboardType)
		{
		case BILLBOARD_TYPE_LIE:
			{
				const float fRad = D3DXToRadian(m_fRotation);
				const float fCos = cosf(fRad);
				const float fSin = sinf(fRad);
				v3Up.x = fCos;
				v3Up.y = -fSin;
				v3Up.z = 0.0f;
				v3Cross.x = fSin;
				v3Cross.y = fCos;
				v3Cross.z = 0.0f;
			}
			break;
		case BILLBOARD_TYPE_2FACE:
		case BILLBOARD_TYPE_3FACE:
		case BILLBOARD_TYPE_Y:
			{
				v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
				const D3DXVECTOR3 & c_rv3View = pCurrentCamera->GetView();
				static D3DXVECTOR3 s_lastView(0.0f, 0.0f, 0.0f);
				static D3DXVECTOR3 s_cachedCross(1.0f, 0.0f, 0.0f);
				if (c_rv3View.x != s_lastView.x || c_rv3View.y != s_lastView.y)
				{
					s_lastView = c_rv3View;
					D3DXVECTOR3 val(c_rv3View.x, c_rv3View.y, 0.0f);
					D3DXVec3Cross(&s_cachedCross, &v3Up, &val);
					D3DXVec3Normalize(&s_cachedCross, &s_cachedCross);
				}
				v3Cross = s_cachedCross;

				if (m_fRotation)
				{
					const float fRad = D3DXToRadian(m_fRotation);
					const float fCos = -sinf(fRad);
					const float fSin = cosf(fRad);

					const D3DXVECTOR3 v3Temp = v3Up * fCos - v3Cross * fSin;
					v3Cross = v3Cross * fCos + v3Up * fSin;
					v3Up = v3Temp;
				}
			}
			break;
		case BILLBOARD_TYPE_ALL:
		default:
			{
				if (m_fRotation == 0.0f)
				{
					v3Up = -c_rv3Cross;
					v3Cross = c_rv3Up;
				}
				else
				{
					const float fRad = D3DXToRadian(m_fRotation);
					const float fCos = cosf(fRad);
					const float fSin = sinf(fRad);
					v3Up = -c_rv3Cross * fCos + c_rv3Up * fSin;
					v3Cross = c_rv3Up * fCos + c_rv3Cross * fSin;
				}
			}
			break;
		}
	}
	else
	{
		v3Up = m_v3Position - m_v3LastPosition;

		if (c_matLocal)
		{
			D3DXVec3TransformNormal(&v3Up, &v3Up, c_matLocal);
		}

		const float length = D3DXVec3Length(&v3Up);
		if (length == 0.0f)
			v3Up = D3DXVECTOR3(0.0f, 0.0f, 1.0f);
		else
			v3Up *= (1.0f + logf(1.0f + length)) / length;

		const D3DXVECTOR3 & c_rv3View = pCurrentCamera->GetView();
		D3DXVec3Cross(&v3Cross, &v3Up, &c_rv3View);
		D3DXVec3Normalize(&v3Cross, &v3Cross);
	}

	if (c_fZRotation)
	{
		const float fCos = cosf(c_fZRotation);
		const float fSin = sinf(c_fZRotation);

		float x = v3Up.x;
		float y = v3Up.y;
		v3Up.x = x * fCos - y * fSin;
		v3Up.y = y * fCos + x * fSin;

		x = v3Cross.x;
		y = v3Cross.y;
		v3Cross.x = x * fCos - y * fSin;
		v3Cross.y = y * fCos + x * fSin;
	}

	v3Cross = -(m_v2HalfSize.x * m_v2Scale.x) * v3Cross;
	v3Up = (m_v2HalfSize.y * m_v2Scale.y) * v3Up;

	if (c_matLocal && m_pParticleProperty->m_bAttachFlag)
	{
		D3DXVECTOR3 v3Position;
		D3DXVec3TransformCoord(&v3Position, &m_v3Position, c_matLocal);
		m_ParticleMesh[0].position = v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = v3Position + v3Up - v3Cross;
	}
	else
	{
		m_ParticleMesh[0].position = m_v3Position - v3Up + v3Cross;
		m_ParticleMesh[1].position = m_v3Position - v3Up - v3Cross;
		m_ParticleMesh[2].position = m_v3Position + v3Up + v3Cross;
		m_ParticleMesh[3].position = m_v3Position + v3Up - v3Cross;
	}
}

void CParticleInstance::Destroy()
{
	if (m_pDecorator)
		m_pDecorator->DeleteThis();

	__Initialize();
}

void CParticleInstance::__Initialize()
{
	//*
	m_pDecorator=NULL;

	m_v3Position = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	m_v3LastPosition = m_v3Position;
	m_v3Velocity = D3DXVECTOR3(0.0f, 0.0f, 0.0f);

	m_v2Scale = D3DXVECTOR2(1.0f, 1.0f);
#ifdef WORLD_EDITOR
	m_Color = D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f);
#else
	m_dcColor.m_dwColor = 0xffffffff;
#endif

	m_byFrameIndex = 0;
	m_ParticleMesh[0].texCoord = D3DXVECTOR2(0.0f, 1.0f);
	m_ParticleMesh[1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	m_ParticleMesh[2].texCoord = D3DXVECTOR2(1.0f, 1.0f);
	m_ParticleMesh[3].texCoord = D3DXVECTOR2(1.0f, 0.0f);
}

CParticleInstance::CParticleInstance()
{
	__Initialize();
}

CParticleInstance::~CParticleInstance()
{
	Destroy();
}

/*CRayParticleInstance::CRayParticleInstance()
{
#ifdef RAY_TO_AFTERIMAGE
	int i;
	for(i=0;i<RAY_VERTEX_COUNT*2;i+=2)
	{
		m_ParticleMesh[i].texCoord = D3DXVECTOR2(i+0.0f, 1.0f);
		m_ParticleMesh[i+1].texCoord = D3DXVECTOR2(i+1.0f, 0.0f);
	}
#else
	m_ParticleMesh[0].texCoord = D3DXVECTOR2(0.0f, 1.0f);
	m_ParticleMesh[1].texCoord = D3DXVECTOR2(0.0f, 0.0f);
	m_ParticleMesh[2].texCoord = D3DXVECTOR2(0.5f, 1.0f);
	m_ParticleMesh[3].texCoord = D3DXVECTOR2(0.5f, 0.0f);
	m_ParticleMesh[4].texCoord = D3DXVECTOR2(1.0f, 1.0f);
	m_ParticleMesh[5].texCoord = D3DXVECTOR2(1.0f, 0.0f);

	m_bStart = false;
#endif
}

CRayParticleInstance::~CRayParticleInstance()
{
}*/

TPTVertex * CParticleInstance::GetParticleMeshPointer()
{
	return m_ParticleMesh;
}

/*TPTVertex * CRayParticleInstance::GetParticleMeshPointer()
{
	return m_ParticleMesh;
}

void CRayParticleInstance::Transform(const D3DXMATRIX * c_matLocal, const float c_fZRotation)
{
	assert(false && "NOT_REACHED");
}*/
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
