// stdafx.cpp : source file that includes just the standard includes
//	scriptLib.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

void SetExceptionSender(IPythonExceptionSender * pkExceptionSender)
{
	g_pkExceptionSender = pkExceptionSender;
}
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
