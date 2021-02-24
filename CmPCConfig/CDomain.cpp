#include "stdafx.h"
#include "CDomain.h"
#include "CDomain.hpp"
#include "Privileges.h"
#include "BaseLogger.h"
#include "comdef.h"
//GOALS:
// 1. Remove a panel from Workgroup and join it to a Domain
// 2. Remove a panel from a Domain and join it to a Workgroup
// 3. Rename panel when in Domain
// 4. Rename panel when in Workgroup
// 5. Rename Workgroup
// 6. Get a panel name


//---------------------------------------------------------------------------
// USAGE: bRetv = CfgJoinPanelToDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgJoinPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgJoinPanelToDomain"));

	CDomain d;
	d.JoinPanelToDomain(domainName, userD, passwordD);

	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRemovePanelFromDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRemovePanelFromDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgRemovePanelFromDomain"));


	CDomain d;
	d.RemovePanelFromDomain(domainName, userD, passwordD);
	
	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenamePanelInDomain("panelNewName", "Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenamePanelInDomain(TCHAR panelNewName[], TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgRenamePanelInDomain"));

	CDomain d;
	d.RenamePanelInDomain(panelNewName, domainName, userD, passwordD);
	
	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenamePanelInWorkgroup("panelNewName");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenamePanelInWorkgroup(TCHAR panelNewName[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgRenamePanelInWorkgroup"));

	CDomain d;
	d.RenamePanelInWorkgroup(panelNewName);
	
	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenameWorkgroup("WorkGroupNewName");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenameWorkgroup(TCHAR WorkGroupNewName[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgRenameWorkgroup"));

	CDomain d;
	d.RenameWorkgroup(WorkGroupNewName);
	
	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgGetResponseString(tResponseBuf, ResponseBufLen/*in&out*/);
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgGetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgGetResponseString"));

	CDomain d;
	d.GetResponseString(tResponseBuf, ResponseBufLen);
	
	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgJoinPanelToDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgAddPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgAddPanelToDomain"));

	CDomain d;
	d.AddPanelToDomain(domainName, userD, passwordD);
	
	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgQueryPanelsInDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgQueryPanelsInDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgQueryPanelsInDomain"));

	CDomain d;
	d.QueryPanelsInDomain(domainName, userD, passwordD);

	d.GetResponseString(tResponse);

	return d.CheckForSuccess( _T("success") );
}

//---------------------------------------------------------------------------
// USAGE: CDeviceNamesInfo info; CfgGetDeviceNamesInfo_Obsolete(info);
CmPCConfig_DeclSpec BOOL CfgGetDeviceNamesInfo_Obsolete(CDeviceNamesInfo &info, tstring &tResponse)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CfgGetDeviceNamesInfo"));

	CDomain d;
	BOOL bRetv = FALSE;

	bRetv = d.GetDeviceNamesInfo(info);

	d.GetResponseString(tResponse);

	return bRetv;
		
}

//
// HRESULT CfgNetJoinDomain()
// A sample call:
//    TCHAR Domain[] = _T("CRESTRON.CRESTRON.COM");
//    TCHAR AccountOU[] = NULL or _T("OU=testOU,DC=domain,DC=Domain,DC=com")
//    TCHAR Account[] = _T("CRESTRON\sengdev");
//    TCHAR Password[] = _T("Crestron123");
//    DWORD joinOption = NETSETUP_JOIN_DOMAIN|NETSETUP_DOMAIN_JOIN_IF_JOINED|NETSETUP_ACCT_CREATE
// 	  NET_API_STATUS retv = CfgNetJoinDomain(Domain, AccountOU, Account, Password, joinOption);
//
CmPCConfig_DeclSpec HRESULT CfgNetJoinDomain(_TCHAR Domain[], _TCHAR AccountOU[], _TCHAR Account[], _TCHAR Password[],  DWORD joinOption, tstring& tResponse)	
{	
	tResponse.clear();
	HRESULT hRes = S_OK;

	CPrivileges priv;
	if (priv.EnablePrivilege( SE_MACHINE_ACCOUNT_NAME, TRUE))
	{
		if(_tcsicmp(AccountOU, _T("NULL"))  == 0)
			AccountOU = NULL;

		USES_CONVERSION;
		NET_API_STATUS ret = NetJoinDomain(NULL,
			T2W(Domain),
			AccountOU,
			T2W(Account),
			T2W(Password),
			joinOption);
		if (NERR_Success !=	ret)
		{
			TCHAR szMsg[BUFSIZ] = {0};

			DWORD err = GetLastError();
			_stprintf_s(szMsg, BUFSIZ, _T("\nNetJoinDomain failed. ret (%d), Last Err(%d), Error Msg %s \n"), ret, err, _com_error(err).ErrorMessage() );
			tResponse = szMsg;
			BaseLogger::WriteLog(szMsg);
			hRes = ret;
		}

	} 

	return hRes;
}
CmPCConfig_DeclSpec HRESULT CfgNetUnJoinDomain(_TCHAR Account[], _TCHAR Password[],  DWORD joinOption, tstring& tResponse)	
{	
	HRESULT hRes = S_OK;

	CPrivileges priv;
	if (priv.EnablePrivilege( SE_MACHINE_ACCOUNT_NAME, TRUE))
	{
		USES_CONVERSION;
		NET_API_STATUS ret = NetUnjoinDomain(NULL,
			T2W(Account),
			T2W(Password),
			joinOption);
		if (NERR_Success !=	ret)
		{
			TCHAR szMsg[BUFSIZ] = {0};

			DWORD err = GetLastError();
			_stprintf_s(szMsg, BUFSIZ, _T("\nNetUnJoinDomain failed. ret (%d), Last Err(%d)\n"), ret, err);
			tResponse = szMsg;
			BaseLogger::WriteLog(szMsg);
			hRes = ret;
		}

	} 

	return hRes;
}


CmPCConfig_DeclSpec HRESULT CfgRestoreHostName(tstring& tResponse)
{
	HRESULT hRes = S_OK;

	CDomain d;
	hRes = d.RestoreHostName( tResponse );

	return hRes;
}

CmPCConfig_DeclSpec tstring CfgGetWorkgroupName()
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);	
	tstring tValue = _T("");

	try
	{
		CDomain dm;
		tValue = dm.GetWorkgroupName( );
	}
	catch(...)
	{
		BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	

	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

	return tValue;
}

CmPCConfig_DeclSpec tstring CfgGetDomainName()
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);	
	tstring tValue = _T("");

	try
	{
		CDomain dm;
		tValue = dm.GetDomainName( );
	}
	catch(...)
	{
		BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	

	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

	return tValue;
}

CmPCConfig_DeclSpec tstring CfgGetPrimaryDnsSuffix()
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);	
	tstring tValue = _T("");

	try
	{
		CDomain dm;
		tValue = dm.GetPrimaryDnsSuffix( );
	}
	catch(...)
	{
		BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	

	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

	return tValue;
}

CmPCConfig_DeclSpec BOOL CfgSetPrimaryDnsSuffix(tstring suffix, tstring& tResponse )
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);	
	BOOL bValue = FALSE;
	DNS_STATUS dnsStatus;

	try
	{
		dnsStatus = DnsValidateName(suffix.c_str(), DnsNameDomain);
		if(dnsStatus == ERROR_SUCCESS)
		{
			tResponse.clear();
			CDomain dm;
			bValue = dm.SetPrimaryDnsSuffix( suffix );
		}
	}
	catch(...)
	{
		BaseLogger::WriteFormattedLog(_T("ERROR: catch %s OUT"), __WFUNCTION__);	

	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

	return bValue;
}


CmPCConfig_DeclSpec BOOL CfgChangeHostname(TCHAR newHostName[], tstring& tResponse)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
	BOOL bRetv;
	tstring strWorkgrup;
	CDomain d;

	strWorkgrup = CfgGetWorkgroupName();

	if(strWorkgrup.length() > 0)
	{
		bRetv = CfgRenamePanelInWorkgroup(newHostName, tResponse);		
	}
	else
	{
		bRetv = d.RenameMachineInDomain(newHostName);
	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);	

	return bRetv;
}			  

CmPCConfig_DeclSpec BOOL CfgUpdateManifest()
{
	return CfgUpdateManifest(NULL);
}

CmPCConfig_DeclSpec BOOL CfgUpdateManifest(TCHAR* szOverride)
{
	static tstring strPreviousHostname;

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	
	// get current hostname
	TCHAR currentHostName[MAX_PATH];
	DWORD dwBufSize = MAX_PATH;
	HRESULT hr = GetComputerName(currentHostName, &dwBufSize) ? S_OK : E_FAIL;
	if (hr != S_OK)
		return FALSE;

	// Use address override instead
	if (NULL != szOverride)
	{
		_tcscpy(currentHostName, szOverride);
	}

	// Don't call the app if the last thing we called it with is the same value
	if (strPreviousHostname == currentHostName)
		return FALSE;
	else
		strPreviousHostname = currentHostName;
	
	
	
	TCHAR cmd[MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(L"CRESTRON_FIRMWARE", cmd, MAX_PATH) != 0)
	{
		// Updated manifest file only if device name is changed successfully 
		CDomain d;
		_tcscat_s(cmd, MAX_PATH, L"\\CCSEngineManifestUpdater.exe -h ");
		_tcscat_s(cmd, MAX_PATH, currentHostName);
		_tcscat_s(cmd, MAX_PATH, L" -f C:\\inetpub\\wwwroot");
		
		if (d.StartAppAsUser(cmd, false) == TRUE)
		{
			// save hostname
			//WritePrivateProfileString(_T("upxpanel"), _T("HostName"), currentHostName, inifile);
		}
	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);

	return TRUE;
}

BOOL WritetoIni(int DeviceNbr, TCHAR name[], TCHAR fw[], TCHAR fpga[])
{
	BOOL bRetv = FALSE;
	// read from ini 
	TCHAR inifile[MAX_PATH] = { 0 };

	if (GetEnvironmentVariable(L"CRESTRON_USER", inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"hwSettings\\hw.dat");

		{
			std::wstring tSectiton = L"USB3_DEVICES";
			std::wstring totat = std::to_wstring(DeviceNbr);
			::WritePrivateProfileString(tSectiton.c_str(), _T("TOTAL"), totat.c_str(), inifile);
		}


		std::wstring tSectiton = L"USB3_VERSION_CHECKER_" + std::to_wstring(DeviceNbr);
		::WritePrivateProfileString(tSectiton.c_str(), _T("NAME"), name, inifile);
		::WritePrivateProfileString(tSectiton.c_str(), _T("FW"), fw, inifile);
		::WritePrivateProfileString(tSectiton.c_str(), _T("FPGA"), fpga, inifile);
		bRetv = TRUE;
	}

	return bRetv;

	return bRetv;
}


CmPCConfig_DeclSpec BOOL CfgGetCaptureDevieInfo(BOOL bWriteIni, tstring& tResponse)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	BOOL bReturnValue = FALSE;

	tResponse.clear();

	if (bWriteIni)// run app that will write into ini
	{
		TCHAR cmd[MAX_PATH] = { 0 };
		if (GetEnvironmentVariable(L"CRESTRON_FIRMWARE", cmd, MAX_PATH) != 0)
		{
			// Updated manifest file only if device name is changed successfully 
			CDomain d;
			_tcscat_s(cmd, MAX_PATH, L"\\USB3_Version_Checker.exe");
			if (d.StartAppAsUserAndWait(cmd, false, 5000) == TRUE)
				BaseLogger::WriteFormattedLog(_T("SUCCESS: The %s success to run app"), __WFUNCTION__);
			else
				BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to run app"), __WFUNCTION__);
		}
	}


	// read from ini 
	TCHAR inifile[MAX_PATH] = { 0 };
	TCHAR Value[MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(L"CRESTRON_USER", inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\hwSettings\\hw.dat");
		//::WritePrivateProfileString(_T("Section"), _T("Key"), Value, inifile);

		int total = GetPrivateProfileInt(L"USB3_DEVICES", L"TOTAL", 0, inifile);
		for(int d = 0; d < total; d++)
		{
			tstring tSection  = _T("USB3_VERSION_CHECKER_");
			tSection += std::to_wstring(d+1);
			if (::GetPrivateProfileString(tSection.c_str(), _T("NAME"), NULL, Value, MAX_PATH, inifile) > 0)
			{
				if(d>0)
					tResponse += L"\r\n"; // empty line between device list

				tResponse += L"Name:\t\t";
				tResponse += Value;
				tResponse += L"\r\n";
				bReturnValue = TRUE;
			}
			if (::GetPrivateProfileString(tSection.c_str(), _T("FW"), NULL, Value, MAX_PATH, inifile) > 0)
			{
				tResponse += L"FW:\t\t";
				tResponse += Value;
				tResponse += L"\r\n";
				bReturnValue = TRUE;
			}
			if (::GetPrivateProfileString(tSection.c_str(), _T("FPGA"), NULL, Value, MAX_PATH, inifile) > 0)
			{
				tResponse += L"FPGA:\t\t";
				tResponse += Value;
				tResponse += L"\r\n";
				bReturnValue = TRUE;
			}
		}

		if (total==0)
		{
			tResponse += L"No Capture Device\r\n";
			bReturnValue = TRUE;
		}
	}
	else
		BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to read hw.dat ini"), __WFUNCTION__);

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);

	return bReturnValue;

}


CmPCConfig_DeclSpec BOOL CfgGetSoundBar(BOOL bWriteIni, tstring& tResponse,bool g)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	BOOL bReturnValue = FALSE;

	tResponse.clear();

	if (bWriteIni)// run app that will write into ini
	{
		TCHAR cmd[MAX_PATH] = { 0 };
		if (GetEnvironmentVariable(L"CRESTRON_FIRMWARE", cmd, MAX_PATH) != 0)
		{
			// Updated manifest file only if device name is changed successfully 
			CDomain d;
			_tcscat_s(cmd, MAX_PATH, L"\\SBUpgrader.exe -v");
			if (d.StartAppAsUserAndWait(cmd, false, 5000) == TRUE)
				BaseLogger::WriteFormattedLog(_T("SUCCESS: The %s success to run app"), __WFUNCTION__);
			else
				BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to run app"), __WFUNCTION__);
		}
	}


	// read from ini 
	TCHAR inifile[MAX_PATH] = { 0 };
	TCHAR Value[MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(L"CRESTRON_USER", inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\hwSettings\\hw.dat");

		int f = ::GetPrivateProfileString(_T("SB"), _T("Model"), NULL, Value, MAX_PATH, inifile);
		if (f == 0)
			tResponse += _T("Not Connected");
		else
		{
	
			tResponse += L"Name:\t\t";
			tResponse += Value;
			tResponse += L"\r\n";
			f = ::GetPrivateProfileString(_T("SB"), _T("Version"), NULL, Value, MAX_PATH, inifile);
			if (g == true)
				tResponse += L"Version:\t\t";
			else
				tResponse += L"Version:\t";
			tResponse += Value;
		}
		tResponse += L"\r\n";
		bReturnValue = TRUE;
	}
	else
		BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to read hw.dat ini"), __WFUNCTION__);

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);

	return bReturnValue;

}
CmPCConfig_DeclSpec BOOL CfgGetHuddlyIQ(BOOL bWriteIni, tstring& tResponse, bool g)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	BOOL bReturnValue = FALSE;

	tResponse.clear();

	/* No need to run anything, huddly info is filled upon startup or when ever camera is attached 
	if (bWriteIni)// run app that will write into ini
	{
		TCHAR cmd[MAX_PATH] = { 0 };
		if (GetEnvironmentVariable(L"CRESTRON_FIRMWARE", cmd, MAX_PATH) != 0)
		{
			// Updated manifest file only if device name is changed successfully 
			CDomain d;
			_tcscat_s(cmd, MAX_PATH, L"\\Huddly\\HuddlyIQ.exe Info");
			if (d.StartAppAsUserAndWait(cmd, false, 5000) == TRUE)
				BaseLogger::WriteFormattedLog(_T("SUCCESS: The %s success to run app"), __WFUNCTION__);
			else
				BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to run app"), __WFUNCTION__);
		}
	}*/


	// read from ini 
	TCHAR inifile[MAX_PATH] = { 0 };
	TCHAR Value[MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(L"CRESTRON_USER", inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\hwSettings\\HuddlyIQV2.dat");

		int f = ::GetPrivateProfileString(_T("HuddlyIQ"), _T("Huddly_Camera_Version"), NULL, Value, MAX_PATH, inifile);
		if (f == 0)
			tResponse += _T("Not Connected");
		else
		{
			if (_tcsicmp(Value, L"Not Connected") != 0) // no match
			{
				if (g == true)
					tResponse += L"Version:\t\t";
				else
					tResponse += L"Version:\t";
			}
			tResponse += Value;
		}
		tResponse += L"\r\n";
		bReturnValue = TRUE;
	}
	else
		BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to read HuddlyIQ.dat ini"), __WFUNCTION__);

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);

	return bReturnValue;

}


CmPCConfig_DeclSpec BOOL CfgIsSkypeMode()
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);

	TCHAR init[MAX_PATH] = { 0 };
	TCHAR tValue[MAX_PATH] = {};
	if (GetEnvironmentVariable(L"CRESTRON_SETTINGS", init, MAX_PATH) != 0)
	{
		_tcscat_s(init, MAX_PATH, L"\\Camera Huddly IQ.ini");
		DWORD iRetv = GetPrivateProfileString(L"LoggedUser", L"Name", _T(""), tValue, MAX_PATH, init);
		BaseLogger::WriteFormattedLog(L"LoggedUser: %s", tValue);
		if ( _tcsicmp(tValue, L"Skype") ==0 )
		{
			
			return TRUE;
		}
	}
	
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);
	return FALSE;
}

CmPCConfig_DeclSpec BOOL CfgHandlHuddlyIQ(HUDDLY action, tstring& tResponse)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	BOOL bReturnValue = FALSE;
	tResponse.clear();

	TCHAR cmd[MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(L"CRESTRON_FIRMWARE", cmd, MAX_PATH) != 0)
	{
		CDomain d;
		TCHAR buf[MAX_PATH] = {};

		if (action == HUDDLY_STOP)
			_stprintf(buf, L"\\Huddly\\HuddlyIQKill.exe");
		else
		{
			/*TCHAR logFile[MAX_PATH] = L".\\HuddlyApp.log";
			if (GetEnvironmentVariable(L"CRESTRON_USER", logFile, MAX_PATH) != 0)
				_tcscat_s(logFile, MAX_PATH, L"\\Logs\\HuddlyApp.log");

			_stprintf(buf, L"\\Huddly\\HuddlyIQ.exe people_count5 > '%s' ", logFile);
			_stprintf(buf, L"\\Huddly\\HuddlyIQ.exe people_count5");*/

			// Make backup of log file
			tstring logPath = GetFullPath_(L"CRESTRON_LOGS", L"HuddlyIQApp.log");
			tstring logBkPath = GetFullPath_(L"CRESTRON_LOGS", L"HuddlyIQAppBk.log");
			struct _stat filestatus;
			_tstat(logPath.c_str(), &filestatus);
			if (filestatus.st_size > 100 * 1024)
			{
				_tremove(logBkPath.c_str());
				_trename(logPath.c_str(), logBkPath.c_str());
				BaseLogger::WriteFormattedLog(L"The HuddlyIQApp.log file is backedup to HuddlyIQApp.log");
			}

			_stprintf(buf, L"\\Huddly\\HuddlyIQ.bat");
		}

		_tcscat_s(cmd, MAX_PATH, buf);

		BaseLogger::WriteFormattedLog(TRACESTACK, L"cmd %s , bReturnValue %d,  %s", cmd, bReturnValue, __WFUNCTION__);

		bReturnValue = d.StartAppAsUserAndWait(cmd, false, 1000);

		if (bReturnValue == TRUE)
			BaseLogger::WriteFormattedLog(TRACESTACK, _T("SUCCESS: The %s success to run app"), __WFUNCTION__);
		else
			BaseLogger::WriteFormattedLog(_T("ERROR: The %s failed to run app, %s"), __WFUNCTION__, tResponse.c_str());
	}
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);
	return bReturnValue;
}

CmPCConfig_DeclSpec bool CfgShouldRunHuddlyIQ()
{
	CDomain d;
	return
		d.ShouldRunHuddlyIQ();
}

CmPCConfig_DeclSpec tstring GetFullPath(TCHAR varName[], TCHAR fileName[])
{
	TCHAR inifile[MAX_PATH] = L".\\";
	TCHAR Value[MAX_PATH] = {};
	if (GetEnvironmentVariable(varName, inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\");
		_tcscat_s(inifile, MAX_PATH, fileName);
	}

	return inifile;
}

CmPCConfig_DeclSpec tstring GetFullPath_(TCHAR varName[], TCHAR fileName[])
{
	TCHAR inifile[MAX_PATH] = L".\\";
	TCHAR Value[MAX_PATH] = {};
	/*CString str;
	if (GetEnvironmentVariable(varName, inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\");
		_tcscat_s(inifile, MAX_PATH, fileName);
		str = inifile;
		str.Replace(L"Program Files", L"Progra~1");
	}
	return str.GetBuffer();
	*/
	tstring str;
	if (GetEnvironmentVariable(varName, inifile, MAX_PATH) != 0)
	{
		_tcscat_s(inifile, MAX_PATH, L"\\");
		_tcscat_s(inifile, MAX_PATH, fileName);
		str = inifile;
		str.replace(str.find(L"Program Files"), TSIZEOF(L"Program Files")-1, L"Progra~1");
	}

	return str;
}
