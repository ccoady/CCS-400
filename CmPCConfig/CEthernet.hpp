#pragma once;

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "CExecuteCommand.hpp"
#include "tchar.h"
#include <winsock2.h>
#include <WS2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include "AtlConv.h"
#include "shellapi.h"
#include "Wininet.h"

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
/* Note: could also use malloc() and free() */

#define WORKING_BUFFER_SIZE 40*1024
#define MAX_TRIES 20


#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Wininet.lib")
using namespace std;


class CAdapterInfo
{
public:
	CAdapterInfo(){Init();}
	~CAdapterInfo(){}

	tstring strAdapterName;
	tstring strDnsSuffix;
	tstring strDescription;
	tstring strFriendlyName;
	tstring strPhysicalAddress;
	DWORD dwDdnsEnabled;
	DWORD dwDhcpv4Enabled;
	DWORD dwIfType; //ethernet, token ring, etc... 
	tstring strNetworkGuid;
	tstring strIPMask;
	vector<tstring> strlistUnicastIPA;
	vector<tstring> strlistAnycastIPA;
	vector<tstring> strlistMulticastIPA;
	vector<tstring> strlistDnsServerIPA;
	vector<tstring> strlistWinsIPA;
	vector<tstring> strlistGatwayIPA;
	vector<tstring> strlistDhcpv4Server;

	BOOL bInterfaceConnected;
	BOOL bDnsDhcpEnabled;
	BOOL bWinsDhcpEnabled;

	void Init()
	{
		strAdapterName.clear();
		strDnsSuffix.clear();
		strDescription.clear();
		strFriendlyName.clear();
		strPhysicalAddress.clear();
		dwDdnsEnabled = 0;
		dwDhcpv4Enabled = 0;
		dwIfType = 0; 
		strNetworkGuid.clear();
		strIPMask.clear();
		strlistUnicastIPA.clear();
		strlistAnycastIPA.clear();
		strlistMulticastIPA.clear();
		strlistDnsServerIPA.clear();
		strlistWinsIPA.clear();
		strlistGatwayIPA.clear();
		strlistDhcpv4Server.clear();
		bInterfaceConnected = 0;
		bDnsDhcpEnabled = 0;
		bWinsDhcpEnabled = 0;
	}

	BOOL PrintfAllFields(tstring& tStr)
	{
		BaseLogger::WriteLog(__WFUNCTION__);

		TCHAR buf[2048];

		try
		{
			_stprintf_s(buf, TSIZEOF(buf), L"AdapterName %s \r\n", strAdapterName.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"DnsSuffix %s \r\n", strDnsSuffix.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"Description %s \r\n", strDescription.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"FriendlyName %s \r\n", strFriendlyName.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"PhysicalAddress %s \r\n", strPhysicalAddress.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"NetworkGuid %s \r\n", strNetworkGuid.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"IPMask %s \r\n", strIPMask.c_str() );
			tStr += buf;

			for(int i=0; i<strlistUnicastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"UnicastIPA %s \r\n", strlistUnicastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistAnycastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"AnycastIPA %s \r\n", strlistAnycastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistMulticastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"MulticastIPA %s \r\n", strlistMulticastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistDnsServerIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"DnsServerIPA %s \r\n", strlistDnsServerIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistWinsIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"WinsIPA %s \r\n", strlistWinsIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistGatwayIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"GatwayIPA %s \r\n", strlistGatwayIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<strlistDhcpv4Server.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"Dhcpv4Server %s \r\n", strlistDhcpv4Server.at(i).c_str() );
				tStr += buf;
			}

			_stprintf_s(buf, TSIZEOF(buf), L"DdnsEnabled %ld \r\n", dwDdnsEnabled );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"Dhcpv4Enabled %ld \r\n", dwDhcpv4Enabled );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"IfType %ld \r\n", dwIfType );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"bInterfaceConnected %ld \r\n", bInterfaceConnected );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"bDnsDhcpEnabled %ld \r\n", bDnsDhcpEnabled );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"bWinsDhcpEnabled %ld \r\n", bWinsDhcpEnabled );
			tStr += buf;
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			BaseLogger::WriteLog(buf);
		}

		_stprintf_s(buf, TSIZEOF(buf), L"**** %s \r\n %s \r\n", __WFUNCTION__, tStr.c_str() );
		BaseLogger::WriteLog(buf);

		return TRUE;
	}

	BOOL ShowIpSettings(tstring& tStr)
	{
		BaseLogger::WriteLog(__WFUNCTION__);

		TCHAR buf[2048];
		tStr.clear();

		try
		{
			_stprintf_s(buf, TSIZEOF(buf), L"  DNS Suffix:    %s \r\n", strDnsSuffix.c_str());
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  Description:   %s \r\n", strDescription.c_str());
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  FriendlyName:  %s \r\n", strFriendlyName.c_str());
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  --------------------------------\r\n" ); tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"  Status:  %s \r\n", bInterfaceConnected ? L"Connected": L"Not Connected");
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  --------------------------------\r\n"); tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"  DHCP IP:  \t %s \r\n", dwDhcpv4Enabled ? L"On" : L"Off");
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  DHCP DNS: \t %s \r\n", bDnsDhcpEnabled ? L"On" : L"Off");
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  DHCP WINS: \t %s \r\n", bWinsDhcpEnabled ? L"On" : L"Off");
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"  --------------------------------\r\n"); tStr += buf;


			for each(tstring s in strlistUnicastIPA)
			{
				if (s.length() < 16) // exclude ipv6
				{
					_stprintf_s(buf, TSIZEOF(buf), L"  IPA:         %s \r\n", s.c_str());
					tStr += buf;
				}
			}
			    _stprintf_s(buf, TSIZEOF(buf), L"  Subnet Mask: %s \r\n", strIPMask.c_str());
			tStr += buf;
			for each(tstring s in strlistGatwayIPA)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"  DefRouter:   %s \r\n", s.c_str());
				tStr += buf;
			}
			_stprintf_s(buf, TSIZEOF(buf), L"  --------------------------------\r\n"); tStr += buf;
		

			for each(tstring s in strlistGatwayIPA)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"  DHCP Server: %s \r\n", s.c_str());
				tStr += buf;
			}

			for each(tstring s in strlistDnsServerIPA)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"  DNS Server:  %s \r\n", s.c_str());
				tStr += buf;
			}

			for each(tstring s in strlistWinsIPA)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"  WINS Server: %s \r\n", s.c_str());
				tStr += buf;
			}
		}
		catch (...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			BaseLogger::WriteLog(buf);
		}

		_stprintf_s(buf, TSIZEOF(buf), L"**** %s \r\n %s \r\n", __WFUNCTION__, tStr.c_str());
		BaseLogger::WriteLog(buf);

		return TRUE;
	}

};

extern CRITICAL_SECTION g_CS_CEthernet_hpp;

class CEthernet: public CExecuteCommand
{

public:

	CEthernet() {}

	CEthernet(tstring byDescripton)
	{
		::EnterCriticalSection(&g_CS_CEthernet_hpp);
				
		WriteLog(__WFUNCTION__, TRACESTACK);

		Init(byDescripton);

		// Commented it, because it should leave critical section when this object is destroyed 
		//::LeaveCriticalSection(&g_CS_CEthernet_hpp);

	}
	~CEthernet()
	{
		// Commented it, because it already entered critical section in constructor
		//::EnterCriticalSection(&g_CS_CEthernet_hpp);

		Destroy();

		WriteLog(__WFUNCTION__, TRACESTACK);

		::LeaveCriticalSection(&g_CS_CEthernet_hpp);
	}
	
	BOOL IpConfigAll()
	{
		BOOL bRetv = FALSE;		
		WriteLog(__WFUNCTION__, TRACESTACK);
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("ipconfig /all") );

			if(CExecuteCommand::Execut(cmd, 500) )
			{
				TCHAR tResponseBuf[MAX_RESPONSE_STR_LEN];
				DWORD ResponseBufLen = MAX_RESPONSE_STR_LEN-2;
				CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
				BaseLogger::WriteLog(tResponseBuf);
				if( _tcsstr(tResponseBuf, L"Windows IP Configuration")!=NULL )
					bRetv = TRUE;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return bRetv;

	}

	BOOL ClearAllFields(CAdapterInfo &Info)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);


		try
		{
			std::string str;

			Info.strAdapterName.clear();
			Info.strDnsSuffix.clear();
			Info.strDescription.clear();
			Info.strFriendlyName.clear();
			Info.strPhysicalAddress.clear();
			Info.dwDdnsEnabled = 0;
			Info.dwDhcpv4Enabled = 0;
			Info.dwIfType = 0; 
			Info.strNetworkGuid.clear();
			Info.strIPMask.clear();
			Info.strlistUnicastIPA.clear();
			Info.strlistAnycastIPA.clear();
			Info.strlistMulticastIPA.clear();
			Info.strlistDnsServerIPA.clear();
			Info.strlistWinsIPA.clear();
			Info.strlistGatwayIPA.clear();
			Info.strlistDhcpv4Server.clear();
			Info.bInterfaceConnected = 0;
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}
		return TRUE;
	}

	BOOL CopyFieldByField(CAdapterInfo &DestInfo, CAdapterInfo &SrcInfo)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		try
		{
			// ethernet cable is connect or disconnect, the following hardware related fields needs to be copied
			DestInfo.strAdapterName = SrcInfo.strAdapterName;
			DestInfo.strDescription = SrcInfo.strDescription;
			DestInfo.strFriendlyName = SrcInfo.strFriendlyName;
			DestInfo.strPhysicalAddress = SrcInfo.strPhysicalAddress;

			//if(SrcInfo.bInterfaceConnected)                               // like ipconfig command
			//??if(SrcInfo.bInterfaceConnected || !SrcInfo.bInterfaceConnected) // like netsh command
			{
				DestInfo.strDnsSuffix = SrcInfo.strDnsSuffix;
				DestInfo.strFriendlyName = SrcInfo.strFriendlyName;
				DestInfo.strPhysicalAddress = SrcInfo.strPhysicalAddress;
				DestInfo.dwDdnsEnabled = SrcInfo.dwDdnsEnabled;
				DestInfo.dwDhcpv4Enabled = SrcInfo.dwDhcpv4Enabled;
				DestInfo.dwIfType = SrcInfo.dwIfType; 
				DestInfo.strNetworkGuid = SrcInfo.strNetworkGuid;
				DestInfo.strIPMask = SrcInfo.strIPMask;
				DestInfo.strlistUnicastIPA = SrcInfo.strlistUnicastIPA;
				DestInfo.strlistAnycastIPA = SrcInfo.strlistAnycastIPA;
				DestInfo.strlistMulticastIPA = SrcInfo.strlistMulticastIPA;
				DestInfo.strlistDnsServerIPA = SrcInfo.strlistDnsServerIPA;
				DestInfo.strlistWinsIPA = SrcInfo.strlistWinsIPA;
				DestInfo.strlistGatwayIPA = SrcInfo.strlistGatwayIPA;
				DestInfo.strlistDhcpv4Server = SrcInfo.strlistDhcpv4Server;
				DestInfo.bInterfaceConnected = SrcInfo.bInterfaceConnected;
				DestInfo.bDnsDhcpEnabled = SrcInfo.bDnsDhcpEnabled;
				DestInfo.bWinsDhcpEnabled = SrcInfo.bWinsDhcpEnabled;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return TRUE;
	}

	BOOL PrintfAllFields(CAdapterInfo &AdapterInfo, tstring& tStr)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR buf[2048];

		try
		{
			_stprintf_s(buf, TSIZEOF(buf), L"AdapterName %s \r\n", AdapterInfo.strAdapterName.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"DnsSuffix %s \r\n", AdapterInfo.strDnsSuffix.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"Description %s \r\n", AdapterInfo.strDescription.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"FriendlyName %s \r\n", AdapterInfo.strFriendlyName.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"PhysicalAddress %s \r\n", AdapterInfo.strPhysicalAddress.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"NetworkGuid %s \r\n", AdapterInfo.strNetworkGuid.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"IPMask %s \r\n", AdapterInfo.strIPMask.c_str() );
			tStr += buf;

			for(int i=0; i<AdapterInfo.strlistUnicastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"UnicastIPA %s \r\n", AdapterInfo.strlistUnicastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistAnycastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"AnycastIPA %s \r\n", AdapterInfo.strlistAnycastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistMulticastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"MulticastIPA %s \r\n", AdapterInfo.strlistMulticastIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistDnsServerIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"DnsServerIPA %s \r\n", AdapterInfo.strlistDnsServerIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistWinsIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"WinsIPA %s \r\n", AdapterInfo.strlistWinsIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistGatwayIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"GatwayIPA %s \r\n", AdapterInfo.strlistGatwayIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistDhcpv4Server.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"Dhcpv4Server %s \r\n", AdapterInfo.strlistDhcpv4Server.at(i).c_str() );
				tStr += buf;
			}

			_stprintf_s(buf, TSIZEOF(buf), L"DdnsEnabled %ld \r\n", AdapterInfo.dwDdnsEnabled );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"Dhcpv4Enabled %ld \r\n", AdapterInfo.dwDhcpv4Enabled );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"IfType %ld \r\n", AdapterInfo.dwIfType );
			tStr += buf;

			_stprintf_s(buf, TSIZEOF(buf), L"AdapterInfo.bInterfaceConnected %ld \r\n", AdapterInfo.bInterfaceConnected );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"AdapterInfo.bDnsDhcpEnabled %ld \r\n", AdapterInfo.bDnsDhcpEnabled );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"AdapterInfo.bWinsDhcpEnabled %ld \r\n", AdapterInfo.bWinsDhcpEnabled );
			tStr += buf;
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		_stprintf_s(buf, TSIZEOF(buf), L"**** %s \r\n %s \r\n", __WFUNCTION__, tStr.c_str() );
		BaseLogger::WriteLog(buf);

		return TRUE;
	}

	BOOL PrintSelectFields(CAdapterInfo &AdapterInfo, tstring& tStr)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR buf[2048];

		try
		{
			_stprintf_s(buf, TSIZEOF(buf), L"Description %s \r\n", AdapterInfo.strDescription.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"FriendlyName %s \r\n", AdapterInfo.strFriendlyName.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"DnsSuffix %s \r\n", AdapterInfo.strDnsSuffix.c_str() );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"bConnected %ld \r\n", AdapterInfo.bInterfaceConnected );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"bDnsDhcpEnabled %ld \r\n", AdapterInfo.bDnsDhcpEnabled );
			tStr += buf;
			_stprintf_s(buf, TSIZEOF(buf), L"bWinsDhcpEnabled %ld \r\n", AdapterInfo.bWinsDhcpEnabled );
			tStr += buf;

			for(int i=0; i<AdapterInfo.strlistUnicastIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"UnicastIPA %s \r\n", AdapterInfo.strlistUnicastIPA.at(i).c_str() );
				tStr += buf;
			}
			
			_stprintf_s(buf, TSIZEOF(buf), L"IPMask %s \r\n", AdapterInfo.strIPMask.c_str() );
			tStr += buf;

			for(int i=0; i<AdapterInfo.strlistDnsServerIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"DnsServerIPA %s \r\n", AdapterInfo.strlistDnsServerIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistWinsIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"WinsIPA %s \r\n", AdapterInfo.strlistWinsIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistGatwayIPA.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"GatwayIPA %s \r\n", AdapterInfo.strlistGatwayIPA.at(i).c_str() );
				tStr += buf;
			}
			for(int i=0; i<AdapterInfo.strlistDhcpv4Server.size(); i++)
			{
				_stprintf_s(buf, TSIZEOF(buf), L"Dhcpv4Server %s \r\n", AdapterInfo.strlistDhcpv4Server.at(i).c_str() );
				tStr += buf;
			}

		}
		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("catch(...) %d  %s "), __LINE__, __WFUNCTION__);
		}

		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s \r\n%s"), __WFUNCTION__, tStr.c_str() );

		return TRUE;
	}

	BOOL Firewall(tstring tParameter, tstring &tResponse )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		BOOL bRetv = TRUE;
		tResponse.clear();

		try
		{
			if( _tcsicmp(tParameter.c_str(), L"on")==0
				|| _tcsicmp(tParameter.c_str(), L"off")==0 ) // set enable or disable
			{
				_stprintf_s(cmd, sz, _T("netsh firewall set opmode %s "), 
					_tcsicmp(tParameter.c_str(), L"on")==0 ? L"Enable" : L"Disable"); 
				CExecuteCommand::Execut(cmd);

				bRetv = CheckForSuccess( _T("Ok.") );
				if(bRetv && _tcsicmp(tParameter.c_str(), L"on")==0 )
					tResponse = _T("Successfuly turn on firewall, please reboot for changes to take effect."); 
				else if(bRetv && _tcsicmp(tParameter.c_str(), L"off")==0 )
					tResponse = _T("Successfuly turn off firewall, please reboot for changes to take effect."); 
				else 
				{
					GetResponseString(tResponse);
					BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s ERROR: Firewall set failed bRetv %d OUT"), __WFUNCTION__, bRetv );
					bRetv = FALSE;
				}
			}
			else if( tParameter.empty() 
				||  _tcsicmp(tParameter.c_str(), L"status")==0 ) // show status
			{
				_stprintf_s(cmd, sz, _T("netsh firewall show state ") );
				CExecuteCommand::Execut(cmd);
				{
					TCHAR tResponseBuf[MAX_RESPONSE_STR_LEN];
					DWORD ResponseBufLen = MAX_RESPONSE_STR_LEN;
					CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
					vector<tstring> vLines;
					vLines = split(tResponseBuf, _T('\n') );
					if(vLines.size()>4)
					{
						tstring str;
						str = vLines.at(4).c_str();
						if(_tcsstr(str.c_str(), L"Enable") )
							tResponse = L"Current Firewall State: On";
						else if(_tcsstr(str.c_str(), L"Disable") )
							tResponse = L"Current Firewall State: Off";
						else
						{
							tResponse = L"Error: Firewall status unknown"; // detailed response
							bRetv = FALSE;
						}

						BaseLogger::WriteFormattedLog(TRACESTACK, _T("Firewall response parsed, bRetv %d, tResponse %s  OUT"), bRetv, tResponse.c_str() );
					}
				}
			}
			else
			{
				tResponse = L"ERROR: Command Syntax Error - Check command usage.";
				bRetv = FALSE;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		BaseLogger::WriteFormattedLog(TRACESTACK, _T("Firewall bRetv %d, tResponse %s  OUT"), bRetv, tResponse.c_str() );

		return bRetv;
	}

	CAdapterInfo* GetAdapterInfo(TCHAR byDescripton[])
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		try
		{
			//Init(); // re-init in case addresses changed

			int sz = (int)m_AdaptersList.size();

			if(sz > 0)
			{
				for(int i=0; i<sz; i++)
				{
					if(m_AdaptersList.at(i)->strDescription.find(byDescripton) == 0)
					{
						GetInterfaceStatus( m_AdaptersList.at(i)->strFriendlyName.c_str() );
						m_AdaptersList.at(i)->bInterfaceConnected = CheckForSuccess( _T("Connected") );

						TCHAR* cmd = new TCHAR[KB(1)]; 
						_stprintf_s(cmd, KB(1)-1, _T("netsh interface ip show config name=\"%s\" "),   m_AdaptersList.at(i)->strFriendlyName.c_str() );

						if(CExecuteCommand::Execut(cmd ) )
						{
							GetIPSubnetMask(m_AdaptersList.at(i)->strFriendlyName, m_AdaptersList.at(i)->strIPMask); // fix/revisit for better method
							m_AdaptersList.at(i)->bDnsDhcpEnabled = GetDnsDhcpEnabled(m_AdaptersList.at(i)->strFriendlyName);
							m_AdaptersList.at(i)->bWinsDhcpEnabled = GetWinsDhcpEnabled(m_AdaptersList.at(i)->strFriendlyName);
						}
						delete[] cmd;

						tstring str;
						PrintSelectFields(*m_AdaptersList.at(i), str);

						return m_AdaptersList.at(i);
					}
				}
			}
		}

		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return NULL;
	}

	//
	// Returns information for an adapter or NULL
	//
	CAdapterInfo* GetAdapterInfo(int nbrOfAdapter)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//Init(); // re-init in case addresses changed

		int sz = (int)m_AdaptersList.size();
		if(sz > nbrOfAdapter)
		{
			return 
				m_AdaptersList.at(nbrOfAdapter);
		}
		else
			return NULL;
	}

	//
	// Returns nbr of adpater in the panel
	//
	BOOL GetNbrOfAdapter(int &nbrOfAdapter)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		Init(_T("")); // re-init in case addresses changed

		int sz = (int)m_AdaptersList.size();
		nbrOfAdapter = sz;

		return TRUE;
	}

	//
	// Enables DHCP for IP Address, DNS Address, and WINS Address
	//
	BOOL EnableAllDhcp(TCHAR InterfaceName[] )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		BOOL bRetv = FALSE;
		try
		{
			bRetv = EnableDhcpIPAddress(InterfaceName);
			if(bRetv)
				bRetv = EnableDhcpWinsAddress(InterfaceName);
			if(bRetv)
				bRetv = EnableDhcpDnsAddress(InterfaceName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return bRetv;
	}

	//
	// Enables DHCP for IP Address
	//
	BOOL EnableDhcpIPAddress(TCHAR InterfaceName[] )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{ 
			_stprintf_s(cmd, sz, _T("netsh interface ip set ADDRESS name=\"%s\" source=dhcp"), 
				InterfaceName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd);

		return CheckNetshResponse();
	}

	//
	// Enables DHCP for DNS Address
	//
	BOOL EnableDhcpDnsAddress(TCHAR InterfaceName[] )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			ClearDnsAddresses(InterfaceName);// will clear Dns table

			_stprintf_s(cmd, sz, _T("netsh interface ip set DNS name=\"%s\" source=dhcp"), 
				InterfaceName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd);

		return CheckNetshResponse();
	}

	//
	// Enables DHCP for WINS Address
	//
	BOOL EnableDhcpWinsAddress(TCHAR InterfaceName[] )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			ClearWinsAddresses(InterfaceName);// will clear Wins table

			_stprintf_s(cmd, sz, _T("netsh interface ip set WINS name=\"%s\" source=dhcp"), 
				InterfaceName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd);

		return CheckNetshResponse();

	}


	BOOL SetStaticAddresses(TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], TCHAR gateway[],tstring& resultBuf,bool &status  )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//L"netsh interface ip set address name=\"Wireless Network Connection\" source=static addr=172.30.161.24 mask=255.255.240.0 gateway=172.30.160.1 1");

		if(ValidateIPV4Address(IpAddr)==FALSE )
			return FALSE;
		if(ValidateIPV4Address(mask)==FALSE )
			return FALSE;
		if(ValidateIPV4Address(gateway, TRUE)==FALSE )
				return FALSE;

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface ip set address name=\"%s\" source=static addr=%s mask=%s gateway=%s 1"), 
				InterfaceName, IpAddr, mask, gateway);
		}

		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd,resultBuf,status);

		return CheckNetshResponse();

	}

	BOOL SetStaticIPAddressAndMask(TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], tstring& resultBuf, bool &status)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//L"netsh interface ip set address name=\"Wireless Network Connection\" source=static addr=172.30.161.24 mask=255.255.240.0 gateway=172.30.160.1 1");

		if (ValidateIPV4Address(IpAddr) == FALSE)
			return FALSE;
		if (ValidateIPV4Address(mask) == FALSE)
			return FALSE;
		
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface ip set address name=\"%s\" source=static addr=%s mask=%s"),
				InterfaceName, IpAddr, mask);
		}

		catch (...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd, resultBuf, status);

		return CheckNetshResponse();

	}



	BOOL SetStaticIPAddress(TCHAR InterfaceName[], TCHAR IpAddr[],tstring& resultBuf ,bool &status)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//L"netsh interface ip set dns     name=\"Local Area Connection\" source=static 172.30.101.201" 
		if(ValidateIPV4Address(IpAddr)==FALSE )
			return FALSE;

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface ip set address name=\"%s\" source=static addr=%s"), 
			InterfaceName, IpAddr);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd,resultBuf,status);

		return CheckNetshResponse();
	}

	BOOL ClearDnsAddresses(TCHAR InterfaceName[])
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface ip set dns name=\"%s\" source=static %s"), InterfaceName, _T("none"));
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd);

		return CheckNetshResponse();
	}

	BOOL SetStaticDnsAddress(TCHAR InterfaceName[], TCHAR DnsAddres[],tstring& resultBuf ,bool &status, bool bReset)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//L"netsh interface ip set dns name=\"Local Area Connection\" source=static  172.30.103.203
		//L"netsh interface ip add dns name=\"Local Area Connection\"  172.30.104.204
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);

		try
		{
			if(ValidateIPV4Address(DnsAddres, TRUE)==TRUE )
			{
				if (bReset)
					_stprintf_s(cmd, sz, _T("netsh interface ip set dns name=\"%s\"  source=static %s validate=no"), InterfaceName, DnsAddres);
				else
					_stprintf_s(cmd, sz, _T("netsh interface ip add dns  name=\"%s\"  %s   validate=no"), InterfaceName, DnsAddres);

				CExecuteCommand::Execut(cmd,resultBuf,status);
				if(!CheckNetshResponse() )
					return FALSE;
			}			
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return CheckNetshResponse();
	}

	BOOL ClearWinsAddresses(TCHAR InterfaceName[])
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface ip set WINS name=\"%s\" source=static %s"), 
			InterfaceName, _T("none"));
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		CExecuteCommand::Execut(cmd);

		return CheckNetshResponse();
	}

	BOOL SetStaticWinsAddress(TCHAR InterfaceName[], TCHAR WinsAddres[], tstring& resultBuf,bool &status, bool bReset  )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//L"netsh interface ip set wins name=\"Local Area Connection\" source=static 172.30.102.202"
		//L"netsh interface ip add wins name=\"Local Area Connection\" 172.30.102.203"
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);

		try
		{
			if(ValidateIPV4Address(WinsAddres, TRUE)==TRUE )
			{
				if (bReset)
					_stprintf_s(cmd, sz, _T("netsh interface ip set wins name=\"%s\" source=static %s"), InterfaceName, WinsAddres);
				else
					_stprintf_s(cmd, sz, _T("netsh interface ip add wins name=\"%s\"  %s"), InterfaceName, WinsAddres);

				CExecuteCommand::Execut(cmd,resultBuf,status);

				if(!CheckNetshResponse() )
					return FALSE;
			}			
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return CheckNetshResponse();
	}	
	

	BOOL CaptureIP(TCHAR cmd[], TCHAR param[]  )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		BOOL bRetv = ExecElevatedShell(NULL, cmd, param);

		return bRetv;
	}	

	BOOL GetInterfaceStatus(const TCHAR InterfaceName[] )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		// returns TRUE if connection, also returns one of the following flags
		//INTERNET_CONNECTION_CONFIGURED 0x40 Local system has a valid connection to the Internet, but it might or might not be currently connected.
		//INTERNET_CONNECTION_LAN        0x02 Local system uses a local area network to connect to the Internet.
		//INTERNET_CONNECTION_MODEM      0x01 Local system uses a modem to connect to the Internet.
		//INTERNET_CONNECTION_MODEM_BUSY 0x08 No longer used.
		//INTERNET_CONNECTION_OFFLINE    0x20 Local system is in offline mode.
		//INTERNET_CONNECTION_PROXY      0x04
		//return 
		//	InternetGetConnectedState(&dwConnectionType, 0);

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);
		try
		{
			_stprintf_s(cmd, sz, _T("netsh interface show interface name=\"%s\" "), 
				InterfaceName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return 
			CExecuteCommand::Execut(cmd);
	}

	BOOL ShowProcesses(tstring tCmd, tstring &tRessponse  )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		TCHAR cmd[MAX_RESPONSE_STR_LEN];
		int sz = TSIZEOF(cmd);
		_stprintf_s(cmd, sz, _T("powershell (Get-Process %s )"), tCmd.c_str() );
		//_stprintf_s(cmd, sz, _T("powershell (Get-WmiObject -Class Win32_OperatingSystem).MUILanguages")  );

		CExecuteCommand::Execut(cmd);

		DWORD RespBufLen = MAX_RESPONSE_STR_LEN;
		CExecuteCommand::GetResponseString(cmd, RespBufLen);

		tRessponse = cmd;

		return TRUE;
	}


protected:

	vector <CAdapterInfo*>
		m_AdaptersList;

	int Init(tstring tAdapterName)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		try
		{
			Destroy();
			EnumerateAdapters(tAdapterName);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return 0;
	}
	int Destroy()
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		CAdapterInfo *p;
		int sz = (int)m_AdaptersList.size();
		try
		{
			for(int i=0; i<sz; i++)
			{
				p = m_AdaptersList.at(i);
				if(p)
					delete p;
			}
			m_AdaptersList.clear();
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return 0;
	}

	BOOL ValidateIPV4Address(TCHAR AddressString[], BOOL bAcceptZeroIP=FALSE)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//Method 1: WSAStringToAddress;
		//Method 2: inet_pton;
		//Method 3: inet_addr;

		if(AddressString==NULL)
			return FALSE;

		if( _tcsicmp(AddressString, _T("none") )==0 )
			return TRUE;

		unsigned long retv;
		try
		{
			USES_CONVERSION;
			char *AddressA = T2A(AddressString);

			//retv = inet_addr(AddressA);
			InetPton(AF_INET, AddressString, &retv);
			if(retv == INADDR_NONE)
				printf("invalid ip address");

			// reject ip address 0.0.0.0
			if(bAcceptZeroIP==FALSE && retv==0)
				retv=INADDR_NONE;
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return retv==INADDR_NONE ? FALSE : TRUE;
	}

	int EnumerateAdapters(tstring tAdapterName)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//tstring tAdapterName = _T("Intel(R) 82574L Gigabit Network Connection");
		//tstring tAdapterName = _T("Intel(R) 82579LM Gigabit Network Connection");

		try
		{
			WSAData wsaData;
			int retv = WSAStartup(MAKEWORD(2, 2), &wsaData);
			if ( retv!=0 ) 
				return -1;

			/* Declare and initialize variables */
			DWORD dwSize = 0;
			DWORD dwRetVal = 0;
			unsigned int i = 0;

			// Set the flags to pass to GetAdaptersAddresses
			ULONG flags = GAA_FLAG_INCLUDE_ALL_INTERFACES|GAA_FLAG_INCLUDE_WINS_INFO|GAA_FLAG_INCLUDE_GATEWAYS;

			// default to unspecified address family (both)
			ULONG family = AF_UNSPEC;

			LPVOID lpMsgBuf = NULL;

			PIP_ADAPTER_ADDRESSES pAddresses = NULL;
			ULONG outBufLen = 0;
			ULONG Iterations = 0;

			PIP_ADAPTER_ADDRESSES pCurrAddresses = NULL;
			PIP_ADAPTER_UNICAST_ADDRESS pUnicast = NULL;
			PIP_ADAPTER_ANYCAST_ADDRESS pAnycast = NULL;
			PIP_ADAPTER_MULTICAST_ADDRESS pMulticast = NULL;
			IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = NULL;
			IP_ADAPTER_WINS_SERVER_ADDRESS *pWins = NULL;
			IP_ADAPTER_GATEWAY_ADDRESS *pGateway = NULL;
			IP_ADAPTER_PREFIX *pPrefix = NULL;

			//family = AF_INET;
			//family = AF_INET6;
			family = AF_UNSPEC; //both

			printf("Calling GetAdaptersAddresses function with family = ");
			if (family == AF_INET)
				printf("AF_INET\n");
			if (family == AF_INET6)
				printf("AF_INET6\n");
			if (family == AF_UNSPEC)
				printf("AF_UNSPEC\n\n");

			// Allocate a 15 KB buffer to start with.
			outBufLen = WORKING_BUFFER_SIZE;

			do 
			{
				pAddresses = (IP_ADAPTER_ADDRESSES *) MALLOC(outBufLen);
				if (pAddresses == NULL) 
				{
					BaseLogger::WriteFormattedLog(_T("ERROR: Memory allocation failed for IP_ADAPTER_ADDRESSES struct. Size %d. Calling exit(1)\n"), outBufLen);
					//exit(1);
					WSACleanup();
					return 0;
				}

				dwRetVal = GetAdaptersAddresses(family, flags, NULL, pAddresses, &outBufLen);

				if (dwRetVal == ERROR_BUFFER_OVERFLOW) 
				{
					FREE(pAddresses);
					pAddresses = NULL;
					BaseLogger::WriteFormattedLog( _T("WARNING: GetAdaptersAddresses ERROR_BUFFER_OVERFLOW; Retry count %d, needed buffer length %d"), Iterations, outBufLen );
					Sleep(100);
				} 
				else
				{
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("GetAdaptersAddresses BREAK, last error %d"), GetLastError() );
					break;
				}

				Iterations++;

			} while ((dwRetVal == ERROR_BUFFER_OVERFLOW) && (Iterations < MAX_TRIES));

			if (dwRetVal == NO_ERROR)
			{
				// If successful, output some information from the data we received
				pCurrAddresses = pAddresses;
				int count = 0;
				TCHAR Buf[WORKING_BUFFER_SIZE];
				DWORD Buflen = WORKING_BUFFER_SIZE;
				INT retv;
				while (pCurrAddresses)
				{
					// do I need to keep this adapter
					if( !tAdapterName.empty() )
					{
						//if(tAdapterName.find(pCurrAddresses->Description) != 0)
						bool bContinue = true;
						if (IsPanelName(L"CCS-UC-CODEC-300") == true
							|| IsPanelName(L"UC-ENGINE") == true
							|| IsPanelName(L"CCS-UC-CODEC-250") == true
							)
						{
							tstring strDescription = pCurrAddresses->Description;
							bContinue = (strDescription.find(tAdapterName) != 0);
						}
						else // CCS-100, CCS-200
						{
							bContinue = (tAdapterName.find(pCurrAddresses->Description) != 0);
						}

						if(bContinue)
						{
							pCurrAddresses = pCurrAddresses->Next;
							continue; // will not keep this adapter
						}
					}

					// keep adapter
					CAdapterInfo* pCAdapterInfo = new CAdapterInfo;
					if(!pCAdapterInfo)
					{
						BaseLogger::WriteFormattedLog(_T("ERROR: allocating memory, new CAdapterInfo has failed \n"));
						break;
					}
					ClearAllFields(*pCAdapterInfo);

					USES_CONVERSION;					
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("(%d) Length of the IP_ADAPTER_ADDRESS struct: %ld"), ++count, pCurrAddresses->Length);
					BaseLogger::WriteFormattedLog(_T("Adapter Description: %s"), pCurrAddresses->Description );
					BaseLogger::WriteFormattedLog(_T("Adapter FriendlyName: %s"), pCurrAddresses->FriendlyName );					
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\ttAdapter IfIndex (IPv4 interface): %u"), pCurrAddresses->IfIndex);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tAdapter Name: %s"), A2T(pCurrAddresses->AdapterName) );
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tDNS Suffix: %wS"), pCurrAddresses->DnsSuffix);

					// Unicast addresses
					pUnicast = pCurrAddresses->FirstUnicastAddress;
					if (pUnicast != NULL) 
					{
						for (i = 0; pUnicast != NULL; i++)
						{
							/*if (pUnicast->Address.lpSockaddr->sa_family == AF_INET)
							{
							sockaddr_in *sa_in = (sockaddr_in *)pUnicast->Address.lpSockaddr;
							printf("\tIPV4:%s\n",inet_ntop(AF_INET,&(sa_in->sin_addr),buff,bufflen));
							}
							else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
							{
							sockaddr_in6 *sa_in6 = (sockaddr_in6 *)pUnicast->Address.lpSockaddr;
							printf("\tIPV6:%s\n",inet_ntop(AF_INET6,&(sa_in6->sin6_addr),buff,bufflen));
							}*/

							if (pUnicast->Address.lpSockaddr->sa_family == AF_INET
								|| pUnicast->Address.lpSockaddr->sa_family == AF_INET6)
							{
								Buflen = WORKING_BUFFER_SIZE;
								WSAAddressToString(pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, Buf, &Buflen);
								BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tUnicast IPA %d: %s "), i, Buf);
								pCAdapterInfo->strlistUnicastIPA.push_back( Buf );
							}
							else
							{
								BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t %s UNSPEC"), __WFUNCTION__);
							}
							pUnicast = pUnicast->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*Number of Unicast Addresses: %d"), i);
					} 
					else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("No Unicast Addresses"));

					//Anycast
					pAnycast = pCurrAddresses->FirstAnycastAddress;
					if (pAnycast) 
					{
						for (i = 0; pAnycast != NULL; i++)
						{
							Buflen = WORKING_BUFFER_SIZE;
							WSAAddressToString(pAnycast->Address.lpSockaddr, pAnycast->Address.iSockaddrLength, NULL, Buf, &Buflen);
							BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tAnycast IPA %d: %s "), i, Buf);
							pCAdapterInfo->strlistAnycastIPA.push_back( Buf );
							pAnycast = pAnycast->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNumber of Anycast Addresses: %d"), i);
					} else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNo Anycast Addresses "));

					//Multicast
					pMulticast = pCurrAddresses->FirstMulticastAddress;
					if (pMulticast) 
					{
						for (i = 0; pMulticast != NULL; i++)
						{
							Buflen = WORKING_BUFFER_SIZE;
							WSAAddressToString(pMulticast->Address.lpSockaddr, pMulticast->Address.iSockaddrLength, NULL, Buf, &Buflen);
							BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tMulticast IPA %d: %s "), i, Buf);
							pCAdapterInfo->strlistMulticastIPA.push_back( Buf );
							pMulticast = pMulticast->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*Number of Multicast Addresses: %d"), i);
					} else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNo Multicast Addresses"));

					//Dns
					pDnServer = pCurrAddresses->FirstDnsServerAddress;
					if (pDnServer) 
					{
						for (i = 0; pDnServer != NULL; i++)
						{
							Buflen = WORKING_BUFFER_SIZE;
							retv = WSAAddressToString(pDnServer->Address.lpSockaddr, pDnServer->Address.iSockaddrLength, NULL, Buf, &Buflen);
							BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tDnServer IPA %d: %s"), i,  Buf);
							if( !_tcsstr(Buf, _T("::") ) ) // reject IPV6 for this field
								pCAdapterInfo->strlistDnsServerIPA.push_back( Buf );
							pDnServer = pDnServer->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*Number of DNS Server Addresses: %d "), i);

					} else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*No DNS Server Addresses"));



					// WINS addresses
					pWins = pCurrAddresses->FirstWinsServerAddress;
					if (pWins) 
					{
						for (i = 0; pWins != NULL; i++)
						{
							Buflen = WORKING_BUFFER_SIZE;
							WSAAddressToString(pWins->Address.lpSockaddr, pWins->Address.iSockaddrLength, NULL, Buf, &Buflen);
							_tprintf( _T("\tWins IPA %d: %s"), i, Buf);
							if( !_tcsstr(Buf, _T("::") ) ) // reject IPV6 for this field
								pCAdapterInfo->strlistWinsIPA.push_back( Buf );
							pWins = pWins->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*Number of Wins Addresses: %d "), i);
					} else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNo Wins Addresses"));


					// Gateway addresses
					pGateway = pCurrAddresses->FirstGatewayAddress;
					if (pGateway) 
					{
						for (i = 0; pGateway != NULL; i++)
						{
							Buflen = WORKING_BUFFER_SIZE;
							WSAAddressToString(pGateway->Address.lpSockaddr, pGateway->Address.iSockaddrLength, NULL, Buf, &Buflen);
							_tprintf( _T("\tGateway IPA %d: %s"), i, Buf);
							pCAdapterInfo->strlistGatwayIPA.push_back( Buf );
							pGateway = pGateway->Next;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\t*Number of Gateway Addresses: %d"), i);
					} else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNo Gateway Addresses"));

					// MAC addresses
					if (pCurrAddresses->PhysicalAddressLength != 0) 
					{
						TCHAR buf[BUFSIZ] = {0};
						for (i = 0; i < (int) pCurrAddresses->PhysicalAddressLength; i++) 
						{
							if (i == (pCurrAddresses->PhysicalAddressLength - 1))
								_stprintf_s(buf, BUFSIZ, _T("%.2X"), (int) pCurrAddresses->PhysicalAddress[i]);
							else
								_stprintf_s(buf, BUFSIZ, _T("%.2X-"), (int) pCurrAddresses->PhysicalAddress[i]);

							pCAdapterInfo->strPhysicalAddress += buf;
						}
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tPhysical address: %s"), pCAdapterInfo->strPhysicalAddress.c_str() );
					}

					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tFlags: %ld"), pCurrAddresses->Flags);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tMtu: %lu"), pCurrAddresses->Mtu);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tIfType: %ld"), pCurrAddresses->IfType);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tOperStatus: %ld"), pCurrAddresses->OperStatus);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tIpv6IfIndex (IPv6 interface): %u"),pCurrAddresses->Ipv6IfIndex);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tZoneIndices (hex): "));
					pCAdapterInfo->dwDhcpv4Enabled = pCurrAddresses->Dhcpv4Enabled;
					pCAdapterInfo->dwDdnsEnabled = pCurrAddresses->DdnsEnabled;

					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tTransmit link speed: %I64u"), pCurrAddresses->TransmitLinkSpeed);
					BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tReceive link speed: %I64u"), pCurrAddresses->ReceiveLinkSpeed);

					pPrefix = pCurrAddresses->FirstPrefix;
					if (pPrefix) 
					{
						for (i = 0; pPrefix != NULL; i++)
							pPrefix = pPrefix->Next;
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNumber of IP Adapter Prefix entries: %d"), i);
					} 
					else
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("\tNumber of IP Adapter Prefix entries: 0"));


					//IPv4 Sub Mask
					{
						TCHAR buf[100];
						ULONG mask = 0;
						DWORD r = ConvertLengthToIpv4Mask(20, &mask); // 20 bit mask
						InetNtop(AF_INET, &mask, buf, 100);
						pCAdapterInfo->strIPMask = buf;
					}

					pCAdapterInfo->strFriendlyName = W2T(pCurrAddresses->FriendlyName);
					pCAdapterInfo->strAdapterName = A2T(pCurrAddresses->AdapterName);
					pCAdapterInfo->strDescription = W2T(pCurrAddresses->Description);
					pCAdapterInfo->strDnsSuffix = W2T(pCurrAddresses->DnsSuffix);


					printf("\n");

					m_AdaptersList.push_back(pCAdapterInfo);
					pCurrAddresses = pCurrAddresses->Next;
				}
			}
			else 
			{
				BaseLogger::WriteFormattedLog(L"Call to GetAdaptersAddresses failed with error: %d",
					dwRetVal);
				if (dwRetVal == ERROR_NO_DATA)
					BaseLogger::WriteFormattedLog(L"\tNo addresses were found for the requested parameters");
				else 
				{

					if (FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
						FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
						NULL, dwRetVal, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),   
						// Default language
						(LPTSTR) & lpMsgBuf, 0, NULL)) 
					{
						BaseLogger::WriteFormattedLog(L"\tError: %s", lpMsgBuf);
						LocalFree(lpMsgBuf);
						if (pAddresses)
						{
							FREE(pAddresses);
							pAddresses = 0;
						}
					}
				}
			}

			if (pAddresses) 
			{
				FREE(pAddresses);
			}


			WSACleanup();
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return 0;
	} // end of EnumerateAdapters


	BOOL GetIPSubnetMask(tstring& tConnectioName, tstring& IPSubnet)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		IPSubnet.clear();
		BOOL bRetv = FALSE;
		/*TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);*/
		try
		{
			/*_stprintf_s(cmd, sz, _T("netsh interface ip show config name=\"%s\" "),
				tConnectioName.c_str() );

			if(CExecuteCommand::Execut(cmd) )*/
			{
				TCHAR tResponseBuf[1024];
				DWORD ResponseBufLen = 1024;
				CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
				TCHAR *p, *subnet = _tcsstr(tResponseBuf, _T("Subnet Prefix:") );
				//Subnet Prefix:       172.30.176.0/20 (mask 255.255.240.0)
				if(subnet)
				{
					vector<tstring>  vLines = split(subnet, _T('\n') );
					RemoveWhiteSpaces( vLines.at(0) );
					subnet = (TCHAR*)_tcsstr(vLines.at(0).c_str(), _T("mask") );
					if(subnet) // 'mask' found
					{
						//p  = subnet + _tcslen(subnet)-1; //point to last character ')'
						p = _tcsstr(subnet, _T(")") );
						if(p) //')' found
						{
							subnet += 4; // skip 'mask'
							*p = '\0'; // remove last character ')'
							bRetv = ValidateIPV4Address(subnet);
							if(bRetv)
							{
								BaseLogger::WriteFormattedLog(TRACEALL, L"CEthernet::GetIPSubnetMask OK got a valid subnet mask : '%s' ",  subnet);
								IPSubnet = subnet;
							}
							else
								BaseLogger::WriteFormattedLog(L"CEthernet::GetIPSubnetMask ERROR: 1 could not get a valid subnet mask: '%s' ",  subnet);
						}
						else
							BaseLogger::WriteFormattedLog(L"CEthernet::GetIPSubnetMask ERROR: 2 could not get a valid subnet mask: '%s' ",  subnet);
					}
				}
				else
					BaseLogger::WriteFormattedLog(TRACEALL, L"CEthernet::GetIPSubnetMask ERROR: 3 could not get a valid subnet mask: '%s' ",  subnet);

			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return bRetv;

	}



	BOOL GetDnsDhcpEnabled(tstring& tName)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		BOOL bRetv = FALSE;
		/*TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);*/
		try
		{
			/*_stprintf_s(cmd, sz, _T("netsh interface ip show config name=\"%s\" "),
				tName.c_str() );

			if(CExecuteCommand::Execut(cmd) )*/
			{
				TCHAR tResponseBuf[1024];
				DWORD ResponseBufLen = 1024;
				CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
				
				if( _tcsstr(tResponseBuf, L"DNS servers configured through DHCP")!=NULL )
					bRetv = TRUE;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return bRetv;

	}

	BOOL GetWinsDhcpEnabled(tstring& tName)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		BOOL bRetv = FALSE;
		/*TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);*/
		try
		{
			/*_stprintf_s(cmd, sz, _T("netsh interface ip show config name=\"%s\" "),
				tName.c_str() );

			if(CExecuteCommand::Execut(cmd) )*/
			{
				TCHAR tResponseBuf[1024];
				DWORD ResponseBufLen = 1024;
				CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);

				if( _tcsstr(tResponseBuf, L"WINS servers configured through DHCP")!=NULL )
					bRetv = TRUE;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, __WFUNCTION__);
			WriteLog(buf);
		}

		return bRetv;

	}



	bool IsPanelName(TCHAR DeviceName[] ) // CCS-UC-CODEC-300
	{
		//std::string iniFile = _T(".\\UpxTaxkMgr.ini");
		//GetPrivateProfileString( _T("UpxPanel"), _T("DeviceType"), _T(""), tDeviceType, MAX_PATH, iniFile.c_str() );
		
		TCHAR CurrentFolder[MAX_PATH];
		::GetModuleFileName(NULL, CurrentFolder, _MAX_PATH);
		TCHAR* pLastBackSlash = _tcsrchr(CurrentFolder, '\\');
		*(pLastBackSlash+1) = 0;
		std::wstring iniFile = CurrentFolder;
		iniFile += _T("UpxTaskMgr.ini");
		TCHAR tDeviceType[MAX_PATH];
		TCHAR Section[MAX_PATH] = _T("UpxPanel");
		TCHAR Key[MAX_PATH] = _T("DeviceType");	
		GetPrivateProfileString(Section, Key, _T(""), tDeviceType, MAX_PATH, iniFile.c_str() );

		return 
			( _tcsicmp(tDeviceType, DeviceName)==0 );
	}

};
