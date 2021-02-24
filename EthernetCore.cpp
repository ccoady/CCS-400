////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        EthernetCore.cpp
///
/// Provides helper functions and utilities to the exported Ethernet configuration functions.
///
/// @author      William Levine
/// @date        02/04/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "EthernetCore.h"
#include "Registry.h"
#include <atlbase.h>

#include <Iphlpapi.h>
#pragma comment(lib,"iphlpapi.lib")

const _TCHAR EnetAdaptor_TextTable[][16] = 
{ _T("NULL"),	_T("InternalPrivate"), _T("RESERVED2"), _T("RESERVED3"), 
	_T("InternalPublic"), _T("ExtendedA"), _T("ExtendedB")};

#define g_szName            _T("Name")
#define g_szInternalPublic _T("SOFTWARE\\Crestron Electronics Inc.\\UPX\\Network\\Adapters\\InternalPublic")

/*
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the CLSID of the adaptor denoted using the enumeration by looking up config settings in
/// the registry.
///
/// @param       hRootKey     The root key to look under. Typically HKLM but can be something
///              different if an offline hive is loaded into the registry.  Uses SOFTWARE and
///              SYSTEM sub keys so both must be loaded.
/// @param       eAdaptor    The enumeration denoting which logical adaptor's CLSDI is requested.
/// @param       pstCLSID    A pointer a _TCHAR buffer where the result will be stored,  should be
///              MAX_PATH long.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetGetAdaptorCLSID(HKEY hRootKey, EEthernetAdaptors eAdaptor, _TCHAR *pstCLSID, DWORD *pdwBufferLen, bool bForceSearch)
{
_TCHAR KeyName[MAX_PATH];
HKEY hKey;
HRESULT hRes = -52;
long lRes;

	if (eAdaptor >= EEnetAdaptor_Max)
		return hRes;
	
	_tcscpy(KeyName, "SOFTWARE\\Crestron Electronics Inc.\\UPX\\Network\\Adapters\\");
	_tcscat(KeyName, EnetAdaptor_TextTable[eAdaptor]);

	if ( RegOpenKey(hRootKey, KeyName, &hKey) == ERROR_SUCCESS)
	{
		// try to get a saved CLSID here.
		if ( bForceSearch || ((lRes = RegQueryValueEx(hKey, "CLSID", NULL, NULL, (LPBYTE)pstCLSID, pdwBufferLen)) != ERROR_SUCCESS) )
		{
		_TCHAR Buffer[MAX_PATH];
		DWORD dwNameLen = MAX_PATH;

			// if that fails then we get the name and find the CLSID using it.
			if ( (lRes = RegQueryValueEx(hKey, "Name", NULL, NULL, (LPBYTE)Buffer, &dwNameLen)) == ERROR_SUCCESS)
			{
				hRes = CfgEthernetFindAdaptorCLSID(hRootKey, Buffer, pstCLSID, pdwBufferLen);
				if (hRes == S_OK)
					RegSetValueEx(hKey, "CLSID", NULL, REG_SZ, (LPBYTE)pstCLSID, _tcslen(pstCLSID));
			}
			else
				hRes = -51;
		}
		else
			hRes = S_OK;

		RegCloseKey(hKey);
	}

	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Finds the CLSID of the adaptor with a description equal to the name passed in here.
///
/// @param       hRootKey     The root key to look under. Typically HKLM but can be something
///              different if an offline hive is loaded into the registry.  
/// @param       _TCHAR *pstNameThe name to match.
/// @param       pstCLSID    a ptr to a buffer to accept the CLSID
/// @param       plBufferLen the length of the buffer.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetFindAdaptorCLSID(HKEY hRootKey, const _TCHAR *pstName, _TCHAR *pstCLSID, DWORD *pdwBufferLen)
{
HKEY hKey;
HRESULT hRes = -50;

	// if that failed, then we need to figure out the CLSID and save it here.
	if ( RegOpenKey(hRootKey, "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards", &hKey) == ERROR_SUCCESS )
	{
	DWORD dwIdx = 0, dwcbName = MAX_PATH;
	long lRes;
	_TCHAR KeyBuffer[MAX_PATH], *pNameBuffer;


		do
		{
			dwcbName  = MAX_PATH;
			// enumerate through each key and get the name of the key
			lRes = RegEnumKeyEx(hKey, dwIdx, KeyBuffer, &dwcbName, NULL, NULL, NULL, NULL);
			if ((lRes == ERROR_SUCCESS) || (lRes == ERROR_MORE_DATA))
			{
				// then get the description of that adaptor
				if ( GetRegistryString(hKey, KeyBuffer, _T("Description"), &pNameBuffer) == S_OK)
				{
					 if ( _tcscmp(pNameBuffer, pstName) == NULL )
					 {
						// we get here if the description matches the one passed in
						if ( (hRes = GetRegistryString(hKey, KeyBuffer, _T("ServiceName"), &pNameBuffer)) == S_OK)
							//now get the CLSID and copy it into the buffer passed into us.
							_tcsncpy(pstCLSID, pNameBuffer, *pdwBufferLen);
					 }

					 delete pNameBuffer;
				}
			}

			dwIdx++;
					
		}while ( ( (lRes == ERROR_MORE_DATA) || (lRes == ERROR_SUCCESS) )  && (hRes != S_OK));

		RegCloseKey(hKey);
	}
	
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets an array of the CLSIDs of all the network adaptors.
///
/// @param       hRootKey     The root key to look under.  Normally this is HKEY_LOCAL_MACHINE, but
///              in the case of a hive file that has been loaded to a different root key, this
///              parameter can be this value so that the adaptors of an offline registry can be
///              retrieved.
/// @param       ppszPtrArrayCLSIDs An array a ptrs to _TCHAR strings that contain the CLSIDs of the
///              adaptors.  When done with the data ,a single delete *ppszPtrArrayCLSIDs will free
///              all memory allocated.
/// @return      S_OK on success or E_FAIL if some error occurs.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetGetAdaptorCLSIDs(HKEY hRootKey, _TCHAR **ppszPtrArrayCLSIDs)
{
HKEY hNetworkKey;
int nKeyIdx = 0;
FILETIME LastWrite;

	// first open the key
	if ( RegOpenKey(hRootKey, _T("SYSTEM\\ControlSet001\\control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}"), &hNetworkKey)
		 == ERROR_SUCCESS)
	{
	DWORD dwMaxNameLen, dwSubKeyCount, dwKeyLen, dwBufferSize;
	_TCHAR *pDataBuffer, *pBufferCurrentPos;
	long lRes;

		// find out how many keys there are and the max length
		RegQueryInfoKey(hNetworkKey, NULL, NULL, NULL, &dwSubKeyCount, &dwMaxNameLen, NULL, NULL, NULL, NULL, NULL, NULL);

		// allocate one contiguous buffer for all ptrs and strings
		dwBufferSize = sizeof(_TCHAR)*(dwSubKeyCount+1)*(dwMaxNameLen+1) + sizeof(_TCHAR*)*(dwSubKeyCount+1);
		pDataBuffer = new  char[dwBufferSize];
		memset(pDataBuffer, 0, dwBufferSize);

		// init our array of string ptrs to the beginning of this buffer
		*ppszPtrArrayCLSIDs = (_TCHAR*)pDataBuffer;
		// and the start of our data to one element past the number required. this gives us a NULL terminatedlist.
		pBufferCurrentPos = (_TCHAR*)(*ppszPtrArrayCLSIDs + (dwSubKeyCount+1));

		// iterate through all keys until all have been read.
		do
		{
			dwKeyLen = dwMaxNameLen+1;
			ppszPtrArrayCLSIDs[nKeyIdx] = pBufferCurrentPos;
			
			lRes = RegEnumKeyEx(hNetworkKey, nKeyIdx, ppszPtrArrayCLSIDs[nKeyIdx], &dwKeyLen, NULL, NULL, NULL, &LastWrite);
			
			// only save CLSIDs, and just copy over others
			if ( _tcsncmp(ppszPtrArrayCLSIDs[nKeyIdx], _T("{"), 1) == NULL)
			{
				pBufferCurrentPos += dwKeyLen+1;
				nKeyIdx++;
			}
			else
				// zero out in case this is the last key to be gotten.
				ppszPtrArrayCLSIDs[nKeyIdx] = NULL;	
		
		}while (lRes == ERROR_MORE_DATA);

		RegCloseKey(hNetworkKey);

		return ((lRes == ERROR_SUCCESS) ? S_OK : E_FAIL);
	}
	else
		return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Given the CLSID of a network adaptor, gets the PNPInstanceID and Name of the connection.
///
/// @param hRootKey     The root key to look under.  Normally this is HKEY_LOCAL_MACHINE, but
///              in the case of a hive file that has been loaded to a different root key, this
///              parameter can be this value so that the adaptors of an offline registry can be
///              retrieved.
/// @param pszCLSID		The CLSID of the adaptor to get the info for.  This is one of the values 
///				returned by CfgEthernetGetCLSIDsNetAdaptors.
///	@param ppszPNPInstanceID A ptr to a _TCHAR* variable that on return, will contain the 
///				Plug and Play instance id in the form "PCI\pnp id\instance id".  This memory is 
///				allocated here and must be freed by the caller.
/// @param ppszConnectionName The name of the connection for this adaptor.  Typically "Local Area 
///				Connection" when a single adaptor is installed. This memory is allocated here and 
///				must be freed by the caller.
/// @return      S_OK on success or E_FAIL if some error occurs.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetGetAdaptorInfo(HKEY hRootKey, _TCHAR *pszCLSID, _TCHAR **ppszPNPInstanceID, _TCHAR **ppszConnectionName)
{
_TCHAR szAdaptorKey[MAX_PATH];
HKEY hAdaptorKey;

	// make our string from static info and the passed in CLSID
	_tcscpy(szAdaptorKey, _T("SYSTEM\\ControlSet001\\control\\Network\\{4D36E972-E325-11CE-BFC1-08002BE10318}\\"));
	_tcscat(szAdaptorKey, pszCLSID);
	_tcscat(szAdaptorKey, _T("\\Connection"));

	// Open it up and get the info we want;
	if (RegOpenKey(hRootKey, szAdaptorKey, &hAdaptorKey) == ERROR_SUCCESS)
	{
	long lLen;

		if (RegQueryValue(hAdaptorKey, _T("Name"), NULL, &lLen) == ERROR_SUCCESS)
		{
			*ppszConnectionName = new _TCHAR[lLen+1];
			RegQueryValue(hAdaptorKey, _T("Name"), *ppszConnectionName, &(++lLen));
		}

		if (RegQueryValue(hAdaptorKey, _T("PnpInstanceID"), NULL, &lLen) == ERROR_SUCCESS)
		{
			*ppszPNPInstanceID = new _TCHAR[lLen+1];
			RegQueryValue(hAdaptorKey, _T("PnpInstanceID"), *ppszPNPInstanceID, &(++lLen));
		}

		RegCloseKey(hAdaptorKey);

		return S_OK;
	}
	else
		return E_FAIL;
}

//*********************************************************************************************
// Getting and setting IP settings using adaptor CLSID as a reference.
//*********************************************************************************************

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets current IP settings for the specified adaptor.
///
/// @param hRootKey     The root key to look under.  Normally this is HKEY_LOCAL_MACHINE, but
///              in the case of a hive file that has been loaded to a different root key, this
///              parameter can be this value so that the adaptors of an offline registry can be
///              retrieved.
/// @param pszCLSID		The CLSID of the adaptor to get the info for.  This is one of the values 
///				returned by CfgEthernetGetCLSIDsNetAdaptors.
/// @param pSettings	A pointer to an allocated structure where the IP information will be stored upon return.
/// @return      S_OK on success or E_FAIL if some error occurs.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetGetIPSettings(HKEY hRootKey, const _TCHAR *pszCLSID, SDeviceIPSettings *pSettings)
{	
HRESULT hRes = S_OK;
HKEY hKey;
_TCHAR szSubKey[MAX_PATH];

	_tcscpy(szSubKey, _T("SYSTEM\\ControlSet001\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	_tcscat(szSubKey, pszCLSID);

	memset(pSettings, 0, sizeof(SDeviceIPSettings));

	if ( RegOpenKey(hRootKey, szSubKey, &hKey) == ERROR_SUCCESS)
	{
		DWORD dwLen = sizeof(pSettings->dwEnableDHCP);
		hRes = RegQueryValueEx(hKey, _T("EnableDHCP"), NULL, NULL, (LPBYTE)&pSettings->dwEnableDHCP, &dwLen) == ERROR_SUCCESS ? S_OK : E_FAIL;

		if (hRes == S_OK)
			hRes = CfgEthernetReadDHCPSettings(hKey, pSettings);
	
		if (hRes == S_OK)
			hRes = CfgEthernetReadStaticIPSettings(hKey, pSettings);
		
		return hRes;	
	}
	else
		return E_FAIL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper function for CfgEthernetGetIPSettings which loads DHCP settings into the pSettings parameter.
///
/// @param       hKey         An open key to read the settings from.
/// @param       pSettings	  The struct to store the data in.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetReadDHCPSettings(HKEY hKey, SDeviceIPSettings *pSettings)
{
DWORD dwLen;
_TCHAR szBuffer[MAX_PATH], *pTemp, *pNext;

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("DhcpIPAddress"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPIPAddress, &dwLen);

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("DhcpSubnetMask"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPSubnetMask, &dwLen);

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("DhcpServer"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPServer, &dwLen);
	
	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("DhcpDefaultGateway"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPDefaultGateway, &dwLen);

	dwLen = MAX_PATH;
	RegQueryValueEx(hKey, _T("DhcpNameServer"), NULL, NULL, (LPBYTE)szBuffer, &dwLen);
	
	int nIdx = 0;
	pNext = szBuffer;
	while ( (pTemp = _tcschr(pNext, ' ')) && (nIdx < MAX_NAME_SERVERS) )
	{
		if (pTemp)
			_tcsncpy(pSettings->dhcp.szDHCPNameServers[nIdx], pNext, pTemp - pNext);
		else
			_tcsncpy(pSettings->dhcp.szDHCPNameServers[nIdx], pNext, MAX_IP_ADDRESS_LEN);

		pNext = pTemp+1;
		nIdx++;
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Helper function for CfgEthernetGetIPSettings which loads static ip settings into the pSettings parameter.
///
/// @param       hKey         An open key to read the settings from.
/// @param       pSettings	  The struct to store the data in.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetReadStaticIPSettings(HKEY hKey, SDeviceIPSettings *pSettings)
{
DWORD dwLen;
_TCHAR szBuffer[MAX_PATH], *pTemp;

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("IPAddress"), NULL, NULL, (LPBYTE)pSettings->static_ip.szIPAddress, &dwLen);

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("SubnetMask"), NULL, NULL, (LPBYTE)pSettings->static_ip.szSubnetMask, &dwLen);

	dwLen = MAX_IP_ADDRESS_LEN;
	RegQueryValueEx(hKey, _T("DefaultGateway"), NULL, NULL, (LPBYTE)pSettings->static_ip.szDefaultGateway, &dwLen);

	dwLen = MAX_PATH;
	RegQueryValueEx(hKey, _T("NameServer"), NULL, NULL, (LPBYTE)szBuffer, &dwLen);
	
	pTemp = _tcschr(szBuffer, ' ');
	if (pTemp)
	{
		_tcsncpy(pSettings->static_ip.szNameServers[0], szBuffer, pTemp - szBuffer );
		_tcscpy(pSettings->static_ip.szNameServers[1], pTemp);
	}
	else
		_tcscpy(pSettings->static_ip.szNameServers[0], szBuffer);

	return S_OK;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets Static IP information and enables/disables DHCP according to the data in the pSettings
/// struct and the dwItems flags.
///
/// @param       hRootKey     The root key to look under.  Normally this is HKEY_LOCAL_MACHINE, but
///              in the case of a hive file that has been loaded to a different root key, this
///              parameter can be this value so that the adaptors of an offline registry can be
///              retrieved.
/// @param       ppszPtrArrayCLSIDs An array a ptrs to _TCHAR strings that contain the CLSIDs of the
///              adaptors.  When done with the data ,a single delete *ppszPtrArrayCLSIDs will free
///              all memory allocated.
/// @param       pSettings   The data to be set.
/// @param       dwItems     Flags indicating which elements pof the pSettings struct are to be
///              used.
///
/// @author      William Levine
/// @date        01/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEthernetSetIPSettings(HKEY hRootKey, const _TCHAR *pszCLSID, SDeviceIPSettings *pSettings, DWORD dwItems, int nControlSet = 0 )
{	
HRESULT hRes = S_OK;
HKEY hKey;
_TCHAR szSubKey[MAX_PATH];

	if ( (nControlSet < 0) || (nControlSet > 9))
		return E_FAIL;

	if (nControlSet == 0)
		_tcscpy(szSubKey, _T("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	else
	{
		char ch[2] = {0x30+nControlSet, 0}; 

		_tcscpy(szSubKey, _T("SYSTEM\\ControlSet00"));
		_tcscat(szSubKey, ch);
		_tcscat(szSubKey, _T("\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	}
		
	_tcscat(szSubKey, pszCLSID);

	if ( RegOpenKey(hRootKey, szSubKey, &hKey) == ERROR_SUCCESS)
	{
		// DHCP Enable
		if (dwItems & EDevIP_EnableDHCP)
		{
			hRes = RegSetValueEx(hKey, _T("EnableDHCP"), NULL, REG_DWORD, (BYTE*)&pSettings->dwEnableDHCP, 
				sizeof(pSettings->dwEnableDHCP)) == ERROR_SUCCESS ? S_OK : E_FAIL;
		}

		// DHCP IP Address
		if ((dwItems & EDevIP_DHCPIPAddress) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("DhcpIPAddress"), REG_SZ, pSettings->dhcp.szDHCPIPAddress, TRUE);

		// IP Address
		if ((dwItems & EDevIP_IPAddress) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("IPAddress"), REG_MULTI_SZ, pSettings->static_ip.szIPAddress, TRUE);

		// Subnet mask
		if ((dwItems & EDevIP_SubnetMask) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("SubnetMask"), REG_MULTI_SZ, pSettings->static_ip.szSubnetMask, TRUE);

		// Default gateway
		if ((dwItems & EDevIP_DefaultGateway) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("DefaultGateway"), REG_MULTI_SZ, pSettings->static_ip.szDefaultGateway, TRUE);

		// Name Servers
		if ((dwItems & EDevIP_NameServers) && (hRes == S_OK))
		{
		_TCHAR szBuffer[MAX_PATH];

			_tcscpy(szBuffer, pSettings->static_ip.szNameServers[0]);
			_tcscat(szBuffer, _T(" "));
			_tcscat(szBuffer, pSettings->static_ip.szNameServers[1]);

			hRes = SetRegistryString(hKey, NULL, _T("NameServer, TRUE"), REG_SZ, szBuffer);
		}

		return hRes;	
	}
	else
		return E_FAIL;
}
*/

//*********************************************************************************************
//
//*********************************************************************************************
//*********************************************************************************************
//
//*********************************************************************************************
bool CEthernetRegistry::GetUPXAdapterName(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterName, DWORD dwBufferLen)
{
_TCHAR KeyName[MAX_PATH];
HKEY hKey;
bool bSuccess = FALSE;

	_tcscpy(KeyName, _T("SOFTWARE\\Crestron Electronics Inc.\\UPX\\Network\\Adapters\\"));
	_tcscat(KeyName, EnetAdaptor_TextTable[eAdapter]);

	if ( RegOpenKey(m_hRootKey, KeyName, &hKey) == ERROR_SUCCESS)
	{
		// if that fails then we get the name and find the CLSID using it.
		if ( RegQueryValueEx(hKey, _T("Name"), NULL, NULL, (LPBYTE)pstAdpaterName, &dwBufferLen) == ERROR_SUCCESS)
			bSuccess = TRUE;

		RegCloseKey(hKey);
	}

	return bSuccess;
}

bool CEthernetRegistry::GetUPXAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterCLSID, DWORD dwBufferLen)
{
_TCHAR KeyName[MAX_PATH];
HKEY hKey;
bool bSuccess = FALSE;

	_tcscpy(KeyName, _T("SOFTWARE\\Crestron Electronics Inc.\\UPX\\Network\\Adapters\\"));
	_tcscat(KeyName, EnetAdaptor_TextTable[eAdapter]);

	if ( RegOpenKey(m_hRootKey, KeyName, &hKey) == ERROR_SUCCESS)
	{
		// if that fails then we get the name and find the CLSID using it.
		if ( RegQueryValueEx(hKey, _T("CLSID"), NULL, NULL, (LPBYTE)pstAdpaterCLSID, &dwBufferLen) == ERROR_SUCCESS)
			bSuccess = TRUE;

		RegCloseKey(hKey);
	}

	return bSuccess;
}

bool CEthernetRegistry::SetUPXAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterCLSID)
{
_TCHAR KeyName[MAX_PATH];
HKEY hKey;
bool bSuccess = FALSE;

	_tcscpy(KeyName, _T("SOFTWARE\\Crestron Electronics Inc.\\UPX\\Network\\Adapters\\"));
	_tcscat(KeyName, EnetAdaptor_TextTable[eAdapter]);

	if ( RegOpenKey(m_hRootKey, KeyName, &hKey) == ERROR_SUCCESS)
	{
		if (_tcslen(pstAdpaterCLSID) == 0)
			RegDeleteValue(hKey, _T("CLSID"));
		else
		{
			// if that fails then we get the name and find the CLSID using it.
			if ( RegSetValueEx(hKey, _T("CLSID"), NULL, REG_SZ, (LPBYTE)pstAdpaterCLSID, (DWORD)_tcslen(pstAdpaterCLSID)) == ERROR_SUCCESS)
				bSuccess = TRUE;
		}

		RegCloseKey(hKey);
	}

	return bSuccess;
}

bool CEthernetRegistry::FindAdpaterCLSIDByName(_TCHAR *pstAdapterName, _TCHAR *pstAdpaterCLSID, DWORD dwBufferLen)
{
	HKEY hKey;
	HRESULT hRes = -50;

	// if that failed, then we need to figure out the CLSID and save it here.
	if ( RegOpenKey(m_hRootKey, 
		_T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards"), &hKey) == ERROR_SUCCESS )
	{
		DWORD dwIdx = 0, dwcbName = MAX_PATH;
		long lRes;
		_TCHAR KeyBuffer[MAX_PATH];
		_TCHAR pNameBuffer[MAX_PATH];

		do
		{
			dwcbName  = MAX_PATH;
			// enumerate through each key and get the name of the key
			lRes = RegEnumKeyEx(hKey, dwIdx, KeyBuffer, &dwcbName, NULL, NULL, NULL, NULL);
			if ((lRes == ERROR_SUCCESS) || (lRes == ERROR_MORE_DATA))
			{
				// then get the description of that adaptor
				if ( GetRegistryString(hKey, KeyBuffer, _T("Description"), pNameBuffer, TSIZEOF(pNameBuffer) ) == S_OK)
				{
					 if ( _tcscmp(pNameBuffer, pstAdapterName) == NULL )
					 {
						// we get here if the description matches the one passed in
						if ( (hRes = GetRegistryString(hKey, KeyBuffer, _T("ServiceName"), pNameBuffer, TSIZEOF(pNameBuffer) )) == S_OK)
							//now get the CLSID and copy it into the buffer passed into us.
							_tcsncpy(pstAdpaterCLSID, pNameBuffer, dwBufferLen);
					 }
				}
			}

			dwIdx++;
					
		}while ( ( (lRes == ERROR_MORE_DATA) || (lRes == ERROR_SUCCESS) )  && (hRes != S_OK));

		RegCloseKey(hKey);
	}
	
	return hRes == S_OK;
}

bool CEthernetRegistry::VerifyAdapterCLSID(_TCHAR *pstAdpaterCLSID)
{
_TCHAR Buffer[MAX_PATH];
HKEY hKey;
bool bSuccess;

	_tcscpy(Buffer, _T("SYSTEM\\"));
	_tcscat(Buffer, m_pstControlSet);
	_tcscat(Buffer, _T("\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	_tcscat(Buffer, pstAdpaterCLSID);

	bSuccess = RegOpenKeyEx(m_hRootKey, Buffer, NULL, KEY_QUERY_VALUE, &hKey) == ERROR_SUCCESS;

	if (bSuccess)
		RegCloseKey(hKey);

	return bSuccess;
}

bool CEthernetRegistry::SetIPSettings(EEthernetAdaptors eAdapter, SDeviceIPSettings *pSettings, DWORD dwItems)
{
_TCHAR	pstCLSID[MAX_PATH];

	if (GetVerifiedAdapterCLSID(eAdapter, pstCLSID, MAX_PATH))
		return SetIPSettings(pstCLSID, pSettings, dwItems);
	else
		return FALSE;
}

bool CEthernetRegistry::GetIPSettings(EEthernetAdaptors eAdapter, SDeviceIPSettings *pSettings)
{
_TCHAR	pstCLSID[MAX_PATH];

	if (GetVerifiedAdapterCLSID(eAdapter, pstCLSID, MAX_PATH))
		return GetIPSettings(pstCLSID, pSettings);
	else
		return FALSE;
}

bool CEthernetRegistry::GetVerifiedAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstCLSID, DWORD dwBufferLen)
{
_TCHAR	pstName[MAX_PATH];
bool bSuccess = FALSE;

	if (!GetUPXAdapterCLSID(eAdapter, pstCLSID, dwBufferLen))
	{
		if (GetUPXAdapterName(eAdapter, pstName, MAX_PATH))
		{
			bSuccess = FindAdpaterCLSIDByName(pstName, pstCLSID, dwBufferLen);
			if (bSuccess)
				SetUPXAdapterCLSID(eAdapter, pstCLSID);
		}

		return bSuccess;
	}
	else if (!VerifyAdapterCLSID(pstCLSID))
	{
		// if we cannot verify the stored CLSID, then toss it, and then call again recusively 
		//	to re-find it and save a new one or return failure.
		SetUPXAdapterCLSID(eAdapter, _T(""));
		return GetVerifiedAdapterCLSID(eAdapter, pstCLSID, dwBufferLen);
	}

	return TRUE;
}

bool CEthernetRegistry::SetIPSettings(_TCHAR *pstCLSID, SDeviceIPSettings *pSettings, DWORD dwItems)
{
HRESULT hRes = S_OK;
HKEY hKey;
_TCHAR Buffer[MAX_PATH];

	_tcscpy(Buffer, _T("SYSTEM\\"));
	_tcscat(Buffer, m_pstControlSet);
	_tcscat(Buffer, _T("\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	_tcscat(Buffer, pstCLSID);

	if ( RegOpenKey(m_hRootKey, Buffer, &hKey) == ERROR_SUCCESS)
	{
		// DHCP Enable
		if (dwItems & EDevIP_EnableDHCP)
		{
			hRes = RegSetValueEx(hKey, _T("EnableDHCP"), NULL, REG_DWORD, (BYTE*)&pSettings->dwEnableDHCP, 
				sizeof(pSettings->dwEnableDHCP)) == ERROR_SUCCESS ? S_OK : E_FAIL;
		}

		// DHCP IP Address
		if ((dwItems & EDevIP_DHCPIPAddress) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("DhcpIPAddress"), REG_SZ, pSettings->dhcp.szDHCPIPAddress, TRUE);

		// IP Address
		if ((dwItems & EDevIP_IPAddress) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("IPAddress"), REG_MULTI_SZ, pSettings->static_ip.szIPAddress, TRUE);

		// Subnet mask
		if ((dwItems & EDevIP_SubnetMask) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("SubnetMask"), REG_MULTI_SZ, pSettings->static_ip.szSubnetMask, TRUE);

		// Default gateway
		if ((dwItems & EDevIP_DefaultGateway) && (hRes == S_OK))
			hRes = SetRegistryString(hKey, NULL, _T("DefaultGateway"), REG_MULTI_SZ, pSettings->static_ip.szDefaultGateway, TRUE);

		// Name Servers
		if ((dwItems & EDevIP_NameServers) && (hRes == S_OK))
		{
		_TCHAR szBuffer[MAX_PATH];
		int nIdx  =0;

			do
			{
				_tcscpy(szBuffer, pSettings->static_ip.szNameServers[nIdx++]);
				_tcscat(szBuffer, _T(" "));
			}
			while (_tcslen(pSettings->static_ip.szNameServers[++nIdx]) && (_tcslen(szBuffer) < MAX_PATH-32));

			hRes = SetRegistryString(hKey, NULL, _T("NameServer, TRUE"), REG_SZ, szBuffer);
		}

		return hRes == S_OK;	
	}
	else
		return FALSE;
}

bool CEthernetRegistry::GetIPSettings(_TCHAR *pstCLSID, SDeviceIPSettings *pSettings)
{
HRESULT hRes = S_OK;
HKEY hKey;
_TCHAR Buffer[MAX_PATH];

	_tcscpy(Buffer, _T("SYSTEM\\"));
	_tcscat(Buffer,  m_pstControlSet);
	_tcscat(Buffer,  _T("\\Services\\Tcpip\\Parameters\\Interfaces\\"));
	_tcscat(Buffer,  pstCLSID);

	if ( RegOpenKey(m_hRootKey, Buffer, &hKey) == ERROR_SUCCESS)
	{
	DWORD dwLen;
	_TCHAR szBuffer[MAX_PATH], *pTemp, *pNext;

		dwLen = sizeof(pSettings->dwEnableDHCP);
		RegQueryValueEx(hKey, _T("EnableDHCP"), NULL, NULL, (LPBYTE)&pSettings->dwEnableDHCP, &dwLen) == ERROR_SUCCESS ? S_OK : E_FAIL;

		// load DHCP settings
		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("DhcpIPAddress"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPIPAddress, &dwLen);

		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("DhcpSubnetMask"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPSubnetMask, &dwLen);

		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("DhcpServer"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPServer, &dwLen);
		
		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("DhcpDefaultGateway"), NULL, NULL, (LPBYTE)pSettings->dhcp.szDHCPDefaultGateway, &dwLen);

		dwLen = MAX_PATH;
		RegQueryValueEx(hKey, _T("DhcpNameServer"), NULL, NULL, (LPBYTE)szBuffer, &dwLen);
		
		// parse name servers
		int nIdx = 0;
		pNext = szBuffer;
		while ( (pTemp = _tcschr(pNext, ' ')) && (nIdx < MAX_NAME_SERVERS) )
		{
			if (pTemp)
				_tcsncpy(pSettings->dhcp.szDHCPNameServers[nIdx], pNext, pTemp - pNext);
			else
				_tcsncpy(pSettings->dhcp.szDHCPNameServers[nIdx], pNext, MAX_IP_ADDRESS_LEN);

			pNext = pTemp+1;
			nIdx++;
		}

		// load static ip settings
		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("IPAddress"), NULL, NULL, (LPBYTE)pSettings->static_ip.szIPAddress, &dwLen);

		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("SubnetMask"), NULL, NULL, (LPBYTE)pSettings->static_ip.szSubnetMask, &dwLen);

		dwLen = MAX_IP_ADDRESS_LEN;
		RegQueryValueEx(hKey, _T("DefaultGateway"), NULL, NULL, (LPBYTE)pSettings->static_ip.szDefaultGateway, &dwLen);

		dwLen = MAX_PATH;
		RegQueryValueEx(hKey, _T("NameServer"), NULL, NULL, (LPBYTE)szBuffer, &dwLen);
		
		// parse name servers
		nIdx = 0;
		pNext = szBuffer;
		while ( (pTemp = _tcschr(pNext, ' ')) && (nIdx < MAX_NAME_SERVERS) )
		{
			if (pTemp)
				_tcsncpy(pSettings->static_ip.szNameServers[nIdx], pNext, pTemp - pNext);
			else
				_tcsncpy(pSettings->static_ip.szNameServers[nIdx], pNext, MAX_IP_ADDRESS_LEN);

			pNext = pTemp+1;
			nIdx++;
		}
		
		return TRUE;
	}
	else
		return FALSE;
}

/// *******************************
/// *** HIGHLY DEVICE DEPENDENT ***
/// *******************************
/// *** If we ever switch the ethernet card in the UPX or the TPMC-15, 17, 
/// *** this will need to be addressed accordingly.
bool CEthernetRegistry::SetSpeedLink(const _TCHAR* tszSpeedLink, const _TCHAR* tszCLSID)
{
	if (tszSpeedLink == NULL || tszSpeedLink[0] == 0 || tszCLSID == NULL || tszCLSID[0] == 0)
		return false;

	HRESULT hRes = E_FAIL;
	HKEY	hKey = NULL;
	DWORD	dwBfrSize = MAX_PATH;
	_TCHAR  tszBfr[MAX_PATH];

	// Build the registry key for setting the speed link 
	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\CLSID\0002]
	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\CLSID\0000]
	_tcscpy(tszBfr, _T("SYSTEM\\"));
	_tcscat(tszBfr, m_pstControlSet);
	_tcscat(tszBfr, _T("\\Control\\Class\\"));
	_tcscat(tszBfr, tszCLSID);

	// open the key to the network class guid
	if (::RegOpenKeyEx(m_hRootKey, tszBfr, 0, KEY_WRITE | KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
	{
		dwBfrSize = MAX_PATH;
		FILETIME fileTime;

		// enumerate through each of the entries under the class id.
		for (DWORD dwIndex = 0; (hRes = ::RegEnumKeyEx(hKey, dwIndex, tszBfr, &dwBfrSize, NULL, NULL, NULL, &fileTime)) == ERROR_SUCCESS ; dwIndex++, dwBfrSize=MAX_PATH)
		{
			_TCHAR tszValue[MAX_PATH];
			if (GetRegistryString(hKey, tszBfr, _T("DriverDesc"), tszValue, MAX_PATH) == S_OK)
			{
				const _TCHAR* tszName = NULL;
				EEthernetSpeedLinkEntry* pSpeedTable = NULL;

				// check if it is the intel nic
				if (_tcsicmp(tszValue, tszIntelNIC) == 0)
				{
					pSpeedTable = NICIntel_EEthernetSpeedLinkTable;
					tszName		= tszIntelLink;
				}
				// check if it is the via nic
				else if (_tcsicmp(tszValue, tszViaNIC) == 0)
				{
					pSpeedTable = NICVia_EEthernetSpeedLinkTable;
					tszName		= tszViaLink;
				}

				// if we found any one of the nics, set the 
				if (pSpeedTable != NULL)
				{
					// find the entry
					EEthernetSpeedLink  speedLink = EEnetSpeedLink_AutoDetect;
					int i = EEthernetSpeedLinkEntry::findEntry(pSpeedTable, tszSpeedLink);
					if (i != -1)
					{
						// convert the type to a string
						_itot(pSpeedTable[i].type, tszValue, 10);

						// set the registry
						hRes = SetRegistryString(hKey, tszBfr, tszName, REG_SZ, tszValue, TRUE);
					}
					break;
				}
			}
		}

		ATLTRACE(_T("FAILED Set Speed Link: error=%u (0x%x)\r\n"), hRes, hRes);
		
		::RegCloseKey(hKey);
	}

	return hRes == S_OK ? true : false;
}

/// *******************************
/// *** HIGHLY DEVICE DEPENDENT ***
/// *******************************
/// *** If we ever switch the ethernet card in the UPX or the TPMC-15, 17, 
/// *** this will need to be addressed accordingly.
bool CEthernetRegistry::GetSpeedLink(_TCHAR* tszSpeedLink, const _TCHAR* tszCLSID)
{
	if (tszSpeedLink == NULL || tszCLSID == NULL || tszCLSID[0] == 0)
		return false;

	HRESULT hRes = E_FAIL;
	HKEY	hKey = NULL;
	DWORD	dwBfrSize = MAX_PATH;
	_TCHAR  tszBfr[MAX_PATH];

	// Build the registry key for setting the speed link 
	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\CLSID\0002]
	// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\CLSID\0000]
	_tcscpy(tszBfr, _T("SYSTEM\\"));
	_tcscat(tszBfr, m_pstControlSet);
	_tcscat(tszBfr, _T("\\Control\\Class\\"));
	_tcscat(tszBfr, tszCLSID);

	if (::RegOpenKeyEx(m_hRootKey, tszBfr, 0, KEY_READ | KEY_ENUMERATE_SUB_KEYS, &hKey) == ERROR_SUCCESS)
	{
		dwBfrSize = MAX_PATH;
		FILETIME fileTime;

		// enumerate through each of the entries under the class id.
		for (DWORD dwIndex = 0; (hRes = ::RegEnumKeyEx(hKey, dwIndex, tszBfr, &dwBfrSize, NULL, NULL, NULL, &fileTime)) == ERROR_SUCCESS ; dwIndex++, dwBfrSize=MAX_PATH)
		{
			// find the correct driver entry
			_TCHAR tszValue[MAX_PATH];
			if (GetRegistryString(hKey, tszBfr, _T("DriverDesc"), tszValue, MAX_PATH) == S_OK)
			{
				const _TCHAR* tszName = NULL;
				EEthernetSpeedLinkEntry* pSpeedTable = NULL;

				if (_tcsicmp(tszValue, tszIntelNIC) == 0)
				{
					pSpeedTable = NICIntel_EEthernetSpeedLinkTable;
					tszName		= tszIntelLink;
				}
				else if (_tcsicmp(tszValue, tszViaNIC) == 0)
				{
					pSpeedTable = NICVia_EEthernetSpeedLinkTable;
					tszName		= tszViaLink;
				}

				if (pSpeedTable != NULL)
				{
					// assuming the tszSpeedLink is big enough
					if (GetRegistryString(hKey, tszBfr, tszName, tszSpeedLink, MAX_PATH) == S_OK)
					{
						// find the entry
						EEthernetSpeedLink  speedLink = (EEthernetSpeedLink)_ttoi(tszSpeedLink);
						int i = EEthernetSpeedLinkEntry::findEntry(pSpeedTable, speedLink);
						if (i != -1)
						{
							_tcscpy(tszSpeedLink, pSpeedTable[i].name);
						}
					}
					break;
				}
			}
		}

		ATLTRACE(_T("FAILED Get Speed Link: error=%u (0x%x)\r\n"), hRes, hRes);
		
		::RegCloseKey(hKey);
	}

	return hRes == S_OK ? true : false;
}

CmPCConfig_DeclSpec HRESULT CfgEthernetGetMAC(BYTE *pMacAddress)
{
	USES_CONVERSION;

	HRESULT hr = E_FAIL;
	TCHAR szPublicAdaptorName[MAX_PATH];

	// find out what our Public network adaptor is
	CRegKey regPublicNet;
	if (ERROR_SUCCESS == 
		regPublicNet.Open(HKEY_LOCAL_MACHINE,g_szInternalPublic,KEY_READ))
	{
		DWORD dwSize = TSIZEOF(szPublicAdaptorName);
#if (_MSC_VER >= 1400) // VS2005 Compiler
		if (ERROR_SUCCESS == regPublicNet.QueryStringValue(g_szName,szPublicAdaptorName,&dwSize))
#else // other than VS2005 Compiler
		if (ERROR_SUCCESS == regPublicNet.QueryValue(szPublicAdaptorName,g_szName,&dwSize))
#endif // (_MSC_VER >= 1400) VS2005 Compiler
		{
			// now get our mac address
			PIP_ADAPTER_INFO pAdapterInfo = NULL;
			DWORD dwBufLen = 0;
			GetAdaptersInfo(NULL,&dwBufLen);

			bool fFoundIt = false;
			pAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[dwBufLen];
			if (ERROR_SUCCESS == GetAdaptersInfo(pAdapterInfo,&dwBufLen))  //lint !e433
			{
				PIP_ADAPTER_INFO pInfo = pAdapterInfo; 
				do 
				{
					if (!_tcsnicmp (A2T(pInfo->Description),szPublicAdaptorName,lstrlen(szPublicAdaptorName)))
					{
						memcpy(pMacAddress,pInfo->Address,MAX_ADAPTER_ADDRESS_LENGTH);
						hr = S_OK;
						break;
					}
					pInfo = pInfo->Next;    // Progress through 
				} while(pInfo);                    // Terminate if last adapter
			}
			delete [] pAdapterInfo;

		}
	}


	return hr;


}

/////////////////////////////////////////////////////////////////////
// TABLE for setting the Link Speed
// Intel(R) PRO/100 VE Network Connection
const _TCHAR tszIntelNIC[]	= _T("Intel(R) PRO/100 VE Network Connection");
const _TCHAR tszIntelLink[]	= _T("SpeedDuplex");

EEthernetSpeedLinkEntry NICIntel_EEthernetSpeedLinkTable[] = 
{
	EEthernetSpeedLinkEntry(EEnetSpeedLink_AutoDetect,					_T("ON")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICIntel_10Mbps_HalfDuplex,	_T("10HALF")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICIntel_10Mbps_FullDuplex,	_T("10FULL")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICIntel_100Mbps_HalfDuplex,	_T("100HALF")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICIntel_100Mbps_FullDuplex,	_T("100FULL")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_Unkown, NULL)
};

/////////////////////////////////////////////////////////////////////
// TABLE for setting the Link Speed
// VIA Rhine II Fast Ethernet Adapter
const _TCHAR tszViaNIC[]	= _T("VIA Rhine II Fast Ethernet Adapter");
const _TCHAR tszViaLink[]	= _T("ConnectionType");

EEthernetSpeedLinkEntry NICVia_EEthernetSpeedLinkTable[] = 
{
	EEthernetSpeedLinkEntry(EEnetSpeedLink_AutoDetect,					_T("ON")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICVia_10Mbps_HalfDuplex,	_T("10HALF")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICVia_10Mbps_FullDuplex,	_T("10FULL")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICVia_100Mbps_HalfDuplex,	_T("100HALF")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_NICVia_100Mbps_FullDuplex,	_T("100FULL")),
	EEthernetSpeedLinkEntry(EEnetSpeedLink_Unkown, NULL)
};

