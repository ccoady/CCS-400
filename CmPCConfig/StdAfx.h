// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__0BFDFB50_5F5C_42CE_94E7_11274714BEC5__INCLUDED_)
#define AFX_STDAFX_H__0BFDFB50_5F5C_42CE_94E7_11274714BEC5__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#pragma warning( disable : 4786 )

#define WIN32_LEAN_AND_MEAN		// Exclude rarely-used stuff from Windows headers
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#ifndef UNDER_CE
// needed for SetComputerNameEx
#pragma warning( disable : 4005 )
#define _WIN32_WINNT 0x0600
#pragma warning( default : 4005 )
#endif

#ifndef _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#define _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING
#endif 

#include <windows.h>
#include <tchar.h>
#ifndef UNDER_CE
#include <lm.h>  // network join stuff
#endif
#include <atlbase.h> // for W2T

#ifdef _WIN32_WCE
	#include <winsock.h>
#else
	#include <winsock2.h>
#endif


#ifdef UNDER_CE
#define RegOpenKey(key,sub,result) RegOpenKeyEx((key),(sub),0,0,(result))
#endif


#define _CRT_SECURE_CPP_OVERLOAD_SECURE_NAMES 1

//#pragma warning(push)
#pragma warning(disable : 4201)
#pragma warning(disable : 4251)

//#pragma warning(pop)
#include <string>

#include "CmPCConfig.h"
#include "CommonDefs.h"


//??typedef std::basic_string <TCHAR> tstring;

#define WIDEN2(x) L ## x
#define WIDEN(x) WIDEN2(x)
#define __WFILE__ WIDEN(__FILE__)
#define __WFUNCTION__ WIDEN(__FUNCTION__)
#define __WFUNCSIG__ WIDEN(__FUNCSIG__)
#define _AUDIOFUNC_  _T("|AUDIO| ") __WFUNCTION__
#define _AUDIOFUNC_ERROR_  _T("|AUDIO|ERROR| ") __WFUNCTION__
#define _AUDIOFUNC_INFO_  _T("|AUDIO|INFO| ") __WFUNCTION__
#define _AUDIOFUNC_WARNING_  _T("|AUDIO|WARNING| ") __WFUNCTION__

#define TSIZEOF(a) sizeof(a)/sizeof(TCHAR)

// TODO: reference additional headers your program requires here

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__0BFDFB50_5F5C_42CE_94E7_11274714BEC5__INCLUDED_)
