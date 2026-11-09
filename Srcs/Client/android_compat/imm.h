#pragma once

#ifndef __ANDROID__
  #error "This imm.h is only for Android!"
#endif

#include <windows.h>

typedef HANDLE HIMC;
typedef HANDLE HIMCC;
typedef HANDLE HKL;
typedef DWORD  LGRPID;
typedef DWORD  LCID;
typedef DWORD  LCTYPE;

#ifndef KL_NAMELENGTH
#define KL_NAMELENGTH 9
#endif

#ifndef LOCALE_SENGLANGUAGE
#define LOCALE_SENGLANGUAGE 0x00001001
#define LOCALE_SABBREVLANGNAME 0x00000003
#define LOCALE_IDEFAULTANSICODEPAGE 0x00001004
#define LOCALE_IDEFAULTCODEPAGE 0x0000000B
#endif

#ifndef PRIMARYLANGID
#define PRIMARYLANGID(lgid)    ((WORD)(lgid) & 0x3ff)
#define SUBLANGID(lgid)        ((WORD)(lgid) >> 10)
#define LANGIDFROMLCID(lcid)   ((WORD)(lcid))
#define MAKELANGID(p, s)       ((((WORD)(s)) << 10) | (WORD)(p))
#endif

#ifndef LANG_NEUTRAL
#define LANG_NEUTRAL    0x00
#define LANG_ARABIC     0x01
#define LANG_CHINESE    0x04
#define LANG_ENGLISH    0x09
#define LANG_JAPANESE   0x11
#define LANG_KOREAN     0x12
#define LANG_VIETNAMESE 0x2a
#endif

static inline HKL GetKeyboardLayout(DWORD idThread) { (void)idThread; return (HKL)0x04090409; }
static inline int GetKeyboardLayoutList(int nBuff, HKL *lpList) {
    if (nBuff > 0 && lpList) { lpList[0] = (HKL)0x04090409; return 1; }
    return 1;
}
static inline BOOL GetKeyboardLayoutNameA(LPSTR pwszKLID) {
    if (pwszKLID) strcpy(pwszKLID, "00000409");
    return TRUE;
}
#ifndef GetKeyboardLayoutName
#define GetKeyboardLayoutName GetKeyboardLayoutNameA
#endif
static inline int GetLocaleInfoA(LCID Locale, LCTYPE LCType, LPSTR lpLCData, int cchData) {
    (void)Locale; (void)LCType;
    if (lpLCData && cchData > 0) lpLCData[0] = '\0';
    return 0;
}
#ifndef GetLocaleInfo
#define GetLocaleInfo GetLocaleInfoA
#endif

typedef struct tagCOMPOSITIONFORM {
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} COMPOSITIONFORM, *PCOMPOSITIONFORM, *LPCOMPOSITIONFORM;

typedef struct tagCANDIDATEFORM {
    DWORD dwIndex;
    DWORD dwStyle;
    POINT ptCurrentPos;
    RECT  rcArea;
} CANDIDATEFORM, *PCANDIDATEFORM, *LPCANDIDATEFORM;

typedef struct tagCANDIDATELIST {
    DWORD dwSize;
    DWORD dwStyle;
    DWORD dwCount;
    DWORD dwSelection;
    DWORD dwPageStart;
    DWORD dwPageSize;
    DWORD dwOffset[1];
} CANDIDATELIST, *PCANDIDATELIST, *LPCANDIDATELIST;

typedef struct tagREGISTERWORDA {
    LPSTR lpReading;
    LPSTR lpWord;
} REGISTERWORDA, *PREGISTERWORDA, *LPREGISTERWORDA;

typedef struct tagREGISTERWORDW {
    LPWSTR lpReading;
    LPWSTR lpWord;
} REGISTERWORDW, *PREGISTERWORDW, *LPREGISTERWORDW;

typedef struct tagSTYLEBUFA {
    DWORD dwStyle;
    CHAR  szDescription[32];
} STYLEBUFA, *PSTYLEBUFA, *LPSTYLEBUFA;

typedef struct tagSTYLEBUFW {
    DWORD dwStyle;
    WCHAR szDescription[32];
} STYLEBUFW, *PSTYLEBUFW, *LPSTYLEBUFW;

typedef int (*REGISTERWORDENUMPROCA)(LPCSTR, DWORD, LPCSTR, LPVOID);
typedef int (*REGISTERWORDENUMPROCW)(LPCWSTR, DWORD, LPCWSTR, LPVOID);

// IME Window Messages
#ifndef WM_IME_STARTCOMPOSITION
#define WM_IME_STARTCOMPOSITION     0x010D
#define WM_IME_ENDCOMPOSITION       0x010E
#define WM_IME_COMPOSITION          0x010F
#define WM_IME_KEYLAST              0x010F
#define WM_IME_SETCONTEXT           0x0281
#define WM_IME_NOTIFY               0x0282
#define WM_IME_CONTROL              0x0283
#define WM_IME_COMPOSITIONFULL      0x0284
#define WM_IME_SELECT               0x0285
#define WM_IME_CHAR                 0x0286
#define WM_IME_REQUEST              0x0288
#define WM_IME_KEYDOWN              0x0290
#define WM_IME_KEYUP                0x0291
#endif

#ifndef WM_INPUTLANGCHANGE
#define WM_INPUTLANGCHANGEREQUEST   0x0050
#define WM_INPUTLANGCHANGE          0x0051
#endif

// IMN_* Notification Constants
#ifndef IMN_CLOSESTATUSWINDOW
#define IMN_CLOSESTATUSWINDOW       0x0001
#define IMN_OPENSTATUSWINDOW        0x0002
#define IMN_CHANGECANDIDATE         0x0003
#define IMN_CLOSECANDIDATE          0x0004
#define IMN_OPENCANDIDATE           0x0005
#define IMN_SETCONVERSIONMODE       0x0006
#define IMN_SETSENTENCEMODE         0x0007
#define IMN_SETOPENSTATUS           0x0008
#define IMN_SETCANDIDATEPOS         0x0009
#define IMN_SETCOMPOSITIONFONT      0x000A
#define IMN_SETCOMPOSITIONWINDOW    0x000B
#define IMN_SETSTATUSWINDOWPOS      0x000C
#define IMN_GUIDELINE               0x000D
#define IMN_PRIVATE                 0x000E
#endif

// GCS_* Composition String Constants
#ifndef GCS_COMPREADSTR
#define GCS_COMPREADSTR             0x0001
#define GCS_COMPREADATTR            0x0002
#define GCS_COMPREADCLAUSE          0x0004
#define GCS_COMPSTR                 0x0008
#define GCS_COMPATTR                0x0010
#define GCS_COMPCLAUSE              0x0020
#define GCS_CURSORPOS               0x0080
#define GCS_DELTASTART              0x0100
#define GCS_RESULTREADSTR           0x0200
#define GCS_RESULTREADCLAUSE        0x0400
#define GCS_RESULTSTR               0x0800
#define GCS_RESULTCLAUSE            0x1000
#endif

// IME Conversion / Sentence Mode Constants
#ifndef IME_CMODE_ALPHANUMERIC
#define IME_CMODE_ALPHANUMERIC      0x0000
#define IME_CMODE_NATIVE            0x0001
#define IME_CMODE_CHINESE           IME_CMODE_NATIVE
#define IME_CMODE_HANGUL            IME_CMODE_NATIVE
#define IME_CMODE_JAPANESE          IME_CMODE_NATIVE
#define IME_CMODE_KATAKANA          0x0002
#define IME_CMODE_LANGUAGE          0x0003
#define IME_CMODE_FULLSHAPE         0x0008
#define IME_CMODE_ROMAN             0x0010
#define IME_CMODE_CHARCODE          0x0020
#define IME_CMODE_HANJACONVERT      0x0040
#define IME_CMODE_SOFTKBD           0x0080
#define IME_CMODE_NOCONVERSION      0x0100
#define IME_CMODE_EUDC              0x0200
#define IME_CMODE_SYMBOL            0x0400
#define IME_CMODE_FIXED             0x0800
#endif

#ifndef IME_SMODE_NONE
#define IME_SMODE_NONE              0x0000
#define IME_SMODE_PLAURALCLAUSE     0x0001
#define IME_SMODE_SINGLECONVERT     0x0002
#define IME_SMODE_AUTOMATIC         0x0004
#define IME_SMODE_PHRASEPREDICT     0x0008
#define IME_SMODE_CONVERSATION      0x0010
#endif

// ImmNotifyIME Action Constants
#ifndef NI_OPENCANDIDATE
#define NI_OPENCANDIDATE            0x0010
#define NI_CLOSECANDIDATE           0x0011
#define NI_SELECTCANDIDATESTR       0x0012
#define NI_CHANGECANDIDATELIST      0x0013
#define NI_FINALIZECONVERSIONRESULT 0x0014
#define NI_COMPOSITIONSTR           0x0015
#define NI_SETCANDIDATE_PAGESTART   0x0016
#define NI_SETCANDIDATE_PAGESIZE    0x0017
#define NI_IMEMENUSELECTED          0x0018
#endif

#ifndef CPS_COMPLETE
#define CPS_COMPLETE                0x0001
#define CPS_CONVERT                 0x0002
#define CPS_REVERT                  0x0003
#define CPS_CANCEL                  0x0004
#endif

// ImmGetProperty Constants
#ifndef IGP_GETIMEVERSION
#define IGP_GETIMEVERSION           (DWORD)(-4)
#define IGP_PROPERTY                0x00000004
#define IGP_CONVERSION              0x00000008
#define IGP_SENTENCE                0x0000000c
#define IGP_UI                      0x00000010
#define IGP_SETCOMPSTR              0x00000014
#define IGP_SELECT                  0x00000018
#endif

#ifndef IME_PROP_AT_CARET
#define IME_PROP_AT_CARET           0x00010000
#define IME_PROP_SPECIAL_UI         0x00020000
#define IME_PROP_CANDLIST_START_FROM_1 0x00040000
#define IME_PROP_UNICODE            0x00080000
#define IME_PROP_COMPLETE_ON_UNSELECT 0x00100000
#endif

#ifndef ISC_SHOWUICANDIDATEWINDOW
#define ISC_SHOWUICANDIDATEWINDOW       0x00000001
#define ISC_SHOWUICOMPOSITIONWINDOW     0x80000000
#define ISC_SHOWUIGUIDELINE             0x40000000
#define ISC_SHOWUIALLCANDIDATEWINDOW    0x0000000F
#define ISC_SHOWUIALL                   0xC000000F
#endif

// Stub Functions
static inline HIMC ImmGetContext(HWND hWnd) { (void)hWnd; return (HIMC)1; }
static inline BOOL ImmReleaseContext(HWND hWnd, HIMC hIMC) { (void)hWnd; (void)hIMC; return TRUE; }
static inline HIMC ImmAssociateContext(HWND hWnd, HIMC hIMC) { (void)hWnd; return hIMC; }
static inline BOOL ImmGetOpenStatus(HIMC hIMC) { (void)hIMC; return FALSE; }
static inline BOOL ImmSetOpenStatus(HIMC hIMC, BOOL fOpen) { (void)hIMC; (void)fOpen; return TRUE; }
static inline BOOL ImmGetConversionStatus(HIMC hIMC, LPDWORD lpfdwConversion, LPDWORD lpfdwSentence) {
    (void)hIMC;
    if (lpfdwConversion) *lpfdwConversion = 0;
    if (lpfdwSentence) *lpfdwSentence = 0;
    return TRUE;
}
static inline BOOL ImmSetConversionStatus(HIMC hIMC, DWORD fdwConversion, DWORD fdwSentence) {
    (void)hIMC; (void)fdwConversion; (void)fdwSentence;
    return TRUE;
}
static inline LONG ImmGetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen;
    return 0;
}
static inline LONG ImmGetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen;
    return 0;
}
#ifndef ImmGetCompositionString
#define ImmGetCompositionString ImmGetCompositionStringA
#endif

static inline BOOL ImmSetCompositionStringA(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen) {
    (void)hIMC; (void)dwIndex; (void)lpComp; (void)dwCompLen; (void)lpRead; (void)dwReadLen;
    return TRUE;
}
static inline BOOL ImmSetCompositionStringW(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwCompLen, LPVOID lpRead, DWORD dwReadLen) {
    (void)hIMC; (void)dwIndex; (void)lpComp; (void)dwCompLen; (void)lpRead; (void)dwReadLen;
    return TRUE;
}
#ifndef ImmSetCompositionString
#define ImmSetCompositionString ImmSetCompositionStringA
#endif

static inline DWORD ImmGetCandidateListA(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen) {
    (void)hIMC; (void)deIndex; (void)lpCandList; (void)dwBufLen;
    return 0;
}
static inline DWORD ImmGetCandidateListW(HIMC hIMC, DWORD deIndex, LPCANDIDATELIST lpCandList, DWORD dwBufLen) {
    (void)hIMC; (void)deIndex; (void)lpCandList; (void)dwBufLen;
    return 0;
}
#ifndef ImmGetCandidateList
#define ImmGetCandidateList ImmGetCandidateListA
#endif

static inline DWORD ImmGetCandidateListCountA(HIMC hIMC, LPDWORD lpdwListCount) {
    (void)hIMC;
    if (lpdwListCount) *lpdwListCount = 0;
    return 0;
}
static inline DWORD ImmGetCandidateListCountW(HIMC hIMC, LPDWORD lpdwListCount) {
    (void)hIMC;
    if (lpdwListCount) *lpdwListCount = 0;
    return 0;
}
#ifndef ImmGetCandidateListCount
#define ImmGetCandidateListCount ImmGetCandidateListCountA
#endif

static inline BOOL ImmNotifyIME(HIMC hIMC, DWORD dwAction, DWORD dwIndex, DWORD dwValue) {
    (void)hIMC; (void)dwAction; (void)dwIndex; (void)dwValue;
    return TRUE;
}
static inline DWORD ImmGetProperty(HKL hKL, DWORD fdwIndex) { (void)hKL; (void)fdwIndex; return 0; }
static inline BOOL ImmIsIME(HKL hKL) { (void)hKL; return FALSE; }
static inline UINT ImmGetVirtualKey(HWND hWnd) { (void)hWnd; return 0; }
static inline DWORD ImmGetGuideLineA(HIMC hIMC, DWORD dwIndex, LPSTR lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen; return 0;
}
static inline DWORD ImmGetGuideLineW(HIMC hIMC, DWORD dwIndex, LPWSTR lpBuf, DWORD dwBufLen) {
    (void)hIMC; (void)dwIndex; (void)lpBuf; (void)dwBufLen; return 0;
}
#ifndef ImmGetGuideLine
#define ImmGetGuideLine ImmGetGuideLineA
#endif

static inline UINT ImmGetDescriptionA(HKL hKL, LPSTR lpszDescription, UINT uBufLen) {
    (void)hKL; if (lpszDescription && uBufLen > 0) lpszDescription[0] = '\0'; return 0;
}
static inline UINT ImmGetDescriptionW(HKL hKL, LPWSTR lpszDescription, UINT uBufLen) {
    (void)hKL; if (lpszDescription && uBufLen > 0) lpszDescription[0] = 0; return 0;
}
#ifndef ImmGetDescription
#define ImmGetDescription ImmGetDescriptionA
#endif

static inline UINT ImmGetIMEFileNameA(HKL hKL, LPSTR lpszFileName, UINT uBufLen) {
    (void)hKL; if (lpszFileName && uBufLen > 0) lpszFileName[0] = '\0'; return 0;
}
static inline UINT ImmGetIMEFileNameW(HKL hKL, LPWSTR lpszFileName, UINT uBufLen) {
    (void)hKL; if (lpszFileName && uBufLen > 0) lpszFileName[0] = 0; return 0;
}
#ifndef ImmGetIMEFileName
#define ImmGetIMEFileName ImmGetIMEFileNameA
#endif

static inline BOOL ImmSetCompositionWindow(HIMC hIMC, LPCOMPOSITIONFORM lpCompForm) {
    (void)hIMC; (void)lpCompForm; return TRUE;
}
static inline BOOL ImmSetCandidateWindow(HIMC hIMC, LPCANDIDATEFORM lpCandidate) {
    (void)hIMC; (void)lpCandidate; return TRUE;
}