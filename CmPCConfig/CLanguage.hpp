///
/// @File: CLanguage.hpp
///

#pragma once;

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include "CExecuteCommand.hpp"
#include "tchar.h"
#include <string>
#include <vector>

using namespace std;


class CLanguage: public CExecuteCommand
{

public:
	CLanguage(){ }
	~CLanguage(){ }


	BOOL GetSupportedLanguages(tstring& langues, tstring& tResponse )
	{
		// read from ini file all the languages Crestron is supported
		/*tstring location = _T("D:\\SETTINGS\\Internation.ini");
		TCHAR tszDefault[] = _T("English; The End");
		TCHAR tCrestronSupportedLanguages[10*1024]= _T("");
		::GetPrivateProfileString(_T("INTERNATIONAL"), _T("CrestronSupportedLanguages"), 
			tszDefault, tCrestronSupportedLanguages, TSIZEOF(tCrestronSupportedLanguages), 
			location.c_str());*/
		tstring tCrestronSupportedLanguages(_T("English; Czech; Danish; German; Spanish; Finnish; French; Italian; Japanese; Korean; Norwegian (Bokmal); Dutch; Polish; Portuguese (Brazil); Portuguese (Portugal); Russian; Swedish; Turkish; Chinese (Simplified); Chinese (Traditional); TheEnd"));

		// Verify if languages read from ini file are infact installed on this machine

		// Return Crestron supported languages that are installed on this machine
		langues = tCrestronSupportedLanguages;

		return TRUE;
	}

	BOOL GetInstalledLanguages(vector<tstring>& langues )
	{
		BOOL bRetv = FALSE;
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);

#if 0
		_stprintf_s(cmd, sz, _T("powershell \
								function Get-OSLangPacks { \
								$OSInfo = Get-WmiObject -Class Win32_OperatingSystem \
								$OSLangPacks = $OSInfo.MUILanguages \
								$OSLangPacks \
								} \
								Get-OSLangPacks")
								);
#else
		_stprintf_s(cmd, sz, _T("powershell (Get-WmiObject -Class Win32_OperatingSystem).MUILanguages") );
#endif
		if(CExecuteCommand::Execut(cmd) )
		{
			TCHAR tResponseBuf[1024];
			DWORD ResponseBufLen = 1024;
			CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
			langues = split(tResponseBuf, _T('\n') );
			if(langues.size()>0)
			{
				WriteLog(L"GetInstalledLanguages: Removing White Spaces.", TRACESTACK);
				for(unsigned int i=0; i<langues.size(); i++)
					RemoveWhiteSpaces( langues.at(i) );

				bRetv = TRUE;	
			}
		}

		return bRetv;
	}	

	int GetThreadPreferredUILanguagesf(tstring& curLang)
	{
		BOOL bRetv;
		DWORD dwFlags = MUI_LANGUAGE_NAME; //MUI_LANGUAGE_ID
		ULONG ulNumLanguages = 0;
		TCHAR tLanguagesBuffer[MAX_PATH];
		ULONG sizeLanguagesBuffer = MAX_PATH; 
		bRetv = GetThreadPreferredUILanguages
			(
			dwFlags,
			&ulNumLanguages,
			tLanguagesBuffer,
			&sizeLanguagesBuffer
			);
		if(bRetv)
		{
			curLang = tLanguagesBuffer;
		}

		return ulNumLanguages;
	}

    BOOL GetCurrentLanguage(tstring& curLang )
	{
		return
			GetThreadPreferredUILanguagesf(curLang);

		//Get-Culture
		BOOL bRetv = FALSE;

		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);

		_stprintf_s(cmd, sz, _T("powershell get-culture | format-table -property Name") ); //table, list, date

		if(CExecuteCommand::Execut(cmd) )
		{
			TCHAR tResponseBuf[1024];
			DWORD ResponseBufLen = 1024;
			CExecuteCommand::GetResponseString(tResponseBuf, ResponseBufLen);
			vector<tstring> v = split(tResponseBuf, _T('\n') );
			if(v.size()>3)
			{
				WriteLog(L"GetCurrentLanguage: Removing White Spaces.", TRACESTACK);
				curLang = v.at(3);
				RemoveWhiteSpaces(curLang);

				bRetv = TRUE;				
			}
		}

		return bRetv;
	}

	BOOL GetCurrentLanguageFromRegistry(tstring& curLang )
	{
        // requested language is set
		HKEY hKey;
		DWORD dwDisposition = 0;
		TCHAR tszName[_MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwNameLen = _MAX_PATH;
		tstring tLocale;

		LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS /*| KEY_WOW64_64KEY*/, &hKey );

		LONG lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);

        TCHAR tszValueData[_MAX_PATH];
        BOOL bFound = FALSE;

		while ( (ERROR_SUCCESS == lResultEnum) && !bFound)
		{
			// Exclude short keys
			if (dwNameLen > 9)
			{
				// Exclude "Classes" keys
				if (NULL == (_tcsstr(tszName, _T("_Classes"))))
				{
					TCHAR tszKey[_MAX_PATH];

                    
					HKEY hKeyLanguage;
                    

					// Geo
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\International\\Geo"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
					if(ERROR_SUCCESS == lResult)
					{
						// set PreferredUILanguages for this user
						DWORD cbData = _MAX_PATH;
						LPCTSTR lpValueName = L"Nation";

						if(ERROR_SUCCESS == RegGetValue( hKeyLanguage, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
						{
							BaseLogger::WriteLog(TRACESTACK, _T("Geo Found"));
							(_tcslen(tszValueData) > 0) ? curLang = tszValueData : curLang = _T("244"); // USA as default
							bFound = true;
						}
						else
						{
							curLang = _T("244");
						}

						RegCloseKey(hKeyLanguage);
					}

					// UI
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\Desktop"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
					if(ERROR_SUCCESS == lResult)
					{
						// set PreferredUILanguages for this user
						DWORD cbData = _MAX_PATH;
						LPCTSTR lpValueName = L"PreferredUILanguages";

						LONG lRetv = RegGetValue( hKeyLanguage, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData);
						if(lRetv != ERROR_SUCCESS) 
							lRetv = RegGetValue( hKeyLanguage, NULL, lpValueName, RRF_RT_REG_MULTI_SZ, NULL, tszValueData, &cbData);
						if(lRetv == ERROR_SUCCESS)
						{
							BaseLogger::WriteLog(TRACESTACK, _T("FOUND LANGUAGE"));
							curLang.append(L";");
							(_tcslen(tszValueData) > 0) ? curLang.append(tszValueData) : curLang.append(_T("en-US") );
							(_tcslen(tszValueData) > 0) ? tLocale = tszValueData : tLocale = _T("en-US");
							bFound = true;
						}
						else
						{
							DWORD err = GetLastError();
							curLang.append(L";");
							curLang.append(_T("en-US") );
							tLocale = _T("en-US");
						}

						RegCloseKey(hKeyLanguage);
					}

					// Keyboard
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Keyboard Layout\\Preload"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
					if(ERROR_SUCCESS == lResult)
					{
						// set PreferredUILanguages for this user
						DWORD cbData = _MAX_PATH;
						LPCTSTR lpValueName = L"1";

						if(ERROR_SUCCESS == RegGetValue( hKeyLanguage, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
						{
							BaseLogger::WriteLog(TRACESTACK, _T("Keyboard Found"));
							curLang.append(L";");
							(_tcslen(tszValueData) > 0) ? curLang.append(&tszValueData[4]) : curLang.append(_T("0409") );
							bFound = true;
						}
						else
						{
							curLang.append(L";");
							curLang.append(_T("0409") );
						}
						RegCloseKey(hKeyLanguage);
					}

					// Locale
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\International"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
					if(ERROR_SUCCESS == lResult)
					{
						// set PreferredUILanguages for this user
						DWORD cbData = _MAX_PATH;
						LPCTSTR lpValueName = L"LocaleName";

						curLang += L";";

						if(ERROR_SUCCESS == RegGetValue( hKeyLanguage, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
						{
							BaseLogger::WriteLog(TRACESTACK, _T("Locale Found"));
							(_tcslen(tszValueData) > 0) ? curLang += tszValueData : curLang += _T("en-US"); // USA as default
							bFound = true;
						}
						else
						{
							curLang += _T("en-US");
						}

						RegCloseKey(hKeyLanguage);
					}




				}
			}

			dwNameLen = _MAX_PATH;
			++dwIndex;
			lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
		}

        // If absent then return english. The function below always returns English because it is
        // read from the system account under which we are executing.
        if(!bFound)
        {
            BaseLogger::WriteLog(_T("DID NOT FIND LANGUAGE"));
            GetCurrentLanguage(curLang );
			if(curLang.size() > 0)
				bFound = TRUE;
        }

		RegCloseKey(hKey);

		return bFound;
	}

	BOOL SetCurrentLanguage(TCHAR lang[] )
	{
		// Make sure requested language is install on this machine
		vector<tstring> langues;
		GetInstalledLanguages(langues);
		if (std::find(langues.begin(), langues.end(), lang) == langues.end())
			return FALSE; // requested language is not installed

		// requested language is installed
		TCHAR cmd[1024];
		int sz = TSIZEOF(cmd);

		_stprintf_s(cmd, sz, _T("powershell \
								function Set-OSLangPack {\
								param ($lang)\
								Set-ItemProperty 'HKCU:\\Control Panel\\Desktop' -name \"PreferredUILanguages\" -value $lang\
								}\
								Set-OSLangPack '%s' "),  lang );

		return 
			CExecuteCommand::Execut(cmd);

	}

	BOOL SetLanguageForAllUsers(TCHAR tszNewLanguage[], TCHAR tszNewKeyboard[]=L"", TCHAR Locale[]=L"", TCHAR Geo[]=L"" )
	{
		// Make sure requested language is install on this machine
		//vector<tstring> langues;
		//GetInstalledLanguages(langues);
		//if (std::find(langues.begin(), langues.end(), tszNewLanguage) == langues.end())
		//	return FALSE; // requested language is not installed

		// requested language is set
		HKEY hKey;
		DWORD dwDisposition = 0;
		TCHAR tszName[_MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwNameLen = _MAX_PATH;

		LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS, &hKey );

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
					HKEY hKeyLanguage;

					// UI
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\Desktop"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
					if(ERROR_SUCCESS == lResult)
					{
						// set PreferredUILanguages for this user
						DWORD cbData = DWORD(  _tcslen(tszNewLanguage)*sizeof(TCHAR) );
						LPCTSTR lpValueName = L"PreferredUILanguages";
						if( ERROR_SUCCESS != RegSetValueEx( hKeyLanguage, lpValueName, 0, REG_SZ, (BYTE*)tszNewLanguage, cbData ) )
						{
							BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - ERROR: Failed to set PreferredUILanguages"));
						}
						RegCloseKey(hKeyLanguage);
					}

					// Clear existing keyboards before adding new keyboards, 
					//delete registry key CTP\TIP
					if(1)// for laguages like chinese, stops keyboard list from growing
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Software\\Microsoft\\CTF"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							if( ERROR_SUCCESS == RegDeleteTree(hKey, L"TIP") )
							{
								BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - INFO: RegDeleteTree deleted Software\\Microsoft\\CTF key"));
							}
							RegCloseKey(hKey);
						}
					}

					// add keyboards
					if(_tcslen(tszNewKeyboard) > 0)
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Keyboard Layout\\Preload"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							//first delete all old keyboards for this user
							TCHAR tKeyValue[MAX_PATH];
							for(int i=2; i<20; i++) // 
							{
								_stprintf_s(tKeyValue, _MAX_PATH, _T("%d"), i);
								LONG lResult = RegDeleteValue(hKey, tKeyValue);
								//LONG lResult = RegDeleteKeyEx(hKey, tKeyValue, KEY_WOW64_64KEY, 0);
								if(lResult == ERROR_SUCCESS)
								{
									BaseLogger::WriteFormattedLog(_T("SetLanguageForAllUsers() - INFO: deleted keyboard i=%d, lResult %d"), i, lResult );
								}
								else
								{
									BaseLogger::WriteFormattedLog(_T("SetLanguageForAllUsers() - INFO: No keyboard deleted from Preload key, i=%d, lResult %d"), i, lResult );
									break; // no more fields in this key
								}
							}

							// set keyboard for this user
							LPCTSTR lpValueName;
							DWORD cbData = DWORD(  _tcslen(tszNewKeyboard)*sizeof(TCHAR) );
							//lpValueName = L"1";
							//TCHAR tszEngKeyboard[] = _T("00000409");
							//if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)tszEngKeyboard, cbData ) )
							//{
							//	BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - ERROR: Failed to set Keyboard 2"));
							//}
							lpValueName = L"2";
							if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)tszNewKeyboard, cbData ) )
							{
								BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - ERROR: Failed to set Keyboard 1"));
							}
							RegCloseKey(hKey);
						}
					}

					// Locale
					if(_tcslen(Locale) > 0)
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\International"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							// set keyboard for this user
							DWORD cbData = DWORD(  _tcslen(Locale)*sizeof(TCHAR) );
							LPCTSTR lpValueName = L"LocaleName";
							if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)Locale, cbData ) )
							{
								BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - ERROR: Failed to set Locale"));
							}
							RegCloseKey(hKey);
						}
					}


					// Geo
					if(_tcslen(Geo) > 0)
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\International\\Geo"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							// set keyboard for this user
							DWORD cbData = DWORD(  _tcslen(Geo)*sizeof(TCHAR) );
							LPCTSTR lpValueName = L"Nation";
							if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)Geo, cbData ) )
							{
								BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - ERROR: Failed to set Geo"));
							}
							RegCloseKey(hKey);
						}
					}

					// Language Bar status, 0 means visible
					if(_tcslen(Geo) > 0)
					{
						HKEY hKey;
					//	lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\Microsoft\\CTF\\LangBar"), 0, KEY_ALL_ACCESS, &hKey );
						lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\CTF\\LangBar"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
						if(ERROR_SUCCESS == lResult)
						{
							// set langbar status visible
							DWORD dwShowStatus = 0; // visible
							DWORD cbData = (DWORD)sizeof(DWORD);
							LPCTSTR lpValueName = L"ShowStatus";
							if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_DWORD, (const BYTE*)&dwShowStatus, cbData ) )
							{
								BaseLogger::WriteLog(_T("SetLanguageForCurrentUser() - ERROR: Failed to set LangBar"));
							}
							RegCloseKey(hKey);
						}
					}


				}
			}

			dwNameLen = _MAX_PATH;
			++dwIndex;
			lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
		}

		RegCloseKey(hKey);

		return TRUE;
	}

	BOOL SetLanguageForCurUser(TCHAR tszNewLanguage[], TCHAR tszNewKeyboard[]=L"", TCHAR Locale[]=L"", TCHAR Geo[]=L"" )
	{
		// Make sure requested language is install on this machine
		//vector<tstring> langues;
		//GetInstalledLanguages(langues);
		//if (std::find(langues.begin(), langues.end(), tszNewLanguage) == langues.end())
		//	return FALSE; // requested language is not installed

		// UI		
		if(_tcslen(tszNewLanguage) > 0)
		{
			HKEY hKey;
			//_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Control Panel\\Desktop"), tszName);
			//lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLanguage );
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\Desktop"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set PreferredUILanguages for this user
				DWORD cbData = DWORD(  _tcslen(tszNewLanguage)*sizeof(TCHAR) );
				LPCTSTR lpValueName = L"PreferredUILanguagesPending";
				if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)tszNewLanguage, cbData ) )
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUsers() - ERROR: Failed to set PreferredUILanguages"));
				}
				RegCloseKey(hKey);
			}
		}

		// Clear existing keyboards list for all users (not only curretn user)
		ClearKeyboardsList();

		// add keyboards
		if(_tcslen(tszNewKeyboard) > 0)
		{
			HKEY hKey;
			LONG lResult = RegOpenKey( HKEY_CURRENT_USER, _T("Keyboard Layout\\Preload")	, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set keyboard for this user
				LPCTSTR lpValueName;
				DWORD cbData = DWORD(  _tcslen(tszNewKeyboard)*sizeof(TCHAR) );
				/*lpValueName = L"1";
				TCHAR tszEngKeyboard[] = _T("00000409");
				if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)tszEngKeyboard, cbData ) )
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUsers() - ERROR: Failed to set Keyboard 2"));
				}*/
				lpValueName = L"2";
				if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)tszNewKeyboard, cbData ) )
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUsers() - ERROR: Failed to set Keyboard 1"));
				}
				RegCloseKey(hKey);
			}
		}

		// Locale
		if(_tcslen(Locale) > 0)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\International"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set keyboard for this user
				DWORD cbData = DWORD(  _tcslen(Locale)*sizeof(TCHAR) );
				LPCTSTR lpValueName = L"LocaleName";
				if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)Locale, cbData ) )
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUsers() - ERROR: Failed to set Locale"));
				}
				RegCloseKey(hKey);
			}
		}

		// Geo
		if(_tcslen(Geo) > 0)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\International\\Geo"), 0, KEY_ALL_ACCESS, &hKey );

			if(ERROR_SUCCESS == lResult)
			{
				// set keyboard for this user
				DWORD cbData = DWORD(  _tcslen(Geo)*sizeof(TCHAR) );
				LPCTSTR lpValueName = L"Nation";
				if( ERROR_SUCCESS != RegSetValueEx( hKey, lpValueName, 0, REG_SZ, (BYTE*)Geo, cbData ) )
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUser() - ERROR: Failed to set Geo"));
				}
				RegCloseKey(hKey);
			}
		}

		// Language Bar status, 0 means visible and floating
		if(1)
		{
			HKEY hKey;
			DWORD dwDisposition;
			//LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Software\\Microsoft\\CTF\\LangBar"), 0, KEY_ALL_ACCESS, &hKey );
			LONG lResult = RegCreateKeyEx(HKEY_CURRENT_USER, _T("Software\\Microsoft\\CTF\\LangBar"), 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, &dwDisposition);
			if(ERROR_SUCCESS == lResult)
			{
				// set langbar status visible
				DWORD dwShowStatus = 0; // visible and floating
				lResult = RegSetValueEx( hKey, L"ShowStatus", 0, REG_DWORD, (const BYTE*)&dwShowStatus, sizeof(DWORD) );
				if( lResult != ERROR_SUCCESS)
				{
					BaseLogger::WriteLog(_T("SetLanguageForCurUser() - ERROR: Failed to set LangBar"));
				}
				RegCloseKey(hKey);
			}
		}

		return TRUE;
	}

	void ClearKeyboardsList()
	{

		// requested language is set
		HKEY hKey;
		DWORD dwDisposition = 0;
		TCHAR tszName[_MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwNameLen = _MAX_PATH;

		LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS, &hKey );

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

					// Clear existing keyboards before adding new keyboards, 
					//delete registry key CTP\TIP
					if(1)// for laguages like chinese, stops keyboard list from growing
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Software\\Microsoft\\CTF"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							if( ERROR_SUCCESS == RegDeleteTree(hKey, L"TIP") )
							{
								BaseLogger::WriteLog(_T("SetLanguageForAllUsers() - INFO: RegDeleteTree deleted Software\\Microsoft\\CTF key"));
							}
							RegCloseKey(hKey);
						}
					}

					// clear keyboards list
					if(1)
					{
						_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Keyboard Layout\\Preload"), tszName);
						HKEY hKey;
						lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKey );
						if(ERROR_SUCCESS == lResult)
						{
							//first delete all old keyboards for this user
							TCHAR tKeyValue[MAX_PATH];
							for(int i=2; i<20; i++) // 
							{
								_stprintf_s(tKeyValue, _MAX_PATH, _T("%d"), i);
								LONG lResult = RegDeleteValue(hKey, tKeyValue);
								//LONG lResult = RegDeleteKeyEx(hKey, tKeyValue, KEY_WOW64_64KEY, 0);
								if(lResult == ERROR_SUCCESS)
								{
									BaseLogger::WriteFormattedLog(_T("SetLanguageForAllUsers() - INFO: deleted keyboard i=%d, lResult %d"), i, lResult );
								}
								else
								{
									BaseLogger::WriteFormattedLog(_T("SetLanguageForAllUsers() - INFO: No keyboard deleted from Preload key, i=%d, lResult %d"), i, lResult );
									break; // no more fields in this key
								}
							}

							RegCloseKey(hKey);
						}
					}

				}
			}

			dwNameLen = _MAX_PATH;
			++dwIndex;
			lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
		}

		RegCloseKey(hKey);

	}



	BOOL GetLanguageForCurUser(tstring& curLang )
	{
		tstring tLocale;
        TCHAR tszValueData[_MAX_PATH];
        BOOL bFound = FALSE;


		// Geo
		if(1)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\International\\Geo"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set PreferredUILanguages for this user
				DWORD cbData = _MAX_PATH;
				LPCTSTR lpValueName = L"Nation";

				if(ERROR_SUCCESS == RegGetValue( hKey, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
				{
					BaseLogger::WriteLog(TRACESTACK, _T("Geo Found"));
					(_tcslen(tszValueData) > 0) ? curLang = tszValueData : curLang = _T("244"); // USA as default
					bFound = true;
				}
				else
				{
					curLang = _T("244");
				}

				RegCloseKey(hKey);
			}
		}

		// UI
		if(1)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\Desktop"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set PreferredUILanguages for this user
				DWORD cbData = _MAX_PATH;
				LPCTSTR lpValueName = L"PreferredUILanguagesPending";

				LONG lRetv = RegGetValue( hKey, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData);
				if(lRetv != ERROR_SUCCESS) 
					lRetv = RegGetValue( hKey, NULL, lpValueName, RRF_RT_REG_MULTI_SZ, NULL, tszValueData, &cbData);
				if(lRetv == ERROR_SUCCESS)
				{
					BaseLogger::WriteLog(TRACESTACK, _T("FOUND LANGUAGE"));
					curLang.append(L";");
					(_tcslen(tszValueData) > 0) ? curLang.append(tszValueData) : curLang.append(_T("en-US") );
					(_tcslen(tszValueData) > 0) ? tLocale = tszValueData : tLocale = _T("en-US");
					bFound = true;
				}
				else
				{
					DWORD err = GetLastError();
					curLang.append(L";");
					curLang.append(_T("en-US") );
					tLocale = _T("en-US");
				}

				RegCloseKey(hKey);
			}
		}

		// Keyboard
		if(1)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Keyboard Layout\\Preload"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set PreferredUILanguages for this user
				DWORD cbData = _MAX_PATH;
				LPCTSTR lpValueName = L"1";

				if(ERROR_SUCCESS == RegGetValue( hKey, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
				{
					BaseLogger::WriteLog(TRACESTACK, _T("Keyboard Found"));
					curLang.append(L";");
					(_tcslen(tszValueData) > 0) ? curLang.append(&tszValueData[4]) : curLang.append(_T("0409") );
					bFound = true;
				}
				else
				{
					curLang.append(L";");
					curLang.append(_T("0409") );
				}
				RegCloseKey(hKey);
			}
		}

		// Locale
		if(1)
		{
			HKEY hKey;
			LONG lResult = RegOpenKeyEx( HKEY_CURRENT_USER, _T("Control Panel\\International"), 0, KEY_ALL_ACCESS, &hKey );
			if(ERROR_SUCCESS == lResult)
			{
				// set PreferredUILanguages for this user
				DWORD cbData = _MAX_PATH;
				LPCTSTR lpValueName = L"LocaleName";

				curLang += L";";

				if(ERROR_SUCCESS == RegGetValue( hKey, NULL, lpValueName, RRF_RT_REG_SZ, NULL, tszValueData, &cbData))
				{
					BaseLogger::WriteLog(TRACESTACK, _T("Locale Found"));
					(_tcslen(tszValueData) > 0) ? curLang += tszValueData : curLang += _T("en-US"); // USA as default
					bFound = true;
				}
				else
				{
					curLang += _T("en-US");
				}

				RegCloseKey(hKey);
			}
		}

        // If absent then return english. The function below always returns English because it is
        // read from the system account under which we are executing.
        if(!bFound)
        {
            BaseLogger::WriteLog(_T("DID NOT FIND LANGUAGE"));
            GetCurrentLanguage(curLang );
			if(curLang.size() > 0)
				bFound = TRUE;
        }

					//		RegCloseKey(hKey);

		return bFound;
	}

	BOOL CfgSetInternationLanguage(TCHAR tszNewLangSettings[])
	{
		BOOL bRetv = FALSE;
		TCHAR cmd[MAX_PATH*4];

		_stprintf_s(cmd, TSIZEOF(cmd), _T("%s"),  tszNewLangSettings);
		CExecuteCommand::Execut(cmd);
		bRetv = CheckForSuccess( _T("Ok.") );

		return bRetv;
	}

protected:

};
