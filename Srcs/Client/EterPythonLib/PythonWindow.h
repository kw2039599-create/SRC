#pragma once

#include "../eterBase/Utils.h"
#ifdef ENABLE_TWEENY_LIBRARY
#include <optional.hpp> //Boost kütüphanesinden de yararlanıyoruz.
#include <tweeny-3.2.0.h>
#endif
#include "../UserInterface/Locale_inc.h"

namespace UI
{
	class CWindow
	{
		public:
			typedef std::list<CWindow *> TWindowContainer;

			static DWORD Type();
			BOOL IsType(DWORD dwType);

			enum EHorizontalAlign
			{
				HORIZONTAL_ALIGN_LEFT = 0,
				HORIZONTAL_ALIGN_CENTER = 1,
				HORIZONTAL_ALIGN_RIGHT = 2,
			};

			enum EVerticalAlign
			{
				VERTICAL_ALIGN_TOP = 0,
				VERTICAL_ALIGN_CENTER = 1,
				VERTICAL_ALIGN_BOTTOM = 2,
			};

			enum EFlags
			{
				FLAG_MOVABLE			= (1 <<  0),
				FLAG_LIMIT				= (1 <<  1),
				FLAG_SNAP				= (1 <<  2),
				FLAG_DRAGABLE			= (1 <<  3),
				FLAG_ATTACH				= (1 <<  4),
				FLAG_RESTRICT_X			= (1 <<  5),
				FLAG_RESTRICT_Y			= (1 <<  6),
				FLAG_NOT_CAPTURE		= (1 <<  7),
				FLAG_FLOAT				= (1 <<  8),
				FLAG_NOT_PICK			= (1 <<  9),
				FLAG_IGNORE_SIZE		= (1 << 10),
				FLAG_RTL				= (1 << 11),	// Right-to-left
#ifdef ENABLE_TWEENY_LIBRARY
				FLAG_ANIMATED_BOARD = (1 << 12),
#endif
			};

		public:
			CWindow(PyObject * ppyObject);
			virtual ~CWindow();

			void			AddChild(CWindow * pWin);

			void			Clear();
			void			DestroyHandle();
			void			Update();
			void			Render();

			void			SetName(const char * c_szName);
			const char *	GetName()		{ return m_strName.c_str(); }
			void			SetSize(long width, long height);
			long			GetWidth()		{ return m_lWidth; }
			long			GetHeight()		{ return m_lHeight; }
#ifdef ENABLE_TWEENY_LIBRARY
			void			SetScale(float x, float y);
#endif
			void			SetHorizontalAlign(DWORD dwAlign);
			void			SetVerticalAlign(DWORD dwAlign);
			void			SetPosition(long x, long y);
			void			GetPosition(long * plx, long * ply);
			long			GetPositionX( void ) const		{ return m_x; }
			long			GetPositionY( void ) const		{ return m_y; }
			RECT &			GetRect()		{ return m_rect; }
			void			GetLocalPosition(long & rlx, long & rly);
			void			GetMouseLocalPosition(long & rlx, long & rly);
			long			UpdateRect();

			RECT &			GetLimitBias()	{ return m_limitBiasRect; }
			void			SetLimitBias(long l, long r, long t, long b) { m_limitBiasRect.left = l, m_limitBiasRect.right = r, m_limitBiasRect.top = t, m_limitBiasRect.bottom = b; }

			void			Show();
			void			Hide();
			bool			IsShow() { return m_bShow; }
			bool			IsRendering();

			bool			HasParent()		{ return m_pParent ? true : false; }
			bool			HasChild()		{ return m_pChildList.empty() ? false : true; }
			int				GetChildCount()	{ return m_pChildList.size(); }

			CWindow *		GetRoot();
			CWindow *		GetParent();
			bool			IsChild(CWindow * pWin);
			void			DeleteChild(CWindow * pWin);
			void			SetTop(CWindow * pWin);

			bool			IsIn(long x, long y);
			bool			IsIn();
			CWindow *		PickWindow(long x, long y);
			CWindow *		PickTopWindow(long x, long y);

			void			__RemoveReserveChildren();

			void			AddFlag(DWORD flag)		{ SET_BIT(m_dwFlag, flag);		}
			void			RemoveFlag(DWORD flag)	{ REMOVE_BIT(m_dwFlag, flag);	}
			bool			IsFlag(DWORD flag)		{ return (m_dwFlag & flag) ? true : false;	}
			/////////////////////////////////////

			virtual void	OnRender();
			virtual void	OnUpdate();
			virtual void	OnChangePosition(){}

			virtual void	OnSetFocus();
			virtual void	OnKillFocus();

			virtual void	OnMouseDrag(long lx, long ly);
			virtual void	OnMouseOverIn();
			virtual void	OnMouseOverOut();
			virtual void	OnMouseOver();
			virtual void	OnDrop();
			virtual void	OnTop();
			virtual void	OnIMEUpdate();

			virtual void	OnMoveWindow(long x, long y);

			///////////////////////////////////////

			BOOL			RunIMETabEvent();
			BOOL			RunIMEReturnEvent();
			BOOL			RunIMEKeyDownEvent(int ikey);

			CWindow *		RunKeyDownEvent(int ikey);
			BOOL			RunKeyUpEvent(int ikey);
			BOOL			RunPressEscapeKeyEvent();
			BOOL			RunPressExitKeyEvent();

			virtual BOOL	OnIMETabEvent();
			virtual BOOL	OnIMEReturnEvent();
			virtual BOOL	OnIMEKeyDownEvent(int ikey);

			virtual BOOL	OnIMEChangeCodePage();
			virtual BOOL	OnIMEOpenCandidateListEvent();
			virtual BOOL	OnIMECloseCandidateListEvent();
			virtual BOOL	OnIMEOpenReadingWndEvent();
			virtual BOOL	OnIMECloseReadingWndEvent();

			virtual BOOL	OnMouseLeftButtonDown();
			virtual BOOL	OnMouseLeftButtonUp();
			virtual BOOL	OnMouseLeftButtonDoubleClick();
			virtual BOOL	OnMouseRightButtonDown();
			virtual BOOL	OnMouseRightButtonUp();
			virtual BOOL	OnMouseRightButtonDoubleClick();
			virtual BOOL	OnMouseMiddleButtonDown();
			virtual BOOL	OnMouseMiddleButtonUp();
#ifdef ENABLE_MOUSEWHEEL_EVENT
			virtual BOOL	OnMouseWheel(short wDelta);
#endif
			virtual BOOL	OnKeyDown(int ikey);
			virtual BOOL	OnKeyUp(int ikey);
			virtual BOOL	OnPressEscapeKey();
			virtual BOOL	OnPressExitKey();
			///////////////////////////////////////

			virtual void	SetColor(DWORD dwColor){}
			virtual BOOL	OnIsType(DWORD dwType);
			/////////////////////////////////////

			virtual BOOL	IsWindow() { return TRUE; }
#if defined(__BL_CLIP_MASK__)
			virtual void	SetClippingMaskRect(const RECT& rMask);
			virtual void	SetClippingMaskWindow(CWindow* pMaskWindow, int iAdjustValue);
#endif
#if defined(__BL_MOUSE_WHEEL_TOP_WINDOW__)
			virtual bool	OnMouseWheelButtonUp();
			virtual bool	OnMouseWheelButtonDown();
#endif
			/////////////////////////////////////

		protected:
			std::string			m_strName;

			EHorizontalAlign	m_HorizontalAlign;
			EVerticalAlign		m_VerticalAlign;
			long				m_x, m_y;
			long				m_lWidth, m_lHeight;
			RECT				m_rect;
			RECT				m_limitBiasRect;

			bool				m_bMovable;
			bool				m_bShow;

			DWORD				m_dwFlag;

			PyObject *			m_poHandler;

			CWindow	*			m_pParent;
			TWindowContainer	m_pChildList;

			BOOL				m_isUpdatingChildren;
			TWindowContainer	m_pReserveChildList;
#if defined(__BL_CLIP_MASK__)
			bool				m_bEnableMask;
			CWindow*			m_pMaskWindow;
			RECT				m_rMaskRect;
			int					m_iMaskAdjustValue;
#endif
#ifdef ENABLE_TWEENY_LIBRARY
			D3DXVECTOR2			m_v2Scale;
			D3DXMATRIX			m_matScaling; //Yeni kütüphane tanımları - Adalet for Nileya
			std::experimental::optional<tweeny::tween<float>> m_sizeAnimation;
#endif
#ifdef _DEBUG
		public:
			DWORD				DEBUG_dwCounter;
#endif
	};

	class CLayer : public CWindow
	{
		public:
			CLayer(PyObject * ppyObject) : CWindow(ppyObject) {}
			virtual ~CLayer() {}

			BOOL IsWindow() { return FALSE; }
	};

	class CBox : public CWindow
	{
		public:
			CBox(PyObject * ppyObject);
			virtual ~CBox();

			void SetColor(DWORD dwColor);

		protected:
			void OnRender();

		protected:
			DWORD m_dwColor;
	};

	class CBar : public CWindow
	{
		public:
			CBar(PyObject * ppyObject);
			virtual ~CBar();

			void SetColor(DWORD dwColor);

		protected:
			void OnRender();

		protected:
			DWORD m_dwColor;
	};

	class CLine : public CWindow
	{
		public:
			CLine(PyObject * ppyObject);
			virtual ~CLine();

			void SetColor(DWORD dwColor);

		protected:
			void OnRender();

		protected:
			DWORD m_dwColor;
	};

	class CBar3D : public CWindow
	{
		public:
			static DWORD Type();

		public:
			CBar3D(PyObject * ppyObject);
			virtual ~CBar3D();

			void SetColor(DWORD dwLeft, DWORD dwRight, DWORD dwCenter);

		protected:
			void OnRender();

		protected:
			DWORD m_dwLeftColor;
			DWORD m_dwRightColor;
			DWORD m_dwCenterColor;
	};

	// Text
	class CTextLine : public CWindow
	{
		public:
			CTextLine(PyObject * ppyObject);
			virtual ~CTextLine();

			void SetMax(int iMax);
			void SetHorizontalAlign(int iType);
			void SetVerticalAlign(int iType);
			void SetSecret(BOOL bFlag);
			void SetOutline(BOOL bFlag);
			void SetFeather(BOOL bFlag);
			void SetMultiLine(BOOL bFlag);
			void SetFontName(const char * c_szFontName);
			void SetFontColor(DWORD dwColor);
			void SetLimitWidth(float fWidth);

			void ShowCursor();
			void HideCursor();
			int GetCursorPosition();

			void SetText(const char * c_szText);
			const char * GetText();

			void GetTextSize(int* pnWidth, int* pnHeight);
			void SetLineHeight(int iHeight);
			int GetLineHeight();
			int GetTextLineCount();

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();

			virtual void OnSetText(const char * c_szText);

		protected:
			CGraphicTextInstance m_TextInstance;
	};

	class CNumberLine : public CWindow
	{
		public:
			CNumberLine(PyObject * ppyObject);
			CNumberLine(CWindow * pParent);
			virtual ~CNumberLine();

			void SetPath(const char * c_szPath);
			void SetHorizontalAlign(int iType);
			void SetNumber(const char * c_szNumber);

		protected:
			void ClearNumber();
			void OnRender();
			void OnChangePosition();

		protected:
			std::string m_strPath;
			std::string m_strNumber;
			std::vector<CGraphicImageInstance *> m_ImageInstanceVector;

			int m_iHorizontalAlign;
			DWORD m_dwWidthSummary;
	};

	// Image
	class CImageBox : public CWindow
	{
		public:
			CImageBox(PyObject * ppyObject);
			virtual ~CImageBox();

			BOOL LoadImage(CGraphicImageInstance* imageInstance);

			BOOL LoadImage(const char * c_szFileName);
			void SetDiffuseColor(float fr, float fg, float fb, float fa);
			void SetScale(float sx, float sy);
			int GetWidth();
			int GetHeight();
#ifdef ENABLE_12ZI
			void SetCoolTime(float fCoolTime);
			void SetCoolTimeStart(float fCoolTimeStart);
#endif
			void EnableFlagRTL();

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			void OnChangePosition();
#ifdef ENABLE_12ZI
			float m_fCoolTime;
			float m_fCoolTimeStart;
#endif
		protected:
			CGraphicImageInstance * m_pImageInstance;
			bool m_rtl = false;
	};
	class CMarkBox : public CWindow
	{
		public:
			CMarkBox(PyObject * ppyObject);
			virtual ~CMarkBox();

			void LoadImage(const char * c_szFilename);
			void SetDiffuseColor(float fr, float fg, float fb, float fa);
			void SetIndex(UINT uIndex);
			void SetScale(FLOAT fScale);

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			void OnChangePosition();
		protected:
			CGraphicMarkInstance * m_pMarkInstance;
	};
	class CExpandedImageBox : public CImageBox
	{
		public:
			static DWORD Type();

		public:
			CExpandedImageBox(PyObject * ppyObject);
			virtual ~CExpandedImageBox();

			void SetScale(float fx, float fy);
			void SetOrigin(float fx, float fy);
			void SetRotation(float fRotation);
			void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
			void SetRenderingMode(int iMode);
			void SetSharpenMode(bool bEnable);
#ifdef ENABLE_CAPTCHA
			BOOL LoadImageFromBase64(const char* captcha);
#endif

		protected:
			void OnCreateInstance();
			void OnDestroyInstance();
			bool m_bIsSharpened;

			virtual void OnUpdate();
			virtual void OnRender();

			BOOL OnIsType(DWORD dwType);
	};
	class CAniImageBox : public CWindow
	{
		public:
			static DWORD Type();

		public:
			CAniImageBox(PyObject * ppyObject);
			virtual ~CAniImageBox();

			void SetDelay(int iDelay);
			#ifdef ENABLE_HIGHLIGHT_NEW_ITEM
			void SetDiffuseColor(const D3DXCOLOR& color);
			#endif
			void AppendImage(const char * c_szFileName);
			void AppendImageScale(const char * c_szFileName, float scale_x, float scale_y);
			void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
			void SetRenderingMode(int iMode);

			void ResetFrame();

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();
			virtual void OnEndFrame();

			BOOL OnIsType(DWORD dwType);

		protected:
			BYTE m_bycurDelay;
			BYTE m_byDelay;
			BYTE m_bycurIndex;
			std::vector<CGraphicExpandedImageInstance*> m_ImageVector;
	};

	// Button
	class CButton : public CWindow
	{
		public:
			CButton(PyObject * ppyObject);
			virtual ~CButton();

			BOOL SetUpVisual(const char * c_szFileName);
			BOOL SetOverVisual(const char * c_szFileName);
			BOOL SetDownVisual(const char * c_szFileName);
			BOOL SetDisableVisual(const char * c_szFileName);
#ifdef ENABLE_NEW_GAMEOPTION
			static DWORD Type();
			void SetRenderingRect(float fLeft, float fTop, float fRight, float fBottom);
#endif
			const char * GetUpVisualFileName();
			const char * GetOverVisualFileName();
			const char * GetDownVisualFileName();
			void FlashEx();

			void Flash();
			
#if defined(__BL_MAILBOX__)
			void EnableFlash();
			void DisableFlash();
#endif
#ifdef ENABLE_12ZI
			void SetAlwaysToolTip(bool func);
#endif
			void Enable();
			void Disable();

			void SetUp();
			void Up();
			void Over();
			void Down();

			BOOL IsDisable();
			BOOL IsPressed();

		protected:
			void OnUpdate();
			void OnRender();
			void OnChangePosition();

			BOOL OnMouseLeftButtonDown();
			BOOL OnMouseLeftButtonDoubleClick();
			BOOL OnMouseLeftButtonUp();
			void OnMouseOverIn();
			void OnMouseOverOut();

			BOOL IsEnable();

#ifdef ENABLE_NEW_GAMEOPTION
			void SetCurrentVisual(CGraphicExpandedImageInstance* pVisual);
			BOOL OnIsType(DWORD dwType);
#else
			void SetCurrentVisual(CGraphicImageInstance* pVisual);
#endif

		protected:
			BOOL m_bEnable;
			BOOL m_isPressed;
			BOOL m_isFlashEx;
			BOOL m_isFlash;
#if defined(__BL_MAILBOX__)
			bool m_isNewFlash;
#endif
#ifdef ENABLE_12ZI
			BOOL m_isEnableFlash;
			BOOL m_isAlwaysTooltip;
#endif
#ifdef ENABLE_NEW_GAMEOPTION
			CGraphicExpandedImageInstance* m_pcurVisual;
			CGraphicExpandedImageInstance m_upVisual;
			CGraphicExpandedImageInstance m_overVisual;
			CGraphicExpandedImageInstance m_downVisual;
			CGraphicExpandedImageInstance m_disableVisual;
#else
			CGraphicImageInstance* m_pcurVisual;
			CGraphicImageInstance m_upVisual;
			CGraphicImageInstance m_overVisual;
			CGraphicImageInstance m_downVisual;
			CGraphicImageInstance m_disableVisual;
#endif
	};
	class CRadioButton : public CButton
	{
	public:
#ifdef ENABLE_NEW_GAMEOPTION
		static DWORD Type();
#endif
		CRadioButton(PyObject* ppyObject);
		virtual ~CRadioButton();

	protected:
		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonUp();
		void OnMouseOverIn();
		void OnMouseOverOut();
#ifdef ENABLE_NEW_GAMEOPTION
		BOOL OnIsType(DWORD dwType);
#endif
	};
	class CToggleButton : public CButton
	{
	public:
#ifdef ENABLE_NEW_GAMEOPTION
		static DWORD Type();
#endif
		CToggleButton(PyObject* ppyObject);
		virtual ~CToggleButton();

	protected:
		BOOL OnMouseLeftButtonDown();
		BOOL OnMouseLeftButtonUp();
		void OnMouseOverIn();
		void OnMouseOverOut();
#ifdef ENABLE_NEW_GAMEOPTION
		BOOL OnIsType(DWORD dwType);
#endif
	};
	class CDragButton : public CButton
	{
		public:
			CDragButton(PyObject * ppyObject);
			virtual ~CDragButton();

			void SetRestrictMovementArea(int ix, int iy, int iwidth, int iheight);

		protected:
			void OnChangePosition();
			void OnMouseOverIn();
			void OnMouseOverOut();

		protected:
			RECT m_restrictArea;
	};
#ifdef ENABLE_MINI_GAME_YUTNORI
	class CMoveImageBox : public CImageBox
	{
		public:
			static DWORD Type();
			BOOL OnIsType(DWORD dwType);

			CMoveImageBox(PyObject * ppyObject);
			virtual ~CMoveImageBox();

			void SetMoveSpeed(float fSpeed);
			void SetMovePosition(float fDstX, float fDstY);
			void MoveStart();
			void MoveStop();
			bool GetMove();

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();
			virtual void OnRender();
			void OnEndMove();

		protected:
			D3DXVECTOR2 m_v2SrcPos;
			D3DXVECTOR2 m_v2DstPos;
			D3DXVECTOR2 m_v2NextPos;
			D3DXVECTOR2 m_v2Direction;
			D3DXVECTOR2 m_v2NextDistance;
			float m_fDistance;
			float m_fMoveSpeed;
			bool m_bIsMove;
			bool m_bMoveAll;
	};

	class CMoveTextLine : public CTextLine
	{
		public:
			static DWORD Type();
			BOOL OnIsType(DWORD dwType);

			CMoveTextLine(PyObject * ppyObject);
			virtual ~CMoveTextLine();

			void SetMoveSpeed(float fSpeed);
			void SetMovePosition(float fDstX, float fDstY);
			void MoveStart();
			void MoveStop();
			bool GetMove();

		protected:
			virtual void OnUpdate();
			virtual void OnRender();
			void OnChangePosition();
			void OnEndMove();

		protected:
			D3DXVECTOR2 m_v2SrcPos;
			D3DXVECTOR2 m_v2DstPos;
			D3DXVECTOR2 m_v2NextPos;
			D3DXVECTOR2 m_v2Direction;
			D3DXVECTOR2 m_v2NextDistance;
			float m_fDistance;
			float m_fMoveSpeed;
			bool m_bIsMove;
	};

	class CMoveScaleImageBox : public CMoveImageBox
	{
		public:
			static DWORD Type();
			BOOL OnIsType(DWORD dwType);

			CMoveScaleImageBox(PyObject * ppyObject);
			virtual ~CMoveScaleImageBox();

			void SetMaxScale(float fMaxScale);
			void SetMaxScaleRate(float fMaxScaleRate);

		protected:
			virtual void OnCreateInstance();
			virtual void OnDestroyInstance();

			virtual void OnUpdate();

		protected:
			float m_fMaxScale;
			float m_fMaxScaleRate;
			float m_fScaleDistance;
			float m_fAdditionalScale;
			D3DXVECTOR2 m_v2CurScale;
	};
#endif
};

extern BOOL g_bOutlineBoxEnable;
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
