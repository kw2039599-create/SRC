#pragma once

#include "MapUtil.h"
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#endif

class IPhysicsWorld
{
public:
	IPhysicsWorld()
	{
		assert(ms_pWorld == NULL);
		ms_pWorld = this;
	}

	virtual ~IPhysicsWorld()
	{
		if (this == ms_pWorld)
			ms_pWorld = NULL;
	}

	static IPhysicsWorld* GetPhysicsWorld()
	{
		return ms_pWorld;
	}

	virtual bool isPhysicalCollision(const D3DXVECTOR3 & c_rvCheckPosition) = 0;

private:
	static IPhysicsWorld* ms_pWorld;
};

class CActorInstance;
class IObjectManager
{
public:
	IObjectManager()
	{
		assert(ms_ObjManager == NULL);
		ms_ObjManager = this;
	}

	virtual ~IObjectManager()
	{
		if (this == ms_ObjManager)
			ms_ObjManager = NULL;
	}

	static IObjectManager* GetObjectManager()
	{
		return ms_ObjManager;
	}

	virtual void AdjustCollisionWithOtherObjects(CActorInstance* pInst ) = 0;

private:
	static IObjectManager* ms_ObjManager;
};

class CPhysicsObject
{
	public:
		CPhysicsObject();
		virtual ~CPhysicsObject();

		void Initialize();

		void Update(float fElapsedTime);

		bool isBlending();

		void SetDirection(const D3DXVECTOR3 & c_rv3Direction);
		void IncreaseExternalForce(const D3DXVECTOR3 & c_rvBasePosition, float fForce);
		void SetLastPosition(const TPixelPosition & c_rPosition, float fBlendingTime);
		void GetLastPosition(TPixelPosition * pPosition);

		float GetXMovement();
		float GetYMovement();
		void ClearSavingMovement();

#ifdef ENABLE_CSHIELD
		void SetActorInstance( CActorInstance* pInst ) { m_pActorInstance.Set(pInst); }
		CActorInstance* GetActorInstance() { return m_pActorInstance.Get(); }
#else
		void SetActorInstance(CActorInstance* pInst) { m_pActorInstance = pInst; }
		CActorInstance* GetActorInstance() { return m_pActorInstance; }
#endif
	protected:
		void Accumulate(D3DXVECTOR3 * pv3Position);

	protected:
		float m_fMass;
		float m_fFriction;
		D3DXVECTOR3 m_v3Direction;
		D3DXVECTOR3 m_v3Acceleration;
		D3DXVECTOR3 m_v3Velocity;

		D3DXVECTOR3 m_v3LastPosition;
		CEaseOutInterpolation m_xPushingPosition;
		CEaseOutInterpolation m_yPushingPosition;

#ifdef ENABLE_CSHIELD
		EncryptedPtr<CActorInstance> m_pActorInstance;
#else
		CActorInstance* m_pActorInstance;
#endif
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
