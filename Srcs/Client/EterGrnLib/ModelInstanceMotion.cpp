#include "StdAfx.h"
#include "ModelInstance.h"
#include "Model.h"

void CGrannyModelInstance::CopyMotion(CGrannyModelInstance * pModelInstance, bool bIsFreeSourceControl)
{
	if (!pModelInstance || !pModelInstance->m_pgrnAni || !pModelInstance->m_pgrnCtrl)
		return;

	if (m_pgrnCtrl)
		GrannyFreeControl(m_pgrnCtrl);

	SetLocalTime(pModelInstance->GetLocalTime());
	float localTime = GetLocalTime();
	m_pgrnAni = pModelInstance->m_pgrnAni;
	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, m_pgrnAni, m_pgrnModelInstance);

	if (!m_pgrnCtrl)
		return;

	GrannySetControlSpeed(m_pgrnCtrl, GrannyGetControlSpeed(pModelInstance->m_pgrnCtrl));
	GrannySetControlLoopCount(m_pgrnCtrl, GrannyGetControlLoopCount(pModelInstance->m_pgrnCtrl));

	GrannySetControlEaseIn(m_pgrnCtrl, true);
	GrannySetControlEaseOut(m_pgrnCtrl, false);

	GrannySetControlRawLocalClock(m_pgrnCtrl, GrannyGetControlRawLocalClock(pModelInstance->m_pgrnCtrl));

	GrannyFreeControlOnceUnused(m_pgrnCtrl);
	m_dwOldUpdateFrame = 0;

	if (bIsFreeSourceControl)
	{
		GrannyFreeControl(pModelInstance->m_pgrnCtrl);
		pModelInstance->m_pgrnCtrl = NULL;
	}
}

bool CGrannyModelInstance::IsMotionPlaying()
{
	if (!m_pgrnCtrl)
		return false;

	if (GrannyControlIsComplete(m_pgrnCtrl))
		return false;

	return true;
}

void CGrannyModelInstance::SetMotionPointer(const CGrannyMotion * pMotion, float blendTime, int loopCount, float speedRatio)
{
	if (!m_pgrnWorldPoseReal)
		return;

	granny_model_instance * pgrnModelInstance = m_pgrnModelInstance;
	if (!pgrnModelInstance)
		return;

	float localTime = GetLocalTime();

	bool isFirst=false;
	if (m_pgrnCtrl)
	{
#ifdef __ANDROID__
		if (localTime < 0.05f || blendTime <= 0.0f)
		{
			GrannyFreeControl(m_pgrnCtrl);
			m_pgrnCtrl = NULL;
			isFirst = true;
		}
		else
#endif
		{
			GrannySetControlEaseOutCurve(m_pgrnCtrl, localTime, localTime + blendTime, 1.0f, 1.0f, 0.0f, 0.0f);

			GrannySetControlEaseIn(m_pgrnCtrl, false);
			GrannySetControlEaseOut(m_pgrnCtrl, true);

			GrannyCompleteControlAt(m_pgrnCtrl, localTime + blendTime);
			GrannyFreeControlIfComplete(m_pgrnCtrl);
		}
	}
	else
	{
		isFirst=true;
	}

	m_pgrnAni = pMotion->GetGrannyAnimationPointer();
	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, m_pgrnAni, pgrnModelInstance);
	if (!m_pgrnCtrl)
		return;

#ifdef __ANDROID__
	static int s_iMotLog = 0;
	if (++s_iMotLog <= 20)
	{
		Tracenf("[GrnMotion] name='%s' dur=%.3f localTime=%.3f blend=%.3f loop=%d speed=%.2f",
			pMotion->GetName() ? pMotion->GetName() : "?", pMotion->GetDuration(), localTime, blendTime, loopCount, speedRatio);
	}
#endif

	GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
	GrannySetControlLoopCount(m_pgrnCtrl, loopCount);

	if (isFirst)
	{
		GrannySetControlEaseIn(m_pgrnCtrl, false);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
	}
	else
	{
		GrannySetControlEaseIn(m_pgrnCtrl, true);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
		if (blendTime > 0.0f)
			GrannySetControlEaseInCurve(m_pgrnCtrl, localTime, localTime + blendTime, 0.0f, 0.0f, 1.0f, 1.0f);
	}

	GrannyFreeControlOnceUnused(m_pgrnCtrl);
	m_dwOldUpdateFrame = 0;
}

void CGrannyModelInstance::ChangeMotionPointer(const CGrannyMotion* pMotion, int loopCount, float speedRatio)
{
	granny_model_instance * pgrnModelInstance = m_pgrnModelInstance;
	if (!pgrnModelInstance)
		return;

	float fSkipTime = 0.3f;
	float localTime = GetLocalTime() - fSkipTime;

	if (m_pgrnCtrl)
	{
		GrannySetControlEaseIn(m_pgrnCtrl, false);
		GrannySetControlEaseOut(m_pgrnCtrl, false);
		GrannyCompleteControlAt(m_pgrnCtrl, localTime);
		GrannyFreeControlIfComplete(m_pgrnCtrl);
	}

	m_pgrnAni = pMotion->GetGrannyAnimationPointer();
	m_pgrnCtrl = GrannyPlayControlledAnimation(localTime, m_pgrnAni, pgrnModelInstance);
	if (!m_pgrnCtrl)
		return;

	GrannySetControlSpeed(m_pgrnCtrl, speedRatio);
	GrannySetControlLoopCount(m_pgrnCtrl, loopCount);
	GrannySetControlEaseIn(m_pgrnCtrl, false);
	GrannySetControlEaseOut(m_pgrnCtrl, false);

	GrannyFreeControlOnceUnused(m_pgrnCtrl);
	m_dwOldUpdateFrame = 0;
}

void CGrannyModelInstance::SetMotionAtEnd()
{
	if (!m_pgrnCtrl)
		return;

	float endingTime = GrannyGetControlLocalDuration(m_pgrnCtrl);
	GrannySetControlRawLocalClock(m_pgrnCtrl, endingTime);
	m_dwOldUpdateFrame = 0;
}