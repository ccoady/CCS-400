////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Ethernet.h
///
/// Header for functions for Ethernet adaptor and network settings.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_ETHERNET_H_)
#define _ETHERNET_H_

#include "EthernetDefs.h"


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Retrieve the NETBIOS name of the computer.
///
/// @param       ppstComputerNameShould be a buffer of length CFG_HOSTNAME_SIZE TCHARs
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef MAX_COMPUTERNAME_LENGTH
	#define MAX_COMPUTERNAME_LENGTH 15
#endif
#define CFG_HOSTNAME_SIZE (MAX_COMPUTERNAME_LENGTH + 1)

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Retrieve the Network ID for this system.
/// This retrieves the computer name.
///
/// @param       ppstComputerNamePointer to buffer for the computer name
///
/// @author      John Pavlik
/// @date        06/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgEthernetGetComputerName(_TCHAR *ppstComputerName);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the Network ID of this computer.
/// This function uses SetComputerNameEx as well as manually setting the appropriate registry
/// entries.  In a system without protected storage, only the SetComputerNameEx should be necessary
/// but that won't work for the UPX
///
/// @param       pstComputerName New computer name
/// @param       fAllowDuplicate If TRUE will allow the user to set the computer to a name that
///              already exists on the network.  Use with caution!!  If fAllowDuplicate is FALSE
///              this method can return ERROR_ALREADY_EXISTS.
///
/// @author      John Pavlik
/// @date        06/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgEthernetSetComputerName(const _TCHAR *pstComputerName, BOOL fAllowDuplicate);

#define CFG_WORKGROUP_SIZE CFG_HOSTNAME_SIZE
CmPCConfig_DeclSpec HRESULT CfgEthernetGetWorkgroup(_TCHAR *ppstWorkgroup);
CmPCConfig_DeclSpec HRESULT CfgEthernetSetWorkgroup(const _TCHAR *pstWorkgroup);

CmPCConfig_DeclSpec HRESULT CfgEthernetGetAdaptorCLSID(HKEY hRootKey, EEthernetAdaptors eAdaptor, _TCHAR *pstCLSID, DWORD dwBufferLen);

CmPCConfig_DeclSpec HRESULT CfgEthernetGetIPSettings(HKEY hRootKey, EEthernetAdaptors eAdaptor, SDeviceIPSettings *pSettings);
CmPCConfig_DeclSpec HRESULT CfgEthernetSetIPSettings(HKEY hRootKey, EEthernetAdaptors eAdaptor, SDeviceIPSettings *pSettings, DWORD dwItems);


////////////////////////////////////////////////////////////////////////////////
/// Retrieve our Mac address for our built in adaptor.
///
/// @param       pMacAddress must be at least MAX_ADAPTER_ADDRESS_LENGTH
///
/// @author      John Pavlik
/// @date        12/17/04
////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgEthernetGetMAC(BYTE *pMacAddress);

//#include <winsock2.h>
#ifdef _WINSOCK2API_

#include <Iptypes.h>
CmPCConfig_DeclSpec HRESULT CfgEthernetSetDomainName(const _TCHAR* pszDomainName);
CmPCConfig_DeclSpec HRESULT CfgEthernetGetDomainName(_TCHAR* pszDomainName, DWORD& dwBfrSize);
CmPCConfig_DeclSpec HRESULT CfgEthernetDHCPRenew();
CmPCConfig_DeclSpec HRESULT CfgEthernetGetSpeedLink(_TCHAR* tszSpeedLink);
CmPCConfig_DeclSpec HRESULT CfgEthernetSetSpeedLink(const _TCHAR* tszSpeedLink);
CmPCConfig_DeclSpec HRESULT CfgEthernetFindAdapterAddr(const EEthernetAdaptors enetAdapter, IP_ADAPTER_ADDRESSES* pAdapterAddrList, IP_ADAPTER_ADDRESSES*& pAdapterAddr);
CmPCConfig_DeclSpec HRESULT CfgEthernetGetAdapterAddrList(IP_ADAPTER_ADDRESSES*& pAdapterAddrList);
CmPCConfig_DeclSpec HRESULT CfgEthernetDeleteAdapterAddr(IP_ADAPTER_ADDRESSES*& pAdapterAddrList);

#else // _WINSOCK2API_ is not defined

//#	error winsock2.h is not included defined

#endif


#endif