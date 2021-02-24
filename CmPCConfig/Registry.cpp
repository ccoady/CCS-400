////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Registry.cpp
///
/// General Registry helper functions.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "stdlib.h"
#include "Registry.h"
#include "Privileges.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// This function gets a registry string.
///
/// @param       hRootHandle  see hRootHandle for SetRegistryString
/// @param       pstSubKey   see hRootHandle for SetRegistryString
/// @param       pstValueNamesee hRootHandle for SetRegistryString
/// @param       ppstResult  
///
/// @author      William Levine
/// @date        01/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT GetRegistryString(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, _TCHAR *pszResult, int buffLen)
{
HRESULT hRes = S_OK;
HKEY hKey;

	if ( pszSubKey )
		hRes = RegOpenKey(hRootHandle, pszSubKey, &hKey) == ERROR_SUCCESS ? S_OK : E_FAIL;
	else
		hKey = hRootHandle;

	if ( hKey && (hRes == S_OK) )
	{
		DWORD dwCBSize = 0;
#ifdef _DEBUG

		hRes = RegQueryValueEx(hKey, pszValueName, NULL, NULL, NULL, &dwCBSize);
		if (hRes == ERROR_SUCCESS)
			ATLASSERT(dwCBSize <= (DWORD)buffLen);

#endif
		dwCBSize = buffLen;
		hRes = RegQueryValueEx(	hKey, pszValueName, NULL, NULL, 
									reinterpret_cast<LPBYTE>(pszResult), &dwCBSize);

		if (pszSubKey)
			RegCloseKey(hKey);
	}
	
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is a helper function which sets a registry value that is a string to the specified string.
///
/// @param       hRootHandle  The root key, the sub key is located under.  This is something like
///              HKEY_LOCAL_MACHINE or one of the others.  An already opened subkey can also be
///              passed in.  See RegOpenKey in MSDN for more info.
/// @param       pstSubKey   The subkey to be opened under the key passed in as the first parameter
/// @param       pstValueNameThe name of the value to be set.
/// @param       dwRegType   The registry type, this should be REG_SZ or REG_MULTI_SZ
/// @param       pstValue    The value to set.
///
/// @author      William Levine
/// @date        01/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT SetRegistryString(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD dwRegType, const _TCHAR *pszValue, bool bConvertToWcs)
{
HRESULT hRes = S_OK;
HKEY hKey;

	if ( pszSubKey )
		hRes = RegOpenKey(hRootHandle, pszSubKey, &hKey) == ERROR_SUCCESS ? S_OK : E_FAIL;
	else
		hKey = hRootHandle;

	if ( hKey && (hRes == S_OK) )
	{
	DWORD dwCBSize = 0;

//		if (!bConvertToWcs)
//		{
			hRes = RegSetValueEx(	hKey, pszValueName, NULL, dwRegType, 
								reinterpret_cast<const BYTE*>(pszValue),  (DWORD)_tcslen(pszValue)*sizeof(_TCHAR));
/*		}
		else
		{
		unsigned short *pData;

			dwCBSize = mbstowcs(NULL, pszValue, strlen(pszValue));
			if (dwCBSize != -1)
			{
				pData = new unsigned short[dwCBSize+2];
				dwCBSize = mbstowcs(pData, pszValue, strlen(pszValue));
				pData[dwCBSize++] = 0x0000;
				if (dwRegType == REG_MULTI_SZ)
					pData[dwCBSize++] = 0x0000;
				hRes = RegSetValueEx(	hKey, pszValueName, NULL, dwRegType, 
								reinterpret_cast<const unsigned TCHAR*>(pData), dwCBSize*sizeof(unsigned short));
				delete pData;
			}

		}*/

		if (pszSubKey)
			RegCloseKey(hKey);
	}
	
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is a helper function which gets a registry value that is a DWORD.
/// The parameters are the same as the string version GetRegistryString(HKEY hRootHandle, _TCHAR *pstSubKey, _TCHAR *pstValueName, _TCHAR **ppstResult)
///
/// @author      William Levine
/// @date        01/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT GetRegistryDWORD(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD *pdwResult)
{
HRESULT hRes = S_OK;
HKEY hKey = NULL;

	if ( pszSubKey )
		hRes = RegOpenKey(hRootHandle, pszSubKey, &hKey) == ERROR_SUCCESS ? S_OK : E_FAIL;
	else
		hKey = hRootHandle;

	if ( hKey && (hRes == S_OK) )
	{
	DWORD dwCBSize = sizeof(DWORD);

		hRes = RegQueryValueEx(hKey, pszValueName, NULL, NULL, reinterpret_cast<BYTE *>(pdwResult), &dwCBSize);

		if (pszSubKey)
			RegCloseKey(hKey);
	}
	
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// This is a helper function which sets a registry value that is a DWORD to the specified value.
/// The parameters are the same as the string version SetRegistryString(HKEY hRootHandle, _TCHAR *pstSubKey, _TCHAR *pstValueName, DWORD dwRegType, _TCHAR *pstValue)
/// except there is no need for the caller to delete an memory since the memeory is already allocated on
/// the stack for the function call.
/// @author      William Levine
/// @date        01/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT SetRegistryDWORD(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD dwValue)
{
HRESULT hRes = S_OK;
HKEY hKey = NULL;

	if ( pszSubKey )
		hRes = RegOpenKey(hRootHandle, pszSubKey, &hKey) == ERROR_SUCCESS ? S_OK : E_FAIL;
	else
		hKey = hRootHandle;

	if ( hKey && (hRes == S_OK) )
	{
	DWORD dwCBSize = sizeof(DWORD);

		hRes = RegSetValueEx(hKey, pszValueName, NULL, REG_DWORD, reinterpret_cast<LPBYTE>(&dwValue), dwCBSize);

		if (pszSubKey)
			RegCloseKey(hKey);
	}
	
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Loads an offline hive (SOFTWARE and SYSTEM branches under a specific key.  This key is opened
/// and the handle is returned for use later on.
///	
/// @param       TCHAR *pcszFilePath		[in] The path to the offline hive files, eg. c:\Windows\system32\config
/// @param       phNewRootKey			[out] The handle to the root path of the loaded hive files.
///											Under this handle will be subkeys "SYSTEM", "SOFTWARE" etc...	
///
/// @author      William Levine
/// @date        03/04/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT LoadOfflineHive(TCHAR *pcszFilePath, HKEY *phNewRootKey)
{
#ifdef UNDER_CE
	return E_NOTIMPL;
#else
HRESULT hRes = S_OK;
bool bSuccess;
_TCHAR Buffer[MAX_PATH];

	// this privilege required for loading a registry key
	CPrivileges	Privileges(SE_RESTORE_NAME, TRUE);

	*phNewRootKey = HKEY_USERS;

	_tcscpy(Buffer, pcszFilePath);
	_tcscat(Buffer, _T("\\SOFTWARE"));
	bSuccess = RegLoadKey(*phNewRootKey, _T("SOFTWARE"), Buffer) == ERROR_SUCCESS;

	_tcscpy(Buffer, pcszFilePath);
	_tcscat(Buffer, _T("\\SYSTEM"));
	bSuccess &= (RegLoadKey(*phNewRootKey, _T("SYSTEM"), Buffer) == ERROR_SUCCESS);

	return bSuccess ? S_OK : E_FAIL;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Unloads the offline hives files, and saving them if specified.
///
/// @param       TCHAR *pcszFilePath		[in] The path to the offline hive files, eg. c:\Windows\system32\config
/// @param       hNewRootKey			[out] The handle to the root path of the loaded hive files.
///											Under this handle are subkeys "SYSTEM", "SOFTWARE" etc...	
/// @param       bSave					[in] Set to true to save the files, if false they will just be unloaded.
///
/// @author      William Levine
/// @date        03/04/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT UnLoadOfflineHive(TCHAR *pcszFilePath, HKEY hNewRootKey, bool bSave)
{
#ifdef UNDER_CE
	return E_NOTIMPL;
#else
	HRESULT hResSoftware = S_OK, hResSystem = S_OK;
	HKEY hSystem, hSoftware;

	if (bSave)
	{
	_TCHAR Buffer[MAX_PATH];

		if (RegOpenKey(hNewRootKey, _T("SOFTWARE"), &hSoftware) == ERROR_SUCCESS)
		{
			_tcscpy(Buffer, pcszFilePath);
			_tcscat(Buffer, _T("\\SOFTWARE"));
			hResSoftware = RegSaveKey(hSoftware, Buffer, NULL) == ERROR_SUCCESS ? S_OK : E_FAIL;
			RegCloseKey(hSoftware);
		}
		
		if (RegOpenKey(hNewRootKey, _T("SYSTEM"), &hSystem) == ERROR_SUCCESS)
		{
			_tcscpy(Buffer, pcszFilePath);
			_tcscat(Buffer, _T("\\SYSTEM"));
			hResSystem = RegSaveKey(hSystem, Buffer, NULL) == ERROR_SUCCESS ? S_OK : E_FAIL;
			RegCloseKey(hSystem);
		}
	}

	RegUnLoadKey(hNewRootKey, _T("SOFTWARE"));
	RegUnLoadKey(hNewRootKey, _T("SYSTEM")); 

	return ((hResSoftware == S_OK) && (hResSystem == S_OK));
#endif
}