#pragma once

#ifndef __ANDROID__
  #error "This ddstream.h is only for Android!"
#endif

#ifndef __DDSTREAM_H__
#define __DDSTREAM_H__

#include <windows.h>
#include <unknwn.h>

// Forward declarations for DirectDraw Media Stream & base streaming interfaces
typedef struct IMediaStream IMediaStream;
typedef struct IStreamSample IStreamSample;
typedef struct IDirectDrawMediaStream IDirectDrawMediaStream;
typedef struct IDirectDrawStreamSample IDirectDrawStreamSample;
typedef struct IDirectDrawPalette IDirectDrawPalette;

#endif // __DDSTREAM_H__