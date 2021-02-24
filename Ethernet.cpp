////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Ethernet.cpp
///
/// Functions for Ethernet adaptor and network settings.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "CmPCConfig.h"
#include "Registry.h"
#include "Ethernet.h"
#include "EthernetCore.h"
#include "privileges.h"
#include <Ipexport.h>
#include <Iphlpapi.h>

#define  INTERNAL_ADAPTER_NAME_PRIVATE	_T("EthernetInternal")
#define  INTERNAL_ADAPTER_NAME_PUBLIC	_T("EthernetExternal")
#define  CLSID_NETWORK_CLASS			_T("{4D36E972-E325-11CE-BFC1-08002bE10318}")

#ifndef UNDER_CE
	#define BOOT_IMAGE_CONFIG_PATH	_T("C:\\BOOT\\SYSTEM32\\CONFIG")

	#define _DUPLICATE_IN_BOOT
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Obtains or sets the enabled state of the Ethernet controller.
/// This is done by enabling or disabling the appropriate network adaptor.
/// Set HKCC\System\CurrentControlSet\Enum\PCI\{adaptor pci pnp dev id}\{instance id}\CSConfigFlags
///	to 1 and reboot to disable, or set to 0 and reboot to enable.
///
/// @param       piResult     the result is placed in this pre-allocated integer. 0 = Disabled, 1 = Enabled
/// @param       piCmd = NULL if this points to a valid integer, the ethernet controller is 
///								enabled (if 1) or disable d(if 0) accordingly.
///
/// @author      William Levine
/// @date        01/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetEnable(int *piResult, int *piCmd /* = NULL */)
{
	if (piResult)
	{
		// assume always enabled now and do not try to disable since we don't know how!
		*piResult = 1;

		return S_OK;
	}

	return E_POINTER;
}
#endif
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Obtains the CLSID relating to the adapter specified by the enum value.
///
/// @param       hRootKey     
/// @param       eAdaptor    
/// @param       pstCLSID    
/// @param       dwBufferLen 
///
/// @author      William Levine
/// @date        03/16/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetAdaptorCLSID(HKEY hRootKey, EEthernetAdaptors eAdaptor, _TCHAR *pstCLSID, DWORD dwBufferLen)
{
CEthernetRegistry	Reg(hRootKey);

	return Reg.GetVerifiedAdapterCLSID(eAdaptor, pstCLSID, dwBufferLen) ? S_OK : E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets IP settings according to the adaptor spec, placing them in the setting struct.
///
/// @param       hRootKey		The root key of the registry to use, usually HKEY_LOCAL_MACHINE.     
/// @param       eAdaptor		The adaptor to get settings for..
/// @param       pSettings		The settings.
///
/// @author      William Levine
/// @date        03/03/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetIPSettings(HKEY hRootKey, EEthernetAdaptors eAdaptor, SDeviceIPSettings *pSettings)
{
CEthernetRegistry	Reg(hRootKey);

	return Reg.GetIPSettings(eAdaptor, pSettings) ? S_OK : E_FAIL;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets IP settings according to the adaptor spec, the setting struct, and the flags indicating
/// which entries of the struct to set.
///
/// @param       hRootKey		The root key of the registry to use, usually HKEY_LOCAL_MACHINE.     
/// @param       eAdaptor		The adaptor to set.
/// @param       pSettings		The settings.
/// @param       dwItems		Flags indicating which settings to apply.
///
/// @author      William Levine
/// @date        03/03/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetSetIPSettings(HKEY hRootKey, EEthernetAdaptors eAdaptor, SDeviceIPSettings *pSettings, DWORD dwItems)
{
HRESULT hRes;
CEthernetRegistry	Reg(hRootKey);

#ifndef _DEBUG
	hRes = Reg.SetIPSettings(eAdaptor, pSettings, dwItems) ? S_OK : E_FAIL;
#else
	hRes = S_OK;
#endif

	#ifdef _DUPLICATE_IN_BOOT		
	HKEY hBootRoot;

	// load the offline registry
	if ( LoadOfflineHive(BOOT_IMAGE_CONFIG_PATH, &hBootRoot) == S_OK)
	{
	CEthernetRegistry	OfflineReg(hBootRoot);
	_TCHAR pstName[MAX_PATH], pstCLSID[MAX_PATH];
			
		_tcscpy(OfflineReg.m_pstControlSet, _T("ControlSet001"));
		if ( Reg.GetUPXAdapterName(eAdaptor, pstName, MAX_PATH) &&
			 OfflineReg.FindAdpaterCLSIDByName(pstName, pstCLSID, MAX_PATH) )
			OfflineReg.SetIPSettings(pstCLSID, pSettings, dwItems);

		UnLoadOfflineHive(BOOT_IMAGE_CONFIG_PATH, hBootRoot, hRes == S_OK);
	}
	#endif	

	return hRes;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the PC's Computer Name. The PC must be rebooted before these settings take effect.
/// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\ComputerName\ComputerName
///
///	@param pszComputerName	
///
/// @author      William Levine
/// @date        01/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetComputerName(_TCHAR *pszComputerName)	
{
	DWORD dwBufSize = CFG_HOSTNAME_SIZE;

	return GetComputerName(pszComputerName, &dwBufSize) ? S_OK : E_FAIL;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the PC's Computer Name.  The PC must be rebooted before these settings take effect.
/// HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Services\Tcpip\Parameters\NV Hostname
///
/// @author      William Levine
/// @date        01/12/04
/// @return      jp - 5.12.04 Returns ERROR_ALREADY_EXISTS if the given computer name is already 
///              present on the network
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetSetComputerName(const _TCHAR *pszComputerName, BOOL fAllowDuplicate)
{
	HRESULT hRes = S_OK;

	if(_tcslen( pszComputerName ) > MAX_COMPUTERNAME_LENGTH)
	{
		hRes = HRESULT_FROM_WIN32(ERROR_INSUFFICIENT_BUFFER);
		return hRes;
	}

	if(_tcslen( pszComputerName ) <= 0 )
	{
		hRes = HRESULT_FROM_WIN32(ERROR_NO_SUCH_USER);
		return hRes;
	}

	TCHAR szCurName[MAX_COMPUTERNAME_LENGTH + 1];
	DWORD size = MAX_COMPUTERNAME_LENGTH + 1;
	::GetComputerName(szCurName,&size);
	if (lstrcmpi(pszComputerName,szCurName)) // see if its not already set to this
	{
		WSADATA wsdDummy;
		if (WSAStartup(MAKEWORD(1,1), &wsdDummy) == 0)
		{
			USES_CONVERSION;
			if (gethostbyname( CT2A(pszComputerName) )) // make sure it doesn't already exist
			{
				if (!fAllowDuplicate)
				{
					hRes = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
				}
			}
			WSACleanup();
		}
	}

	if (FAILED(hRes))
		return hRes;

   	if (!SetComputerNameEx(ComputerNamePhysicalDnsHostname,pszComputerName))
		hRes = GetLastError();

	//
	// Change the TCP/IP name
	//
	HKEY hKey;
	if (ERROR_SUCCESS == RegCreateKey( HKEY_LOCAL_MACHINE, 
		_T("System\\CurrentControlSet\\Services\\Tcpip\\Parameters"), &hKey )) 
	{
		if (ERROR_SUCCESS != RegSetValueEx( hKey, _T("Hostname"), 0, REG_SZ, (PBYTE) pszComputerName, 
			(DWORD)(_tcslen( pszComputerName )+1) * sizeof(TCHAR)))
			hRes = E_FAIL;
		RegCloseKey( hKey );
	}
	else
		hRes = E_FAIL;

	// Force these also, it seems that the ActiveComputerName is set on 
	// shutdown somehow and we need to make sure the key is properly set before
	// we do our commit.
	if (ERROR_SUCCESS == RegCreateKey( HKEY_LOCAL_MACHINE, 
		_T("System\\CurrentControlSet\\Control\\ComputerName\\ComputerName"), &hKey )) 
	{
		if (ERROR_SUCCESS != RegSetValueEx( hKey, _T("ComputerName"), 0, REG_SZ, (PBYTE) pszComputerName, 
			(DWORD)(_tcslen( pszComputerName )+1) * sizeof(TCHAR)))
			hRes = E_FAIL;
		RegCloseKey( hKey );
	}
	else
		hRes = E_FAIL;

	if (ERROR_SUCCESS == RegCreateKey( HKEY_LOCAL_MACHINE, 
		_T("System\\CurrentControlSet\\Control\\ComputerName\\ActiveComputerName"), &hKey )) 
	{
		if (ERROR_SUCCESS != RegSetValueEx( hKey, _T("ComputerName"), 0, REG_SZ, (PBYTE) pszComputerName, 
			(DWORD)(_tcslen( pszComputerName )+1) * sizeof(TCHAR)))
			hRes = E_FAIL;
		RegCloseKey( hKey );
	}
	else
		hRes = E_FAIL;


	if (ERROR_SUCCESS == RegCreateKey(HKEY_USERS, 
		_T(".Default\\Software\\Microsoft\\Windows\\ShellNoRoam"), &hKey )) 
	{
		if (ERROR_SUCCESS != RegSetValueEx( hKey, NULL, 0, REG_SZ, (PBYTE) pszComputerName, 
			(DWORD)(_tcslen( pszComputerName )+1) * sizeof(TCHAR)))
			hRes = E_FAIL;
		RegCloseKey( hKey );
	}
	else
		hRes = E_FAIL;

	return hRes;
}
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Gets the PC's workgroup
///
///	@param ppstHostName	
/// @return ERROR_MEMBER_NOT_IN_GROUP if the computer is not in a workgroup
///
/// @author      John Pavlik
/// @date        05/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetWorkgroup(_TCHAR *pszWorkgroup)	
{
	NETSETUP_JOIN_STATUS status;
	LPWSTR lpWNameBuffer;
	if (NERR_Success == NetGetJoinInformation(NULL,
				  &lpWNameBuffer, &status ))
	{
		if (status != NetSetupWorkgroupName)
			return HRESULT_FROM_WIN32(ERROR_MEMBER_NOT_IN_GROUP);

		USES_CONVERSION;
		LPCTSTR pNameBuffer = W2T(lpWNameBuffer);
		lstrcpy(pszWorkgroup,pNameBuffer);

		return S_OK;

	}
	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the PC's workgroup. The PC must be rebooted before these settings take effect.
///
/// @author      John Pavlik
/// @date        05/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetSetWorkgroup(const _TCHAR *pszWorkgroup)	
{	
	HRESULT hRes = S_OK;

	CPrivileges priv;
	if (priv.EnablePrivilege(SE_MACHINE_ACCOUNT_NAME,TRUE))
	{
		USES_CONVERSION;
		NET_API_STATUS ret = NetJoinDomain(NULL,CT2W(pszWorkgroup),NULL,NULL,NULL,0);
		if (NERR_Success !=	ret)
			hRes = ret;

	} 
	
	return hRes;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the PC's DNS domain name (primary DNS suffix of the computer)
///
///	@param pszDomainName
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetSetDomainName(const _TCHAR* pszDomainName)
{
//	if (pszDomainName == NULL || pszDomainName[0] == 0)
//		return E_FAIL;

	if (::SetComputerNameEx(ComputerNamePhysicalDnsDomain, pszDomainName))
		return S_OK;
	
	ATLTRACE(_T("FAILED Set DNS Domain Name: error=%u (0x%x)\r\n"), ::GetLastError(), ::GetLastError());

	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the PC's DNS domain name (primary DNS suffix of the computer)
///
///	@param pszDomainName : should be at least MAX_COMPUTERNAME_LENGTH, may be more.
/// @param dwBfrSize : size of the buffer passed in, if it fails, a non-zero value on the return
///					will indicate the required size.
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetDomainName(_TCHAR* pszDomainName, DWORD& dwBfrSize)
{
	if (pszDomainName == NULL || dwBfrSize == 0)
		return E_FAIL;

	// get the size of the string
	DWORD dwRequiredSize = 0;
	BOOL bRes = ::GetComputerNameEx(ComputerNamePhysicalDnsDomain, NULL, &dwRequiredSize);
	if (bRes == FALSE)
	{
		if (::GetLastError() != ERROR_MORE_DATA)
		{
			ATLTRACE(_T("FAILED Get DNS Domain Name: error=%u (0x%x)\r\n"), ::GetLastError(), ::GetLastError());
			dwBfrSize = 0;
			return E_FAIL;
		}
		else if (dwRequiredSize > dwBfrSize)
		{
			ATLTRACE(_T("FAILED Get DNS Domain Name: error=ERROR_MORE_DATA, required size=%u\r\n"), dwRequiredSize);
			dwBfrSize = dwRequiredSize;
			return E_FAIL;
		}
	}

	// get the string
	if (::GetComputerNameEx(ComputerNamePhysicalDnsDomain, pszDomainName, &dwBfrSize))
		return S_OK;

	ATLTRACE(_T("FAILED Get DNS Domain Name: error=%u (0x%x)\r\n"), ::GetLastError(), ::GetLastError());

	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Find either internal/external adapter.
/// 
///	@param enetAdapter : type of adapter (internal or external).
///	@param pAdapterAddrList : a valid adapter list.
///	@param pAdapterAddr : the adapter (do not delete).
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetFindAdapterAddr(const EEthernetAdaptors enetAdapter, IP_ADAPTER_ADDRESSES* pAdapterAddrList, IP_ADAPTER_ADDRESSES*& pAdapterAddr)
{
	USES_CONVERSION;

	if (pAdapterAddrList == NULL)
		return E_FAIL;

	const _TCHAR* tszAdapterName = NULL;
	switch (enetAdapter)
	{
		case EEnetAdaptor_InternalPrivate: 
		{
			tszAdapterName = INTERNAL_ADAPTER_NAME_PRIVATE;
		} break;
		
		case EEnetAdaptor_InternalPublic:
		{
			tszAdapterName = INTERNAL_ADAPTER_NAME_PUBLIC;
		} break;
		
		default:
			return E_FAIL;
	}

	// get the adapter description
	_TCHAR tszAdapterDesc[MAX_ADAPTER_NAME];
	CEthernetRegistry  registry(HKEY_LOCAL_MACHINE);
	if (registry.GetUPXAdapterName(enetAdapter, tszAdapterDesc, MAX_ADAPTER_NAME) == false)
	{
		ATLTRACE(_T("FAILED get adapter description\r\n"));
		return E_FAIL;
	}
	
	// If successful, output some information from the data we received
	while (pAdapterAddrList)
	{
		if (_tcsicmp(W2CT(pAdapterAddrList->FriendlyName), tszAdapterName) == 0 
			&& _tcsnicmp(W2CT(pAdapterAddrList->Description), tszAdapterDesc, _tcslen(tszAdapterDesc)) == 0)
		{
			pAdapterAddr = pAdapterAddrList;
			return S_OK;
		}
		
		pAdapterAddrList = pAdapterAddrList->Next;
	}
	
	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Find either internal/external adapter.
/// 
///	@param enetAdapter : type of adapter (internal or external).
///	@param pAdapterInfoList : a valid adapter list.
///	@param pAdapterInfo : the adapter (do not delete).
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetFindAdapterInfo(const EEthernetAdaptors enetAdapter, IP_INTERFACE_INFO* pAdapterInfoList, IP_ADAPTER_INDEX_MAP*& pAdapterInfo)
{
	USES_CONVERSION;

	if (pAdapterInfoList == NULL)
		return E_FAIL;

	const _TCHAR* tszAdapterName = NULL;
	switch (enetAdapter)
	{
		case EEnetAdaptor_InternalPrivate: 
		{
			tszAdapterName = INTERNAL_ADAPTER_NAME_PRIVATE;
		} break;
		
		case EEnetAdaptor_InternalPublic:
		{
			tszAdapterName = INTERNAL_ADAPTER_NAME_PUBLIC;
		} break;
		
		default:
			return E_FAIL;
	}

	// get the adapter description
	_TCHAR tszAdapterClsid[MAX_ADAPTER_NAME];
	CEthernetRegistry  registry(HKEY_LOCAL_MACHINE);
	if (registry.GetUPXAdapterCLSID(enetAdapter, tszAdapterClsid, MAX_ADAPTER_NAME) == false)
	{
		ATLTRACE(_T("FAILED get adapter clsid\r\n"));
		return E_FAIL;
	}
	
	// If successful, output some information from the data we received
	for (int i=0 ; i<pAdapterInfoList->NumAdapters ; i++)
	{
		
		if (_tcsstr(W2CT(pAdapterInfoList->Adapter[i].Name), tszAdapterClsid) != NULL)
		{
			pAdapterInfo = &(pAdapterInfoList->Adapter[i]);
			return S_OK;
		}
	}
	
	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Get the network adapter list, use CfgEthernetDeleteAdapterAddr() to delete the list once
/// the client is finished with the list.
/// 
///	@param pAdapterAddrList : a valid adapter list.
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetAdapterInfoList(IP_INTERFACE_INFO*& pAdapterInfoList)
{
	// Declare and initialize variables
	ULONG ulBfrSize = sizeof(IP_INTERFACE_INFO);
	pAdapterInfoList = (IP_INTERFACE_INFO*)(new BYTE[ulBfrSize]);
	
	// Make an initial call to GetInterfaceInfo to get the necessary size in the ulBfrSize variable
	DWORD dwRes = ::GetInterfaceInfo(pAdapterInfoList, &ulBfrSize);
	if (dwRes != ERROR_SUCCESS)
	{
		delete[] pAdapterInfoList;
		if (dwRes == ERROR_INSUFFICIENT_BUFFER)
		{
			pAdapterInfoList = (IP_INTERFACE_INFO*)(new BYTE[++ulBfrSize]);
		}
		else
		{
			pAdapterInfoList = NULL;
			ATLTRACE(_T("FAILED to get adapter list: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
			return E_FAIL;
		}
	}

	dwRes = ::GetInterfaceInfo(pAdapterInfoList, &ulBfrSize);
	if (dwRes != ERROR_SUCCESS)
	{
		delete[] pAdapterInfoList;
		pAdapterInfoList = NULL;

		ATLTRACE(_T("FAILED to get adapter list: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
		return E_FAIL;
	}
	
	return S_OK;
}	
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Delete the allocated adapter address list, call this in conjunction with CfgEthernetGetAdapterInfoList()
/// 
///	@param pAdapterInfoList : a valid adapter list, allocated by CfgEthernetGetAdapterInfoList().
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetDeleteAdapterInfoList(IP_INTERFACE_INFO*& pAdapterInfoList)
{
	if (pAdapterInfoList != NULL)
	{
		delete[] pAdapterInfoList;
		pAdapterInfoList = NULL;
	}

	if (pAdapterInfoList != NULL)
		return E_FAIL;
	return S_OK;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Get the network adapter list, use CfgEthernetDeleteAdapterAddr() to delete the list once
/// the client is finished with the list.
/// 
///	@param pAdapterAddrList : a valid adapter list.
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetAdapterAddr(IP_ADAPTER_ADDRESSES*& pAdapterAddrList)
{
	pAdapterAddrList = NULL;

	ULONG ulBfrSize = sizeof(IP_ADAPTER_ADDRESSES);
	pAdapterAddrList = (IP_ADAPTER_ADDRESSES*)(new BYTE[ulBfrSize]);

	// get the required buffer size for the adapter list
	DWORD dwRes = ::GetAdaptersAddresses(AF_INET, 0, NULL, pAdapterAddrList, &ulBfrSize);
	if (dwRes != ERROR_SUCCESS)
	{
		delete[] pAdapterAddrList;
		if (dwRes == ERROR_BUFFER_OVERFLOW) 
		{
			pAdapterAddrList = (IP_ADAPTER_ADDRESSES*)(new BYTE[++ulBfrSize]);
		}
		else
		{
			pAdapterAddrList = NULL;
			ATLTRACE(_T("FAILED to get adapter list: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
			return E_FAIL;
		}
	}

	// get the adapter list
	dwRes = ::GetAdaptersAddresses(AF_INET, 0, NULL, pAdapterAddrList, &ulBfrSize);
	if (dwRes != ERROR_SUCCESS)
	{
		delete[] pAdapterAddrList;
		pAdapterAddrList = NULL;

		ATLTRACE(_T("FAILED to get adapter list: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
		return E_FAIL;
	}
	
	return S_OK;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Delete the allocated adapter address list, call this in conjunction with CfgEthernetGetAdapterAddr()
/// 
///	@param pAdapterAddrList : a valid adapter list, allocated by CfgEthernetGetAdapterAddr().
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetDeleteAdapterAddr(IP_ADAPTER_ADDRESSES*& pAdapterAddrList)
{
	if (pAdapterAddrList != NULL)
	{
		delete[] pAdapterAddrList;
		pAdapterAddrList = NULL;
	}

	if (pAdapterAddrList != NULL)
		return E_FAIL;
	return S_OK;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Renews the DHCP IP address
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetDHCPRenew()
{
	USES_CONVERSION;

	IP_INTERFACE_INFO*		pAdapterList	= NULL;
	IP_ADAPTER_INDEX_MAP*	pAdapter		= NULL;

	// retrieve the adapter list
	if (CfgEthernetGetAdapterInfoList(pAdapterList) != S_OK)
		return E_FAIL;

	if (CfgEthernetFindAdapterInfo(EEnetAdaptor_InternalPublic, pAdapterList, pAdapter) != S_OK)
	{
		CfgEthernetDeleteAdapterInfoList(pAdapterList);
		return E_FAIL;
	}
	
	// release the ip address
	DWORD dwRes = IpReleaseAddress(pAdapter);
	if (dwRes != NO_ERROR)
	{
		CfgEthernetDeleteAdapterInfoList(pAdapterList);
		ATLTRACE(_T("FAILED release DHCP IP address: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
		return E_FAIL;
	}

	// renew the ip address
	dwRes = IpRenewAddress(pAdapter);
	if (dwRes != NO_ERROR)
	{
		CfgEthernetDeleteAdapterInfoList(pAdapterList);
		ATLTRACE(_T("FAILED renew DHCP IP address: adapter=, error=%u (0x%x)\r\n"), dwRes, dwRes);
		return E_FAIL;
	}

	CfgEthernetDeleteAdapterInfoList(pAdapterList);
	return S_OK;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the speed link of the ethernet device
///
///	@param speedLink : 1 of several enumerated settings.
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetSetSpeedLink(const _TCHAR* tszSpeedLink)
{
	// Network adapters: class Net
	const _TCHAR tszCLSID[] = CLSID_NETWORK_CLASS;

	CEthernetRegistry  registry(HKEY_LOCAL_MACHINE);
	if (registry.SetSpeedLink(tszSpeedLink, tszCLSID) == true)
		return S_OK;

	ATLTRACE(_T("FAILED Set Speed Link: error=%u (0x%x)\r\n"), ::GetLastError(), ::GetLastError());
	return E_FAIL;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the speed link of the ethernet device
/// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\CLSID\0002\Ndi\params]
///		SpeedDuplex : 
///
///	@param speedLink : 1 of several enumerated settings.
///
/// @author      Rob LoPresti
/// @date        01/19/05
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetSpeedLink(_TCHAR* tszSpeedLink)
{
	// Network adapters: class Net
	const _TCHAR tszCLSID[] = CLSID_NETWORK_CLASS;

	CEthernetRegistry  registry(HKEY_LOCAL_MACHINE);
	if (registry.GetSpeedLink(tszSpeedLink, tszCLSID) == true)
		return S_OK;

	ATLTRACE(_T("FAILED Get Speed Link: error=%u (0x%x)\r\n"), ::GetLastError(), ::GetLastError());
	return E_FAIL;
}
#endif

