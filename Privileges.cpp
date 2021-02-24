////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Privileges.cpp
///
/// Implementation file for the CPrivileges class.
///
/// @author      William Levine
/// @date        03/03/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "Privileges.h"

#ifndef UNDER_CE

CPrivileges::CPrivileges(const TCHAR *pcszPrivilegeConstant /* = NULL */, bool bEnable /* = FALSE */)
{
	m_bSuccess = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &m_hTokenHandle);

	if (!m_bSuccess)
		m_hTokenHandle = NULL;
	else if (pcszPrivilegeConstant)
		EnablePrivilege(pcszPrivilegeConstant, bEnable);
}

CPrivileges::~CPrivileges()
{
	if (m_hTokenHandle)
		CloseHandle(m_hTokenHandle);
}


BOOL CPrivileges::EnablePrivilege(const TCHAR *pcszPrivilegeConstant, bool bEnable)
{
TOKEN_PRIVILEGES	privileges;

	m_bSuccess = FALSE;

	if ( LookupPrivilegeValue(NULL, pcszPrivilegeConstant, &privileges.Privileges[0].Luid) )
	{
		privileges.PrivilegeCount = 1;
		privileges.Privileges[0].Attributes = bEnable ? SE_PRIVILEGE_ENABLED : 0;

		m_bSuccess = AdjustTokenPrivileges( m_hTokenHandle,	//	HANDLE TokenHandle, 
											FALSE,			//	BOOL DisableAllPrivileges, 
											&privileges,	//	PTOKEN_PRIVILEGES NewState, 
											0,				//	DWORD BufferLength, 
											NULL,			//  PTOKEN_PRIVILEGES PreviousState, 
											0);				//  PDWORD ReturnLength
	}
	
	return m_bSuccess;
}


#endif