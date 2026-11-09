#pragma once

#define WIN32_LEAN_AND_MEAN // Exclude rarely-used stuff from Windows headers

#include <assert.h>
#include "../UserInterface/Locale_inc.h"

#ifdef DIRECTX9
#include <MicrosoftDirectX/d3d9.h>
#include <MicrosoftDirectX/d3dx9.h>
#include <MicrosoftDirectX/d3d9types.h>
#else
#include <d3d8.h>
#include <d3d8types.h>
#include <d3dx8.h>
#endif

#include <map>
#include <vector>
#include <list>
#include <memory>

#include "SpeedTreeForestDirectX8.h"

// Armadillo nanomite protection
#ifndef NANOBEGIN
#	ifdef __ANDROID__
#		define NANOBEGIN
#		define NANOEND
#	elif defined(__BORLANDC__)
#		define NANOBEGIN __emit__ (0xEB, 0x03, 0xD6, 0xD7, 0x01)
#		define NANOEND __emit__ (0xEB, 0x03, 0xD6, 0xD7, 0x00)
#	else
#		define NANOBEGIN __asm _emit 0xEB __asm _emit 0x03 __asm _emit 0xD6 __asm _emit 0xD7 __asm _emit 0x01
#		define NANOEND __asm _emit 0xEB __asm _emit 0x03 __asm _emit 0xD6 __asm _emit 0xD7 __asm _emit 0x00
#	endif
#endif

//martysama0134's fb0d8d5d0566583187a94f9a823b2075
