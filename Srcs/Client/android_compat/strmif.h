#pragma once

#ifndef __ANDROID__
  #error "This strmif.h is only for Android!"
#endif

#ifndef __STRMIF_H__
#define __STRMIF_H__

#include <windows.h>
#include <unknwn.h>

// Forward declarations for DirectShow streaming interfaces
typedef struct IPin IPin;
typedef struct IEnumPins IEnumPins;
typedef struct IEnumMediaTypes IEnumMediaTypes;
typedef struct IFilterGraph IFilterGraph;
typedef struct IMediaFilter IMediaFilter;
typedef struct IMediaSample IMediaSample;
typedef struct IMemAllocator IMemAllocator;
typedef struct IMemInputPin IMemInputPin;

// Basic DirectShow filter states
typedef enum _FilterState {
    State_Stopped = 0,
    State_Paused = 1,
    State_Running = 2
} FILTER_STATE;

#endif // __STRMIF_H__