////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        EthernetCore.h
///
/// Provides a header for helper functions and utilities to the exported Ethernet configuration functions.
///
/// @author      William Levine
/// @date        02/04/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_ETHERNETCORE_H_)
#define _ETHERNETCORE_H_

#include "EthernetDefs.h"

//
// Non-exported functions
//
//HRESULT CfgEthernetGetIPSettings(HKEY hRootKey, const _TCHAR *pszCLSID, SDeviceIPSettings *pSettings);
//HRESULT CfgEthernetSetIPSettings(HKEY hRootKey, const _TCHAR *pszCLSID, SDeviceIPSettings *pSettings, DWORD dwItems, int nControlSet = 0 );

//HRESULT CfgEthernetGetAdaptorCLSID(HKEY hRootKey, EEthernetAdaptors eAdaptor, _TCHAR *pstCLSID, DWORD *pdwBufferLen, bool bForceSearch = FALSE);
//HRESULT CfgEthernetFindAdaptorCLSID(HKEY hRootKey, const _TCHAR *pstName, _TCHAR *pstCLSID, DWORD *pdwBufferLen);

//HRESULT CfgEthernetGetAdaptorCLSIDs(HKEY hRootKey, _TCHAR **ppszPtrArrayCLSIDs);
//HRESULT CfgEthernetGetAdaptorInfo(HKEY hRootKey, _TCHAR *pszCLSID, _TCHAR **ppszPNPInstanceID, _TCHAR **ppszConnectionName);

//HRESULT CfgEthernetReadStaticIPSettings(HKEY hKey, SDeviceIPSettings *pSettings);
//HRESULT CfgEthernetReadDHCPSettings(HKEY hKey, SDeviceIPSettings *pSettings);

class CEthernetRegistry
{
public:
	CEthernetRegistry(HKEY hRootKey) : m_hRootKey(hRootKey)	{_tcscpy(m_pstControlSet, _T("CurrentControlSet"));}
	~CEthernetRegistry()										{}

	bool GetUPXAdapterName(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterName, DWORD dwBufferLen);
	bool GetUPXAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterCLSID, DWORD dwBufferLen);
	bool SetUPXAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstAdpaterCLSID);

	bool FindAdpaterCLSIDByName(_TCHAR *pstAdapterName, _TCHAR *pstAdpaterCLSID, DWORD dwBufferLen);
	bool VerifyAdapterCLSID(_TCHAR *pstAdpaterCLSID);

	bool SetIPSettings(EEthernetAdaptors eAdapter, SDeviceIPSettings *pSettings, DWORD dwItems);
	bool GetIPSettings(EEthernetAdaptors eAdapter, SDeviceIPSettings *pSettings);

	bool SetIPSettings(_TCHAR *pstCLSID, SDeviceIPSettings *pSettings, DWORD dwItems);
	bool GetIPSettings(_TCHAR *pstCLSID, SDeviceIPSettings *pSettings);

	bool GetVerifiedAdapterCLSID(EEthernetAdaptors eAdapter, _TCHAR *pstCLSID, DWORD dwBufferLen);

	bool GetSpeedLink(_TCHAR* tszSpeedLink, const _TCHAR* tszCLSID);
	bool SetSpeedLink(const _TCHAR* tszSpeedLink, const _TCHAR* tszCLSID);

	_TCHAR	m_pstControlSet[MAX_PATH];
	_TCHAR	m_pstAdpaterCLSID[MAX_PATH];

protected:
	HKEY m_hRootKey;
};

/////////////////////////////////////////////////////////////////////
// TABLE for setting the Link Speed
// Intel(R) PRO/100 VE Network Connection
extern const _TCHAR tszIntelNIC[];
extern const _TCHAR tszIntelLink[];
extern EEthernetSpeedLinkEntry NICIntel_EEthernetSpeedLinkTable[];

/////////////////////////////////////////////////////////////////////
// TABLE for setting the Link Speed
// VIA Rhine II Fast Ethernet Adapter
extern const _TCHAR tszViaNIC[];
extern const _TCHAR tszViaLink[];
extern EEthernetSpeedLinkEntry NICVia_EEthernetSpeedLinkTable[];

#endif