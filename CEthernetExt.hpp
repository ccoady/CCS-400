#pragma once
#include "stdafx.h"
#include "CEthernet.hpp"
#include <winsock2.h>
#include <iphlpapi.h>
#include <wlanapi.h>
#include <stdio.h>
#include <time.h>
#include <vector>

#pragma comment(lib, "wlanapi.lib")
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "SHELL32.LIB") 


#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))
#define WORKING_BUFFER_SIZE  40*1024


using namespace std;
typedef std::basic_string <TCHAR> tstring;

//#define printf TRACE
#define printf ATLTRACE //theApp.m_gCLogMessages->LogMsg
#define tPrint ATLTRACE //theApp.m_gCLogMessages->LogMsg

class CEthernetExt
	: public CEthernet
{
	tstring mtAdapterName = L"";
	tstring mtDescription = L"";
	tstring mtFriendlyName = L"";
	tstring mtPhysicalAddress = L"";
	vector<tstring> mtListUnicastIPA;
	vector<tstring> mtListSubnetMask;
	vector<tstring> mtListGatwayIPA;
	vector<tstring> mtListDhcpv4Server;
	vector<tstring> mtListPrimaryWinsIPA;
	vector<tstring> mtListSecondayWinsIPA;
	vector<tstring> mtListDnsServerIPA;
	bool  mbConnected = 0;
	bool mbDnsEnabled = 0;
	UINT mbDhcp4Enabled = 0;
	bool mbIpv4Enabled = 0;

	tstring tDesiredAdapter = L"Realtek USB FE Family Controller";
	tstring tDesiredIPAddress = L"169.254.0.1"; 
	tstring tDesiredSubnetMask = L"255.255.0.0"; 
	tstring tDesiredGatewayAddress = L"0.0.0.0"; 
	tstring tDesiredDnsAddress = L"0.0.0.0"; 

	vector<tstring> vPermittedAdapters;

public:
	CEthernetExt() 
	{
		vPermittedAdapters.clear();
		//GetDesiredSetting(); // from ini
		ReadAcceptedAdapters(); // from registry
	}
	~CEthernetExt() {}
	CEthernetExt(tstring adpaterName) 
	{ 
		tDesiredAdapter = adpaterName;
		fGetAdapterInfo();
		fGetAdpaterAddresses();
	}
	//
	// Get desired static informaton 
	//
	void GetDesiredSetting()
	{
		tPrint(L"Reading user preferences \r\n");

		tstring location = L".\\";
		TCHAR defaultDir[MAX_PATH] = {};
		if (GetEnvironmentVariable(_T("CRESTRON_USER"), defaultDir, MAX_PATH ) != 0)
			location = defaultDir;
		location += L"\\hwSettings\\DirectConnect.ini";
		TCHAR tResult[MAX_PATH];
		
		ZeroMemory(tResult, MAX_PATH);
		::GetPrivateProfileString(_T("DirectConnect"), _T("DesiredAdapter"), L"", tResult, MAX_PATH, location.c_str());
		tDesiredAdapter = tResult;
		tPrint(L"\t tDesiredAdapter = %s \r\n", tDesiredAdapter.c_str());
//-
		vPermittedAdapters = split(tDesiredAdapter.c_str(), L';');
//-

		ZeroMemory(tResult, MAX_PATH);
		::GetPrivateProfileString(_T("DirectConnect"), _T("DesiredIPAddress"), L"", tResult, MAX_PATH, location.c_str());
		tDesiredIPAddress = tResult;
		tPrint(L"\t tDesiredIPAddress = %s \r\n", tDesiredIPAddress.c_str());

		ZeroMemory(tResult, MAX_PATH);
		::GetPrivateProfileString(_T("DirectConnect"), _T("DesiredSubnetMask"), L"", tResult, MAX_PATH, location.c_str());
		tDesiredSubnetMask = tResult;
		tPrint(L"\t tDesiredSubnetMask = %s \r\n", tDesiredSubnetMask.c_str());

		ZeroMemory(tResult, MAX_PATH);
		::GetPrivateProfileString(_T("DirectConnect"), _T("DesiredGatewayAddress"), L"", tResult, MAX_PATH, location.c_str());
		tDesiredGatewayAddress = tResult;
		tPrint(L"\t tDesiredGatewayAddress = %s \r\n", tDesiredGatewayAddress.c_str());

		ZeroMemory(tResult, MAX_PATH);
		::GetPrivateProfileString(_T("DirectConnect"), _T("DesiredDnsAddress "), L"", tResult, MAX_PATH, location.c_str());
		tDesiredDnsAddress = tResult;
		tPrint(L"\t tDesiredGatewayAddress = %s \r\n", tDesiredGatewayAddress.c_str());
	}

	int ReadAcceptedAdapters()
	{
		HKEY hKey;
		if(ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Crestron Electronics Inc.\\ImageSettings", NULL, KEY_QUERY_VALUE, &hKey) )
		{
			DWORD type, size;
			vector<wstring> target;

			if (RegQueryValueExW(
				hKey, // HKEY
				L"DirectConnectAdapters",
				NULL,
				&type,
				NULL,
				&size) != ERROR_SUCCESS)
				return 1;

			if (type == REG_MULTI_SZ)
			{
				vector<wchar_t> temp(size / sizeof(wchar_t));

				if (RegQueryValueExW(
					hKey, // HKEY
					L"DirectConnectAdapters",
					NULL,
					NULL,
					reinterpret_cast<LPBYTE>(&temp[0]),
					&size) != ERROR_SUCCESS)
					return 2;

				size_t index = 0;
				size_t len = wcslen(&temp[0]);
				while (len > 0)
				{
					target.push_back(&temp[index]);
					index += len + 1;
					len = wcslen(&temp[index]);
				}
			}

			// remove extra quote "" and save 
			for each(wstring s in target)
			{
				s.erase(std::remove(s.begin(), s.end(), L'\"'), s.end());
				vPermittedAdapters.push_back(s);
			}

			// log all the vids and pids
			wstring str = L"DirectConnect: Supported Adpaters' Name: ";
			int sz = str.length();
			for each(wstring s in vPermittedAdapters)
			{
				if (str.length()> sz) str += L", "; // append a ,
				str += s;
			}
			BaseLogger::WriteFormattedLog(VERBOSE, (TCHAR*)str.c_str() );
		}

		return 0;
	}

	BOOL isDesiredAdapter(tstring ThisAdapter, tstring desireAdapter)
	{
		if ( ThisAdapter.find(desireAdapter)==0
			|| ThisAdapter.find(L"DisplayLink Ethernet")==0 )
		{
			tDesiredAdapter = ThisAdapter;
			return TRUE;
		}
		
		return FALSE;
	}

	BOOL isDesiredAdapter(tstring ThisAdapter)
	{
		for each(tstring p in vPermittedAdapters)
		{
			if (ThisAdapter.find(p) == 0
				|| ThisAdapter.find(L"DisplayLink Ethernet") == 0)
			{
				tDesiredAdapter = ThisAdapter;
				return TRUE;
			}
		}

		return FALSE;
	}

	//
	// Gets information for the selected adapter
	//
	int fGetAdapterInfo()
	{
		BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: fGetAdapterInfo IN");

		USES_CONVERSION;

		DWORD Err;
		PFIXED_INFO pFixedInfo;
		DWORD FixedInfoSize = 0;

		PIP_ADAPTER_INFO pAdapterInfo, pAdapt;
		DWORD AdapterInfoSize;
		PIP_ADDR_STRING pAddrStr;

		//
		// Get the main IP configuration information for this machine using a FIXED_INFO structure
		//
		if ((Err = GetNetworkParams(NULL, &FixedInfoSize)) != 0)
		{
			if (Err != ERROR_BUFFER_OVERFLOW)
			{
				printf("GetNetworkParams sizing failed with error %d\n", Err);
				return -1;
			}
		}

		// Allocate memory from sizing information
		if ((pFixedInfo = (PFIXED_INFO)GlobalAlloc(GPTR, FixedInfoSize)) == NULL)
		{
			printf("Memory allocation error\n");
			return -1;
		}

		if ((Err = GetNetworkParams(pFixedInfo, &FixedInfoSize)) == 0)
		{
			printf("\tHost Name . . . . . . . . . : %s\n", pFixedInfo->HostName);
			printf("\tDNS Servers . . . . . . . . : %s\n", pFixedInfo->DnsServerList.IpAddress.String);
			pAddrStr = pFixedInfo->DnsServerList.Next;
			while (pAddrStr)
			{
				printf("%51s\n", pAddrStr->IpAddress.String);
				pAddrStr = pAddrStr->Next;
			}

			printf("\tNode Type . . . . . . . . . : ");
			switch (pFixedInfo->NodeType)
			{
			case 1:
				printf("%s\n", "Broadcast");
				break;
			case 2:
				printf("%s\n", "Peer to peer");
				break;
			case 4:
				printf("%s\n", "Mixed");
				break;
			case 8:
				printf("%s\n", "Hybrid");
				break;
			default:
				printf("\n");
			}

			printf("\tNetBIOS Scope ID. . . . . . : %s\n", pFixedInfo->ScopeId);
			printf("\tIP Routing Enabled. . . . . : %s\n", (pFixedInfo->EnableRouting ? "yes" : "no"));
			printf("\tWINS Proxy Enabled. . . . . : %s\n", (pFixedInfo->EnableProxy ? "yes" : "no"));
			printf("\tNetBIOS Resolution Uses DNS : %s\n", (pFixedInfo->EnableDns ? "yes" : "no"));
		}
		else
		{
			printf("GetNetworkParams failed with error %d\n", Err);
			return -1;
		}

		//
		// Enumerate all of the adapter specific information using the IP_ADAPTER_INFO structure.
		// Note:  IP_ADAPTER_INFO contains a linked list of adapter entries.
		//
		AdapterInfoSize = 0;
		if ((Err = GetAdaptersInfo(NULL, &AdapterInfoSize)) != 0)
		{
			if (Err != ERROR_BUFFER_OVERFLOW)
			{
				printf("GetAdaptersInfo sizing failed with error %d\n", Err);
				return -1;
			}
		}

		// Allocate memory from sizing information
		if ((pAdapterInfo = (PIP_ADAPTER_INFO)GlobalAlloc(GPTR, AdapterInfoSize)) == NULL)
		{
			printf("Memory allocation error\n");
			return -1;
		}

		// Get actual adapter information
		if ((Err = GetAdaptersInfo(pAdapterInfo, &AdapterInfoSize)) != 0)
		{
			printf("GetAdaptersInfo failed with error %d\n", Err);
			return -1;
		}

		pAdapt = pAdapterInfo;

		while (pAdapt)
		{
			mtDescription = A2T(pAdapt->Description);
			//if (mtDescription.find(tDesiredAdapter) == 0)
			//if( isDesiredAdapter(mtDescription, tDesiredAdapter) )
			if (isDesiredAdapter(mtDescription))
			{
				printf("\n");
				printf("\tDescription . . . . . . . . : %s\n", pAdapt->Description);
				printf("\tAdapterName . . . . . . . . : %s\n", pAdapt->AdapterName);
				mtAdapterName = A2T(pAdapt->AdapterName);

				char adapterType[] = "\tAdapterType . . . . . . . . : %s";
				switch (pAdapt->Type)
				{
				case MIB_IF_TYPE_ETHERNET:
					printf(adapterType, "Ethernet adapter \n");
					break;
				case MIB_IF_TYPE_TOKENRING:
					printf(adapterType, "Token Ring adapter \n");
					break;
				case MIB_IF_TYPE_FDDI:
					printf(adapterType, "FDDI adapter \n");
					break;
				case MIB_IF_TYPE_PPP:
					printf(adapterType, "PPP adapter \n");
					break;
				case MIB_IF_TYPE_LOOPBACK:
					printf(adapterType, "Loopback adapter \n");
					break;
				case MIB_IF_TYPE_SLIP:
					printf(adapterType, "Slip adapter \n");
					break;
				case MIB_IF_TYPE_OTHER:
				default:
					printf(adapterType, "Other adapter \n");
				}

				char physicalAddress[MAX_PATH] = {};
				char tmp[MAX_PATH] = {};
				for (UINT i = 0; i < pAdapt->AddressLength; i++)
				{
					if (i == (pAdapt->AddressLength - 1))
						//printf("%.2X\n", (int)pAdapt->Address[i]);
						sprintf_s(tmp, "%.2X\n", (int)pAdapt->Address[i]);
					else
						//printf("%.2X-", (int)pAdapt->Address[i]);
						sprintf_s(tmp, "%.2X-", (int)pAdapt->Address[i]);

					strcat_s(physicalAddress, tmp);

				}

				printf("\tPhysical Address. . . . . . : %s", physicalAddress);
				mtPhysicalAddress = A2T(physicalAddress);

				printf("\tDHCP Enabled. . . . . . . . : %s\n", (pAdapt->DhcpEnabled ? "yes" : "no"));
				mbDhcp4Enabled = pAdapt->DhcpEnabled;


				pAddrStr = &(pAdapt->IpAddressList);
				while (pAddrStr)
				{
					printf("\tIP Address. . . . . . . . . : %s\n", pAddrStr->IpAddress.String);
					mtListUnicastIPA.push_back(A2T(pAddrStr->IpAddress.String));
					printf("\tSubnet Mask . . . . . . . . : %s\n", pAddrStr->IpMask.String);
					mtListSubnetMask.push_back(A2T(pAddrStr->IpMask.String));

					pAddrStr = pAddrStr->Next;
				}

				printf("\tDefault Gateway . . . . . . : %s\n", pAdapt->GatewayList.IpAddress.String);
				mtListGatwayIPA.push_back(A2T(pAdapt->GatewayList.IpAddress.String));
				pAddrStr = pAdapt->GatewayList.Next;
				while (pAddrStr)
				{
					printf("%51s\n", pAddrStr->IpAddress.String);
					mtListGatwayIPA.push_back(A2T(pAdapt->GatewayList.IpAddress.String));
					pAddrStr = pAddrStr->Next;
				}

				printf("\tDHCP Server . . . . . . . . : %s\n", pAdapt->DhcpServer.IpAddress.String);
				mtListDhcpv4Server.push_back(A2T(pAdapt->DhcpServer.IpAddress.String));
				printf("\tPrimary WINS Server . . . . : %s\n", pAdapt->PrimaryWinsServer.IpAddress.String);
				mtListPrimaryWinsIPA.push_back(A2T(pAdapt->PrimaryWinsServer.IpAddress.String));
				printf("\tSecondary WINS Server . . . : %s\n", pAdapt->SecondaryWinsServer.IpAddress.String);
				mtListSecondayWinsIPA.push_back(A2T(pAdapt->SecondaryWinsServer.IpAddress.String));

				break; // found the required adapter 

				/*
					struct tm newtime;
					char buffer[32];
					errno_t error;

						// Display coordinated universal time - GMT
				#ifdef WIN64
						error = _localtime64_s(&newtime, &pAdapt->LeaseObtained);
				#else
						error = _localtime32_s(&newtime, &pAdapt->LeaseObtained);
				#endif
						if (error)
						{
							printf("Invalid Argument to _localtime32_s.");
						}
						else {
							// Convert to an ASCII representation
							error = asctime_s(buffer, 32, &newtime);
							if (error)
							{
								printf("Invalid Argument to asctime_s.");
							}
							else {
								printf("\tLease Obtained. . . . . . . : %s", buffer);
							}
						}

				#ifdef WIN64
						error = _localtime64_s(&newtime, &pAdapt->LeaseExpires);
				#else
						error = _localtime32_s(&newtime, &pAdapt->LeaseExpires);
				#endif
						if (error)
						{
							printf("Invalid Argument to _localtime32_s.");
						}
						else {
							// Convert to an ASCII representation
							error = asctime_s(buffer, 32, &newtime);
							if (error)
							{
								printf("Invalid Argument to asctime_s.");
							}
							else {
								printf("\tLease Expires . . . . . . . : %s", buffer);
							}
						}
				*/
			}
			pAdapt = pAdapt->Next;
		}


		FREE(pAdapterInfo);

		BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: fGetAdapterInfo OUT");

		return 0;
	}

	//
	// Get addresses for the selected adapter
	int fGetAdpaterAddresses()
	{
		BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: fGetAdpaterAddresses IN");

		const DWORD SNA_ERROR_MALLOC = 1;
		const DWORD SNA_ERROR_GETADAPTERSADDRESS = 2;

		// Get network adapters for IPv4 using GetAdaptersAddresses
		ULONG nFlags = GAA_FLAG_INCLUDE_PREFIX;
		ULONG nFamily = AF_INET;
		PIP_ADAPTER_ADDRESSES pAddresses = NULL;
		PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;
		ULONG nBufLen = 16324, nTries = 0, nMaxTries = 3;
		DWORD dwResult;

		do
		{
			pAddresses = (IP_ADAPTER_ADDRESSES*)MALLOC(nBufLen);
			if (pAddresses == NULL)
			{
				tPrint(L"Memory allocation failed for IP_ADAPTER_ADDRESSES struct\n" );
				return SNA_ERROR_MALLOC;
			}

			dwResult = GetAdaptersAddresses(nFamily, nFlags, NULL, pAddresses, &nBufLen);
			if (dwResult == ERROR_BUFFER_OVERFLOW)
			{
				FREE(pAddresses);
				pAddresses = NULL;
			}
			else
				break;

			nTries++;
		} while (dwResult == ERROR_BUFFER_OVERFLOW && nTries < nMaxTries);



		if (dwResult != NO_ERROR)
		{
			tPrint(L"Call to GetAdaptersAddresses failed with error: %d\n", dwResult);
			FREE(pAddresses);
			return SNA_ERROR_GETADAPTERSADDRESS;
		}


		// iterate through all the adapters, chose one 
		USES_CONVERSION;
		pCurrAddress = pAddresses;
		while (pCurrAddress)
		{
			//??if (pCurrAddress->IfType == IF_TYPE_ETHERNET_CSMACD)
			{
				mtDescription = pCurrAddress->Description;      //L"Realtek USB FE Family Controller";
				//if (mtDescription.find(tDesiredAdapter) == 0)
				//if ( isDesiredAdapter(mtDescription, tDesiredAdapter) )
				if (isDesiredAdapter(mtDescription))
				{
					mtAdapterName = A2T(pCurrAddress->AdapterName); //"{6297DFD9-84D3-4014-9F7C-111457B40D5F}";
					mtFriendlyName = pCurrAddress->FriendlyName;    //L"Ethernet 4";

					// adapter is connected becuase this code reached here, ethernet cable could be Connected or Disconnected
					IF_OPER_STATUS connected = pCurrAddress->OperStatus; //1 up, 2 down
					mbConnected = 1; // connected ? 1 : 0;
					
					/*// adapter is connected, is ethernet cable is connected ?
					GetInterfaceStatus(mtFriendlyName.c_str()); 
					mbConnected = CheckForSuccess(_T("Connected")); */
					
					mbDnsEnabled = pCurrAddress->DdnsEnabled ? 1 : 0;
					mbDhcp4Enabled = pCurrAddress->Dhcpv4Enabled ? 1 : 0;
					mbIpv4Enabled = pCurrAddress->Ipv4Enabled ? 1 : 0;

					//mtListDnsServerIPA
					IP_ADAPTER_DNS_SERVER_ADDRESS *pDnServer = pCurrAddress->FirstDnsServerAddress;
					if (pDnServer)
					{
						DWORD Buflen = WORKING_BUFFER_SIZE;
						TCHAR Buf[WORKING_BUFFER_SIZE] = {};
						int i = 0;
						for (i = 0; pDnServer != NULL; i++)
						{
							INT retv = WSAAddressToString(pDnServer->Address.lpSockaddr, pDnServer->Address.iSockaddrLength, NULL, Buf, &Buflen);
							tPrint(L"\tDnServer IPA %d: %s", i, Buf);
							if (!_tcsstr(Buf, _T("::"))) // reject IPV6 for this field
								mtListDnsServerIPA.push_back(Buf);
							pDnServer = pDnServer->Next;
						}
						printf("\t*Number of DNS Server Addresses: %d ", i);
					}
					else
						printf("\t*No DNS Server Addresses");

					break; //found the adapter
				}
			}

			pCurrAddress = pCurrAddress->Next;
		}

		FREE(pAddresses);

		BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: fGetAdpaterAddresses OUT");

		return 0;
	}

	//
	// Set dhcp enable for the selected adapter
	//
	void SetDhcpInfo()
	{
		/*CString sNetshIp;
		sNetshIp.Format(L"/C netsh interface ip set ADDRESS name=\"%s\" source=dhcp", mtFriendlyName.c_str());
		sNetshIp.Trim();

		SHELLEXECUTEINFO shExInfo = {};
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpVerb = _T("runas");                // Operation to perform
		shExInfo.lpFile = _T("C:\\Windows\\System32\\cmd.exe");       // Application to start    
		shExInfo.lpParameters = sNetshIp.GetBuffer();                  // Additional parameters
		shExInfo.lpDirectory = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.hInstApp = 0;

		if (ShellExecuteEx(&shExInfo))
			CloseHandle(shExInfo.hProcess);*/
	}

	//
	// Set dhcp disable, or static, for the selected adapter
	//
	int SetStaticInfo0()
	{
		/*
		// IP
		CString sNetshIp;
		sNetshIp.Format(L"/C netsh interface ip set address name=\"%s\" source=static addr=169.254.0.1 mask=255.255.0.0 gateway=0.0.0.0 1", mtFriendlyName.c_str());
		SHELLEXECUTEINFO shExInfo = {};
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpDirectory = 0;
		shExInfo.hInstApp = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.lpVerb = _T("runas"); // Operation to perform
		shExInfo.lpFile = _T("C:\\Windows\\System32\\cmd.exe"); // Application to start    
		shExInfo.lpParameters = sNetshIp.GetBuffer(); // Additional parameters
		if (ShellExecuteEx(&shExInfo))
			CloseHandle(shExInfo.hProcess);

		//DNS
		CString sNetshDns;
		sNetshDns.Format(L"/C netsh interface ip set dns name=\"%s\" source=static  0.0.0.0", mtFriendlyName.c_str() );
		ZeroMemory(&shExInfo, sizeof(SHELLEXECUTEINFO));
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpDirectory = 0;
		shExInfo.hInstApp = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.lpVerb = _T("runas"); // Operation to perform
		shExInfo.lpFile = _T("C:\\Windows\\System32\\cmd.exe"); // Application to start    
		shExInfo.lpParameters = sNetshDns.GetBuffer(); // Additional parameters
		if (ShellExecuteEx(&shExInfo))
			CloseHandle(shExInfo.hProcess);*/

		return 1;
	}

	int SetStaticInfo()
	{
		/*int iRetv = 1;

		if (IsMediaConnected() == false)
		{
			printf("ERROR: Media disconnected. No changes can be made \r\n"); 
			return -1;
		}

		// check 
		if (tDesiredAdapter.empty()
			|| tDesiredIPAddress.empty()
			|| tDesiredSubnetMask.empty()
			|| tDesiredGatewayAddress.empty()
			)
		{
			printf("ERROR: Failed to make IP satic. Because one of the required parameter is null \r\n");
			return -1;
		}


		// format 
		CString sNetshIp;
		sNetshIp.Format(_T("/C netsh interface ip set address name=\"%s\" static %s %s %s"),
			mtFriendlyName.c_str(),
			tDesiredIPAddress.c_str(),
			tDesiredSubnetMask.c_str(),
			tDesiredGatewayAddress.c_str());

		// apply ip changes
		SHELLEXECUTEINFO shExInfo = {};
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpDirectory = 0;
		shExInfo.hInstApp = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.lpVerb = _T("runas"); // Operation to perform
		shExInfo.lpFile = _T("C:\\Windows\\System32\\cmd.exe"); // Application to start    
		shExInfo.lpParameters = sNetshIp.GetBuffer(); // Additional parameters
		if (ShellExecuteEx(&shExInfo))
			CloseHandle(shExInfo.hProcess);


		// check
		if (tDesiredDnsAddress.empty())
		{
			printf("ERROR: Failed to set DNS satic. Because dns address is null \r\n");
			return -1;
		}

		// format
		CString sNetshDns;
		sNetshDns.Format(_T("/C netsh interface ip set dns name=\"%s\" static %s"),
			mtFriendlyName.c_str(),
			tDesiredDnsAddress.c_str());

		// apply dns changes
		ZeroMemory(&shExInfo, sizeof(SHELLEXECUTEINFO));
		shExInfo.cbSize = sizeof(shExInfo);
		shExInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
		shExInfo.hwnd = 0;
		shExInfo.lpDirectory = 0;
		shExInfo.hInstApp = 0;
		shExInfo.nShow = SW_SHOW;
		shExInfo.lpVerb = _T("runas"); // Operation to perform
		shExInfo.lpFile = _T("C:\\Windows\\System32\\cmd.exe"); // Application to start    
		shExInfo.lpParameters = sNetshDns.GetBuffer(); // Additional parameters
		if (ShellExecuteEx(&shExInfo))
			CloseHandle(shExInfo.hProcess);*/

		return 1;
	}

	bool NeedsChange()
	{
		if (mbConnected == 0)
		{
			BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: Media disconnected or the \"%s\" adapter is not connected", tDesiredAdapter.c_str()  );
			//return false; // ethernet cable not connected
			BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: Media disconnected but will still apply settings");
		}

		//if (mtDescription.find(tDesiredAdapter) != 0)
		//if (isDesiredAdapter(mtDescription, tDesiredAdapter) == FALSE)
		if (isDesiredAdapter(mtDescription) == FALSE)
		{
			BaseLogger::WriteFormattedLog(VERBOSE, L"DirectConnect: Adapter name is not specified");
			return false; // desired adapter is not connected
		}

		if (mtFriendlyName.length() == 0)
		{
			BaseLogger::WriteFormattedLog(L"DirectConnect: Friendly name is not found");
			return false; // connection name is not found
		}
	
		if (mtListUnicastIPA.size() > 0)
			if (mtListUnicastIPA.at(0).find(tDesiredIPAddress) == 0) // its already correct ip address
				if (mbDhcp4Enabled == 0) // its already static 
			{
				BaseLogger::WriteFormattedLog(L"DirectConnect: The adpater already has desired IP %s, and DHCP is %d", tDesiredIPAddress.c_str(), mbDhcp4Enabled);
				return false; // current ip and desired ip are the same 
			}

		return true; // proceed to change ip address 
	}


	bool IsConnected()
	{
		if (isDesiredAdapter(mtDescription) == FALSE)
		{
			BaseLogger::WriteFormattedLog(L"DirectConnect: Adapter name is not specified");
			return false; // desired adapter is not connected
		}

		if (mtFriendlyName.length() == 0)
		{
			BaseLogger::WriteFormattedLog(L"DirectConnect: Friendly name is not found");
			return false; // connection name is not found
		}

		return true;
	}
	



	TCHAR* ConnectioName()
	{
		return (TCHAR*)mtFriendlyName.c_str();
	}

	BOOL CopyFieldByField(CAdapterInfo& DestInfo )
	{
		DestInfo.strAdapterName = mtAdapterName;
		DestInfo.strDescription = mtDescription;
		DestInfo.strFriendlyName = mtFriendlyName;
		DestInfo.strPhysicalAddress = mtPhysicalAddress;

		//?? DestInfo.strDnsSuffix = SrcInfo.strDnsSuffix;
		DestInfo.dwDdnsEnabled = mbDnsEnabled;
		DestInfo.dwDhcpv4Enabled = mbDhcp4Enabled;
		//??DestInfo.dwIfType = SrcInfo.dwIfType;
		//??DestInfo.strNetworkGuid = SrcInfo.strNetworkGuid;
		DestInfo.strIPMask = mtListSubnetMask.size() > 0 ? mtListSubnetMask.at(0) : L"";
		DestInfo.strlistUnicastIPA = mtListUnicastIPA;
		//??DestInfo.strlistAnycastIPA = SrcInfo.strlistAnycastIPA;
		//??DestInfo.strlistMulticastIPA = SrcInfo.strlistMulticastIPA;
		DestInfo.strlistDnsServerIPA = mtListDnsServerIPA;
		DestInfo.strlistWinsIPA = mtListPrimaryWinsIPA;
		DestInfo.strlistGatwayIPA = mtListGatwayIPA;
		DestInfo.strlistDhcpv4Server = mtListDhcpv4Server;
		DestInfo.bInterfaceConnected = mbConnected;
		//?? DestInfo.bDnsDhcpEnabled = SrcInfo.bDnsDhcpEnabled;
		//?? DestInfo.bWinsDhcpEnabled = SrcInfo.bWinsDhcpEnabled;

		return TRUE;
	}

	BOOL EnableDhcp()
	{
		if (mtFriendlyName.length() == 0)
		{
			printf("ERROR: Connection is unknown \r\n");
			return FALSE;
		}

		if (mbDhcp4Enabled)
		{
			printf("INFO: DHCP is already ON \r\n");
			return FALSE;
		}
		
		BOOL bReturnValue = 0;
		TCHAR cmd[MAX_PATH] = {};

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set address name=\"%s\" source=dhcp", 
			mtFriendlyName.c_str() );
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		return bReturnValue;
	}

	BOOL DisableDhcp()
	{
		if (mtFriendlyName.length() == 0)
		{
			printf("ERROR: Connection is unknown \r\n");
			return FALSE;
		}

		if (!mbDhcp4Enabled)
		{
			printf("INFO: DHCP is already OFF \r\n");
			return FALSE;
		}

		if (!(mtListUnicastIPA.size() > 0
			&& mtListSubnetMask.size() > 0
			&& mtListGatwayIPA.size() > 0) )
		{
			printf("ERROR: Invalid IP addresses\r\n");
			return FALSE;
		}


		BOOL bReturnValue = 0;
		TCHAR cmd[MAX_PATH] = {};

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set address name=\"%s\" source=static addr=%s mask=%s gateway=%s",
			mtFriendlyName.c_str(),
			mtListUnicastIPA.at(0).c_str(),
			mtListSubnetMask.at(0).c_str(),
			mtListGatwayIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		if (! mtListDnsServerIPA.size() > 0 )
		{
			printf("ERROR: Invalid DNS addresses\r\n");
			return FALSE;
		}

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set dns name=\"%s\" source=static  %s",
			mtFriendlyName.c_str(),
			mtListDnsServerIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		tPrint(L"DHCP changed to static \r\n");

		return bReturnValue;
	}
	BOOL DisableDhcpDns()
	{
		if (mtFriendlyName.length() == 0)
		{
			printf("ERROR: Connection is unknown \r\n");
			return FALSE;
		}

		/*if (!mbDhcp4Enabled)
		{
			printf("INFO: DHCP is already OFF \r\n");
			return FALSE;
		}*/

		if (!(mtListUnicastIPA.size() > 0
			&& mtListSubnetMask.size() > 0
			&& mtListGatwayIPA.size() > 0))
		{
			printf("ERROR: Invalid IP addresses\r\n");
			return FALSE;
		}


		BOOL bReturnValue = 0;
		TCHAR cmd[MAX_PATH] = {};

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set address name=\"%s\" source=static addr=%s mask=%s gateway=%s",
			mtFriendlyName.c_str(),
			mtListUnicastIPA.at(0).c_str(),
			mtListSubnetMask.at(0).c_str(),
			mtListGatwayIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		if (!mtListDnsServerIPA.size() > 0)
		{
			printf("ERROR: Invalid DNS addresses\r\n");
			return FALSE;
		}

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set dns name=\"%s\" source=static  %s",
			mtFriendlyName.c_str(),
			mtListDnsServerIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		tPrint(L"DHCP changed to static \r\n");

		return bReturnValue;
	}
	BOOL DisableDhcpWins()
	{
		if (mtFriendlyName.length() == 0)
		{
			printf("ERROR: Connection is unknown \r\n");
			return FALSE;
		}

		if (!mbDhcp4Enabled)
		{
			printf("INFO: DHCP is already OFF \r\n");
			return FALSE;
		}

		if (!(mtListUnicastIPA.size() > 0
			&& mtListSubnetMask.size() > 0
			&& mtListGatwayIPA.size() > 0))
		{
			printf("ERROR: Invalid IP addresses\r\n");
			return FALSE;
		}


		BOOL bReturnValue = 0;
		TCHAR cmd[MAX_PATH] = {};

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set address name=\"%s\" source=static addr=%s mask=%s gateway=%s",
			mtFriendlyName.c_str(),
			mtListUnicastIPA.at(0).c_str(),
			mtListSubnetMask.at(0).c_str(),
			mtListGatwayIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		if (!mtListDnsServerIPA.size() > 0)
		{
			printf("ERROR: Invalid DNS addresses\r\n");
			return FALSE;
		}

		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set dns name=\"%s\" source=static  %s",
			mtFriendlyName.c_str(),
			mtListDnsServerIPA.at(0).c_str());
		bReturnValue = StartAppAsUserAndWait(cmd, false, 5000);

		tPrint(L"DHCP changed to static \r\n");

		return bReturnValue;
	}

};

