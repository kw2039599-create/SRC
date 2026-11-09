#pragma once

#ifndef __ANDROID__
  #error "This mmstream.h is only for Android!"
#endif

#include <windows.h>
#include <unknwn.h>

#ifndef __MMSTREAM_H__
#define __MMSTREAM_H__

typedef struct IMultiMediaStream IMultiMediaStream;
typedef struct IMediaStream IMediaStream;
typedef struct IAudioMediaStream IAudioMediaStream;
typedef struct IVideoMediaStream IVideoMediaStream;

#ifndef STREAM_TIME
typedef LONGLONG STREAM_TIME;
#endif

#ifndef FIND_NAME_FLAG
typedef DWORD FIND_NAME_FLAG;
#endif

#endif // __MMSTREAM_H__