#pragma once;

#include "windows.h"

#include <sstream>
#include <string>
#include <vector>
//#include "CStringUtil.hpp"
#include "CExecuteCommand.hpp"
#include <lm.h>

#include <DSRole.h>
#pragma comment(lib, "Netapi32.lib")

#include "Windns.h"
#pragma comment(lib, "Dnsapi.lib")


using namespace std;

struct CDeviceNamesInfo
{
	tstring PanelName;
	tstring WorkStationName;
	tstring DomainName;
	tstring MoreNames[ComputerNameMax];
};

class CDomain : public CExecuteCommand
{

public:
	CDomain(){}
	~CDomain(){}

	//
	// Adds a panel to a specified domain, but does not joins
	//
	BOOL AddPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[])
	{
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("netdom ADD %s /d:%s /userD:%s  /passwordD:%s"), 
				tPanelName, domainName, userD, passwordD);

			return CExecuteCommand::Execut(cmd);
		}

		return FALSE;
	}

	//
	// Joins a panel to a specified domain
	//
	BOOL JoinPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[])
	{
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("netdom JOIN %s /domain:%s /userD:%s  /passwordD:%s"), 
				tPanelName, domainName, userD, passwordD);

			return CExecuteCommand::Execut(cmd);
		}

		return FALSE;
	}

	//
	// Removs panel from domain
	//
	BOOL RemovePanelFromDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[])
	{
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("netdom REMOVE %s /domain:%s /userD:%s  /passwordD:%s"), 
				tPanelName, domainName, userD, passwordD);

			return CExecuteCommand::Execut(cmd);
		}
		return FALSE;

	}

	//
	// Queries the domain for a list of panels
	//
	BOOL QueryPanelsInDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[])
	{
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("netdom QUERY  /domain:%s /userD:%s  /passwordD:%s  WORKSTATION"), 
				domainName, userD, passwordD);

			return CExecuteCommand::Execut(cmd);
		}

		return FALSE;
	}

	//
	// Renames panel in domain
	//
	BOOL RenamePanelInDomain(TCHAR panelNewName[], TCHAR domainName[], TCHAR userD[], TCHAR passwordD[])
	{
		//method1: netdom renamecomputer oldComputerName /newname:newComputerName /userD:mydomain\administrator /passwordd:* /usero:administrator /passwordo:* /reboot:60
		//method2: WMIC ComputerSystem where Name="COMPUTER-NAME" call Rename Name="NewName"
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("netdom RENAMECOMPUTER %s  /newName:%s  /userD:%s\\%s  /passwordD:%s"), 
				tPanelName, panelNewName, domainName, userD, passwordD);

			return CExecuteCommand::Execut(cmd);
		}

		return FALSE;
	}

	BOOL RenamePanelInWorkgroup(TCHAR panelNewName[])
	{
		//method1: netdom renamecomputer oldComputerName /newname:newComputerName /userD:mydomain\administrator /passwordd:* /usero:administrator /passwordo:* /reboot:60
		//method2: WMIC ComputerSystem where Name="COMPUTER-NAME" call Rename Name="NewName"
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("WMIC ComputerSystem Where Name=\"%s\" call Rename Name=\"%s\""), 
				tPanelName, panelNewName);

			return CExecuteCommand::Execut(cmd);
		}

		return FALSE;
	}

	BOOL RenameWorkgroup(TCHAR WorkGroupNewName[])
	{
		// cmd: Wmic computersystem where name="%computername%" call joindomainorworkgroup name="New_Workgroup_Name"
		TCHAR tPanelName[256];
		DWORD dwSize = TSIZEOF(tPanelName);
		if(GetPanelName(tPanelName, dwSize, ComputerNameNetBIOS) )
		{
			TCHAR cmd[1024];
			_stprintf_s(cmd, TSIZEOF(cmd), _T("Wmic computersystem where name=\"%s\" call joindomainorworkgroup name=\"%s\""), 
				tPanelName, WorkGroupNewName);
			return CExecuteCommand::Execut(cmd);
		}

		//if  ( strstr(resultBuf, _T("Method execution successful.") )!=NULL ) // save data and stop
		//	_stprintf_s(resultBuf, resultBufLen, _T("OK") );
		//else
		//	_stprintf_s(resultBuf, resultBufLen, _T("Error") );

		return FALSE;
	}

	BOOL GetPanelName(TCHAR tPanelName[], DWORD &dwSize/*in&out*/, COMPUTER_NAME_FORMAT cnf)
	{
		TCHAR szDescription[8][32] = 
		{
			TEXT("NetBIOS"), 
			TEXT("DNS hostname"), 
			TEXT("DNS domain"), 
			TEXT("DNS fully-qualified"), 
			TEXT("Physical NetBIOS"), 
			TEXT("Physical DNS hostname"), 
			TEXT("Physical DNS domain"), 
			TEXT("Physical DNS fully-qualified")
		};

		if (!GetComputerNameEx(cnf, tPanelName, &dwSize))
		{
			_tprintf(TEXT("GetComputerNameEx failed (%d)\n"), GetLastError());
			return FALSE;
		}
		else 
			_tprintf(TEXT("%s: %s\n"), szDescription[cnf], tPanelName);

		return TRUE;
	}

	//
	// Gets panel, workgroup and dns names
	//
	BOOL GetDeviceNamesInfo(CDeviceNamesInfo &info)
	{
		//API GetPanelNameEx for all 8 names of the computer specifified in enum COMPUTER_NAME_FORMAT
		{
			TCHAR tPanelName[1024];
			DWORD dwSize = 1024;
			TCHAR szDescription[8][32] = 
			{
				TEXT("NetBIOS"), 
				TEXT("DNS hostname"), 
				TEXT("DNS domain"), 
				TEXT("DNS fully-qualified"), 
				TEXT("Physical NetBIOS"), 
				TEXT("Physical DNS hostname"), 
				TEXT("Physical DNS domain"), 
				TEXT("Physical DNS fully-qualified")
			};

			for(int i=0; i<ComputerNameMax; i++)
			{
				if ( GetComputerNameEx((COMPUTER_NAME_FORMAT)i, tPanelName, &dwSize))
				{
					if(i==ComputerNamePhysicalNetBIOS && _tcslen(tPanelName) )
						info.PanelName = tPanelName;
					if(i==ComputerNameDnsDomain && _tcslen(tPanelName) )
						info.DomainName = tPanelName;
					//info.MoreNames[i] = tPanelName;
					//_tprintf(TEXT("%s: %s\n"), szDescription[i], tPanelName);
					tPanelName[0] = 0;
					dwSize = 1024;
				}
				else 
					_tprintf(TEXT("ERROR: GetPanelNameEx failed, ddwSize %d, GetLastError %d \n"), dwSize, GetLastError());
			}
		}


		//API NetWkstaGetInfo for Workgroup name
		if( info.DomainName.empty() )
		{
			DWORD dwLevel = 102;
			LPWKSTA_INFO_102 pBuf = NULL;
			NET_API_STATUS nStatus;
			LPWSTR pszServerName = NULL;

			// Call the NetWkstaGetInfo function, specifying level 102.
			nStatus = NetWkstaGetInfo(pszServerName, dwLevel, (LPBYTE *)&pBuf);

			// If the call is successful, print the workstation data.
			if (nStatus == NERR_Success)
			{
				printf("Platform: %d\n", pBuf->wki102_platform_id);
				wprintf(L"Name:     %s\n", pBuf->wki102_computername); //unicode string

				USES_CONVERSION;
				info.PanelName = W2T(pBuf->wki102_computername);
				//printf("Version:  %d.%d\n", pBuf->wki102_ver_major, pBuf->wki102_ver_minor);
				//wprintf(L"Domain/WorkSatation  %s\n", pBuf->wki102_langroup);
				//if( info.MoreNames[ComputerNameDnsDomain].empty() )
					info.WorkStationName = W2T(pBuf->wki102_langroup); //unicode string
				//else
				//	info.DomainName = W2T(pBuf->wki102_langroup); // unicode string
				//wprintf(L"Lan Root: %s\n", pBuf->wki102_lanroot);
				//wprintf(L"# Logged On Users: %d\n", pBuf->wki102_logged_on_users);
			}
			else
				_ftprintf(stderr, _T("ERROR: A system error has occurred: Net API Status %d, GetLastError %d \n"), nStatus, GetLastError() );

			// Free the allocated memory.
			if (pBuf != NULL)
				NetApiBufferFree(pBuf);
		}

		return TRUE;
	}


	//
	// Adds a panel to a specified domain, but does not joins
	//
	BOOL Raw(TCHAR cmd[])
	{
		return 
			CExecuteCommand::Execut(cmd);
	}

	BOOL RestoreHostName(tstring& tResponse)// This funciton require c:\\windows\\system32\\AutoHost.exe
	{
		BOOL bRetv = FALSE;
		TCHAR cmd[1024];
		_stprintf_s(cmd, TSIZEOF(cmd), _T("c:\\windows\\system32\\AutoHost.exe HDCS") );
		CExecuteCommand::Execut(cmd);

		bRetv = CheckForSuccess( _T("Now setting hostname to") );
		if(bRetv)
		{
			TCHAR tResponseBuf[MAX_RESPONSE_STR_LEN];
			DWORD ResponseBufLen = MAX_RESPONSE_STR_LEN;
			CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
			vector<tstring> vLines;
			vLines = split(tResponseBuf, _T('\n') );
			if(vLines.size() > 2)
			{
				vector<tstring> vWords;
				vWords = split(vLines.at(0).c_str(), _T(' ') );
				if(vWords.size()==5)
				{
					tResponse = vWords.at(4);
					BaseLogger::WriteFormattedLog(_T("OK: Hostname is %s"), tResponse.c_str() );
				}
				else
				{
					bRetv = FALSE;
				}
			}
		}

		if(bRetv==FALSE)
		{
			tResponse = _T("HDCS_CCS"); // default hostname
			BaseLogger::WriteFormattedLog(_T("ERROR: Hostname could not be restored. Using default hostname %s"), tResponse.c_str() );
		}

		return bRetv;
	} 

 	tstring GetWorkgroupName()
	{
		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
		tstring tValue = _T("");

		try
		{
			DSROLE_PRIMARY_DOMAIN_INFO_BASIC * info;
			if(ERROR_SUCCESS == DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *)&info) )
			{
				if (info->DomainNameDns == NULL)
				{
					tValue = info->DomainNameFlat;
					BaseLogger::WriteFormattedLog(TRACESTACK, L"Workgroup Name: %s", info->DomainNameFlat);
				}
				DsRoleFreeMemory(info);
			}
			else
				BaseLogger::WriteFormattedLog(_T("ERROR: DsRoleGetPrimaryDomainInformation in %"), __WFUNCTION__);

		}

		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	
		}

		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

		return tValue;
	}

	tstring GetDomainName()
	{
		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
		tstring tValue = _T("");

		try
		{
			DSROLE_PRIMARY_DOMAIN_INFO_BASIC * info;
			if(ERROR_SUCCESS == DsRoleGetPrimaryDomainInformation(NULL, DsRolePrimaryDomainInfoBasic, (PBYTE *)&info) )
			{
				if (info->DomainNameDns != NULL)
				{
					tValue = info->DomainNameDns;
					BaseLogger::WriteFormattedLog(TRACESTACK, L"Domain Name: %s", info->DomainNameDns);
				}
				DsRoleFreeMemory(info);
			}
			else
				BaseLogger::WriteFormattedLog(_T("ERROR: DsRoleGetPrimaryDomainInformation in %"), __WFUNCTION__);

		}
		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	
		}

		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

		return tValue;
	}

	tstring GetPrimaryDnsSuffix()
	{
		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
		tstring tValue = _T("");

		try
		{
			//API GetPanelNameEx for all 8 names of the computer specifified in enum COMPUTER_NAME_FORMAT
			TCHAR tPanelName[KB(1)];
			DWORD dwSize = KB(1);

			TCHAR szDescription[8][32] = 
			{
				TEXT("NetBIOS"), 
				TEXT("DNS hostname"), 
				TEXT("DNS domain"),  // 2. Primary DNS Suffix Name
				TEXT("DNS fully-qualified"), 
				TEXT("Physical NetBIOS"), 
				TEXT("Physical DNS hostname"), 
				TEXT("Physical DNS domain"), 
				TEXT("Physical DNS fully-qualified")
			};

			int i = ComputerNameDnsDomain; // 2. Primary DNS Suffix Name
			if ( GetComputerNameEx((COMPUTER_NAME_FORMAT)i, tPanelName, &dwSize))
			{
				if(i== ComputerNameDnsDomain && _tcslen(tPanelName) )
					tValue = tPanelName;
			}	
		}
		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	
		}

		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

		return tValue;
	}

	BOOL SetPrimaryDnsSuffix(tstring suffix)
	{
		BOOL bRetv; 
		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);

		// see if wants to delete name
		if(_tcsicmp(suffix.c_str(), _T("delete")) == 0 )
			suffix = _T("");

		try
		{
			bRetv = SetComputerNameEx(ComputerNamePhysicalDnsDomain, suffix.c_str() );
		}
		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	
		}

		BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

		return bRetv;
	}

	bool RenameMachineInDomain(tstring tNewHostname )
	{
		bool bRetv = false;
		try
		{
			BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);		
			BaseLogger::WriteFormattedLog(TRACESTACK, _T("New name desired '%s' "), tNewHostname.c_str() );


			TCHAR tCurrentName[KB(1)];
			DWORD dwSize = TSIZEOF(tCurrentName);

			USES_CONVERSION;

			if(GetPanelName(tCurrentName, dwSize, ComputerNameNetBIOS) )
			{
				NET_API_STATUS status = NetRenameMachineInDomain(NULL, ( tNewHostname.c_str()),  NULL/*User*/, NULL/*Pwd*/, 0);
				if (status == NERR_Success)
				{
					BaseLogger::WriteFormattedLog(TRACESTACK, _T("Name changed from '%s' to '%s' "), tCurrentName, tNewHostname.c_str() );
					bRetv = true;
				}
				else
					BaseLogger::WriteFormattedLog(TRACESTACK, _T("Error: Name change failed: '%s' "), _com_error(status).ErrorMessage() );
			}
			else
				BaseLogger::WriteFormattedLog(TRACESTACK, _T("ERROR: Failed to get machine's current host name ") );

			BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);
		}
		catch(...)
		{
			BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	
		}

		return bRetv;
	}


};