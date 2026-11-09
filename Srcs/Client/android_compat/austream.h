#pragma once

#ifndef __ANDROID__
  #error "This austream.h is only for Android!"
#endif

#ifndef __AUSTREAM_H__
#define __AUSTREAM_H__

#include <windows.h>
#include <unknwn.h>

struct IAudioStreamSample;
struct IAudioMediaStream;

struct IAudioStreamSample : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetAudioData(void** ppAudioData) = 0;
};

struct IAudioMediaStream : public IUnknown {
    virtual HRESULT STDMETHODCALLTYPE GetFormat(void* pWaveFormat) = 0;
    virtual HRESULT STDMETHODCALLTYPE CreateSample(void* pAudioData, DWORD cbAudioData, DWORD dwFlags, IAudioStreamSample** ppSample) = 0;
};

#endif // __AUSTREAM_H__