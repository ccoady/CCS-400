// stdafx.cpp : source file that includes just the standard includes
//	CmPCConfig.pch will be the pre-compiled header
//	stdafx.obj will contain the pre-compiled type information

#include "stdafx.h"

// TODO: reference any additional headers you need in STDAFX.H
// and not in this file
#ifndef UNDER_CE
#pragma comment(lib,"netapi32.lib")
#pragma comment(lib,"Ws2_32.lib")
#pragma comment(lib,"WinInet.lib")
#endif
