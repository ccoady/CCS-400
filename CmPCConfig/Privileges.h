////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Privileges.h
///
/// Header file for the CProvileges class.
///
/// @author      William Levine
/// @date        03/03/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_PRIVILEGES_H_)
#define _PRIVILEGES_H_

#ifndef UNDER_CE

class CPrivileges
{
public:
	CPrivileges(const TCHAR *pcszPrivilegeConstant = NULL, bool bEnable = FALSE);
	~CPrivileges();

	BOOL EnablePrivilege(const TCHAR *pcszPrivilegeConstant, bool bEnable);

	HANDLE	m_hTokenHandle;
	BOOL	m_bSuccess;
};

#endif
#endif