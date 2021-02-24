#pragma once;

#include "CmPCConfig.h"
#include <iphlpapi.h>
#include "CEthernet.hpp"
#include "CEthernetExt.hpp"


//GOALS:
// 1. For Wired ethernet interface, configure static or dhcp addresses for IP, DNS, and WINS
// 2. For Wireless ethernet interface, configure static or dhcp address for IP, DNS, and WINS


//---------------------------------------------------------------------------
// USAGE: bRetv = CfgGetResponseString(tResponseBuf, ResponseBufLen/*in&out*/);
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgGetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen, tstring& tResponse );

// by descripton field
CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(tstring tDescripton, CAdapterInfo *pAdapterInfo, tstring& tResponse );
CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(TCHAR byDescripton[], CAdapterInfo *pAdapterInfo, tstring& tResponse );

//
// Returns information for an adapter or returns NULL
//
CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(tstring tAdapterName, int nbrOfAdapter,  CAdapterInfo *adapterIno, tstring& tResponse );

//
// returns total number of adapters in the panel
//
CmPCConfig_DeclSpec BOOL CfgGetNbrOfAdapters(int &nbrOfAdapter, tstring& tResponse );

//
// Enables DHCP for IP Address, DNS Address, and WINS Address
//
CmPCConfig_DeclSpec BOOL CfgEnableAllDhcp(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse  );

//
// Enables DHCP for IP Address
//
CmPCConfig_DeclSpec BOOL CfgEnableDhcpIPAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse  );

//
// Enables DHCP for DNS Address
//
CmPCConfig_DeclSpec BOOL CfgEnableDhcpDnsAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse  );

//
// Enables DHCP for WINS Address
//
CmPCConfig_DeclSpec BOOL CfgEnableDhcpWinsAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse  );

//
// Sets static IP address, mask, and default gateway
//
CmPCConfig_DeclSpec BOOL CfgSetStaticAddresses(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], TCHAR gateway[],tstring& resultBuf,bool &status  );

//
// Sets static IP address and mask
//
CmPCConfig_DeclSpec BOOL CfgSetStaticIPAddressAndMask(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], tstring& resultBuf, bool &status);

//
// Sets static IP address
//
CmPCConfig_DeclSpec BOOL CfgSetStaticIPAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[],tstring& resultBuf,bool &status );

// Sets DNS address
CmPCConfig_DeclSpec BOOL CfgSetStaticDnsAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR DnsAddres[], tstring& resultBuf,bool &status,BOOL bClearCurAddresses);

// Sets WINS address
CmPCConfig_DeclSpec BOOL CfgSetStaticWinsAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR WinsAddres[], tstring& resultBuf,bool &status, BOOL bClearCurAddresses);

//
CmPCConfig_DeclSpec BOOL CfgCaptureIP(tstring tAdapterName, TCHAR cmd[], TCHAR param[] );

CmPCConfig_DeclSpec BOOL CfgCaptureEthernet(tstring tAdapterName, TCHAR cmd[], TCHAR param[] );

CmPCConfig_DeclSpec BOOL CfgGetInterfaceStatus(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse  );


CmPCConfig_DeclSpec BOOL CfgShowProcesses(tstring tAdapterName, tstring tCmd, tstring &tressponse  );

CmPCConfig_DeclSpec BOOL CfgPrintfAllFields(tstring tAdapterName, CAdapterInfo& AdapterInfo, tstring& tressponse);

CmPCConfig_DeclSpec BOOL CfgFirewall(tstring tAdapterName, tstring tCmd, tstring &tRessponse );
CmPCConfig_DeclSpec BOOL CfgIpConfigAllLog();

CmPCConfig_DeclSpec BOOL CfgHandleDirectConnect(TCHAR mode[], tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgCaptureUSB(TCHAR cmd[], bool bShow);
