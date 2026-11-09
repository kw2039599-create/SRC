#include "StdAfx.h"
#include "PythonApplication.h"
#include "CRenderTargetManager.h"
#include "CRenderTarget.h"

PyObject* renderTargetCreateRenderTarget(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int width, height;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &width)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 2, &height)) return Py_BadArgument();

	CRenderTargetManager::Instance().CreateRenderTarget(index, width, height);
	return Py_BuildNone();
}

PyObject* renderTargetSetArmor(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetArmor(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSelectModel(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SelectModel(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSetAcce(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetAcce(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSetWeapon(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetWeapon(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSetHair(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->ChangeHair(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSetVisibility(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	bool isShow = false;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetBoolean(poArgs, 1, &isShow)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetVisibility(isShow);
	return Py_BuildNone();
}

PyObject* renderTargetSetBackground(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	char* szPathName;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetString(poArgs, 1, &szPathName)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->CreateBackground(szPathName, CPythonApplication::Instance().GetWidth(), CPythonApplication::Instance().GetHeight());
	return Py_BuildNone();
}

PyObject* renderTargetRender(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int x, y, w, h;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &x)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 2, &y)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 3, &w)) return Py_BuildException();
	if (!PyTuple_GetInteger(poArgs, 4, &h)) return Py_BuildException();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT)
	{
		RECT rect = { x, y, x + w, y + h };
		pRT->SetRenderingRect(&rect);
		pRT->RenderTexture();
	}
	return Py_BuildNone();
}

PyObject* renderTargetSetMount(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int modelIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &modelIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetMount(modelIndex);
	return Py_BuildNone();
}

PyObject* renderTargetSetRotation(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	bool bAuto = false;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetBoolean(poArgs, 1, &bAuto)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->SetAutoRotation(bAuto);
	return Py_BuildNone();
}

PyObject* renderTargetRotateEyeAroundTarget(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	float fPitch = 0.0f;
	float fYaw = 0.0f;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetFloat(poArgs, 1, &fPitch)) return Py_BadArgument();
	if (!PyTuple_GetFloat(poArgs, 2, &fYaw)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->RotateEyeAroundTarget(fPitch, fYaw);
	return Py_BuildNone();
}

PyObject* renderTargetZoom(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int iRatio = 0;
	
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &iRatio)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) pRT->Zoom(static_cast<float>(iRatio));
	
	return Py_BuildNone();
}

PyObject* renderTargetSetMotionIndex(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	int iMotionIndex = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();
	if (!PyTuple_GetInteger(poArgs, 1, &iMotionIndex)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT)
	{
		bool bSuccess = pRT->SetMotionIndex(iMotionIndex);
		return Py_BuildValue("i", bSuccess ? 1 : 0);
	}
	return Py_BuildValue("i", 0);
}

PyObject* renderTargetIsShow(PyObject* poSelf, PyObject* poArgs)
{
	BYTE index = 0;
	if (!PyTuple_GetByte(poArgs, 0, &index)) return Py_BadArgument();

	auto pRT = CRenderTargetManager::Instance().GetRenderTarget(index);
	if (pRT) return Py_BuildValue("i", pRT->IsShow() ? 1 : 0);
	return Py_BuildValue("i", 0);
}

PyObject* renderTargetResetModel(PyObject* poSelf, PyObject* poArgs)
{
	return Py_BuildNone(); // SelectModel automatically cleans the model in your implementation
}

void initrenderTarget() 
{
	static PyMethodDef s_methods[] =
	{
		{ "CreateRenderTarget", renderTargetCreateRenderTarget, METH_VARARGS },
		{ "Render", renderTargetRender, METH_VARARGS },
		{ "SelectModel", renderTargetSelectModel, METH_VARARGS },
		{ "SetVisibility", renderTargetSetVisibility, METH_VARARGS },
		{ "SetBackground", renderTargetSetBackground, METH_VARARGS },
		{ "SetArmor", renderTargetSetArmor, METH_VARARGS },
		{ "SetWeapon", renderTargetSetWeapon, METH_VARARGS },
		{ "SetAcce", renderTargetSetAcce, METH_VARARGS },
		{ "SetHair", renderTargetSetHair, METH_VARARGS },
		{ "SetMount", renderTargetSetMount, METH_VARARGS },
		{ "SetRotation", renderTargetSetRotation, METH_VARARGS },
		{ "RotateEyeAroundTarget", renderTargetRotateEyeAroundTarget, METH_VARARGS },
		{ "Zoom", renderTargetZoom, METH_VARARGS },
		{ "SetMotionIndex", renderTargetSetMotionIndex, METH_VARARGS },
		{ "IsShow", renderTargetIsShow, METH_VARARGS },
		{ "ResetModel", renderTargetResetModel, METH_VARARGS },
		{nullptr, nullptr, 0 },
	};

	PyObject* poModule = Py_InitModule("renderTarget", s_methods);
}