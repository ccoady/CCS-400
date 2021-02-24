#ifndef _CWMI_H_
#define _CWMI_H_

#if _MSC_VER > 1000
	#pragma once
	#pragma warning(disable:4251)
#endif // _MSC_VER > 1000

#ifdef new
#undef new
#endif 

#include "iphlpapi.h"
#include <list>

using namespace std;

#define IPA_LENGTH					128

//*****************************************************************************
//
//*****************************************************************************
struct ETHERNETADAPTERINFO
{
	ETHERNETADAPTERINFO()
	{
		dwIndex = 0;
		dwAdapterTypeID = 0;
		memset(m_tsDesc, 0, sizeof(m_tsDesc));
		memset(m_tsManufacturer, 0, sizeof(m_tsManufacturer));

		m_dwDHCP = 0;
		_tcsncpy(m_tsIpAddr,      _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsIpMask,      _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsDnsAddr[0],  _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsWinsAddr[0], _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsDnsAddr[1],  _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsWinsAddr[1], _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsGatewayAddr, _T("0.0.0.0"), IPA_LENGTH);

		m_dwNewDHCP = 0;
		_tcsncpy(m_tsNewIpAddr,      _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewIpMask,      _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewDnsAddr[0],  _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewWinsAddr[0], _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewDnsAddr[1],  _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewWinsAddr[1], _T("0.0.0.0"), IPA_LENGTH);
		_tcsncpy(m_tsNewGatewayAddr, _T("0.0.0.0"), IPA_LENGTH);
	}
	~ETHERNETADAPTERINFO() {}

	DWORD	dwIndex;						// Index number of the network adapter
	DWORD	dwAdapterTypeID;
   _TCHAR 	m_tsDesc[MAX_ADAPTER_DESCRIPTION_LENGTH + 4];
   TCHAR	m_tsManufacturer[MAX_PATH];

   // current settings
   	DWORD	m_dwDHCP;							// DHCP
   _TCHAR	m_tsIpAddr[IPA_LENGTH];				// IP address
   _TCHAR	m_tsIpMask[IPA_LENGTH];				// IP Mask
   _TCHAR	m_tsDnsAddr[2][IPA_LENGTH];			// Primary and secondary DNS addresses
   _TCHAR	m_tsWinsAddr[2][IPA_LENGTH];		// Primary and secondary WINS addresses
   _TCHAR	m_tsGatewayAddr[IPA_LENGTH];		// Default gateway/router address

   // new settings that have not yet been applied
   	DWORD	m_dwNewDHCP;						// DHCP
   _TCHAR	m_tsNewIpAddr[IPA_LENGTH];			// IP address
   _TCHAR	m_tsNewIpMask[IPA_LENGTH];			// IP Mask
   _TCHAR	m_tsNewDnsAddr[2][IPA_LENGTH];		// Primary and secondary DNS addresses
   _TCHAR	m_tsNewWinsAddr[2][IPA_LENGTH];		// Primary and secondary WINS addresses
   _TCHAR	m_tsNewGatewayAddr[IPA_LENGTH];		// Default gateway/router address

};

typedef std::list<ETHERNETADAPTERINFO *> EthernetAdapterList;

//*****************************************************************************
//
//*****************************************************************************
class CmPCConfig_DeclSpec CWMI
{
	public:
		CWMI();
		virtual ~CWMI();

		void Init();
		HRESULT PrepareAdapterInfoList();

		bool IsAdapterInfoListFlag()   { return m_bAdapterInfoRetrieved; }
		void SetAdapterInfoListFlag(bool b)   { m_bAdapterInfoRetrieved = b; }

		EthernetAdapterList GetEthernetAdapterList()   { return m_AdapterList; }

		BOOL ExecuteNetsh(_TCHAR *);
		BOOL SetWin32_NetworkAdapterConfigurationViaWmi(ETHERNETADAPTERINFO *);

		void ClearAdapterList();
		HRESULT RebootSystem();

		HRESULT SetStaticIPAddress(ETHERNETADAPTERINFO *pInfo);
		HRESULT SetDNSAddress(ETHERNETADAPTERINFO *);
		HRESULT SetWINSAddress(ETHERNETADAPTERINFO *);

		HRESULT GetNewEnetAdapterSettings(ETHERNETADAPTERINFO *);

		HRESULT GetEnetAdapterDHCPStatus(DWORD *);
		HRESULT SetEnetAdapterDHCPStatus(DWORD);

		HRESULT GetEnetAdapterIPAddress(_TCHAR *);
		HRESULT SetEnetAdapterIPAddress(const _TCHAR *);

		HRESULT GetEnetAdapterIPMask(_TCHAR *);
		HRESULT SetEnetAdapterIPMask(const _TCHAR *);

		HRESULT GetEnetAdapterIPPDNS(_TCHAR *);
		HRESULT SetEnetAdapterIPPDNS(const _TCHAR *);

		HRESULT GetEnetAdapterIPPWINS(_TCHAR *);
		HRESULT SetEnetAdapterIPPWINS(const _TCHAR *);

		HRESULT GetEnetAdapterIPSDNS(_TCHAR *);
		HRESULT SetEnetAdapterIPSDNS(const _TCHAR *);

		HRESULT GetEnetAdapterIPSWINS(_TCHAR *);
		HRESULT SetEnetAdapterIPSWINS(const _TCHAR *);

		HRESULT GetEnetAdapterDefRouter(_TCHAR *);
		HRESULT SetEnetAdapterDefRouter(const _TCHAR *);

		void RefreshAdapterInfo(void);

		unsigned long ConvertIPAddressStringToLong(const _TCHAR *);

	protected:

	private:
		void Cleanup();

		BOOL GetAdapterInfoViaWmi(void);

		BOOL FindPhysicalAdaptersViaWmi(void);
		BOOL QueryPhysicalAdapterAttribViaWmi(void);

		HANDLE	hRcvdCommandRspEvt;
		bool	m_bAdapterInfoRetrieved;

		bool	m_bSetIPAddress;
		bool	m_bSetIPMask;
		bool	m_bSetDefRouter;

		std::list<ETHERNETADAPTERINFO *>	m_AdapterList;
};

#endif // _CWMI_H_
