#pragma once

#define WIN32_LEAN_AND_MEAN

#include "../UserInterface/Locale_inc.h"
#ifdef ENABLE_OPENGL
	#include "../EterLib/GrpOpenGL.h"
#elif defined(DIRECTX9)
	#include <MicrosoftDirectX/d3d9.h>
	#include <MicrosoftDirectX/d3dx9.h>
#else
	#include <d3d8.h>
	#include <d3dx8.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "../eterBase/StdAfx.h"
#include "../UserInterface/Locale_inc.h"
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
