#include "stdafx.h"
#include "LyncConfig.h"
#include "CExecuteCommand.hpp"
#include "BaseLogger.h"
#include "System.h"











//-----------------------------------------------------------------------------
// Name: DXUtil_ReadStringRegKeyCch()
// Desc: Helper function to read a registry key string
//       cchDest is the size in TCHARs of strDest.  Be careful not to 
//       pass in sizeof(strDest) on UNICODE builds.
//-----------------------------------------------------------------------------
HRESULT ReadStringRegKeyCch( HKEY hKey, TCHAR* strRegName, TCHAR* strDest, 
                                    DWORD cchDest, TCHAR* strDefault )
{
    DWORD dwType;
    DWORD cbDest = cchDest * sizeof(TCHAR);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)strDest, &cbDest ) )
    {
        _tcsncpy( strDest, strDefault, cchDest );
        strDest[cchDest-1] = 0;

        if( dwType != REG_SZ )
            return E_FAIL;

        return S_OK;
    }

    return E_FAIL;
}


//-----------------------------------------------------------------------------
// Name: DXUtil_WriteStringRegKey()
// Desc: Helper function to write a registry key string
//-----------------------------------------------------------------------------
HRESULT WriteStringRegKey( HKEY hKey, TCHAR* strRegName,
                                  TCHAR* strValue )
{
    if( NULL == strValue )
        return E_INVALIDARG;
        
    DWORD cbValue = ((DWORD)_tcslen(strValue)+1) * sizeof(TCHAR);

    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_SZ, 
                                        (BYTE*)strValue, cbValue ) )
        return E_FAIL;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_ReadIntRegKey()
// Desc: Helper function to read a registry key int
//-----------------------------------------------------------------------------
HRESULT ReadIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD* pdwDest, 
                              DWORD dwDefault )
{
    DWORD dwType;
    DWORD dwLength = sizeof(DWORD);

    if( ERROR_SUCCESS != RegQueryValueEx( hKey, strRegName, 0, &dwType, 
                                          (BYTE*)pdwDest, &dwLength ) )
    {
        *pdwDest = dwDefault;
        if( dwType != REG_DWORD )
            return E_FAIL;

        return S_OK;
    }

    return E_FAIL;
}




//-----------------------------------------------------------------------------
// Name: DXUtil_WriteIntRegKey()
// Desc: Helper function to write a registry key int
//-----------------------------------------------------------------------------
HRESULT WriteIntRegKey( HKEY hKey, TCHAR* strRegName, DWORD dwValue )
{
    if( ERROR_SUCCESS != RegSetValueEx( hKey, strRegName, 0, REG_DWORD, 
                                        (BYTE*)&dwValue, sizeof(DWORD) ) )
        return E_FAIL;

    return S_OK;
}




void CfgLyncSwapLargeDisplays()
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	DWORD dwContentOnLeft = 0;

	// Check the left/right content key first
	LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\LyncRoom\\ScreenManager"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey );
    if( ERROR_SUCCESS == lResult )
	{
		ReadIntRegKey(hKey, _T("DisplayContentOnFrontLeftScreen"), &dwContentOnLeft, 0);
		::RegCloseKey(hKey);
	}

	// Swap the values
	if (dwContentOnLeft == 1)
		dwContentOnLeft = 0;
	else
		dwContentOnLeft = 1;

	// Write out the new value
	lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\LyncRoom\\ScreenManager"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		WriteIntRegKey(hKey, _T("DisplayContentOnFrontLeftScreen"), dwContentOnLeft);
		::RegCloseKey(hKey);
	}
}




void CfgLyncSetDisplaySettings(int nTotal, int nControl, int nLeft, int nRight, TCHAR* tszControl, TCHAR* tszLeft, TCHAR* tszRight)
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	DWORD dwContentOnLeft = 0;

	// Check the left/right content key first
	LONG lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\LyncRoom\\ScreenManager"), 0, KEY_READ | KEY_WOW64_64KEY, &hKey );
    if( ERROR_SUCCESS == lResult )
	{
		ReadIntRegKey(hKey, _T("DisplayContentOnFrontLeftScreen"), &dwContentOnLeft, 0);
	}

	if (nTotal == 1)
	{
		// The LRS software doesn't like to have only one monitor, so tell it that there are 2
		//nTotal = 2;
		//tszLeft = tszControl;
		//tszRight = tszControl;
	}


	// Now write our values
	lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\LyncRoom\\ScreenManager"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		BaseLogger::WriteFormattedLog(_T("Writing display parameters to Registry -- Count[%d] Control[%d:%s] Left[%d:%s] Right[%d:%s] ContentOnLeft[%d]"), nTotal, nControl, tszControl,
																																				nLeft, tszLeft, nRight, tszRight,
																																				dwContentOnLeft);


		WriteIntRegKey(hKey, _T("ConsoleMonitorId"), nControl);
		WriteIntRegKey(hKey, _T("FrontLeftScreenId"), nLeft);
		WriteIntRegKey(hKey, _T("FrontRightScreenId"), nRight);
		WriteIntRegKey(hKey, _T("DisplayContentOnFrontLeftScreen"), dwContentOnLeft);

		WriteStringRegKey(hKey, _T("ConsoleScreenDevice"), tszControl);
		WriteStringRegKey(hKey, _T("FrontLeftScreenDevice"), tszLeft);
		WriteStringRegKey(hKey, _T("FrontRightScreenDevice"), tszRight);
		
		WriteIntRegKey(hKey, _T("TotalMonitors"), nTotal);	

		RegFlushKey(hKey);
		::RegCloseKey(hKey);
	}




}








void CfgLyncFixUserSettings()
{
	
	HKEY hKey;
	DWORD dwDisposition = 0;
	
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		WriteIntRegKey(hKey, _T("AutoAdminLogon"), 1);
		WriteStringRegKey(hKey, _T("DefaultUsername"), _T("LyncRoom"));
		WriteStringRegKey(hKey, _T("DefaultPassword"), _T("tpmcuser"));		
		::RegCloseKey(hKey);
	}


}





void CfgLyncInit()
{
	CfgLyncShowUI(false);
	//CfgSetLyncVideoCropping();




}



//
// MeetingRoomApplianceMode 1 means to hide the Lync UI
// 

void CfgLyncShowUI(bool bShow)
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Office\\15.0\\Lync"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if (lResult == ERROR_SUCCESS)
	{
		WriteIntRegKey(hKey, _T("MeetingRoomApplianceMode"), (bShow==true)?0:1);

		::RegCloseKey(hKey);
	}
}





void CfgSetLyncVideoCropping()
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	TCHAR tszName[_MAX_PATH];
	DWORD dwIndex = 0;
	DWORD dwNameLen = _MAX_PATH;
	
	LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey );
		
	
	LONG lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);

	while (ERROR_SUCCESS == lResultEnum)
	{
		// Exclude short keys
		if (dwNameLen > 9)
		{
			// Exclude "Classes" keys
			if (NULL == (_tcsstr(tszName, _T("_Classes"))))
			{
				TCHAR tszKey[_MAX_PATH];
				_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Software\\Policies\\Microsoft\\Office\\15.0\\Lync"), tszName);

				HKEY hKeyUser;
				LONG lResult = ::RegCreateKeyEx(hKey, tszKey, 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKeyUser, &dwDisposition);
				if (lResult == ERROR_SUCCESS)
				{
					WriteIntRegKey(hKeyUser, _T("MediaPlatformSettings"), 241);		
					::RegCloseKey(hKeyUser);
				}
			}
		}
		
		dwNameLen = _MAX_PATH;
		++dwIndex;
		lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
	}

	::RegCloseKey(hKey);

}








CmPCConfig_DeclSpec bool CfgLyncImportCert(const TCHAR* tszFile, BOOL bRootCA, tstring &tstrResult)
{
	bool bResult = false;
	TCHAR* tszCommandLine = new TCHAR[2048];		// on the heap for unicode

	_stprintf_s(tszCommandLine, 1024, _T("certutil -addstore %s \"%s\""), bRootCA?_T("root"):_T("ca"), tszFile);


	bool bStatus = false;
	tstring tstrResults;
	CExecuteCommand ex;
	
	// Call Masoud's process code and save the results
	int nRes = ex.Execut(tszCommandLine, tstrResults, bStatus);
	
	// Get the actual exit code of the process.  It can "succeed", but
	// still fail to import the cert.
	DWORD dwExitCode = 42;
	if (nRes)
		dwExitCode = ex.CheckExitCode();

	tstrResult = tstrResults;

	if (0 == dwExitCode)
		return true;


	BaseLogger::WriteFormattedLog(_T("Import Certificate Failed for file: %s\n%s"), tszFile, tstrResult.c_str());


	return false;

}

	

CmPCConfig_DeclSpec bool CfgLyncImportPersonalCert(const TCHAR* tszFile, const TCHAR* tszPassword, tstring &tstrResult)
{
	bool bResult = false;
	TCHAR* tszCommandLine = new TCHAR[2048];		// on the heap for unicode

	_stprintf_s(tszCommandLine, 1024, _T("certutil -f -p %s -importpfx \"%s\" NoRoot"), tszPassword, tszFile);


	bool bStatus = false;
	tstring tstrResults;
	CExecuteCommand ex;
	
	// Call Masoud's process code and save the results
	int nRes = ex.Execut(tszCommandLine, tstrResults, bStatus);
	
	// Get the actual exit code of the process.  It can "succeed", but
	// still fail to import the cert.
	DWORD dwExitCode = 42;
	if (nRes)
		dwExitCode = ex.CheckExitCode();

	tstrResult = tstrResults;

	if (0 == dwExitCode)
		return true;


	BaseLogger::WriteFormattedLog(_T("Import Certificate Failed for file: %s\n%s"), tszFile, tstrResult.c_str());


	return false;

}

	


CmPCConfig_DeclSpec void CfgLyncSetEWSCredentials(const TCHAR* tszUsername, const TCHAR* tszAddress, const TCHAR* tszPassword)
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	
	// Set development settings for exchange in registry
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\LyncRoom\\DevelopmentSettings"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if( ERROR_SUCCESS == lResult )
	{
		WriteIntRegKey(hKey, _T("OverrideExchangeConfiguration"), 1);
		WriteStringRegKey(hKey, _T("ExchangeUsernameOverride"), (TCHAR*)tszUsername);
		WriteStringRegKey(hKey, _T("ExchangeSmtpAddressOverride"), (TCHAR*)tszAddress);
		WriteStringRegKey(hKey, _T("ExchangePasswordOverride"), (TCHAR*)tszPassword);

		::RegCloseKey(hKey);
	}
}


CmPCConfig_DeclSpec void CfgLyncSetEWSServers(const TCHAR* tszInternal, const TCHAR* tszExternal)
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	
	// Set cache settings for exchange servers in registry
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\LyncRoom\\Ews"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if( ERROR_SUCCESS == lResult )
	{		
		WriteStringRegKey(hKey, _T("InternalEwsUrl"), (TCHAR*)tszInternal);
		WriteStringRegKey(hKey, _T("ExternalEwsUrl"), (TCHAR*)tszExternal);
		
		WriteIntRegKey(hKey, _T("cacheVersion"), 2);
		WriteIntRegKey(hKey, _T("InternalTimeToLive"), 50000);
		WriteIntRegKey(hKey, _T("InternalServerVersion"), 1937801698);
		WriteIntRegKey(hKey, _T("UseDnsAutodiscovery"), 1);


		BYTE bTimestamp[8] = {0x41, 0x3a, 0x65, 0x03, 0x63, 0xf0, 0xcd, 0x01};
		RegSetValueEx( hKey, _T("TimeStamp"), 0, REG_BINARY, (BYTE*)bTimestamp, 8 );

		::RegCloseKey(hKey);
	}
}





CmPCConfig_DeclSpec void CfgLyncSetSKUData(const TCHAR* tszKey, const TCHAR* tszValue)
{
	HKEY hKey;
	DWORD dwDisposition = 0;
	
	// Set development settings for exchange in registry
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\LyncRoom\\SKU"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if( ERROR_SUCCESS == lResult )
	{
		// Check for one special case key that goes in as a DWORD.  All others are strings
		if (0 == _tcscmp(tszKey, _T("NumberofDisplays")))
		{			
			WriteIntRegKey(hKey, (TCHAR*)tszKey, (DWORD)_ttoi(tszValue));
		}
		else
			WriteStringRegKey(hKey, (TCHAR*)tszKey, (TCHAR*)tszValue);

		::RegCloseKey(hKey);
	}
}



CmPCConfig_DeclSpec void CfgLyncClearLyncLogs()
{
	//Clear stale tracing directories from the D:\LyncAppData drive location
	TCHAR defaultDir[200] = { 0 };
	TCHAR tmp[200] = { 0 };
	tstring strLocation;

	if (GetEnvironmentVariable(_T("CRESTRON_INTERNAL"), defaultDir, sizeof(defaultDir)) != 0)
		strLocation = defaultDir;
	else
		strLocation = _T("D:\\");

	strLocation += _T("LyncAppData\\");
	BaseLogger::WriteFormattedLog(_T("Clearing Lync App Tracing data from directory: %s"), strLocation.c_str());

	WIN32_FIND_DATA ffd;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError = 0;
	tstring strSearch = strLocation;
	strSearch += _T("*");

	hFind = FindFirstFile(strSearch.c_str(), &ffd);

	if (INVALID_HANDLE_VALUE != hFind)
	{
		do
		{
			if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				tstring strDirName = ffd.cFileName;
				if ((strDirName.substr(0, 7) == _T("Tracing")) && strDirName != _T("Tracing"))
				{
					strDirName = strLocation + ffd.cFileName;
					BaseLogger::WriteFormattedLog(_T("Deleting directory: %s"), strDirName.c_str());

					CfgClearFolder((wchar_t*)strDirName.c_str(), true);
					::RemoveDirectory(strDirName.c_str());
				}

			}

		} while (FindNextFile(hFind, &ffd) != 0);

		FindClose(hFind);
	}	
}



CmPCConfig_DeclSpec void CfgLyncSetLyncDataBackupMax(DWORD dwMax)
{
	HKEY hKey;
	DWORD dwDisposition = 0;

	// Set development settings for exchange in registry
	LONG lResult = ::RegCreateKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\LyncRoom\\Maintenance"), 0, NULL, 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, NULL, &hKey, &dwDisposition);
	if (ERROR_SUCCESS == lResult)
	{
		DWORD dwMaxRegVal = 11;
		ReadIntRegKey(hKey, _T("LogLyncAppDataBackupMaxCount"), &dwMaxRegVal, 10);

		if (dwMaxRegVal != dwMax)
		{
			BaseLogger::WriteFormattedLog(_T("LogLyncAppDataBackupMaxCount registry value (%d) is wrong.  Changing to %d."), dwMaxRegVal, dwMax);
			WriteIntRegKey(hKey, _T("LogLyncAppDataBackupMaxCount"), dwMax);
		}

		::RegCloseKey(hKey);
	}

}



CmPCConfig_DeclSpec bool GetSfBVersion(tstring& tsVersion)
{
	HKEY hKey;
	LONG Status;
	tsVersion = _T("Unknown");

	_TCHAR tsInfo[256] = { 0 };
	Status = RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Classes\\Local Settings\\Software\\Microsoft\\Windows\\CurrentVersion\\AppModel\\PackageRepository\\Packages"), NULL, KEY_ALL_ACCESS, &hKey);
	if (Status == ERROR_SUCCESS)
	{
		TCHAR tszKey[_MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwKeyLen = _MAX_PATH;
		LONG lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszKey, &dwKeyLen, NULL, NULL, NULL, NULL);

		while (ERROR_SUCCESS == lResultEnum)
		{
			tstring strKey = tszKey;
			
			if (-1 != strKey.find(_T("SkypeRoomSystem")))
			{
				int nStart = (int)strKey.find(_T("_"));
				int nEnd = (int)strKey.find(_T("_"), nStart + 1);
				
				if (nEnd > nStart)
					tsVersion = strKey.substr(nStart+1, nEnd - nStart-1);

				return true;
			}

			dwKeyLen = _MAX_PATH;
			++dwIndex;
			lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszKey, &dwKeyLen, NULL, NULL, NULL, NULL);
		}
	}

	return false;
}