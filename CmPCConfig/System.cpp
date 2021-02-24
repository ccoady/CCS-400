////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        System.cpp
///
/// Implementation file for System related functions.
///
/// @author      William Levine
/// @date        05/05/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <ShlGuid.h>
#include "stdio.h"
#include "CmPCConfig.h"
#include "Registry.h"
#include "System.h"
#include "shlwapi.h"

#include <shlobj.h>
#include <wininet.h>
#include <urlhist.h>

#ifdef  UNDER_CE // == TRUE
#include "UpxPanelDef.h"
#endif // UNDER_CE == TRUE

#ifndef UNDER_CE

#include "shellapi.h"
#pragma comment(lib,"shlwapi.lib")


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Runs the touch screen calibration program.
///
/// @author      William Levine
/// @date        08/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgRunTouchScreenCalibration(bool fConfig)
{
	HRESULT hSuccess = ERROR_SUCCESS_REBOOT_REQUIRED;
	HRESULT hRes = E_FAIL;
	TCHAR szFilename[MAX_PATH];
	CfgGetFirmwarePath(szFilename);
#if defined(_DGE2)
	if (fConfig)
	{
		//3rd paty calibration
		_tcscat(szFilename,_T("\\CtrnTouchConfig.exe"));
	}
	else
		//VPG calibration
		_tcscat(szFilename,_T("\\ctrnusbtouch.exe"));
#else
	_tcscat(szFilename,_T("\\ctrnusbtouch.exe"));
#endif
	
	if (PathFileExists(szFilename))
	{
#if defined(_DGE2)
	if (!fConfig)
	{
		_tcscat(szFilename, _T(" calib"));
	}
#else
		if (fConfig)
		{
			_tcscat(szFilename, _T(" config"));
		}
		else 
#if defined(_DGE1)
			_tcscat(szFilename, _T(" calib"));
#else
			_tcscat(szFilename, _T(" 0"));
#endif // _DGE1
#endif// _DGE2
		STARTUPINFO StartupInfo;
		PROCESS_INFORMATION ProcessInfo;

		::GetStartupInfo(&StartupInfo);
		if (::CreateProcess(NULL, szFilename, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
		{
			CloseHandle(ProcessInfo.hProcess);
			CloseHandle(ProcessInfo.hThread);
#ifdef _TPMC_V12
			hRes = hSuccess;
#else
			hRes = S_OK;
#endif 
		}
	}
	else
	{
		szFilename[0] = 0;
		CfgGetRuntimeImageSystemPath(szFilename);
		_tcscat(szFilename, _T("\\Program Files\\UPDD\\TBCalib.exe") );
		

		if (PathFileExists(szFilename))
		{
			STARTUPINFO StartupInfo;
			PROCESS_INFORMATION ProcessInfo;

			::GetStartupInfo(&StartupInfo);
			if (!::CreateProcess(NULL, szFilename, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
				hRes = E_FAIL;
			else
			{
				CloseHandle(ProcessInfo.hProcess);
				CloseHandle(ProcessInfo.hThread);
				hRes = hSuccess;
			}

		}
	}
	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Runs the touch screen calibration program.
///
/// @author      William Levine
/// @date        08/09/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgEnableTouchPktDebug(bool bEnable)
{
	HRESULT hSuccess = ERROR_SUCCESS_REBOOT_REQUIRED;
	HRESULT hRes = E_FAIL;
	TCHAR szFilename[MAX_PATH];
	CfgGetFirmwarePath(szFilename);

	_tcscat(szFilename,_T("\\ctrnusbtouch.exe"));
	
	if (PathFileExists(szFilename))
	{
		if (bEnable)
			_tcscat(szFilename, _T(" traceon"));
		else
			_tcscat(szFilename, _T(" traceoff"));

		STARTUPINFO StartupInfo;
		PROCESS_INFORMATION ProcessInfo;

		::GetStartupInfo(&StartupInfo);
		if (::CreateProcess(NULL, szFilename, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo))
		{
			CloseHandle(ProcessInfo.hProcess);
			CloseHandle(ProcessInfo.hThread);
		}
	}

	return S_OK;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Checks to see if a drive has been hidden by the NoViewOnDriver registry setting.
///
/// @param       tchDriveLetter The drive letter to be checked.
/// @return      S_OK if the drive is not hidden and is available.  S_FALSE if it is hidden and is
///              NOT available.  Will return E_ error codes if errors occur.
///
/// @author      William Levine
/// @date        07/22/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgGetDriveAccesability(_TCHAR tchDriveLetter)
{
HRESULT hRes = S_OK;
DWORD dwRes = 0;
_TCHAR tchDriveA[] = _T("a");

	if (isalpha(tchDriveLetter))
	{
		if (GetRegistryDWORD(HKEY_LOCAL_MACHINE, _T("Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer"), _T("NoViewOnDrive"), &dwRes) == S_OK)
		{
			_tolower(tchDriveLetter);
			if (dwRes & (0x00000001 << (tchDriveLetter - tchDriveA[0])) )
				hRes = S_FALSE;
		}
	}

	return hRes;
}

////////////////////////////////////////////////////////////////////////////////
BOOL StopWZCSvc()
{
	BOOL bErr = FALSE;
	_TCHAR tsCommLine[BUFSIZ];
	_stprintf_s(tsCommLine, BUFSIZ, _T("%s"), _T("net stop wzcsvc"));
	//_stprintf(tsCommLine, _T("%s"), _T("sc stop wzcsvc"));
	STARTUPINFO si;
	memset(&si,0,sizeof(si));
	si.cb = sizeof(STARTUPINFO);
	si.lpReserved = NULL;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW;

	PROCESS_INFORMATION pi;
	memset(&pi,0,sizeof(pi));

	if (CreateProcess(NULL,
					  tsCommLine,
					  NULL,				// process attribs
					  NULL,				// thread attribs
					  FALSE,			// inherit handles	
					  0,				// creation flags
					  NULL,				// environment block
					  NULL,				// current directory	
					  &si,				// startup info	
					  &pi))				// process info		
	{
		WaitForSingleObject(pi.hProcess, INFINITE);
	}
	else
	{				
		bErr = TRUE;		
	}

	// Close process and thread handles. 
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return bErr;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Uses the regcommit utility written by John Pavlik to store changes made to the registry in a
/// script file that will be used to  make the changes to the offline runtime image.
///
/// @param       pstPathToRegCommit 
/// @param       pstDataPathName
///
/// @author      William Levine
/// @date        05/06/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgRegistryCommitChanges(const _TCHAR *pstOutputScriptPathname)
{
#ifdef UNDER_CE
	return S_OK;
#else
	HRESULT hr = S_OK;

#ifdef _UTP_WIFI
	//Stop WZCSVC for TPMC-8X before doing the commit
	if(StopWZCSvc())
		hr = ERROR_TIMEOUT;
#endif

	if(hr == S_OK)
	{
		const DWORD RegCommitTimeOut = 180000; // if its not back in 3 minutes give an error
		_TCHAR tszCommand[MAX_PATH];
		_TCHAR tszFirmwarePath[MAX_PATH];

		STARTUPINFO StartupInfo;
		PROCESS_INFORMATION ProcessInfo;

		::GetStartupInfo(&StartupInfo);

		CfgGetFirmwarePath(tszFirmwarePath);

		// "/scriptdebug" was added to get computer name saved.  without this, ComputerName key does not get saved. All seems
		//	ok with debugging on and that works fine.  But when we don't put that, it does not get saved properly.  
		//	Other debugging showed that the exported hive files are correct and the script is correct but it does not get 
		//	into the offline hive. 6/18/04 WML
		_stprintf_s(tszCommand, MAX_PATH, _T("%s\\%s /command:%s /scriptdebug"), tszFirmwarePath, _T("regcommit.exe"), pstOutputScriptPathname);

		if (::CreateProcess(NULL,tszCommand,NULL,NULL,FALSE,CREATE_NO_WINDOW,
				NULL,NULL,&StartupInfo,&ProcessInfo))
		{
			if (WAIT_OBJECT_0 != WaitForSingleObject(ProcessInfo.hProcess,RegCommitTimeOut))
			{
				hr = ERROR_TIMEOUT;
			}

			::CloseHandle(ProcessInfo.hProcess);
			::CloseHandle(ProcessInfo.hThread);
		}
		else
		{
			hr = ERROR_FUNCTION_FAILED;
		}


		CfgSetBootmonParams(_T("Committing changes to flash.  Please wait..."),600);
	}
	return hr;
#endif
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the DUA script working path and filename in the boot image.
///
/// @param       pstWorkingPath 
/// @param       pstScriptFilename
///
/// @author      William Levine
/// @date        05/06/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgSetBootDUAScriptFile(const _TCHAR *pstWorkingPath, const _TCHAR *pstScriptFilename)
{
_TCHAR tszBootImagePath[MAX_PATH], tszScriptPathname[MAX_PATH];
HRESULT hRes = E_FAIL;
HKEY hBootRootKey;
	// get the boot image system path
	CfgGetBootImageSystemPath(tszBootImagePath);
	_tcscat(tszBootImagePath, _T("\\SYSTEM32\\CONFIG"));

	if (PathIsFileSpec(pstScriptFilename))
		PathCombine(tszScriptPathname, pstWorkingPath, pstScriptFilename);
	else
		_tcscpy(tszScriptPathname, pstScriptFilename);
	
	if (PathFileExists(tszScriptPathname))
	{
		if (LoadOfflineHive(tszBootImagePath,  &hBootRootKey) == S_OK)
		{
			// set workingdir and command file for the DUAgent
			SetRegistryString(hBootRootKey, _T("SYSTEM\\ControlSet001\\Services\\DUAgent\\Parameters\\Config"), _T("WorkingDir"), REG_EXPAND_SZ,
				pstWorkingPath, FALSE);
			SetRegistryString(hBootRootKey, _T("SYSTEM\\ControlSet001\\Services\\DUAgent\\Parameters\\Config\\Sessions\\0000"), _T("CmdFile"), REG_EXPAND_SZ,
				tszScriptPathname, FALSE);

			// sets an environment variable that can be used in the script to tell where the files should be copied from
			SetRegistryString(hBootRootKey, _T("SYSTEM\\ControlSet001\\Control\\Session Manager\\Environment"), _T("CRESTRON_UPDATE_SOURCE"), REG_SZ,
				pstWorkingPath, FALSE);

			// save and unload it
			UnLoadOfflineHive(tszBootImagePath, hBootRootKey, TRUE);
			hRes = S_OK;
		}
	}
	else
		hRes = E_FAIL;

	CfgSetBootmonParams(_T("Processing update package.  Please wait..."),600);

	return hRes;
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the current default boot image in the boot.ini file on c: and sets the EWF to commit the
/// C: drive.
///
/// @param       bBootToBootImage /* = TRUE */ 
///
/// @author      William Levine
/// @date        05/05/04
////////////////////////////////////////////////////////////////////////////////////////////////////
HRESULT CfgSetNextBootImage(bool bBootAlternate/* = TRUE */)
{
_TCHAR szFilename[MAX_PATH], szPath[MAX_PATH];
STARTUPINFO StartupInfo;
PROCESS_INFORMATION ProcessInfo;
BOOL bRes;

	CfgGetBootImageSystemPath(szPath);
	_tcscat(szPath, _T("\\SYSTEM32") );

	// change boot.ini for boot mode
	::GetStartupInfo(&StartupInfo);
	_stprintf_s(szFilename, MAX_PATH, _T("%s\\%s /default /id %s"), szPath, _T("bootcfg.exe"), bBootAlternate ? _T("2") : _T("1") );
	if ( (bRes = ::CreateProcess(NULL, szFilename, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo)) )
	{
		WaitForSingleObject(ProcessInfo.hProcess,5000);

		::CloseHandle(ProcessInfo.hProcess);
		::CloseHandle(ProcessInfo.hThread);
	}
	
	if (bRes)
	{
		// commit changes to c:
		_stprintf_s(szFilename, MAX_PATH, _T("%s\\%s c: -commit"), szPath, _T("ewfmgr.exe") );
		bRes = ::CreateProcess(NULL, szFilename, NULL, NULL, FALSE, 0, NULL, NULL, &StartupInfo, &ProcessInfo);
		if (bRes)
		{
			WaitForSingleObject(ProcessInfo.hProcess,5000);

			::CloseHandle(ProcessInfo.hProcess);
			::CloseHandle(ProcessInfo.hThread);
		}
	}

	return bRes ? S_OK : E_FAIL;
}


HRESULT CfgGetBootImageSystemPath(_TCHAR *pstPath)
{
	ExpandEnvironmentStrings( _T("%CRESTRON_IMAGE_BOOT%"), pstPath, MAX_PATH);
	return S_OK;
}


HRESULT CfgGetRuntimeImageSystemPath(_TCHAR *pstPath)
{
	ExpandEnvironmentStrings( _T("%CRESTRON_IMAGE_RUNTIME%"), pstPath, MAX_PATH);
	return S_OK;
}
#endif


HRESULT CfgGetFirmwarePath(_TCHAR *pstPath)
{
#ifdef UNDER_CE
	CUpxPanelDef::GetFirmwareDir(pstPath);
#else
	ExpandEnvironmentStrings( _T("%CRESTRON_FIRMWARE%"), pstPath, MAX_PATH);
#endif
	return S_OK;
}

HRESULT CfgGetUserPath(_TCHAR *pstPath)
{
#ifdef UNDER_CE
	CUpxPanelDef::GetUserDir(pstPath);
#else
	ExpandEnvironmentStrings( _T("%CRESTRON_USER%"), pstPath, MAX_PATH);
#endif
	return S_OK;
}

HRESULT CfgGetPrivateStoragePath(_TCHAR *pstPath)
{
#ifdef UNDER_CE
	CUpxPanelDef::GetFirmwareDir(pstPath);
#else
	ExpandEnvironmentStrings( _T("%CRESTRON_INTERNAL%"), pstPath, MAX_PATH);
#endif
	return S_OK;
}

HRESULT CfgGetSettingsPath(_TCHAR *pstPath)
{
#ifdef UNDER_CE
	CUpxPanelDef::GetFirmwareDir(pstPath);
#else
	ExpandEnvironmentStrings( _T("%CRESTRON_SETTINGS%"), pstPath, MAX_PATH);
#endif
	return S_OK;
}



////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
static void BuildPath(_TCHAR *pstDest, _TCHAR *pstSrcPath, _TCHAR *pstSrcFileName)
{
	lstrcpy(pstDest,pstSrcPath);
	int len = lstrlen(pstDest);
	if (pstDest[len - 1] != '\\')
	{
		pstDest[len] = '\\';
		pstDest[len + 1] = '\0';
	}

	if (pstSrcFileName[0] == '\\')
		lstrcat(pstDest,&pstSrcFileName[1]);
	else
		lstrcat(pstDest,pstSrcFileName);
}

////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////
HRESULT CfgClearFolder(_TCHAR *pstPath, bool fRecursive /* = true */)
{
	TCHAR szFile[MAX_PATH];
	BuildPath(szFile,pstPath,_T("*.*"));

	WIN32_FIND_DATA findData;
	HANDLE hSearch = FindFirstFile(szFile, &findData); 
	if (hSearch != INVALID_HANDLE_VALUE) 
	{ 
		do
		{
			if (!findData.cFileName[0] || (findData.cFileName[0] == '.'))
				continue;


			TCHAR szFilePath[MAX_PATH];
			BuildPath(szFilePath,pstPath,findData.cFileName);
			
			if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				if (fRecursive)
				{
					CfgClearFolder(szFilePath,fRecursive);
					::RemoveDirectory(szFilePath);
					continue;
				}
			}
			else
				::DeleteFile(szFilePath);

		} while (FindNextFile(hSearch, &findData));

		::FindClose(hSearch);
	} 

	return S_OK;
}

#ifndef UNDER_CE

////////////////////////////////////////////////////////////////////////
// See SHGetFolderPath for CSIDL options
////////////////////////////////////////////////////////////////////////
HRESULT CfgClearSpecialFolder(int SpecialFolder, bool fRecursive)
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, SpecialFolder, NULL, 0, szPath))) 
		return CfgClearFolder(szPath, fRecursive);
		
	return E_FAIL;
}

#endif

#ifndef UNDER_CE
#define gszProfileName _T("%CRESTRON_SETTINGS%\\bootmon.dat")
#define gszBootMon     _T("BootMon")
#define gszMsg         _T("Msg")
#define gszTimeOut     _T("TimeOut")
#define gszDoSys       _T("DoSys")
#define gszDoInt       _T("DoInt")

HRESULT CfgSetBootmonParams(LPCTSTR pMsg, int timeout)
{	
	TCHAR szProfile[MAX_PATH];
	TCHAR szTimeOut[25];
	ExpandEnvironmentStrings(gszProfileName,szProfile,MAX_PATH);
	wsprintf(szTimeOut,_T("%d"),timeout);
	::WritePrivateProfileString(gszBootMon,gszTimeOut,szTimeOut,szProfile);
	::WritePrivateProfileString(gszBootMon,gszMsg,pMsg,szProfile);
	return S_OK;
}

HRESULT CfgDoSysInstall(bool fDelSettings)
{	
	TCHAR szProfile[MAX_PATH];
	ExpandEnvironmentStrings(gszProfileName,szProfile,MAX_PATH);
	::WritePrivateProfileString(gszBootMon,gszDoSys,_T("1"),szProfile);
	if (fDelSettings)
		::WritePrivateProfileString(gszBootMon,gszDoInt,_T("1"),szProfile);
	return S_OK;
}

#endif

#ifndef UNDER_CE

#define CACHE_TYPE_MASK		 (COOKIE_CACHE_ENTRY|URLHISTORY_CACHE_ENTRY)
#define TEMPFILE_CACHE_ENTRY 0
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Clear the temporary internet file cache.
/// This then deletes the related special folder as well
///
/// @author      John Pavlik
/// @date        06/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////

void Delete_IEAddressBarHistory()
{
	HKEY hKey;
	DWORD dwResult;
	TCHAR szValueName[10];
	int i;

	// Open TypedURLs key.
	dwResult = RegOpenKey(HKEY_CURRENT_USER,
		_T("Software\\Microsoft\\Internet Explorer\\TypedURLs"), &hKey );

	i = 1; wsprintf(szValueName, _T("url%d"), i); 
	while (RegDeleteValue(hKey, szValueName) == ERROR_SUCCESS) 
	{
		i++; wsprintf(szValueName, _T("url%d"), i);
	}

	RegCloseKey(hKey); 
}

HRESULT CfgClearUrlCache(UINT cacheType)
{
    BOOL bRet = FALSE;
	HANDLE hEntry;
    LPINTERNET_CACHE_ENTRY_INFO lpCacheEntry = NULL;  
    DWORD dwEntrySize;

	//delete the files
	dwEntrySize = 0;
    hEntry = FindFirstUrlCacheEntry(NULL, NULL, &dwEntrySize);
	lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
    hEntry = FindFirstUrlCacheEntry(NULL, lpCacheEntry, &dwEntrySize);
	if (!hEntry)
	{
		//::MessageBox(NULL,"can't open cache","error",MB_OK);
		goto cleanup;
	}

	do
    {

		if ((cacheType & CLEAR_CACHE_COOKIES) &&
			(lpCacheEntry->CacheEntryType & CACHE_TYPE_MASK) == COOKIE_CACHE_ENTRY)
		{
			DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
		}
		else if ((cacheType & CLEAR_CACHE_HISTORY) &&
			(lpCacheEntry->CacheEntryType & CACHE_TYPE_MASK) == URLHISTORY_CACHE_ENTRY)
		{
			//::MessageBox(NULL,lpCacheEntry->lpszSourceUrlName,"removing",MB_OK);
			DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
		}
		else if ((cacheType & CLEAR_CACHE_TEMPFILES) && 
			(lpCacheEntry->CacheEntryType & CACHE_TYPE_MASK) == TEMPFILE_CACHE_ENTRY)
		{
			DeleteUrlCacheEntry(lpCacheEntry->lpszSourceUrlName);
		}

		dwEntrySize = 0;
		if (!FindNextUrlCacheEntry(hEntry, NULL, &dwEntrySize))
		{
			if (GetLastError() != ERROR_INSUFFICIENT_BUFFER)
				break;
		}
		if (!dwEntrySize)
			break;
		delete [] lpCacheEntry, lpCacheEntry = NULL; 

		lpCacheEntry = (LPINTERNET_CACHE_ENTRY_INFO) new char[dwEntrySize];
	}
	while (FindNextUrlCacheEntry(hEntry, lpCacheEntry, &dwEntrySize));

	FindCloseUrlCache(hEntry);

	bRet = TRUE;
cleanup:
	if (lpCacheEntry)
	{
		delete [] lpCacheEntry; 
	}

	if (cacheType & CLEAR_CACHE_COOKIES)
		bRet = SUCCEEDED(CfgClearSpecialFolder(CSIDL_COOKIES));

	if (cacheType & CLEAR_CACHE_HISTORY)
	{
		IUrlHistoryStg2* pUrlHistoryStg2 = NULL;
		HRESULT hr = CoCreateInstance(CLSID_CUrlHistory, NULL, 
			CLSCTX_INPROC_SERVER, IID_IUrlHistoryStg2, 
			(void**)&pUrlHistoryStg2);
		if (SUCCEEDED(hr))
		{
			hr = pUrlHistoryStg2->ClearHistory(); 
			pUrlHistoryStg2->Release();
		}


		bRet = SUCCEEDED(CfgClearSpecialFolder(CSIDL_HISTORY));

		Delete_IEAddressBarHistory();
	}

	if (cacheType & CLEAR_CACHE_TEMPFILES)
		bRet = SUCCEEDED(CfgClearSpecialFolder(CSIDL_INTERNET_CACHE));

    return bRet;
}

#endif