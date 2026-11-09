#pragma once

class IFlyEventHandler
{
public:
	IFlyEventHandler() {}
	virtual ~IFlyEventHandler() {}

	// Call by ActorInstance
	virtual void OnSetFlyTarget() {}
	virtual void OnShoot(DWORD dwSkillIndex) {}

	virtual void OnNoTarget() {}
	virtual void OnNoArrow() {}

	// Call by FlyingInstance
	virtual void OnExplodingOutOfRange() {}
	virtual void OnExplodingAtBackground() {}
	virtual void OnExplodingAtAnotherTarget(DWORD dwSkillIndex, DWORD dwVID) {}
	virtual void OnExplodingAtTarget(DWORD dwSkillIndex) {}
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
