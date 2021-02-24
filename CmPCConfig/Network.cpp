////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Network.cpp
///
/// Network connection helper functions.
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CmPCConfig.h"
#include "network.h"
#include "secureapi.h"
#include "registry.h"

#ifndef UNDER_CE

#include "netconnect.h"
#include <atlbase.h>
#include <lm.h>
#include <Accctrl.h>
#include <Aclapi.h>
#pragma comment(lib,"Netapi32.lib")
#pragma comment(lib,"Mpr.lib")

#define gszKeyWinlogon _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon")
#define gszDefPassword _T("DefaultPassword")
#define gszDefUserName _T("DefaultUserName")

//////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetErrorText(HRESULT hResult, _TCHAR *szErrBuff, int buffLen)
{
	DWORD dwErrorCode = HRESULT_CODE(hResult);

    if (dwErrorCode != ERROR_EXTENDED_ERROR) 
	{
		// If its a regular system message
		int pos = wsprintf(szErrBuff,_T("Error (0x%x): "),dwErrorCode);
		
		if (0 == ::FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, dwErrorCode,
								   MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT), 
								   &szErrBuff[pos], buffLen-pos, NULL) )
		{
			lstrcat(szErrBuff,_T("Unknown Error"));
		}
		if (dwErrorCode == ERROR_NO_SUCH_LOGON_SESSION)
		{
			lstrcat(szErrBuff,_T("\nHint: Try domain\\username instead of just username."));


		}
	}
    else 
    { 
		// if its an extended error message
		TCHAR szDescription[CFG_NETWORK_ERROR_BUFFER_SIZE];
		TCHAR szProvider[MAX_PATH];
		DWORD dwWNetResult = WNetGetLastError(&dwErrorCode, // error code
            szDescription,  // buffer for error description 
            TSIZEOF(szDescription),  // size of error buffer
            szProvider,     // buffer for provider name 
            TSIZEOF(szProvider));    // size of name buffer
 
        //
        // Process errors.
        //
        if (dwWNetResult != NO_ERROR) 
            wsprintf(szErrBuff,_T("Unknown extended error %ld"),dwErrorCode); 
		else
		{
			 //
			// Otherwise, print the additional error information.
			//
			wsprintf(szErrBuff, 
				_T("%s failed with code %ld;\n%s"), 
				(LPSTR) szProvider, dwErrorCode, (LPSTR) szDescription); 
		} 
	}

	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
DWORD CleanConnections(LPNETRESOURCE lpnr, bool fForce)
{
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;      // 16K is a good size
	DWORD cEntries = -1;         // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;     // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_CONNECTED, // all network resources
		RESOURCETYPE_ANY,   // all resources
		0,        // enumerate all resources
		lpnr,     // NULL first time the function is called
		&hEnum);  // handle to the resource
	
	if (dwResult != NO_ERROR)
		return dwResult;

	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	
	do
	{  
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,      // resource handle
			&cEntries,  // defined locally as -1
			lpnrLocal,  // LPNETRESOURCE
			&cbBuffer); // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR)
		{
			for(i = 0; i < cEntries; i++)
			{

				// If the NETRESOURCE structure represents a container resource, 
				//  call the EnumerateFunc function recursively.
				
				if(RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
					& RESOURCEUSAGE_CONTAINER))
				{
					dwResult = CleanConnections(&lpnrLocal[i], fForce);
					if (!dwResult != NO_ERROR)
						break;
				}

				#ifdef _DEBUG
					if (0 == _tcsicmp(lpnrLocal[i].lpRemoteName,_T("\\\\j-pavlik\\upxdebug")))
						dwResult = NO_ERROR;
					else
				#endif
				if (lpnrLocal[i].lpLocalName && lpnrLocal[i].lpLocalName[0])
					dwResult = WNetCancelConnection2(lpnrLocal[i].lpLocalName, 
												0, fForce);
				else
					dwResult = WNetCancelConnection2(lpnrLocal[i].lpRemoteName, 
												0, fForce);
				if (dwResult != NO_ERROR)
				{
					if (dwResult == ERROR_NOT_CONNECTED)
						dwResult = NO_ERROR;
					else
						break;
				}
			}
		}
		// Process errors.
		//
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
		{
			dwResult = dwResultEnum;
			break;
		}
	} while ((dwResult == NO_ERROR) && (dwResultEnum != ERROR_NO_MORE_ITEMS));


	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL)lpnrLocal);

	//
	// Call WNetCloseEnum to end the enumeration.
	//
	WNetCloseEnum(hEnum);
	
	return dwResult;
}



////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkClearConnections(bool fForce)
{
	DWORD dwResult = CleanConnections(NULL,fForce);
///	if (NO_ERROR == dwResult)
///		return CfgNetworkRestoreConnections();
	return HRESULT_FROM_WIN32(dwResult);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetAdminName(const _TCHAR *pNewName)
{
	USES_CONVERSION;
	USER_INFO_0 userInfo;
	TCHAR pOldName[CFG_USER_NAME_LENGTH];
	if (SUCCEEDED(CfgNetworkGetAdminName(pOldName)))
	{
		userInfo.usri0_name = (LPTSTR)pNewName;
		NET_API_STATUS nas = NetUserSetInfo(NULL,T2W(pOldName),
											0,(PBYTE)&userInfo,NULL);
		SetRegistryString(HKEY_LOCAL_MACHINE, gszKeyWinlogon, gszDefUserName, REG_SZ, pNewName);

		return HRESULT_FROM_WIN32(nas);
	}
	return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetAdminName(_TCHAR *pUserName)
{
	LPUSER_INFO_1 pUserInfo;
	DWORD dwEntriesRead;
	DWORD dwTotalEntries;

	NET_API_STATUS netstatus = NetUserEnum(
		NULL,
		1,
		FILTER_NORMAL_ACCOUNT,
		(BYTE**)&pUserInfo,
		MAX_PREFERRED_LENGTH,
		&dwEntriesRead,
		&dwTotalEntries,
		NULL);
	if (NERR_Success == netstatus)
	{
		netstatus = NERR_ComputerAccountNotFound;
		for (DWORD i = 0; i < dwEntriesRead; i++)
		{
			if (pUserInfo[i].usri1_priv == USER_PRIV_ADMIN)
			{
				USES_CONVERSION;
				LPCTSTR p = W2T(pUserInfo[i].usri1_name);
				lstrcpy(pUserName,p);
				netstatus = NERR_Success;
				break;
			}
		}

		NetApiBufferFree(pUserInfo);
	}

	return HRESULT_FROM_WIN32(netstatus);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetUserPassword(_TCHAR *pUserName, 
							_TCHAR *pNewPassword, _TCHAR *pOldPassword)
{
	USES_CONVERSION;
	NET_API_STATUS nas = NERR_Success;
	USER_INFO_1 *pUserInfo;
	nas = NetUserGetInfo(NULL, pUserName,1,(BYTE**)&pUserInfo);
	if (NERR_Success == nas)
	{
		pUserInfo->usri1_password = pNewPassword;
		nas = NetUserSetInfo(NULL,T2W(pUserName),
											1,(PBYTE)pUserInfo,NULL);
		SetRegistryString(HKEY_LOCAL_MACHINE, gszKeyWinlogon, gszDefPassword, REG_SZ, pNewPassword);
	}

	return HRESULT_FROM_WIN32(nas);
}


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
struct ShareEnumInfo
{
	SHARE_INFO_2 *m_pShareInfo;
	DWORD         m_nPos;
	DWORD         m_nMaxPos;
};


////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkEnumShareOpen(DWORD *pdwCookie)
{
	NET_API_STATUS nas;
	ShareEnumInfo *pInfo = new ShareEnumInfo;
	pInfo->m_pShareInfo = NULL;
	pInfo->m_nPos = 0;
	pInfo->m_nMaxPos = 0;

	DWORD dwEntries = 0;
	DWORD dwTotalEntries = 0;

	nas = NetShareEnum(NULL,2,(BYTE**)&pInfo->m_pShareInfo,
						MAX_PREFERRED_LENGTH,&pInfo->m_nMaxPos,
						&dwTotalEntries,pdwCookie);
	if (nas == ERROR_SUCCESS)
	{
		ATLASSERT(dwTotalEntries == pInfo->m_nMaxPos);
		*pdwCookie = (DWORD)pInfo;
	}
	return HRESULT_FROM_WIN32(nas);
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkEnumShareClose(DWORD dwCookie)
{
	ShareEnumInfo *pInfo = (ShareEnumInfo *)dwCookie;
	NetApiBufferFree(pInfo->m_pShareInfo);
	delete pInfo;
	return S_OK;
}

////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetUserShare(LPTSTR pShareName,
					LPTSTR pPath, DWORD dwCookie)
{
	HRESULT hr = E_FAIL;
	ShareEnumInfo *pInfo = (ShareEnumInfo *)dwCookie;

	*pShareName = NULL;
	*pPath = NULL;
	while (!*pShareName && (pInfo->m_nPos < pInfo->m_nMaxPos))
	{
		if (pInfo->m_pShareInfo[pInfo->m_nPos].shi2_type == STYPE_DISKTREE)
		{
			USES_CONVERSION;
			LPCTSTR pShi2Share = W2T(pInfo->m_pShareInfo[pInfo->m_nPos].shi2_netname);
			LPCTSTR pShi2Path = W2T(pInfo->m_pShareInfo[pInfo->m_nPos].shi2_path);

			lstrcpy(pShareName,pShi2Share);
			lstrcpy(pPath,pShi2Path);

			hr = S_OK; 
		}

		pInfo->m_nPos++;

	} 

	return hr;

}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkAddUserShare(_TCHAR *pShareName,
					_TCHAR *pPath, int nMaxUsers /* = 10 */)
{
	USES_CONVERSION;
	SHARE_INFO_502 shareInfo = {0};
	shareInfo.shi502_netname = pShareName;
	shareInfo.shi502_path = T2W(pPath);
	shareInfo.shi502_max_uses = 10;
	DWORD dwParmErr = 0;
	NET_API_STATUS nas = NetShareAdd(NULL,502,(BYTE*)&shareInfo,&dwParmErr);
	return HRESULT_FROM_WIN32(nas);
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkDelUserShare(_TCHAR *pShareName)
{
	USES_CONVERSION;
	return HRESULT_FROM_WIN32(NetShareDel(NULL, pShareName,0));
}


////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetShareRights(_TCHAR *pShareName, 
																BOOL *fReadOnly)
{
	*fReadOnly = false; // if there is no security descriptor then its a free for all.

	SHARE_INFO_502 * Info502 = NULL;
	USES_CONVERSION;
	DWORD status = NetShareGetInfo( NULL,  pShareName, 502, (BYTE **)&Info502 );
	if ( status == 0 )  
	{
		CSecurityDescriptor sd(Info502->shi502_security_descriptor);
		if (sd.IsValid())
		{
			CSid admin(CSid::LocalAdmin());
			CAcl acl(sd.GetDacl());
			int count = acl.GetAceCount();
			for (int i = 0; i < count; i++)
			{
				CFileAce ace(acl.GetAce(i));
				CSid sid(ace.GetSid());
				if (sid == admin)
				{
					if (ace.IsAllowedType())
					{
						ATLASSERT(ace.IsRead());
						*fReadOnly = !ace.IsWrite();
						break;
					}

				}
			}

		}
	}
	return HRESULT_FROM_WIN32(status);

}

////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetShareRights(_TCHAR *pShareName, 
																	BOOL fReadOnly)
{
	CSid admins(CSid::LocalAdmin());
	if (!admins.IsValid())
		return E_FAIL;

	int cbAcl = sizeof(ACL);
	cbAcl += admins.GetLength();
	cbAcl += sizeof(ACCESS_ALLOWED_ACE);

	CAcl acl;
	if (!acl.InitializeAcl(cbAcl))
		return E_FAIL;

	if (!acl.AddAccessAllowedAce(
			fReadOnly ? CFileAce::SHARE_READ : CFileAce::SHARE_CHANGE, 
			admins))
		return E_FAIL;


	// Initialize a security descriptor and add our ACL to it  
	CSecurityDescriptor sd;
	if(sd.SetDacl(
		TRUE,     // fDaclPresent flag   
		acl, 
		FALSE))   // not a default DACL 
	{
		// swap in the new security descriptor
		USES_CONVERSION;
		LPWSTR pWShare = pShareName;
		SHARE_INFO_502 * Info502 = NULL;
		DWORD status = NetShareGetInfo( NULL, pWShare, 502, (BYTE **)&Info502 );
		Info502->shi502_security_descriptor = sd;
		status = NetShareSetInfo(NULL,pWShare,502,(BYTE *)Info502,NULL);
		return HRESULT_FROM_WIN32(status);
	}
	return E_FAIL;


}




/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkConnectionsOpen(HANDLE *phConnection)
{
	DWORD dwResult = NO_ERROR;
	*phConnection = NULL;

	do
	{
		CNetConnect *pInfo = new CNetConnect;
		if (!pInfo)
		{
			dwResult = ERROR_OUTOFMEMORY;
			break;
		}

		dwResult = pInfo->RefreshConnections();
		if (dwResult != NO_ERROR)
			break;

		*phConnection = (HANDLE)pInfo;

	} while (0);

	return HRESULT_FROM_WIN32(dwResult);



}
/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetConnectionInfo(
			    HANDLE hConnections, DWORD dwIndex, 
				LPTSTR pRemoteName, LPTSTR pLocalName,
				LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist)

{
	CNetConnect *pInfo = (CNetConnect *)hConnections;
	if (!pInfo || !pRemoteName || !pLocalName || !pUserName || !pUserPwd || !pfPersist)
		return E_INVALIDARG;

	BOOL bResult = pInfo->GetConnectionInfo(dwIndex,
				pRemoteName, pLocalName,
				pUserName, pUserPwd, pfPersist);

	return bResult ? S_OK : E_FAIL;
}


/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetConnectionInfo(
			    HANDLE hConnections,
				LPCTSTR pRemoteName, LPCTSTR pLocalName,
				LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist)

{
	CNetConnect *pInfo = (CNetConnect *)hConnections;
	if (!pInfo || !pRemoteName || !pLocalName || !pUserName || !pUserPwd || !pfPersist)
		return E_INVALIDARG;

	BOOL bResult = pInfo->GetConnectionInfo(pRemoteName, pLocalName,
				pUserName, pUserPwd, pfPersist);

	return bResult ? S_OK : E_FAIL;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkConnectionsClose(HANDLE hConnections)
{
	CNetConnect *pInfo = (CNetConnect *)hConnections;
	if (!pInfo)
		return E_INVALIDARG;
	delete pInfo;
	return S_OK;
}



/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetConnectionInfo(
				HANDLE hConnectionHandle, 
				LPCTSTR pRemoteName, LPCTSTR pLocalName,
				LPCTSTR pUserName, LPCTSTR pUserPwd, BOOL fPersist)
{
	CNetConnect *pInfo = (CNetConnect *)hConnectionHandle;
	if (!pInfo || !pRemoteName)
		return E_INVALIDARG;

	DWORD dwResult = pInfo->AddConnection(pRemoteName, pLocalName,
				pUserName, pUserPwd, fPersist);

	return HRESULT_FROM_WIN32(dwResult);
}



////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkRemoveConnection(
				HANDLE hConnectionHandle, 
				LPCTSTR pRemoteName, LPCTSTR pLocalName, BOOL fForce)
{
	CNetConnect *pInfo = (CNetConnect *)hConnectionHandle;
	if (!pInfo || !pRemoteName)
		return E_INVALIDARG;

	DWORD dwResult = pInfo->RemoveConnection(pRemoteName, pLocalName, fForce, TRUE);

	return HRESULT_FROM_WIN32(dwResult);
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkRestoreConnection(HANDLE hConnectionHandle, DWORD dwIndex)
{
	CNetConnect *pInfo = (CNetConnect *)hConnectionHandle;
	if (dwIndex < pInfo->GetNumConnections())
	{
		DWORD dwResult = pInfo->RestoreConnection(dwIndex);
		return HRESULT_FROM_WIN32(dwResult);
	}
	return S_FALSE;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkValidateDrive(LPCTSTR pFileName)
{
	CNetConnect connect;
	connect.RefreshConnections();
	UINT type = connect.GetDriveType(pFileName);
	if (DRIVE_NO_ROOT_DIR == type || DRIVE_UNKNOWN == type)
		return E_FAIL;
	return S_OK;
}



#endif