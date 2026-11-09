#pragma once
#define TIMER_SYNC
#include <windows.h>
#include "Singleton.h"

class CTimer : public CSingleton<CTimer>
{
	public:
		CTimer();
		virtual ~CTimer();

		void	Advance();
		void	Adjust(int iTimeGap);
		void	SetBaseTime();

#ifdef TIMER_SYNC
		double	GetCurrentSecond();
#else
		float	GetCurrentSecond();
#endif
		DWORD	GetCurrentMillisecond();

#ifdef TIMER_SYNC
		double	GetElapsedSecond();
#else
		float	GetElapsedSecond();
#endif
		DWORD	GetElapsedMilliecond();

		void	UseCustomTime();

	protected:
		bool	m_bUseRealTime;
		DWORD	m_dwBaseTime;
		DWORD	m_dwCurrentTime;
#ifdef TIMER_SYNC
		double	m_fCurrentTime;
#else
		float	m_fCurrentTime;
#endif
		DWORD	m_dwElapsedTime;
		int		m_index;
};

BOOL	ELTimer_Init();

DWORD	ELTimer_GetMSec();

VOID	ELTimer_SetServerMSec(DWORD dwServerTime);
DWORD	ELTimer_GetServerMSec();

VOID	ELTimer_SetFrameMSec();
DWORD	ELTimer_GetFrameMSec();
//martysama0134's fb0d8d5d0566583187a94f9a823b2075