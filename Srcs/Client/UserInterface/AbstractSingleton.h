#pragma once

#ifdef __ANDROID__
#include <assert.h>
#include <stdint.h>
#endif

template <typename T>
class TAbstractSingleton
{
	static T * ms_singleton;

public:
	TAbstractSingleton()
	{
		assert(!ms_singleton);
#ifdef __ANDROID__
		intptr_t offset = (intptr_t) (T*) 1 - (intptr_t) (CSingleton <T>*) (T*) 1;
		ms_singleton = (T*) ((intptr_t) this + offset);
#else
		int offset = (int) (T*) 1 - (int) (CSingleton <T>*) (T*) 1;
		ms_singleton = (T*) ((int) this + offset);
#endif
	}

	virtual ~TAbstractSingleton()
	{
		assert(ms_singleton);
		ms_singleton = 0;
	}

	__forceinline static T & GetSingleton()
	{
		assert(ms_singleton!=NULL);
		return (*ms_singleton);
	}
};

template <typename T> T * TAbstractSingleton <T>::ms_singleton = 0;
//martysama0134's fb0d8d5d0566583187a94f9a823b2075