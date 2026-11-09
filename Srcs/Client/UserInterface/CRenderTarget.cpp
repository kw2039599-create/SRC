#include "StdAfx.h"
#include "CRenderTarget.h"
#include "CRenderTargetManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/GrpRenderTargetTexture.h"
#include "../EterPythonLib/PythonGraphic.h"

#include "../EterBase/CRC32.h"
#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "InstanceBase.h"
#include "../EterLib/ResourceManager.h"


CRenderTarget::CRenderTarget(const DWORD width, const DWORD height) : m_pModel(nullptr),
                                 m_background(nullptr),
                                 m_modelRotation(0),
                                 m_visible(false) 
{
	auto pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16)) {
		delete pTex;
		TraceError("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		throw std::runtime_error("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture");
	}

	m_renderTargetTexture = std::unique_ptr<CGraphicRenderTargetTexture>(pTex);
}

CRenderTarget::~CRenderTarget()
{

}

void CRenderTarget::SetVisibility(bool isShow)
{
	m_visible = isShow;
}
void CRenderTarget::SetArmor(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeArmor(vnum);
}

void CRenderTarget::SetWeapon(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeWeapon(vnum);
}

void CRenderTarget::SetAcce(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeAcce(vnum);
}


void CRenderTarget::ChangeHair(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeHair(vnum);
	//Unnötig//
	/*m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);
	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();*/
}
void CRenderTarget::RenderTexture() const
{
	m_renderTargetTexture->Render();
}

void CRenderTarget::SetRenderingRect(RECT* rect) const
{
	m_renderTargetTexture->SetRenderingRect(rect);
}

void CRenderTarget::CreateTextures() const
{
	m_renderTargetTexture->CreateTextures();
}

void CRenderTarget::ReleaseTextures() const
{
	m_renderTargetTexture->ReleaseTextures();
}

void CRenderTarget::SelectModel(const DWORD index)
{
	CInstanceBase::SCreateData kCreateData{};

	// If the index is higher than 8 (Wolfman), it is a Mount or Pet (NPC).
	kCreateData.m_bType = (index <= 8) ? CActorInstance::TYPE_PC : CActorInstance::TYPE_NPC; 
	kCreateData.m_dwRace = index;

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
		{
			m_pModel.reset();
		}
		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0,0,0));
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();
}

bool CRenderTarget::CreateBackground(const char* imgPath, const DWORD width, const DWORD height)
{
	if (m_background)
		return false;

	m_background = std::make_unique<CGraphicImageInstance>();
	
	const auto graphic_image = dynamic_cast<CGraphicImage*>(CResourceManager::instance().GetResourcePointer(imgPath));
	if (!graphic_image)
	{
		m_background.reset();
		return false;
	}

	m_background->SetImagePointer(graphic_image);
	m_background->SetScale(static_cast<float>(width) / graphic_image->GetWidth(), static_cast<float>(height) / graphic_image->GetHeight());
	return true;
}


void CRenderTarget::RenderBackground() const
{
	if (!m_visible || !m_background)
		return;

	auto pRect = m_renderTargetTexture->GetRenderingRect();
	if (!pRect)
		return;

	const float width = static_cast<float>(pRect->right - pRect->left);
	const float height = static_cast<float>(pRect->bottom - pRect->top);
	if (width <= 0.0f || height <= 0.0f)
		return;

	m_renderTargetTexture->SetRenderTarget();
	CGraphicRenderTargetTexture::Clear();
	CPythonGraphic::Instance().SetInterfaceRenderState();

	CPythonGraphic::Instance().SetOrtho2D(width, height, 400.0f);

	if (m_background->GetWidth() > 0 && m_background->GetHeight() > 0)
	{
		m_background->SetPosition(0.0f, 0.0f);
		m_background->SetScale(
			(width + 1.0f) / static_cast<float>(m_background->GetWidth()),
			(height + 1.0f) / static_cast<float>(m_background->GetHeight())
		);
	}

	m_background->Render();
	m_renderTargetTexture->ResetRenderTarget();
}

void CRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;

	if (m_autoRotation)
	{
		if (m_modelRotation < 360.0f)
			m_modelRotation += 1.0f;
		else
			m_modelRotation = 0.0f;
	}

	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
	m_pModel->Update(); // CRITICAL FIX: Forces accessories, mounts, and hair to physically attach!
}

void CRenderTarget::DeformModel() const
{
	if (!m_visible)
		return;

	if (m_pModel)
		m_pModel->Deform();
}

void CRenderTarget::RenderModel() const
{
	if (!m_visible || !m_pModel)
		return;

	auto pRect = m_renderTargetTexture->GetRenderingRect();
	if (!pRect)
		return; // Safety check: Waits until Python sets the window bounds

	auto& python_graphic = CPythonGraphic::Instance();
	auto& camera_manager = CCameraManager::instance();
	auto& state_manager = CStateManager::Instance();

	m_renderTargetTexture->SetRenderTarget();

	if (!m_background)
		m_renderTargetTexture->Clear();

	python_graphic.ClearDepthBuffer();

	const auto fov = python_graphic.GetFOV();
	const auto aspect = python_graphic.GetAspect();
	const auto near_y = python_graphic.GetNearY();
	const auto far_y = python_graphic.GetFarY();

	const auto width = static_cast<float>(pRect->right - pRect->left);
	const auto height = static_cast<float>(pRect->bottom - pRect->top);


	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);

	python_graphic.SetViewport(0.0f, 0.0f, width, height);

	python_graphic.PushState();

	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetViewParams(
		D3DXVECTOR3{ 0.0f, -m_cameraZoom, 600.0f + m_cameraPitch * 10.0f },
		D3DXVECTOR3{ 0.0f, 0.0f, 95.0f },
		D3DXVECTOR3{0.0f, 0.0f, 1.0f}
	);

	python_graphic.UpdateViewMatrix();

	python_graphic.SetPerspective(10.0f, width / height, 100.0f, 15000.0f);

	m_pModel->Render();
	m_pModel->GetGraphicThingInstanceRef().RenderAllAttachingEffect();

	camera_manager.ResetToPreviousCamera();
	python_graphic.RestoreViewport();
	python_graphic.PopState();
	python_graphic.SetPerspective(fov, aspect, near_y, far_y);
	m_renderTargetTexture->ResetRenderTarget();
	state_manager.SetRenderState(D3DRS_FOGENABLE, TRUE);
}

void CRenderTarget::SetMount(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;

	if (vnum == 0)
		m_pModel->DismountHorse();
	else
		m_pModel->MountHorse(vnum);
}

void CRenderTarget::SetAutoRotation(bool bAuto)
{
	m_autoRotation = bAuto;
}

void CRenderTarget::RotateEyeAroundTarget(float fPitch, float fYaw)
{
	m_cameraPitch += fPitch;
	
	// Safety Clamp: Prevent the camera from flying out of bounds or flipping upside down
	if (m_cameraPitch > 50.0f) m_cameraPitch = 50.0f;
	if (m_cameraPitch < -50.0f) m_cameraPitch = -50.0f;

	m_modelRotation -= fYaw;
}

void CRenderTarget::Zoom(float fRatio)
{
	// Increased scroll speed step from 100 to 400
	m_cameraZoom += (fRatio * 500.0f);
	
	if (m_cameraZoom < 500.0f) m_cameraZoom = 500.0f;
	
	// Increased max distance by 4x
	if (m_cameraZoom > 12000.0f) m_cameraZoom = 12000.0f;
}

bool CRenderTarget::SetMotionIndex(DWORD dwMotionIndex)
{
	if (!m_pModel) 
		return false;
		
	// Access the underlying actor instance directly
	auto pActor = m_pModel->GetGraphicThingInstancePtr();
		
	// 1. Clear previous skill effects so particles don't freeze or stack
	pActor->ClearAttachingEffect();
	
	if (dwMotionIndex == CRaceMotionData::NAME_WAIT)
	{
		// Clear the queue and loop the idle animation
		pActor->InterceptLoopMotion(CRaceMotionData::NAME_WAIT, 0.1f);
	}
	else
	{
		// 2. Intercept clears the current queue and plays the skill exactly once
		pActor->InterceptOnceMotion(dwMotionIndex, 0.1f, 0, 1.0f);
		
		// 3. Push queues the idle animation to seamlessly take over when the skill ends
		pActor->PushLoopMotion(CRaceMotionData::NAME_WAIT, 0.1f, 1.0f);
	}
	
	return true;
}

bool CRenderTarget::IsShow() const
{
	return m_visible;
}