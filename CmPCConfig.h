////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        CmPCConfig.h
///
/// Project header file that defines project global things.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_CMPCCONFIG_H_)
#define _CMPCCONFIG_H_

#ifdef CMPCCONFIG_EXPORTS
	#define CmPCConfig_DeclSpec	_declspec(dllexport)
#elif  defined(_CMPCCONFIG_STATICLIB)
#	define CmPCConfig_DeclSpec								//	for static link
#else
	#define CmPCConfig_DeclSpec	_declspec(dllimport)
#endif

#endif