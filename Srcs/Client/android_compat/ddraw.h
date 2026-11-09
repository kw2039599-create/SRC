#pragma once

#ifndef __ANDROID__
  #error "This ddraw.h is only for Android!"
#endif

#include <windows.h>

#ifndef __DDRAW_INCLUDED__
#define __DDRAW_INCLUDED__

typedef struct IDirectDraw IDirectDraw;
typedef struct IDirectDrawSurface IDirectDrawSurface;
typedef struct IDirectDrawClipper IDirectDrawClipper;

typedef IDirectDraw* LPDIRECTDRAW;
typedef IDirectDrawSurface* LPDIRECTDRAWSURFACE;
typedef IDirectDrawClipper* LPDIRECTDRAWCLIPPER;

#ifndef DD_OK
#define DD_OK 0
#endif

#endif // __DDRAW_INCLUDED__