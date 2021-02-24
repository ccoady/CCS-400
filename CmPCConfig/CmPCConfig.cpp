////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        CmPCConfig.cpp
///
/// Defines the entry point for the DLL application.
///
/// @author      William Levine
/// @date        01/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmPCConfig.h"
#include "BaseLogger.h"

CRITICAL_SECTION g_CS_CEthernet_hpp;

/// Entry point function
BOOL APIENTRY DllMain( HANDLE hModule, 
					  DWORD  ul_reason_for_call, 
					  LPVOID lpReserved
					  )
{
	if(ul_reason_for_call==DLL_PROCESS_ATTACH)
	{		
		::InitializeCriticalSection(&g_CS_CEthernet_hpp);
	}
	else if(ul_reason_for_call==DLL_PROCESS_DETACH)
		::DeleteCriticalSection(&g_CS_CEthernet_hpp);

	return TRUE;
}

// how do I know which adaptor id and instance id



// audio
//	vol / bal / mic / line / bass / treble

// serial
//	switch touch drivers

// touch screen cal routine
// key click

