#pragma once

#include <windows.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef LPSTR
typedef char* LPSTR;
#endif

typedef UINT MMRESULT;
typedef UINT MCIDEVICEID;
typedef DWORD MCIERROR;

typedef HANDLE HWAVE;
typedef HANDLE HWAVEOUT;
typedef HANDLE HWAVEIN;
typedef HWAVEOUT *LPHWAVEOUT;
typedef HWAVEIN *LPHWAVEIN;

typedef HANDLE HMIDI;
typedef HANDLE HMIDIOUT;
typedef HANDLE HMIDIIN;
typedef HANDLE HMIDISTRM;
typedef HMIDI *LPHMIDI;
typedef HMIDIOUT *LPHMIDIOUT;
typedef HMIDIIN *LPHMIDIIN;
typedef HMIDISTRM *LPHMIDISTRM;

typedef HANDLE HMIXER;
typedef HMIXER *LPHMIXER;
typedef HANDLE HMMIO;
typedef HANDLE HDRVR;

#ifndef WAVE_FORMAT_PCM
#define WAVE_FORMAT_PCM 1
#endif

typedef struct wavehdr_tag {
    LPSTR              lpData;
    DWORD              dwBufferLength;
    DWORD              dwBytesRecorded;
    DWORD              dwUser;
    DWORD              dwFlags;
    DWORD              dwLoops;
    struct wavehdr_tag *lpNext;
    DWORD              reserved;
} WAVEHDR, *PWAVEHDR, *LPWAVEHDR;

typedef struct midihdr_tag {
    LPSTR              lpData;
    DWORD              dwBufferLength;
    DWORD              dwBytesRecorded;
    DWORD              dwUser;
    DWORD              dwFlags;
    struct midihdr_tag *lpNext;
    DWORD              reserved;
    DWORD              dwOffset;
    DWORD              dwReserved[8];
} MIDIHDR, *PMIDIHDR, *LPMIDIHDR;

typedef struct waveformat_tag {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
} WAVEFORMAT, *PWAVEFORMAT, *LPWAVEFORMAT;

typedef struct pcmwaveformat_tag {
    WAVEFORMAT wf;
    WORD       wBitsPerSample;
} PCMWAVEFORMAT, *PPCMWAVEFORMAT, *LPPCMWAVEFORMAT;

#ifndef _WAVEFORMATEX_
#define _WAVEFORMATEX_
typedef struct tWAVEFORMATEX {
    WORD  wFormatTag;
    WORD  nChannels;
    DWORD nSamplesPerSec;
    DWORD nAvgBytesPerSec;
    WORD  nBlockAlign;
    WORD  wBitsPerSample;
    WORD  cbSize;
} WAVEFORMATEX, *PWAVEFORMATEX, *LPWAVEFORMATEX;
typedef const WAVEFORMATEX *LPCWAVEFORMATEX;
#endif

#ifdef __cplusplus
}
#endif