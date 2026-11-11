#include "StdAfx.h"
#include "../eterBase/CRC32.h"
#include "PythonWindow.h"
#include "PythonSlotWindow.h"
#include "PythonWindowManager.h"
#ifdef ENABLE_TWEENY_LIBRARY
#include "../EterLib/StateManager.h"
#endif
#ifdef ENABLE_CSHIELD
#include "CShield.h"
#endif
#include <unordered_map>

BOOL g_bOutlineBoxEnable = FALSE;

namespace UI
{
struct SWindowBaseLayout
	{
		long baseX = 0;
		long baseY = 0;
		long baseW = 0;
		long baseH = 0;
		long posScreenW = 0;
		long posScreenH = 0;
		long sizeScreenW = 0;
		long sizeScreenH = 0;
		bool posInitialized = false;
		bool sizeInitialized = false;
	};

	static std::unordered_map<CWindow*, SWindowBaseLayout> gs_kWindowBaseLayoutMap;
	static std::vector<CWindow*> gs_kScreenSizeMovedWindows;
	bool gs_bIsScreenResizing = false;
	bool gs_bIgnoreBaseLayoutRecord = false;

	static void RecordWindowBaseLayout(CWindow* pWin, long x, long y, long w, long h, bool isPosUpdate, bool isSizeUpdate)
	{
		if (gs_bIsScreenResizing || gs_bIgnoreBaseLayoutRecord || !pWin)
			return;

		if (pWin->HasParent())
		{
			CWindow* pParent = pWin->GetParent();
			if (pParent->HasParent() && pParent->GetParent()->HasParent() &&
				pParent->GetParent()->GetParent()->HasParent() && pParent->GetParent()->GetParent()->GetParent()->HasParent())
				return;
		}

		long scrW = CWindowManager::Instance().GetScreenWidth();
		long scrH = CWindowManager::Instance().GetScreenHeight();
		if (scrW <= 0 || scrH <= 0)
			return;

		SWindowBaseLayout& rLayout = gs_kWindowBaseLayoutMap[pWin];
		if (isPosUpdate)
		{
			if (!rLayout.posInitialized || rLayout.baseX != x || rLayout.baseY != y)
			{
				rLayout.baseX = x;
				rLayout.baseY = y;
				rLayout.posScreenW = scrW;
				rLayout.posScreenH = scrH;
				rLayout.posInitialized = true;
			}
		}
		if (isSizeUpdate)
		{
			if (!rLayout.sizeInitialized || rLayout.baseW != w || rLayout.baseH != h)
			{
				rLayout.baseW = w;
				rLayout.baseH = h;
				rLayout.sizeScreenW = scrW;
				rLayout.sizeScreenH = scrH;
				rLayout.sizeInitialized = true;
			}
		}
	}

	void NotifyScreenSizeMovedWindows()
	{
		std::vector<CWindow*> movedList;
		movedList.swap(gs_kScreenSizeMovedWindows);

		for (CWindow* pWin : movedList)
		{
			if (pWin && gs_kWindowBaseLayoutMap.find(pWin) != gs_kWindowBaseLayoutMap.end())
			{
				long lx = 0, ly = 0;
				pWin->GetPosition(&lx, &ly);
				pWin->OnMoveWindow(lx, ly);
			}
		}
	}
	CWindow::CWindow(PyObject * ppyObject) :
		m_x(0),
		m_y(0),
		m_lWidth(0),
		m_lHeight(0),
		m_poHandler(ppyObject),
		m_bShow(false),
		m_pParent(NULL),
		m_dwFlag(0),
		m_isUpdatingChildren(FALSE)
	{
#ifdef _DEBUG
		static DWORD DEBUG_dwGlobalCounter=0;
		DEBUG_dwCounter=DEBUG_dwGlobalCounter++;

		m_strName = "!!debug";
#endif
		//assert(m_poHandler != NULL);
		m_HorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_VerticalAlign = VERTICAL_ALIGN_TOP;
		m_rect.bottom = m_rect.left = m_rect.right = m_rect.top = 0;
#if defined(__BL_CLIP_MASK__)
		m_bEnableMask = false;
		m_pMaskWindow = NULL;
		memset(&m_rMaskRect, 0, sizeof(m_rMaskRect));
		m_iMaskAdjustValue = 0;
#endif
		m_limitBiasRect.bottom = m_limitBiasRect.left = m_limitBiasRect.right = m_limitBiasRect.top = 0;
#ifdef ENABLE_TWEENY_LIBRARY
		D3DXMatrixIdentity(&m_matScaling);
#endif
	}

	CWindow::~CWindow()
	{
		gs_kWindowBaseLayoutMap.erase(this);
	}

	DWORD CWindow::Type()
	{
		static DWORD s_dwType = GetCRC32("CWindow", strlen("CWindow"));
		return (s_dwType);
	}

	BOOL CWindow::IsType(DWORD dwType)
	{
		return OnIsType(dwType);
	}

	BOOL CWindow::OnIsType(DWORD dwType)
	{
		if (CWindow::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	struct FClear
	{
		void operator () (CWindow * pWin)
		{
			pWin->Clear();
		}
	};

	void CWindow::Clear()
	{
		gs_kWindowBaseLayoutMap.erase(this);
#if defined(__BL_CLIP_MASK__)
		m_pMaskWindow = NULL;
#endif
		std::for_each(m_pChildList.begin(), m_pChildList.end(), FClear());
		m_pChildList.clear();

		m_pParent = NULL;
		DestroyHandle();
		Hide();
	}
	void CWindow::DestroyHandle()
	{
		m_poHandler = NULL;
	}

	void CWindow::Show()
	{
#ifdef ENABLE_TWEENY_LIBRARY
		if (IsFlag(FLAG_ANIMATED_BOARD))
		{
			if (!this->m_bShow)
			{
				this->m_bShow = true;

				m_sizeAnimation = tweeny::from(0.0f)
					.to(1.0f)
					.during(175)
// Adalet - Not: Yukarıda verilen değerler scale during ve speed içeriyor
					.via(tweeny::easing::cubicOut)
// Not:cubicOut kısımlarını kütüphane içerisindeki tweeny-3.3.0.h dosyasındaki ''enum class enumerated'' bloğunda bulunan setlerle değiştirip yeni animasyon elde edebilirsiniz.
					.onStep([this](tweeny::tween<float>& t, float scale) {
					if (t.progress() == 0.0f)
					{
						SetScale(0.0f, 0.0f);
						//SetAllAlpha(0.0f);
					}

					SetScale(scale, scale);
					//SetAllAlpha(scale);

					if (t.progress() == 1.0f)
					{
						SetScale(1.0f, 1.0f);
						//SetAllAlpha(1.0f);

						return true;
					}
					else
					{
						return false;
					}
						});
				m_sizeAnimation.value().step(0.0f);
			}
			else
			{
				this->m_bShow = true;
				SetScale(1.0f, 1.0f);
			}
		}
		else
		{
			m_bShow = true;
		}
#else
		m_bShow = true;
#endif
	}

	void CWindow::Hide()
	{
#ifdef ENABLE_TWEENY_LIBRARY
		if (IsFlag(FLAG_ANIMATED_BOARD))
		{
			if (m_bShow)
			{
				m_sizeAnimation = tweeny::from(1.0f)
					.to(0.0f)
					.during(120)
// Adalet - Not: Yukarıda verilen değerler scale during ve speed içeriyor
					.via(tweeny::easing::cubicOut)
// Adalet - Not:cubicOut kısımlarını kütüphane içerisindeki tweeny-3.3.0.h dosyasındaki ''enum class enumerated'' bloğunda bulunan setlerle değiştirip yeni animasyon elde edebilirsiniz.
					.onStep([this](tweeny::tween<float>& t, float scale) {
					if (t.progress() == 0.0f)
					{
						SetScale(1.0f, 1.0f);
						//SetAllAlpha(1.0f);
					}

					SetScale(scale, scale);
					//SetAllAlpha(scale);

					if (t.progress() == 1.0f)
					{
						SetScale(0.0f, 0.0f);
						//SetAllAlpha(0.0f);

						m_bShow = false;
						return true;
					}
					else
					{
						return false;
					}
						});
				m_sizeAnimation.value().step(0.0f);
			}
			else
				m_bShow = false;
		}
		else
		{
			m_bShow = false;
		}
#else
		m_bShow = false;
#endif
	}

#if defined(__BL_MOUSE_WHEEL_TOP_WINDOW__)
	bool CWindow::OnMouseWheelButtonUp()
	{
		bool bValue = false;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseWheelButtonUp", BuildEmptyTuple(), &bValue))
			return bValue;

		return false;
	}

	bool CWindow::OnMouseWheelButtonDown()
	{
		bool bValue = false;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseWheelButtonDown", BuildEmptyTuple(), &bValue))
			return bValue;

		return false;
	}
#endif

#ifdef ENABLE_TWEENY_LIBRARY
#include <directxmath/DirectXMath.h> // Yeni kütüphane directxmath for Nileya - Adalet
	void CWindow::SetScale(float fx, float fy)
	{
		m_v2Scale.x = fx;
		m_v2Scale.y = fy;
		UpdateRect();

		using namespace DirectX;

		XMFLOAT2 vCenter(m_x + (m_lWidth / 2), m_y + (m_lHeight / 2));
		XMStoreFloat4x4((XMFLOAT4X4*)&m_matScaling, XMMatrixTransformation2D(XMLoadFloat2((XMFLOAT2*)&vCenter), NULL, XMLoadFloat2((XMFLOAT2*)&m_v2Scale), XMLoadFloat2((XMFLOAT2*)&vCenter), NULL, XMVectorZero()));

	}
#endif

	bool CWindow::IsRendering()
	{
		if (!IsShow())
			return false;

		if (!m_pParent)
			return true;

		return m_pParent->IsRendering();
	}

	void CWindow::__RemoveReserveChildren()
	{
		if (m_pReserveChildList.empty())
			return;

		TWindowContainer::iterator it;
		for(it = m_pReserveChildList.begin(); it != m_pReserveChildList.end(); ++it)
		{
			m_pChildList.remove(*it);
		}
		m_pReserveChildList.clear();
	}

	void CWindow::Update()
	{
		if (!IsShow())
			return;

		__RemoveReserveChildren();

		OnUpdate();
#ifdef ENABLE_TWEENY_LIBRARY
		if (m_sizeAnimation)
			m_sizeAnimation.value().step(static_cast<uint32_t>(CTimer::instance().GetElapsedMilliecond()));
#endif
		m_isUpdatingChildren = TRUE;

		for (auto& Child : m_pChildList)
			Child->Update();

		m_isUpdatingChildren = FALSE;
	}

	void CWindow::Render()
	{
		if (!IsShow())
			return;

#ifdef __ANDROID__
		static int s_iRootRenderCount = 0;
		if (!m_pParent)
			++s_iRootRenderCount;
		if (s_iRootRenderCount == 20)
		{
			Tracenf("[UI-Dump] Wnd '%s' parent='%s' rect=(%ld,%ld,%ld,%ld) children=%zu",
				m_strName.c_str(),
				m_pParent ? m_pParent->GetName() : "NULL",
				m_rect.left, m_rect.top, m_rect.right, m_rect.bottom,
				m_pChildList.size());
		}
#endif

		OnRender();
#ifdef ENABLE_TWEENY_LIBRARY
		D3DXMATRIX currentMat;
		STATEMANAGER.GetTransform(D3DTS_WORLD, &currentMat);

		D3DXMATRIX newMat;
		D3DXMatrixMultiply(&newMat, &currentMat, &m_matScaling);
		STATEMANAGER.SetTransform(D3DTS_WORLD, &newMat);
#endif
		if (g_bOutlineBoxEnable)
		{
			CPythonGraphic::Instance().SetDiffuseColor(1.0f, 1.0f, 1.0f);
			CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
		}

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::void_mem_fun(&CWindow::Render));
#ifdef ENABLE_TWEENY_LIBRARY
		STATEMANAGER.SetTransform(D3DTS_WORLD, &currentMat);
#endif
	}

	void CWindow::OnUpdate()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		static PyObject* poFuncName_OnUpdate = PyString_InternFromString("OnUpdate");

		//PyCallClassMemberFunc(m_poHandler, "OnUpdate", BuildEmptyTuple());
		PyCallClassMemberFunc_ByPyString(m_poHandler, poFuncName_OnUpdate, BuildEmptyTuple());

	}

	void CWindow::OnRender()
	{
		if (!m_poHandler)
			return;

		if (!IsShow())
			return;

		//PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}

	void CWindow::SetName(const char * c_szName)
	{
		m_strName = c_szName;
	}

	void CWindow::SetSize(long width, long height)
	{
		m_lWidth = width;
		m_lHeight = height;

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;

		RecordWindowBaseLayout(this, m_x, m_y, m_lWidth, m_lHeight, false, true);
	}

	void CWindow::SetHorizontalAlign(DWORD dwAlign)
	{
		m_HorizontalAlign = (EHorizontalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetVerticalAlign(DWORD dwAlign)
	{
		m_VerticalAlign = (EVerticalAlign)dwAlign;
		UpdateRect();
	}

	void CWindow::SetPosition(long x, long y)
	{
		m_x = x;
		m_y = y;

		RecordWindowBaseLayout(this, m_x, m_y, m_lWidth, m_lHeight, true, false);

		UpdateRect();
	}

	void CWindow::GetPosition(long * plx, long * ply)
	{
		*plx = m_x;
		*ply = m_y;
	}

	long CWindow::UpdateRect()
	{
		if (gs_bIsScreenResizing && m_pParent)
		{
			long curScrW = CWindowManager::Instance().GetScreenWidth();
			long curScrH = CWindowManager::Instance().GetScreenHeight();

			bool isTopLevel = (m_pParent->GetParent() && !m_pParent->GetParent()->HasParent());
			bool isSecondLevel = (!isTopLevel && m_pParent->GetParent() && m_pParent->GetParent()->GetParent() && !m_pParent->GetParent()->GetParent()->HasParent());
			bool isThirdLevel = (!isTopLevel && !isSecondLevel && m_pParent->GetParent() && m_pParent->GetParent()->GetParent() &&
				m_pParent->GetParent()->GetParent()->GetParent() && !m_pParent->GetParent()->GetParent()->GetParent()->HasParent());

			// Direct child of an unsized 0x0 container (e.g. GameWindow in game.py)
			bool isUnsizedContainerChild = (isSecondLevel && m_pParent->GetWidth() == 0 && m_pParent->GetHeight() == 0 && m_pParent->m_x == 0 && m_pParent->m_y == 0);

			auto it = gs_kWindowBaseLayoutMap.find(this);
			if (it != gs_kWindowBaseLayoutMap.end() && curScrW > 0 && curScrH > 0)
			{
				const SWindowBaseLayout& layout = it->second;
				long posScrW = layout.posInitialized ? layout.posScreenW : layout.sizeScreenW;
				long posScrH = layout.posInitialized ? layout.posScreenH : layout.sizeScreenH;
				long sizeScrW = layout.sizeInitialized ? layout.sizeScreenW : layout.posScreenW;
				long sizeScrH = layout.sizeInitialized ? layout.sizeScreenH : layout.posScreenH;

				if ((layout.posInitialized || layout.sizeInitialized) && posScrW > 0 && posScrH > 0 && sizeScrW > 0 && sizeScrH > 0)
				{
					long baseW = ::abs(layout.baseW);
					long baseH = ::abs(layout.baseH);

					if (isTopLevel || isUnsizedContainerChild)
					{
						long oldX = m_x;
						long oldY = m_y;
						long oldW = m_lWidth;
						long oldH = m_lHeight;

						if (baseW >= sizeScrW - 4)
						{
							m_lWidth = curScrW - (sizeScrW - baseW);
							if (baseH >= sizeScrH - 4)
								m_lHeight = curScrH - (sizeScrH - baseH);

							if (m_VerticalAlign == VERTICAL_ALIGN_TOP)
							{
								long baseCenterY = layout.baseY + baseH / 2;
								if (baseCenterY > (posScrH * 2) / 3)
								{
									long bottomMargin = posScrH - (layout.baseY + baseH);
									m_y = curScrH - ::abs(m_lHeight) - bottomMargin;
								}
								else if (layout.baseY >= 60 && baseCenterY >= posScrH / 3)
								{
									long centerOffset = baseCenterY - (posScrH / 2);
									m_y = (curScrH / 2) + centerOffset - (::abs(m_lHeight) / 2);
								}
								else
								{
									m_y = layout.baseY;
								}
							}
						}
						else
						{
							long w = ::abs((m_lWidth != 0) ? m_lWidth : layout.baseW);
							long h = ::abs((m_lHeight != 0) ? m_lHeight : layout.baseH);
							bool isSmallHudElement = (w > 0 && w < 120 && h > 0 && h < 120);
							bool isNearBottom = (layout.baseY + h >= posScrH - 220);

							if (m_HorizontalAlign == HORIZONTAL_ALIGN_LEFT)
							{
								long baseCenterX = layout.baseX + w / 2;
								if (baseCenterX > (posScrW * 2) / 3)
								{
									long rightMargin = posScrW - (layout.baseX + w);
									m_x = curScrW - w - rightMargin;
								}
								else if ((!isSmallHudElement || isNearBottom) && layout.baseX >= 60 && baseCenterX >= posScrW / 3)
								{
									long centerOffset = baseCenterX - (posScrW / 2);
									m_x = (curScrW / 2) + centerOffset - (w / 2);
								}
								else
								{
									m_x = layout.baseX;
								}

								if (w > 0 && w <= curScrW)
									m_x = max(0L, min(m_x, curScrW - w));
								else if (w > curScrW)
									m_x = 0;
							}

							if (m_VerticalAlign == VERTICAL_ALIGN_TOP)
							{
								long baseCenterY = layout.baseY + h / 2;
								if (baseCenterY > (posScrH * 2) / 3 || (isSmallHudElement && isNearBottom))
								{
									long bottomMargin = posScrH - (layout.baseY + h);
									m_y = curScrH - h - bottomMargin;
								}
								else if (!isSmallHudElement && layout.baseY >= 60 && baseCenterY >= posScrH / 3)
								{
									long centerOffset = baseCenterY - (posScrH / 2);
									m_y = (curScrH / 2) + centerOffset - (h / 2);
								}
								else
								{
									m_y = layout.baseY;
								}

								if (h > 0 && h <= curScrH)
									m_y = max(0L, min(m_y, curScrH - h));
								else if (h > curScrH)
									m_y = 0;
							}
						}

						if (m_x != oldX || m_y != oldY || m_lWidth != oldW || m_lHeight != oldH)
						{
							if (std::find(gs_kScreenSizeMovedWindows.begin(), gs_kScreenSizeMovedWindows.end(), this) == gs_kScreenSizeMovedWindows.end())
								gs_kScreenSizeMovedWindows.push_back(this);
						}
					}
					else if (isSecondLevel || isThirdLevel)
					{
						auto itParent = gs_kWindowBaseLayoutMap.find(m_pParent);
						if (itParent != gs_kWindowBaseLayoutMap.end() && itParent->second.sizeInitialized &&
							::abs(itParent->second.baseW) >= itParent->second.sizeScreenW - 4)
						{
							const SWindowBaseLayout& parentLayout = itParent->second;
							long parentScrH = parentLayout.sizeScreenH;
							long parentBaseH = ::abs(parentLayout.baseH);
							bool isTaskBarChild = (isSecondLevel && strcmp(m_pParent->GetName(), "TaskBar") == 0);
							bool isLoginWindowChild = (isSecondLevel && strcmp(m_pParent->GetName(), "LoginWindow") == 0);
							bool isFullScreenParent = (parentBaseH >= parentScrH - 4);

							if (m_strName == "Base_Board_01" && IsType(CExpandedImageBox::Type()))
							{
								m_x = 263;
								((CExpandedImageBox*)this)->SetRenderingRect(0.0f, 0.0f, float(curScrW - 263 - 256) / 256.0f, 0.0f);
							}
							else if (IsType(CExpandedImageBox::Type()) && baseW >= sizeScrW - 4 && baseH >= sizeScrH - 4)
							{
								int imgW = ((CExpandedImageBox*)this)->GetWidth();
								int imgH = ((CExpandedImageBox*)this)->GetHeight();
								if (imgW > 0 && imgH > 0)
								{
									// Tiled pattern texture (e.g. 128x128 background_pattern.tga in classic introselect) vs scaled background
									if (imgW <= 256 && imgH <= 256)
									{
										m_lWidth = curScrW;
										m_lHeight = curScrH;
										((CExpandedImageBox*)this)->SetRenderingRect(0.0f, 0.0f, float(curScrW - imgW) / float(imgW), float(curScrH - imgH) / float(imgH));
									}
									else
									{
										((CExpandedImageBox*)this)->SetScale(float(curScrW) / float(imgW), float(curScrH) / float(imgH));
									}
								}
							}
							else if (isTaskBarChild && m_HorizontalAlign == HORIZONTAL_ALIGN_LEFT)
							{
								if (layout.baseX < 263)
									m_x = layout.baseX;
								else if (layout.baseX < (posScrW / 2) + 215)
									m_x = layout.baseX + (curScrW - posScrW) / 2;
								else
									m_x = layout.baseX + (curScrW - posScrW);
							}
							else if (isLoginWindowChild)
							{
								bool isTopLeftCornerBtn = (layout.baseX <= 40 && layout.baseY <= 40 && baseW < 200 && baseH < 100);
								if (!isTopLeftCornerBtn)
								{
									if (m_HorizontalAlign == HORIZONTAL_ALIGN_LEFT)
										m_x = layout.baseX + (curScrW - posScrW) / 2;
									if (m_VerticalAlign == VERTICAL_ALIGN_TOP)
										m_y = layout.baseY + (curScrH - posScrH) / 2;
								}
							}
							else if (isFullScreenParent)
							{
								// Full-screen phase window (SelectCharacterWindow, CreateCharacterWindow, EmpireWindow):
								// Keep top-left toggle button pinned, anchor left/center/right columns horizontally,
								// and shift all scene elements together vertically so slots/faces/buttons never stretch apart
								bool isTopLeftCornerBtn = (layout.baseX <= 40 && layout.baseY <= 40 && baseW < 200 && baseH < 100);
								if (!isTopLeftCornerBtn)
								{
									long w = ::abs((m_lWidth != 0) ? m_lWidth : layout.baseW);
									if (m_HorizontalAlign == HORIZONTAL_ALIGN_LEFT)
									{
										long baseCenterX = layout.baseX + w / 2;
										if (baseCenterX > (posScrW * 2) / 3)
											m_x = layout.baseX + (curScrW - posScrW);
										else if (baseCenterX >= posScrW / 3)
											m_x = layout.baseX + (curScrW - posScrW) / 2;
										else
											m_x = layout.baseX;
									}
									if (m_VerticalAlign == VERTICAL_ALIGN_TOP)
									{
										m_y = layout.baseY + (curScrH - posScrH) / 2;
									}
								}
							}
							else if (isSecondLevel)
							{
								if (m_HorizontalAlign == HORIZONTAL_ALIGN_LEFT)
								{
									long w = ::abs((m_lWidth != 0) ? m_lWidth : layout.baseW);
									long baseCenterX = layout.baseX + w / 2;
									if (baseCenterX > (posScrW * 2) / 3)
										m_x = layout.baseX + (curScrW - posScrW);
									else if (baseCenterX >= posScrW / 3)
										m_x = layout.baseX + (curScrW - posScrW) / 2;
									else
										m_x = layout.baseX;
								}
							}
						}
					}
				}
			}
		}

		m_rect.top		= m_y;
		if (m_pParent)
		{
			switch (m_VerticalAlign)
			{
				case VERTICAL_ALIGN_BOTTOM:
					m_rect.top = m_pParent->GetHeight() - m_rect.top;
					break;
				case VERTICAL_ALIGN_CENTER:
					m_rect.top = (m_pParent->GetHeight() - GetHeight()) / 2 + m_rect.top;
					break;
			}
			m_rect.top += m_pParent->m_rect.top;
		}
		m_rect.bottom	= m_rect.top + m_lHeight;

#if defined( _USE_CPP_RTL_FLIP )
		if( m_pParent == NULL ) {
			m_rect.left		= m_x;
			m_rect.right	= m_rect.left + m_lWidth;
		} else {
			if( m_pParent->IsFlag(UI::CWindow::FLAG_RTL) == true ) {
				m_rect.left = m_pParent->GetWidth() - m_lWidth - m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = - m_x;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() - m_x;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			} else {
				m_rect.left		= m_x;
				switch (m_HorizontalAlign)
				{
					case HORIZONTAL_ALIGN_RIGHT:
						m_rect.left = m_pParent->GetWidth() - m_rect.left;
						break;
					case HORIZONTAL_ALIGN_CENTER:
						m_rect.left = (m_pParent->GetWidth() - GetWidth()) / 2 + m_rect.left;
						break;
				}
				m_rect.left += m_pParent->m_rect.left;
				m_rect.right = m_rect.left + m_lWidth;
			}
		}
#else
		m_rect.left		= m_x;
		if (m_pParent)
		{
			switch (m_HorizontalAlign)
			{
				case HORIZONTAL_ALIGN_RIGHT:
					m_rect.left = ::abs(m_pParent->GetWidth()) - m_rect.left;
					break;
				case HORIZONTAL_ALIGN_CENTER:
					m_rect.left = m_pParent->GetWidth() / 2 - GetWidth() / 2 + m_rect.left;
					break;
			}
			m_rect.left += 0L < m_pParent->GetWidth() ? m_pParent->m_rect.left : m_pParent->m_rect.right + ::abs(m_pParent->GetWidth());
		}
		m_rect.right = m_rect.left + m_lWidth;
#endif
		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		OnChangePosition();

		return 1;
	}

	void CWindow::GetLocalPosition(long & rlx, long & rly)
	{
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::GetMouseLocalPosition(long & rlx, long & rly)
	{
		CWindowManager::Instance().GetMousePosition(rlx, rly);
		rlx = rlx - m_rect.left;
		rly = rly - m_rect.top;
	}

	void CWindow::AddChild(CWindow * pWin)
	{
		m_pChildList.push_back(pWin);
		pWin->m_pParent = this;
	}

	CWindow * CWindow::GetRoot()
	{
		if (m_pParent)
			if (m_pParent->IsWindow())
				return m_pParent->GetRoot();

		return this;
	}

	CWindow * CWindow::GetParent()
	{
		return m_pParent;
	}

	bool CWindow::IsChild(CWindow * pWin)
	{
		auto itor = m_pChildList.begin();

		while (itor != m_pChildList.end())
		{
			if (*itor == pWin)
				return true;

			++itor;
		}

		return false;
	}

	void CWindow::DeleteChild(CWindow * pWin)
	{
		if (m_isUpdatingChildren)
		{
			m_pReserveChildList.push_back(pWin);
		}
		else
		{
			m_pChildList.remove(pWin);
		}
	}

	void CWindow::SetTop(CWindow * pWin)
	{
		if (!pWin->IsFlag(CWindow::FLAG_FLOAT))
			return;

		TWindowContainer::iterator itor = std::find(m_pChildList.begin(), m_pChildList.end(), pWin);
		if (m_pChildList.end() != itor)
		{
			m_pChildList.push_back(*itor);
			m_pChildList.erase(itor);

			pWin->OnTop();
		}
		else
		{
			TraceError(" CWindow::SetTop - Failed to find child window\n");
		}
	}

	void CWindow::OnMouseDrag(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseDrag", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnMoveWindow(long lx, long ly)
	{
		PyCallClassMemberFunc(m_poHandler, "OnMoveWindow", Py_BuildValue("(ii)", lx, ly));
	}

	void CWindow::OnSetFocus()
	{
		//PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
		PyCallClassMemberFunc(m_poHandler, "OnSetFocus", BuildEmptyTuple());
	}

	void CWindow::OnKillFocus()
	{
		PyCallClassMemberFunc(m_poHandler, "OnKillFocus", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverIn()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CWindow::OnMouseOverOut()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

	void CWindow::OnMouseOver()
	{
	}

	void CWindow::OnDrop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnDrop", BuildEmptyTuple());
	}

	void CWindow::OnTop()
	{
		PyCallClassMemberFunc(m_poHandler, "OnTop", BuildEmptyTuple());
	}

	void CWindow::OnIMEUpdate()
	{
		PyCallClassMemberFunc(m_poHandler, "OnIMEUpdate", BuildEmptyTuple());
	}

#if defined(__BL_CLIP_MASK__)
	void CWindow::SetClippingMaskRect(const RECT& rMask)
	{
		m_bEnableMask = true;
		m_rMaskRect = rMask;

		TWindowContainer::const_reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow)
				pWindow->SetClippingMaskRect(rMask);
		}
	}

	void CWindow::SetClippingMaskWindow(CWindow* pMaskWindow, int iAdjustValue)
	{
		if (!pMaskWindow)
			return;

		m_bEnableMask = true;
		m_pMaskWindow = pMaskWindow;
		m_iMaskAdjustValue = iAdjustValue;

		TWindowContainer::const_reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow* pWindow = *itor;

			if (pWindow)
				pWindow->SetClippingMaskWindow(pMaskWindow, iAdjustValue);
		}
	}
#endif

	BOOL CWindow::RunIMETabEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMETabEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMETabEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEReturnEvent()
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEReturnEvent())
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEReturnEvent())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunIMEKeyDownEvent(int ikey)
	{
		if (!IsRendering())
			return FALSE;

		if (OnIMEKeyDownEvent(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunIMEKeyDownEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	CWindow * CWindow::RunKeyDownEvent(int ikey)
	{
		if (OnKeyDown(ikey))
			return this;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			{
				CWindow * pProcessedWindow = pWindow->RunKeyDownEvent(ikey);
				if (NULL != pProcessedWindow)
				{
					return pProcessedWindow;
				}
			}
		}

		return NULL;
	}

	BOOL CWindow::RunKeyUpEvent(int ikey)
	{
		if (OnKeyUp(ikey))
			return TRUE;

		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunKeyUpEvent(ikey))
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::RunPressEscapeKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->IsShow())
			if (pWindow->RunPressEscapeKeyEvent())
				return TRUE;
		}

		if (OnPressEscapeKey())
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::RunPressExitKeyEvent()
	{
		TWindowContainer::reverse_iterator itor;
		for (itor = m_pChildList.rbegin(); itor != m_pChildList.rend(); ++itor)
		{
			CWindow * pWindow = *itor;

			if (pWindow->RunPressExitKeyEvent())
				return TRUE;

			if (pWindow->IsShow())
			if (pWindow->OnPressExitKey())
				return TRUE;
		}

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseLeftButtonUp()
	{
		PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonUp", BuildEmptyTuple());
		return TRUE;
	}

	BOOL CWindow::OnMouseLeftButtonDoubleClick()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseLeftButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonUp()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseRightButtonDoubleClick()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseRightButtonDoubleClick", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonDown()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonDown", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnMouseMiddleButtonUp()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnMouseMiddleButtonUp", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

#ifdef ENABLE_MOUSEWHEEL_EVENT
	BOOL CWindow::OnMouseWheel(short wDelta)
	{
#ifdef _DEBUG
		Tracenf("Mouse Wheel Scroll : wDelta %d ",wDelta);
#endif
		long lValue = 0;
		return PyCallClassMemberFunc(m_poHandler , "OnMouseWheel" , Py_BuildValue("(i)" , wDelta), &lValue) && 0 != lValue;
	}
#endif

	BOOL CWindow::OnIMETabEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMETab", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEReturnEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEReturn", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEKeyDownEvent(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEChangeCodePage()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEChangeCodePage", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenCandidateListEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseCandidateListEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseCandidateList", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMEOpenReadingWndEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMEOpenReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnIMECloseReadingWndEvent()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnIMECloseReadingWnd", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyDown(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyDown", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnKeyUp(int ikey)
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnKeyUp", Py_BuildValue("(i)", ikey), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressEscapeKey()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressEscapeKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	BOOL CWindow::OnPressExitKey()
	{
		long lValue;
		if (PyCallClassMemberFunc(m_poHandler, "OnPressExitKey", BuildEmptyTuple(), &lValue))
		if (0 != lValue)
			return TRUE;

		return FALSE;
	}

	/////

	bool CWindow::IsIn(long x, long y)
	{
#ifdef ENABLE_OPENGL
		if (!IsFlag(CWindow::FLAG_NOT_PICK) && dynamic_cast<CTextLine*>(this) != nullptr)
		{
			if (x >= m_rect.left - 8 && x <= m_rect.right + 8 &&
			    y >= m_rect.top - 8 && y <= m_rect.bottom + 8)
				return true;
		}
#endif
		if (x >= m_rect.left && x <= m_rect.right)
			if (y >= m_rect.top && y <= m_rect.bottom)
				return true;

		return false;
	}

	bool CWindow::IsIn()
	{
		long lx, ly;
		UI::CWindowManager::Instance().GetMousePosition(lx, ly);

		return IsIn(lx, ly);
	}

	CWindow * CWindow::PickWindow(long x, long y)
	{
		auto ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
			{
				if (!pWin->IsFlag(CWindow::FLAG_IGNORE_SIZE))
				{
					if (!pWin->IsIn(x, y)) {
						if (0L <= pWin->GetWidth()) {
							continue;
						}
					}
				}

				CWindow * pResult = pWin->PickWindow(x, y);
				if (pResult)
					return pResult;
			}
		}

		if (IsFlag(CWindow::FLAG_NOT_PICK))
			return NULL;

		return (this);
	}

	CWindow * CWindow::PickTopWindow(long x, long y)
	{
		auto ritor = m_pChildList.rbegin();
		for (; ritor != m_pChildList.rend(); ++ritor)
		{
			CWindow * pWin = *ritor;
			if (pWin->IsShow())
				if (pWin->IsIn(x, y))
					if (!pWin->IsFlag(CWindow::FLAG_NOT_PICK))
						return pWin;
		}

		return NULL;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CBox::CBox(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBox::~CBox()
	{
	}

	void CBox::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBox::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBox2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CBar::CBar(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CBar::~CBar()
	{
	}

	void CBar::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CBar::OnRender()
	{
		CPythonGraphic::Instance().SetDiffuseColor(m_dwColor);
		CPythonGraphic::Instance().RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CLine::CLine(PyObject * ppyObject) : CWindow(ppyObject), m_dwColor(0xff000000)
	{
	}
	CLine::~CLine()
	{
	}

	void CLine::SetColor(DWORD dwColor)
	{
		m_dwColor = dwColor;
	}

	void CLine::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();
		rkpyGraphic.SetDiffuseColor(m_dwColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CBar3D::Type()
	{
		static DWORD s_dwType = GetCRC32("CBar3D", strlen("CBar3D"));
		return (s_dwType);
	}

	CBar3D::CBar3D(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_dwLeftColor = D3DXCOLOR(0.2f, 0.2f, 0.2f, 1.0f);
		m_dwRightColor = D3DXCOLOR(0.7f, 0.7f, 0.7f, 1.0f);
		m_dwCenterColor = D3DXCOLOR(0.0f, 0.0f, 0.0f, 1.0f);
	}
	CBar3D::~CBar3D()
	{
	}

	void CBar3D::SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter)
	{
		m_dwLeftColor = dwLeft;
		m_dwRightColor = dwRight;
		m_dwCenterColor = dwCenter;
	}

	void CBar3D::OnRender()
	{
		CPythonGraphic & rkpyGraphic = CPythonGraphic::Instance();

		rkpyGraphic.SetDiffuseColor(m_dwCenterColor);
		rkpyGraphic.RenderBar2d(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwLeftColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.right, m_rect.top);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.top, m_rect.left, m_rect.bottom);

		rkpyGraphic.SetDiffuseColor(m_dwRightColor);
		rkpyGraphic.RenderLine2d(m_rect.left, m_rect.bottom, m_rect.right, m_rect.bottom);
		rkpyGraphic.RenderLine2d(m_rect.right, m_rect.top, m_rect.right, m_rect.bottom);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CTextLine::CTextLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_TextInstance.SetColor(0.78f, 0.78f, 0.78f);
		m_TextInstance.SetHorizonalAlign(CGraphicTextInstance::HORIZONTAL_ALIGN_LEFT);
		m_TextInstance.SetVerticalAlign(CGraphicTextInstance::VERTICAL_ALIGN_TOP);
	}
	CTextLine::~CTextLine()
	{
		m_TextInstance.Destroy();
	}

	void CTextLine::SetMax(int iMax)
	{
		m_TextInstance.SetMax(iMax);
	}
	void CTextLine::SetHorizontalAlign(int iType)
	{
		m_TextInstance.SetHorizonalAlign(iType);
	}
	void CTextLine::SetVerticalAlign(int iType)
	{
		m_TextInstance.SetVerticalAlign(iType);
	}
	void CTextLine::SetSecret(BOOL bFlag)
	{
		m_TextInstance.SetSecret(bFlag ? true : false);
	}
	void CTextLine::SetOutline(BOOL bFlag)
	{
		m_TextInstance.SetOutline(bFlag ? true : false);
	}
	void CTextLine::SetFeather(BOOL bFlag)
	{
		m_TextInstance.SetFeather(bFlag ? true : false);
	}
	void CTextLine::SetMultiLine(BOOL bFlag)
	{
		m_TextInstance.SetMultiLine(bFlag ? true : false);
	}
	void CTextLine::SetFontName(const char * c_szFontName)
	{
		std::string stFontName = c_szFontName;
		stFontName += ".fnt";

		CResourceManager& rkResMgr=CResourceManager::Instance();
		CResource* pkRes = rkResMgr.GetTypeResourcePointer(stFontName.c_str());
		CGraphicText* pkResFont=static_cast<CGraphicText*>(pkRes);
		m_TextInstance.SetTextPointer(pkResFont);
	}
	void CTextLine::SetFontColor(DWORD dwColor)
	{
		m_TextInstance.SetColor(dwColor);
	}
	void CTextLine::SetLimitWidth(float fWidth)
	{
		m_TextInstance.SetLimitWidth(fWidth);
	}
	void CTextLine::SetText(const char * c_szText)
	{
		OnSetText(c_szText);
	}
	void CTextLine::GetTextSize(int* pnWidth, int* pnHeight)
	{
		m_TextInstance.GetTextSize(pnWidth, pnHeight);
	}
	const char * CTextLine::GetText()
	{
		return m_TextInstance.GetValueStringReference().c_str();
	}
	void CTextLine::ShowCursor()
	{
		m_TextInstance.ShowCursor();
	}
	void CTextLine::HideCursor()
	{
		m_TextInstance.HideCursor();
	}
	int CTextLine::GetCursorPosition()
	{
		long lx, ly;
		CWindow::GetMouseLocalPosition(lx, ly);
		return m_TextInstance.PixelPositionToCharacterPosition(lx);
	}

	void CTextLine::OnSetText(const char * c_szText)
	{
		m_TextInstance.SetValue(c_szText);
		m_TextInstance.Update();
	}

#if defined(__BL_CLIP_MASK__)
	void CTextLine::OnUpdate()
	{
		if (!IsShow())
			return;

		if (m_bEnableMask && m_pMaskWindow)
		{
			m_rMaskRect = m_pMaskWindow->GetRect();
			m_rMaskRect.left -= m_iMaskAdjustValue;
			m_rMaskRect.top -= m_iMaskAdjustValue;
			m_rMaskRect.right += m_iMaskAdjustValue;
			m_rMaskRect.bottom += m_iMaskAdjustValue;
		}

		m_TextInstance.Update();
	}
	void CTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render(m_bEnableMask ? &m_rMaskRect : NULL);
	}
#else
	void CTextLine::OnUpdate()
	{
		if (IsShow())
			m_TextInstance.Update();
	}
	void CTextLine::OnRender()
	{
		if (IsShow())
			m_TextInstance.Render();
	}
#endif

	void CTextLine::OnChangePosition()
	{
		// FOR_ARABIC_ALIGN
		//if (m_TextInstance.GetHorizontalAlign() == CGraphicTextInstance::HORIZONTAL_ALIGN_ARABIC)
		if( GetDefaultCodePage() == CP_ARABIC )
		{
			m_TextInstance.SetPosition(m_rect.right, m_rect.top);
		}
		else
		{
			m_TextInstance.SetPosition(m_rect.left, m_rect.top);
		}
	}

	void CTextLine::SetLineHeight(int iHeight) {
		m_TextInstance.SetLineHeight(iHeight);
		m_TextInstance.Update();
	}

	int CTextLine::GetLineHeight() {
		return m_TextInstance.GetLineHeight();
	}

	int CTextLine::GetTextLineCount() {
		return m_TextInstance.GetTextLineCount();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CNumberLine::CNumberLine(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;
	}
	CNumberLine::CNumberLine(CWindow * pParent) : CWindow(NULL)
	{
		m_strPath = "d:/ymir work/ui/game/taskbar/";
		m_iHorizontalAlign = HORIZONTAL_ALIGN_LEFT;
		m_dwWidthSummary = 0;

		m_pParent = pParent;
	}
	CNumberLine::~CNumberLine()
	{
		ClearNumber();
	}

	void CNumberLine::SetPath(const char * c_szPath)
	{
		m_strPath = c_szPath;
	}
	void CNumberLine::SetHorizontalAlign(int iType)
	{
		m_iHorizontalAlign = iType;
	}
	void CNumberLine::SetNumber(const char * c_szNumber)
	{
		if (0 == m_strNumber.compare(c_szNumber))
			return;

		ClearNumber();

		m_strNumber = c_szNumber;

		for (DWORD i = 0; i < m_strNumber.size(); ++i)
		{
			char cChar = m_strNumber[i];
			std::string strImageFileName;

			if (':' == cChar)
			{
				strImageFileName = m_strPath + "colon.sub";
			}
			else if ('?' == cChar)
			{
				strImageFileName = m_strPath + "questionmark.sub";
			}
			else if ('/' == cChar)
			{
				strImageFileName = m_strPath + "slash.sub";
			}
			else if ('%' == cChar)
			{
				strImageFileName = m_strPath + "percent.sub";
			}
			else if ('+' == cChar)
			{
				strImageFileName = m_strPath + "plus.sub";
			}
			else if ('m' == cChar)
			{
				strImageFileName = m_strPath + "m.sub";
			}
			else if ('g' == cChar)
			{
				strImageFileName = m_strPath + "g.sub";
			}
			else if ('p' == cChar)
			{
				strImageFileName = m_strPath + "p.sub";
			}
			else if (cChar >= '0' && cChar <= '9')
			{
				strImageFileName = m_strPath;
				strImageFileName += cChar;
				strImageFileName += ".sub";
			}
			else
				continue;

			if (!CResourceManager::Instance().IsFileExist(strImageFileName.c_str()))
				continue;

			CGraphicImage * pImage = (CGraphicImage *)CResourceManager::Instance().GetResourcePointer(strImageFileName.c_str());

			CGraphicImageInstance * pInstance = CGraphicImageInstance::New();
			pInstance->SetImagePointer(pImage);
			m_ImageInstanceVector.push_back(pInstance);

			m_dwWidthSummary += pInstance->GetWidth();
		}
	}

	void CNumberLine::ClearNumber()
	{
		m_ImageInstanceVector.clear();
		m_dwWidthSummary = 0;
		m_strNumber = "";
	}

	void CNumberLine::OnRender()
	{
		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->Render();
		}
	}

	void CNumberLine::OnChangePosition()
	{
		int ix = m_x;
		int iy = m_y;

		if (m_pParent)
		{
			ix = m_rect.left;
			iy = m_rect.top;
		}

		switch (m_iHorizontalAlign)
		{
			case HORIZONTAL_ALIGN_LEFT:
				break;
			case HORIZONTAL_ALIGN_CENTER:
				ix -= int(m_dwWidthSummary) / 2;
				break;
			case HORIZONTAL_ALIGN_RIGHT:
				ix -= int(m_dwWidthSummary);
				break;
		}

		for (DWORD i = 0; i < m_ImageInstanceVector.size(); ++i)
		{
			m_ImageInstanceVector[i]->SetPosition(ix, iy);
			ix += m_ImageInstanceVector[i]->GetWidth();
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

#ifdef ENABLE_12ZI
	CImageBox::CImageBox(PyObject * ppyObject): CWindow(ppyObject), m_fCoolTime(0.0f), m_fCoolTimeStart(0.0f)
#else
	CImageBox::CImageBox(PyObject * ppyObject) : CWindow(ppyObject)
#endif
	{
		m_pImageInstance = NULL;
	}
	CImageBox::~CImageBox()
	{
		OnDestroyInstance();
	}

	void CImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicImageInstance::New();
	}
	void CImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	BOOL CImageBox::LoadImage(CGraphicImageInstance* imageInstance)
	{
		if (!imageInstance)
			return FALSE;
		OnCreateInstance();
		m_pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(imageInstance->GetGraphicImagePointer()));
		if (m_pImageInstance->IsEmpty())
			return FALSE;
		SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
		UpdateRect();
		CGraphicImageInstance::Delete(imageInstance);
		return TRUE;
	}

	BOOL CImageBox::LoadImage(const char * c_szFileName)
	{
		if (!c_szFileName[0])
			return FALSE;

		OnCreateInstance();

		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource)
			return FALSE;
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_pImageInstance->IsEmpty())
			return FALSE;

		SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
		UpdateRect();

		if (m_rtl)
			m_pImageInstance->SetRTL();

		return TRUE;
	}

	void CImageBox::SetScale(float sx, float sy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicImageInstance*)m_pImageInstance)->SetScale(sx, sy);
		CWindow::SetSize(long(float(GetWidth())*sx), long(float(GetHeight())*sy));
	}

	void CImageBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	int CImageBox::GetWidth()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetWidth();
	}

	int CImageBox::GetHeight()
	{
		if (!m_pImageInstance)
			return 0;

		return m_pImageInstance->GetHeight();
	}

#if defined(__BL_CLIP_MASK__)
	void CImageBox::OnUpdate()
	{
		if (!m_pImageInstance)
			return;

		if (!IsShow())
			return;

		if (!m_bEnableMask)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
	}
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
#ifdef ENABLE_12ZI
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render(m_bEnableMask ? &m_rMaskRect : NULL);
			else
				// Removed the second parameter since your graphics source doesn't support it
				m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
#else
			m_pImageInstance->Render(m_bEnableMask ? &m_rMaskRect : NULL);
#endif
		}
	}
#else
	void CImageBox::OnUpdate()
	{
	}
	void CImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

#ifdef ENABLE_12ZI
		if (IsShow())
		{
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render();
			else
				m_pImageInstance->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
		}
#else
		if (IsShow())
			m_pImageInstance->Render();
#endif
	}
#endif

#ifdef ENABLE_12ZI
	void CImageBox::SetCoolTime(float fCoolTime)
	{
		m_fCoolTime = fCoolTime;
	}
#endif

	void CImageBox::SetCoolTimeStart(float fCoolTimeStart)
	{
		m_fCoolTimeStart = fCoolTimeStart;
	}

	void CImageBox::OnChangePosition()
	{
		if (!m_pImageInstance)
			return;

		m_pImageInstance->SetPosition(m_rect.left, m_rect.top);
	}

	void CImageBox::EnableFlagRTL()
	{
		m_rtl = true;
		if (m_pImageInstance)
			m_pImageInstance->SetRTL();
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	CMarkBox::CMarkBox(PyObject * ppyObject) : CWindow(ppyObject)
	{
		m_pMarkInstance = NULL;
	}

	CMarkBox::~CMarkBox()
	{
		OnDestroyInstance();
	}

	void CMarkBox::OnCreateInstance()
	{
		OnDestroyInstance();
		m_pMarkInstance = CGraphicMarkInstance::New();
	}

	void CMarkBox::OnDestroyInstance()
	{
		if (m_pMarkInstance)
		{
			CGraphicMarkInstance::Delete(m_pMarkInstance);
			m_pMarkInstance=NULL;
		}
	}

	void CMarkBox::LoadImage(const char * c_szFilename)
	{
		OnCreateInstance();

		m_pMarkInstance->SetImageFileName(c_szFilename);
		m_pMarkInstance->Load();
		SetSize(m_pMarkInstance->GetWidth(), m_pMarkInstance->GetHeight());

		UpdateRect();
	}

	void CMarkBox::SetScale(FLOAT fScale)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetScale(fScale);
	}

	void CMarkBox::SetIndex(UINT uIndex)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetIndex(uIndex);
	}

	void CMarkBox::SetDiffuseColor(float fr, float fg, float fb, float fa)
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetDiffuseColor(fr, fg, fb, fa);
	}

	void CMarkBox::OnUpdate()
	{
	}

	void CMarkBox::OnRender()
	{
		if (!m_pMarkInstance)
			return;

		if (IsShow())
			m_pMarkInstance->Render();
	}

	void CMarkBox::OnChangePosition()
	{
		if (!m_pMarkInstance)
			return;

		m_pMarkInstance->SetPosition(m_rect.left, m_rect.top);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CExpandedImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CExpandedImageBox", strlen("CExpandedImageBox"));
		return (s_dwType);
	}

	BOOL CExpandedImageBox::OnIsType(DWORD dwType)
	{
		if (CExpandedImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CExpandedImageBox::CExpandedImageBox(PyObject * ppyObject) : CImageBox(ppyObject)
	{
		m_bIsSharpened = false;
	}

	void CExpandedImageBox::SetSharpenMode(bool bEnable)
	{
		m_bIsSharpened = bEnable;
	}
	CExpandedImageBox::~CExpandedImageBox()
	{
		OnDestroyInstance();
	}

	void CExpandedImageBox::OnCreateInstance()
	{
		OnDestroyInstance();

		m_pImageInstance = CGraphicExpandedImageInstance::New();
	}
#ifdef ENABLE_CAPTCHA
    BOOL CExpandedImageBox::LoadImageFromBase64(const char* captcha)
    {
        if (!captcha)
            return FALSE;

        OnCreateInstance();

        std::string decodedData = GetCShieldCaptcha(captcha);

        size_t decodedSize = decodedData.size();
        if (decodedSize == 0)
        {
            // TraceError("LoadImageFromBase64: Decoded data is empty.");
            return FALSE;
        }

        CGraphicImageEx* pImage = new CGraphicImageEx("");
        if (!pImage)
        {
            return FALSE;
        }

        if (!pImage->LoadFromMemory(decodedSize, decodedData.data()))
        {
            delete pImage;
            return FALSE;
        }

        m_pImageInstance->SetImagePointer(pImage);

        SetSize(m_pImageInstance->GetWidth(), m_pImageInstance->GetHeight());
        UpdateRect();

        return TRUE;
    }
#endif
	void CExpandedImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicExpandedImageInstance::Delete((CGraphicExpandedImageInstance*)m_pImageInstance);
			m_pImageInstance=NULL;
		}
	}

	void CExpandedImageBox::SetScale(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(fx, fy);
		CWindow::SetSize(long(float(GetWidth())*fx), long(float(GetHeight())*fy));
	}
	void CExpandedImageBox::SetOrigin(float fx, float fy)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetOrigin(fx, fy);
	}
	void CExpandedImageBox::SetRotation(float fRotation)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRotation(fRotation);
	}
	void CExpandedImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		if (!m_pImageInstance)
			return;

		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}

	void CExpandedImageBox::SetRenderingMode(int iMode)
	{
		((CGraphicExpandedImageInstance*)m_pImageInstance)->SetRenderingMode(iMode);
	}

	void CExpandedImageBox::OnUpdate()
	{
	}
	void CExpandedImageBox::OnRender()
	{
		if (!m_pImageInstance)
			return;

		if (IsShow())
		{
			// 1. Force Point Filtering (Extreme Sharpening) before rendering
			if (m_bIsSharpened)
			{
#ifdef DIRECTX9
				STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
				STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
#else
				STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_POINT);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_POINT);
#endif
			}

			// 2. Render the image
#ifdef ENABLE_12ZI
			if (m_fCoolTime == 0.0f)
				m_pImageInstance->Render();
			else
				((CGraphicExpandedImageInstance*)m_pImageInstance)->RenderCoolTime((CTimer::Instance().GetCurrentSecond() - m_fCoolTimeStart) / m_fCoolTime);
#else
			m_pImageInstance->Render();
#endif

			// 3. Restore the default rendering state for other UI elements
			if (m_bIsSharpened)
			{
#ifdef DIRECTX9
				STATEMANAGER.SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
				STATEMANAGER.SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
#else
				STATEMANAGER.SetTextureStageState(0, D3DTSS_MINFILTER, D3DTEXF_NONE);
				STATEMANAGER.SetTextureStageState(0, D3DTSS_MAGFILTER, D3DTEXF_NONE);
#endif
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	DWORD CAniImageBox::Type()
	{
		static DWORD s_dwType = GetCRC32("CAniImageBox", strlen("CAniImageBox"));
		return (s_dwType);
	}

	BOOL CAniImageBox::OnIsType(DWORD dwType)
	{
		if (CAniImageBox::Type() == dwType)
			return TRUE;

		return FALSE;
	}

	CAniImageBox::CAniImageBox(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_bycurDelay(0),
			m_byDelay(4),
			m_bycurIndex(0)
	{
		m_ImageVector.clear();
	}
	CAniImageBox::~CAniImageBox()
	{
		for_each(m_ImageVector.begin(), m_ImageVector.end(), CGraphicExpandedImageInstance::DeleteExpandedImageInstance);
	}

	void CAniImageBox::SetDelay(int iDelay)
	{
		m_byDelay = iDelay;
	}

	#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
	void CAniImageBox::SetDiffuseColor(const D3DXCOLOR & color)
	{
		for (auto & elem : m_ImageVector)
			elem->SetDiffuseColor(color.r, color.g, color.b, color.a);
	}
	#endif

	void CAniImageBox::AppendImage(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;

		CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();
		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));

		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}

		m_ImageVector.push_back(pImageInstance);
		m_bycurIndex = rand() % m_ImageVector.size();
	}
	void CAniImageBox::AppendImageScale(const char * c_szFileName, float scale_x, float scale_y)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return;

		CGraphicExpandedImageInstance * pImageInstance = CGraphicExpandedImageInstance::New();

		pImageInstance->SetImagePointer(static_cast<CGraphicImage*>(pResource));
		pImageInstance->SetScale(scale_x, scale_y);
		if (pImageInstance->IsEmpty())
		{
			CGraphicExpandedImageInstance::Delete(pImageInstance);
			return;
		}

		m_ImageVector.push_back(pImageInstance);

		m_bycurIndex = rand() % m_ImageVector.size();
		//		SetSize(pImageInstance->GetWidth(), pImageInstance->GetHeight());
		//		UpdateRect();
	}

	struct FSetRenderingRect
	{
		float fLeft, fTop, fRight, fBottom;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingRect(fLeft, fTop, fRight, fBottom);
		}
	};
	void CAniImageBox::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		FSetRenderingRect setRenderingRect;
		setRenderingRect.fLeft = fLeft;
		setRenderingRect.fTop = fTop;
		setRenderingRect.fRight = fRight;
		setRenderingRect.fBottom = fBottom;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingRect);
	}

	struct FSetRenderingMode
	{
		int iMode;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetRenderingMode(iMode);
		}
	};
	void CAniImageBox::SetRenderingMode(int iMode)
	{
		FSetRenderingMode setRenderingMode;
		setRenderingMode.iMode = iMode;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), setRenderingMode);
	}

	void CAniImageBox::ResetFrame()
	{
		m_bycurIndex = 0;
	}

	void CAniImageBox::OnUpdate()
	{
		++m_bycurDelay;
		if (m_bycurDelay < m_byDelay)
			return;

		m_bycurDelay = 0;

		++m_bycurIndex;
		if (m_bycurIndex >= m_ImageVector.size())
		{
			m_bycurIndex = 0;

			OnEndFrame();
		}
	}
	void CAniImageBox::OnRender()
	{
		if (m_bycurIndex < m_ImageVector.size())
		{
			CGraphicExpandedImageInstance * pImage = m_ImageVector[m_bycurIndex];
			pImage->Render();
		}
	}

	struct FChangePosition
	{
		float fx, fy;
		void operator () (CGraphicExpandedImageInstance * pInstance)
		{
			pInstance->SetPosition(fx, fy);
		}
	};

	void CAniImageBox::OnChangePosition()
	{
		FChangePosition changePosition;
		changePosition.fx = m_rect.left;
		changePosition.fy = m_rect.top;
		for_each(m_ImageVector.begin(), m_ImageVector.end(), changePosition);
	}

	void CAniImageBox::OnEndFrame()
	{
		PyCallClassMemberFunc(m_poHandler, "OnEndFrame", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CButton::CButton(PyObject * ppyObject)
		:	CWindow(ppyObject),
			m_pcurVisual(NULL),
			m_bEnable(TRUE),
			m_isPressed(FALSE),
			m_isFlash(FALSE)
#if defined(__BL_MAILBOX__)
			, m_isNewFlash(false)
#endif
#ifdef ENABLE_12ZI
			, m_isEnableFlash(FALSE), m_isAlwaysTooltip(FALSE)
#endif
			, m_isFlashEx(FALSE)
	{
		CWindow::AddFlag(CWindow::FLAG_NOT_CAPTURE);
	}
	CButton::~CButton()
	{
	}

	BOOL CButton::SetUpVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_upVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_upVisual.IsEmpty())
			return FALSE;

		SetSize(m_upVisual.GetWidth(), m_upVisual.GetHeight());

		SetCurrentVisual(&m_upVisual);

		return TRUE;
	}
	BOOL CButton::SetOverVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_overVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_overVisual.IsEmpty())
			return FALSE;

		SetSize(m_overVisual.GetWidth(), m_overVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDownVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_downVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_downVisual.GetWidth(), m_downVisual.GetHeight());

		return TRUE;
	}
	BOOL CButton::SetDisableVisual(const char * c_szFileName)
	{
		CResource * pResource = CResourceManager::Instance().GetResourcePointer(c_szFileName);
		if (!pResource->IsType(CGraphicImage::Type()))
			return FALSE;

		m_disableVisual.SetImagePointer(static_cast<CGraphicImage*>(pResource));
		if (m_downVisual.IsEmpty())
			return FALSE;

		SetSize(m_disableVisual.GetWidth(), m_disableVisual.GetHeight());

		return TRUE;
	}

	const char * CButton::GetUpVisualFileName()
	{
		return m_upVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetOverVisualFileName()
	{
		return m_overVisual.GetGraphicImagePointer()->GetFileName();
	}
	const char * CButton::GetDownVisualFileName()
	{
		return m_downVisual.GetGraphicImagePointer()->GetFileName();
	}

	void CButton::Flash()
	{
		m_isFlash = TRUE;
	}

#ifdef ENABLE_12ZI
	void CButton::SetAlwaysToolTip(bool func)
	{
		m_isAlwaysTooltip = func;
	}
#endif

#if defined(__BL_MAILBOX__)
	void CButton::EnableFlash()
	{
		m_isNewFlash = true;
#ifdef ENABLE_12ZI
		m_isEnableFlash = TRUE;
#endif
	}
	void CButton::DisableFlash()
	{
		m_isNewFlash = false;
	}
#endif

	void CButton::Enable()
	{
		SetUp();
		m_bEnable = TRUE;
	}

	void CButton::FlashEx()
	{
		m_isFlashEx = TRUE;
	}

	void CButton::Disable()
	{
		m_bEnable = FALSE;
		if (!m_disableVisual.IsEmpty())
			SetCurrentVisual(&m_disableVisual);
	}

	BOOL CButton::IsDisable()
	{
		return m_bEnable;
	}

	void CButton::SetUp()
	{
		SetCurrentVisual(&m_upVisual);
		m_isPressed = FALSE;
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}
	void CButton::Up()
	{
		if (IsIn())
			SetCurrentVisual(&m_overVisual);
		else
			SetCurrentVisual(&m_upVisual);

		PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
	}
	void CButton::Over()
	{
		SetCurrentVisual(&m_overVisual);
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}
	void CButton::Down()
	{
		m_isPressed = TRUE;
		SetCurrentVisual(&m_downVisual);
		PyCallClassMemberFunc(m_poHandler, "DownEvent", BuildEmptyTuple());
	}

#if defined(__BL_CLIP_MASK__)
	void CButton::OnUpdate()
	{
		if (!m_bEnableMask)
			return;

		if (!m_pMaskWindow)
			return;

		m_rMaskRect = m_pMaskWindow->GetRect();
	}
	void CButton::OnRender()
	{
		if (!IsShow())
			return;

#ifdef ENABLE_12ZI
		if (!IsIn())
		{
			if (m_isAlwaysTooltip)
				return;
		}
#endif

		if (m_pcurVisual)
		{
#if defined(__BL_MAILBOX__)
			if ((m_isFlash || m_isNewFlash) && !IsIn() && !IsPressed())
			{
				if (int(timeGetTime() / 500) % 2)
				{
					if (m_isNewFlash && !m_overVisual.IsEmpty())
						SetCurrentVisual(&m_overVisual);
					else
						return;
				}
				else
				{
					if (m_isNewFlash && !m_upVisual.IsEmpty())
						SetCurrentVisual(&m_upVisual);
				}
			}
#ifdef ENABLE_12ZI
			if (m_isEnableFlash && !IsIn())
			{
				if (int(timeGetTime() / 500) % 2)
					Over();
				if (int(timeGetTime() / (500 - 15)) % 2)
					Down();
			}
#endif
#else
			if (m_isFlash)
			if (!IsIn())
			if (int(timeGetTime() / 500) % 2)
			{
				return;
			}
#endif

			m_pcurVisual->Render(m_bEnableMask ? &m_rMaskRect : NULL);
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}
#else
	void CButton::OnUpdate()
	{
	}
	void CButton::OnRender()
	{
		if (!IsShow())
			return;

#ifdef ENABLE_12ZI
		if (!IsIn())
		{
			if (m_isAlwaysTooltip)
				return;
		}
#endif

		if (m_pcurVisual)
		{
#if defined(__BL_MAILBOX__)
			if ((m_isFlash || m_isNewFlash) && !IsIn() && !IsPressed())
			{
				if (int(timeGetTime() / 500) % 2)
				{
					if (m_isNewFlash && !m_overVisual.IsEmpty())
						SetCurrentVisual(&m_overVisual);
					else
						return;
				}
				else
				{
					if (m_isNewFlash && !m_upVisual.IsEmpty())
						SetCurrentVisual(&m_upVisual);
				}
			}
#ifdef ENABLE_12ZI
			if (m_isEnableFlash && !IsIn())
			{
				if (int(timeGetTime() / 500) % 2)
					Over();
				if (int(timeGetTime() / (500 - 15)) % 2)
					Down();
			}
#endif
#else
			if (m_isFlash)
			if (!IsIn())
			if (int(timeGetTime() / 500)%2)
			{
				return;
			}
#endif
			if (m_isFlashEx && !IsIn())
			{
				if (int(timeGetTime() / 500) % 2)
					Over();
				if (int(timeGetTime() / (500 - 15)) % 2)
					Down();
			}
			m_pcurVisual->Render();
		}

		PyCallClassMemberFunc(m_poHandler, "OnRender", BuildEmptyTuple());
	}
#endif
	void CButton::OnChangePosition()
	{
		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		m_isPressed = TRUE;
		Down();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonDoubleClick()
	{
		if (!IsEnable())
			return TRUE;

		OnMouseLeftButtonDown();

		return TRUE;
	}
	BOOL CButton::OnMouseLeftButtonUp()
	{
		if (!IsEnable())
			return TRUE;
		if (!IsPressed())
			return TRUE;

		m_isPressed = FALSE;
		Up();

		return TRUE;
	}
	void CButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		Over();
		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		SetUp();
		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CButton", strlen("CButton"));
		return (s_dwType);
	}
	BOOL CButton::OnIsType(DWORD dwType)
	{
		if (CButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
	void CButton::SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom)
	{
		m_upVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_overVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
		m_downVisual.SetRenderingRect(fLeft, fTop, fRight, fBottom);
	}
	void CButton::SetCurrentVisual(CGraphicExpandedImageInstance* pVisual)
#else
	void CButton::SetCurrentVisual(CGraphicImageInstance* pVisual)
#endif
	{
		m_pcurVisual = pVisual;
		m_pcurVisual->SetPosition(m_rect.left, m_rect.top);
	}

	BOOL CButton::IsEnable()
	{
		return m_bEnable;
	}

	BOOL CButton::IsPressed()
	{
		return m_isPressed;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CRadioButton::CRadioButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CRadioButton::~CRadioButton()
	{
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CRadioButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CRadioButton", strlen("CRadioButton"));
		return (s_dwType);
	}
	BOOL CRadioButton::OnIsType(DWORD dwType)
	{
		if (CRadioButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif

	BOOL CRadioButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (!m_isPressed)
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "CallEvent", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CRadioButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}
	void CRadioButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CRadioButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CToggleButton::CToggleButton(PyObject * ppyObject) : CButton(ppyObject)
	{
	}
	CToggleButton::~CToggleButton()
	{
	}

#ifdef ENABLE_NEW_GAMEOPTION
	DWORD CToggleButton::Type()
	{
		static DWORD s_dwType = GetCRC32("CToggleButton", strlen("CToggleButton"));
		return (s_dwType);
	}
	BOOL CToggleButton::OnIsType(DWORD dwType)
	{
		if (CToggleButton::Type() == dwType)
			return TRUE;
		return FALSE;
	}
#endif

	BOOL CToggleButton::OnMouseLeftButtonDown()
	{
		if (!IsEnable())
			return TRUE;

		if (m_isPressed)
		{
			SetUp();
			if (IsIn())
				SetCurrentVisual(&m_overVisual);
			else
				SetCurrentVisual(&m_upVisual);
			PyCallClassMemberFunc(m_poHandler, "OnToggleUp", BuildEmptyTuple());
		}
		else
		{
			Down();
			PyCallClassMemberFunc(m_poHandler, "OnToggleDown", BuildEmptyTuple());
		}

		return TRUE;
	}
	BOOL CToggleButton::OnMouseLeftButtonUp()
	{
		return TRUE;
	}

	void CToggleButton::OnMouseOverIn()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_overVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "ShowToolTip", BuildEmptyTuple());
	}
	void CToggleButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		if (!m_isPressed)
		{
			SetCurrentVisual(&m_upVisual);
		}

		PyCallClassMemberFunc(m_poHandler, "HideToolTip", BuildEmptyTuple());
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////
	///////////////////////////////////////////////////////////////////////////////////////////////

	CDragButton::CDragButton(PyObject * ppyObject) : CButton(ppyObject)
	{
		CWindow::RemoveFlag(CWindow::FLAG_NOT_CAPTURE);
		m_restrictArea.left = 0;
		m_restrictArea.top = 0;
		m_restrictArea.right = 0;
		m_restrictArea.bottom = 0;
	}
	CDragButton::~CDragButton()
	{
	}

	void CDragButton::SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight)
	{
		m_restrictArea.left = ix;
		m_restrictArea.top = iy;
		m_restrictArea.right = ix + iwidth;
		m_restrictArea.bottom = iy + iheight;
	}

	void CDragButton::OnChangePosition()
	{
		long rightLimit = (m_restrictArea.right > 0) ? m_restrictArea.right : CWindowManager::Instance().GetScreenWidth();
		long bottomLimit = (m_restrictArea.bottom > 0) ? m_restrictArea.bottom : CWindowManager::Instance().GetScreenHeight();

		m_x = max(m_x, m_restrictArea.left);
		m_y = max(m_y, m_restrictArea.top);
		m_x = min(m_x, max(0L, rightLimit - m_lWidth));
		m_y = min(m_y, max(0L, bottomLimit - m_lHeight));

		m_rect.left = m_x;
		m_rect.top = m_y;

		if (m_pParent)
		{
			const RECT & c_rRect = m_pParent->GetRect();
			m_rect.left += c_rRect.left;
			m_rect.top += c_rRect.top;
		}

		m_rect.right = m_rect.left + m_lWidth;
		m_rect.bottom = m_rect.top + m_lHeight;

		std::for_each(m_pChildList.begin(), m_pChildList.end(), std::mem_fn(&CWindow::UpdateRect));

		if (m_pcurVisual)
			m_pcurVisual->SetPosition(m_rect.left, m_rect.top);

		if (IsPressed())
			PyCallClassMemberFunc(m_poHandler, "OnMove", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverIn()
	{
		if (!IsEnable())

			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverIn", BuildEmptyTuple());
	}

	void CDragButton::OnMouseOverOut()
	{
		if (!IsEnable())
			return;

		CButton::OnMouseOverIn();
		PyCallClassMemberFunc(m_poHandler, "OnMouseOverOut", BuildEmptyTuple());
	}

#ifdef ENABLE_MINI_GAME_YUTNORI
	// ==========================================
	// CMoveImageBox
	// ==========================================
	CMoveImageBox::CMoveImageBox(PyObject * ppyObject) :
		CImageBox(ppyObject),
		m_v2SrcPos(0.0f, 0.0f), m_v2DstPos(0.0f, 0.0f), m_v2NextPos(0.0f, 0.0f),
		m_v2Direction(0.0f, 0.0f), m_v2NextDistance(0.0f, 0.0f), m_fDistance(0.0f),
		m_fMoveSpeed(10.0f), m_bIsMove(false), m_bMoveAll(false)
	{
	}

	CMoveImageBox::~CMoveImageBox() { OnDestroyInstance(); }

	void CMoveImageBox::OnCreateInstance()
	{
		OnDestroyInstance();
		m_pImageInstance = CGraphicImageInstance::New();
	}

	void CMoveImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicImageInstance::Delete(m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	DWORD CMoveImageBox::Type() { return GetCRC32("CMoveImageBox", strlen("CMoveImageBox")); }
	BOOL CMoveImageBox::OnIsType(DWORD dwType) { return (CMoveImageBox::Type() == dwType); }

	void CMoveImageBox::OnEndMove() { PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple()); }

	void CMoveImageBox::SetMovePosition(float fDstX, float fDstY)
	{
		if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
		{
			m_v2SrcPos.x = m_rect.left; m_v2SrcPos.y = m_rect.top;
			m_v2DstPos.x = fDstX; m_v2DstPos.y = fDstY;

			D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);
			m_fDistance = (m_v2Direction.x * m_v2Direction.x + m_v2Direction.y * m_v2Direction.y);
			D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

			if (m_pImageInstance && m_v2SrcPos != m_v2NextPos)
			{
				float fDist = sqrtf(m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
				m_v2NextPos = m_v2Direction * fDist;
				
				if(m_bMoveAll) SetPosition(m_v2NextPos.x, m_v2NextPos.y);
				else m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			}
		}
	}

	void CMoveImageBox::SetMoveSpeed(float fSpeed) { m_fMoveSpeed = fSpeed; }
	void CMoveImageBox::MoveStart() { m_bIsMove = true; m_v2NextPos = m_v2SrcPos; }
	void CMoveImageBox::MoveStop() { m_bIsMove = false; }
	bool CMoveImageBox::GetMove() { return m_bIsMove; }

	void CMoveImageBox::OnUpdate()
	{
		if (!m_pImageInstance) return;

		if (IsShow() && GetMove())
		{
			// FIX: Use native math operators instead of taking addresses of temporary variables
			m_v2NextPos += (m_v2Direction * m_fMoveSpeed);
			m_v2NextDistance = m_v2NextPos - m_v2SrcPos;

			float fNextDistance = (m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
			if (fNextDistance >= m_fDistance)
			{
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}

			if(m_bMoveAll) SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			else m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
		
		if (!m_poHandler || !IsShow()) return;
		PyCallClassMemberFunc(m_poHandler, "OnUpdate", BuildEmptyTuple());
	}

	void CMoveImageBox::OnRender()
	{
		if (m_pImageInstance && IsShow()) m_pImageInstance->Render();
	}

	// ==========================================
	// CMoveTextLine
	// ==========================================
	CMoveTextLine::CMoveTextLine(PyObject * ppyObject) : CTextLine(ppyObject),
		m_v2SrcPos(0.0f, 0.0f), m_v2DstPos(0.0f, 0.0f), m_v2NextPos(0.0f, 0.0f),
		m_v2Direction(0.0f, 0.0f), m_v2NextDistance(0.0f, 0.0f), m_fDistance(0.0f),
		m_fMoveSpeed(10.0f), m_bIsMove(false) {}

	CMoveTextLine::~CMoveTextLine() { m_TextInstance.Destroy(); }

	DWORD CMoveTextLine::Type() { return GetCRC32("CMoveTextLine", strlen("CMoveTextLine")); }
	BOOL CMoveTextLine::OnIsType(DWORD dwType) { return (CMoveTextLine::Type() == dwType); }

	void CMoveTextLine::SetMoveSpeed(float fSpeed) { m_fMoveSpeed = fSpeed; }
	bool CMoveTextLine::GetMove() { return m_bIsMove; }
	void CMoveTextLine::MoveStart() { m_bIsMove = true; m_v2NextPos = m_v2SrcPos; }
	void CMoveTextLine::MoveStop() { m_bIsMove = false; }
	void CMoveTextLine::OnEndMove() { PyCallClassMemberFunc(m_poHandler, "OnEndMove", BuildEmptyTuple()); }
	void CMoveTextLine::OnChangePosition() { m_TextInstance.SetPosition((GetDefaultCodePage() == CP_1256) ? m_rect.right : m_rect.left, m_rect.top); }

	void CMoveTextLine::SetMovePosition(float fDstX, float fDstY)
	{
		if (fDstX != m_v2DstPos.x || fDstY != m_v2DstPos.y || m_rect.left != m_v2SrcPos.x || m_rect.top != m_v2SrcPos.y)
		{
			m_v2SrcPos.x = m_rect.left; m_v2SrcPos.y = m_rect.top;
			m_v2DstPos.x = fDstX; m_v2DstPos.y = fDstY;

			D3DXVec2Subtract(&m_v2Direction, &m_v2DstPos, &m_v2SrcPos);
			m_fDistance = (m_v2Direction.y * m_v2Direction.y + m_v2Direction.x * m_v2Direction.x);
			D3DXVec2Normalize(&m_v2Direction, &m_v2Direction);

			if (m_v2SrcPos != m_v2NextPos)
			{
				float fDist = sqrtf(m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
				m_v2NextPos = m_v2Direction * fDist;
				m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			}
		}
	}

	void CMoveTextLine::OnUpdate()
	{
		if (IsShow() && GetMove())
		{
			// FIX: Use native math operators
			m_v2NextPos += (m_v2Direction * m_fMoveSpeed);
			m_v2NextDistance = m_v2NextPos - m_v2SrcPos;

			float fNextDistance = m_v2NextDistance.y * m_v2NextDistance.y + m_v2NextDistance.x * m_v2NextDistance.x;
			if (fNextDistance >= m_fDistance)
			{
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}
			m_TextInstance.SetPosition(m_v2NextPos.x, m_v2NextPos.y);
			m_TextInstance.Update();
		}
	}

	void CMoveTextLine::OnRender() { if (IsShow()) m_TextInstance.Render(); }

	// ==========================================
	// CMoveScaleImageBox (With native scaling and crash fix)
	// ==========================================
	CMoveScaleImageBox::CMoveScaleImageBox(PyObject * ppyObject) :
		CMoveImageBox(ppyObject), m_fMaxScale(1.0f), m_fMaxScaleRate(1.0f),
		m_fScaleDistance(0.0f), m_fAdditionalScale(0.0f), m_v2CurScale(1.0f, 1.0f) {}

	CMoveScaleImageBox::~CMoveScaleImageBox() { OnDestroyInstance(); }

	void CMoveScaleImageBox::OnCreateInstance()
	{
		OnDestroyInstance();
		// Use native expanded image instance for safe scale transforms
		m_pImageInstance = CGraphicExpandedImageInstance::New(); 
	}

	void CMoveScaleImageBox::OnDestroyInstance()
	{
		if (m_pImageInstance)
		{
			CGraphicExpandedImageInstance::Delete((CGraphicExpandedImageInstance*)m_pImageInstance);
			m_pImageInstance = NULL;
		}
	}

	DWORD CMoveScaleImageBox::Type() { return GetCRC32("CMoveScaleImageBox", strlen("CMoveScaleImageBox")); }
	BOOL CMoveScaleImageBox::OnIsType(DWORD dwType) { return (CMoveScaleImageBox::Type() == dwType); }

	void CMoveScaleImageBox::SetMaxScale(float fMaxScale) { m_fMaxScale = fMaxScale; }

	void CMoveScaleImageBox::SetMaxScaleRate(float fMaxScaleRate)
	{
		m_fMaxScaleRate = fMaxScaleRate;
		float fDistanceRate = m_fDistance * fMaxScaleRate;
		m_fScaleDistance = fDistanceRate;
		m_v2CurScale = D3DXVECTOR2(1.0f, 1.0f); // Default scale base
		float fDiffScale = m_fMaxScale - m_v2CurScale.x;
		
		// CRITICAL CRASH FIX: Protect against Division by Zero when an animation is skipped or instantly completes
		float fDivisor = sqrtf(fDistanceRate) / m_fMoveSpeed;
		if (fDivisor > 0.0f)
			m_fAdditionalScale = fDiffScale / fDivisor;
		else
			m_fAdditionalScale = 0.0f;
	}

	void CMoveScaleImageBox::OnUpdate()
	{
		if (!m_pImageInstance) return;

		if (IsShow() && GetMove())
		{
			// FIX: Use native math operators
			m_v2NextPos += (m_v2Direction * m_fMoveSpeed);
			m_v2NextDistance = m_v2NextPos - m_v2SrcPos;

			float fNextDistance = (m_v2NextDistance.x * m_v2NextDistance.x + m_v2NextDistance.y * m_v2NextDistance.y);
			if (m_fScaleDistance < fNextDistance)
				m_fAdditionalScale *= -1.0f;
			
			// FIX: Prevent D3DXVECTOR2 temporary r-value address errors
			D3DXVECTOR2 v2NewScale = m_v2CurScale + D3DXVECTOR2(m_fAdditionalScale, m_fAdditionalScale);
			
			if (m_fMaxScale < v2NewScale.x)
				v2NewScale = D3DXVECTOR2(m_fMaxScale, m_fMaxScale);

			if (m_v2CurScale.x > v2NewScale.x)
				v2NewScale = m_v2CurScale;

			((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(v2NewScale.x, v2NewScale.y);

			if (fNextDistance >= m_fDistance)
			{
				((CGraphicExpandedImageInstance*)m_pImageInstance)->SetScale(m_v2CurScale.x, m_v2CurScale.y);
				m_v2NextPos = m_v2DstPos;
				MoveStop();
				OnEndMove();
			}

			m_pImageInstance->SetPosition(m_v2NextPos.x, m_v2NextPos.y);
		}
	}
#endif

};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
