////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        SystemApps.cpp
///
/// Implementation file for System Apps related functions.
///
/// @author      William Levine
/// @date        07/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CmPCConfig.h"
#include "Registry.h"
#include "SystemApps.h"

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the system time.
///
/// @param       SYSTEMTIME *pSystemTime The new system time.
///
/// @author      William Levine
/// @date        02/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgSystemAppGetIEEnablePopups(BOOL &bEnable)
{
DWORD dwRes;
HRESULT hRes;

	hRes = GetRegistryDWORD(HKEY_LOCAL_MACHINE, _T("Software\\Policies\\Microsoft\\Internet Explorer\\Restrictions"), _T("NoOpeninNewWnd"), &dwRes);
	if ( hRes == S_OK)
		bEnable = dwRes ? FALSE : TRUE;

	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the system time.
///
/// @param       SYSTEMTIME *pSystemTime A pointer to a SYSTEMTIME struct wher e the system time will be 
///											stored upon return
///
/// @author      William Levine
/// @date        02/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgSystemAppSetIEEnablePopups(BOOL bEnable)
{
HRESULT hRes;

	hRes = SetRegistryDWORD(HKEY_LOCAL_MACHINE, _T("Software\\Policies\\Microsoft\\Internet Explorer\\Restrictions"), _T("NoOpeninNewWnd"), bEnable ? 0x00 : 0x01);
		
	return hRes;
}
