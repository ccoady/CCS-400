///
/// @File: CLanguage.h
///

#include "stdafx.h"
#include <iostream>     // std::cout
#include <fstream>      // std::ifstream
#include "CLanguage.h"

#include <cstdio>
#include <clocale>
#include <ctime>
#include <cwchar>

/////////////////////////////////////////////////////////////////
/// @Function   CfgGetSupportedLanguages
///
/// @param                  
///
/// @author      Masoud Qurashi
/// @date        02/20/14
CmPCConfig_DeclSpec BOOL CfgGetSupportedLanguages(tstring& langues, tstring& tResponse )
{
	BOOL bRetv = FALSE;
	tResponse.clear();

	CLanguage cfg;

	bRetv = cfg.GetSupportedLanguages(langues, tResponse );

	cfg.GetResponseString(tResponse);

	return bRetv;
}
/////////////////////////////////////////////////////////////////
/// @Function   CfgGetInstalledLanguages
///
/// @param                  
///
/// @author      Masoud Qurashi
/// @date        03/21/13
CmPCConfig_DeclSpec BOOL CfgGetInstalledLanguages(vector<tstring>& langues, tstring& tResponse )
{
	BOOL bRetv = FALSE;
	CLanguage cfg;

	bRetv = cfg.GetInstalledLanguages(langues);

	cfg.GetResponseString(tResponse);

	return bRetv;
}

/////////////////////////////////////////////////////////////////
/// @Function   CfgGetCurrentLanguage
///
/// @param                  
///
/// @author      Masoud Qurashi
/// @date        03/21/13
CmPCConfig_DeclSpec BOOL CfgGetCurrentLanguage(tstring& curLang, tstring& tResponse )
{
	BOOL bRetv = FALSE;
	CLanguage cfg;
	//cfg.GetCurrentLanguage(curLang);
	//cfg.GetThreadPreferredUILanguagesf(curLang);
	//LANGID ULID = GetUserDefaultLangID();
	//LANGID SLID = GetSystemDefaultLangID();

	 
	//bRetv = cfg.GetCurrentLanguageFromRegistry(curLang);
	bRetv = cfg.GetLanguageForCurUser(curLang);

	cfg.GetResponseString(tResponse);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgGetThreadPreferredUILanguages(tstring& curLang)
{
	curLang.clear();
	tstring str;
	TCHAR buf[MAX_PATH];
	CLanguage cfg;

	cfg.GetThreadPreferredUILanguagesf(str);
	curLang = L"\t ThreadPreferredUILanguages " + str + L";\r\n";

	cfg.GetCurrentLanguage(str);
	curLang += L"\t CurrentLanguage " + str + L";\r\n";

	LANGID ULID = GetUserDefaultLangID();
	_stprintf_s(buf, MAX_PATH, L"ox%X", ULID );
	str = buf;
	curLang += L"\t UserDefaultLangID " + str + L";\r\n";

	LANGID SLID = GetSystemDefaultLangID();
	_stprintf_s(buf, MAX_PATH, L"ox%X", SLID );
	str = buf;
	curLang += L"\t SystemDefaultLangID " + str + L";\r\n";


	return TRUE;
}


/////////////////////////////////////////////////////////////////
/// @Function   CfgSetCurrentLanguage
///
/// @param                  
///
/// @author      Masoud Qurashi
/// @date        03/21/13
CmPCConfig_DeclSpec BOOL CfgSetCurrentLanguage(TCHAR lang[], tstring& tResponse )
{
	CLanguage cfg;
	BOOL bRetv = FALSE;
	
	bRetv = cfg.SetCurrentLanguage(lang);

	cfg.GetResponseString(tResponse);
	
	return bRetv;
}

/////////////////////////////////////////////////////////////////
/// @Function   SetLanguageForAllUsers
///
/// @param                  
///
/// @author      Masoud Qurashi
/// @date        04/08/13
CmPCConfig_DeclSpec BOOL CfgSetLanguageForAllUsers(TCHAR tszNewLanguage[], tstring& tResponse )
{
	CLanguage cfg;
	BOOL bRetv = FALSE;

	bRetv = cfg.SetLanguageForAllUsers(tszNewLanguage);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetInternationLanguage( string strLangSettings, tstring& tResponse )
{
	USES_CONVERSION;	
	BaseLogger::WriteFormattedLog(_T("CfgSetInternationLanguage() - IN Set international language to %s"), A2T(strLangSettings.c_str())  );

	BOOL bRetv = FALSE;
	char * pBuffer = 0;
	char * pBuffer2 = 0;	
	tResponse.clear();
	CLanguage cfg;
	char GeoID[MAX_PATH]; 
	char MUILang[MAX_PATH];
	char InputLanguageID[MAX_PATH];
	char Locale[MAX_PATH];
	vector <string> vTokens;

	vTokens = cfg.split(strLangSettings.c_str(), ';');


	// Set UI;Keyboard;GeoID for all users
	if(vTokens.size() > 3)
	{
		sprintf_s(MUILang, MAX_PATH, "%s", vTokens.at(1).c_str() );
		sprintf_s(InputLanguageID, MAX_PATH, "0000%s", vTokens.at(2).c_str()  );
		sprintf_s(Locale, MAX_PATH, "%s", vTokens.at(3).c_str()  );
		sprintf_s(GeoID, MAX_PATH, "%s", vTokens.at(0).c_str()  );

		////if(!cfg.SetLanguageForAllUsers( A2T(MUILang), A2T( InputLanguageID ), A2T( Locale ), A2T( GeoID ) )  )
		//if(!cfg.SetLanguageForCurUser( A2T(MUILang), A2T( InputLanguageID ), A2T( Locale ), A2T( GeoID ) )  )
		//{
		//	BaseLogger::WriteFormattedLog(_T("CfgSetInternationLanguage() - ERROR: Failed to set international language, %s"), strLangSettings.c_str()  );
		//	return FALSE;
		//}

		// apply changes using xml file
		sprintf_s(InputLanguageID, MAX_PATH, "%s:0000%s", vTokens.at(2).c_str(), vTokens.at(2).c_str()  );
		sprintf_s(Locale, sizeof(Locale), "%s", vTokens.at(3).c_str() );
	}
	else
	{
		BaseLogger::WriteFormattedLog(_T("CfgSetInternationLanguage() - ERROR: Failed to set international language, %d"), vTokens.size()  );
		return FALSE;
	}

	std::ifstream is ("c:\\crestron\\bin\\LangData\\International\\International_t.xml", std::ifstream::binary);
	if (is) 
	{
		// get length of file:
		is.seekg (0, is.end);
		unsigned int length = (unsigned int)is.tellg();
		is.seekg (0, is.beg);

		pBuffer = new char [length+10];
		if(pBuffer)
		{
			memset(pBuffer, NULL, length+10);

			std::cout << "Reading " << length << " characters... ";
			// read data as a block:
			is.read (pBuffer, length);

			if (is)
				std::cout << "all characters read successfully.";
			else
				std::cout << "error: only " << is.gcount() << " could be read";
			is.close();
		}

		// ...pBuffer contains the entire file...
		pBuffer2 = new char [length+100];
		if(pBuffer2)
		{
			memset(pBuffer2, NULL, length+100);
			sprintf_s(pBuffer2, length+100, pBuffer, GeoID, MUILang, InputLanguageID, Locale);
			std::cout << pBuffer2;
		}


		std::ofstream os ("c:\\crestron\\bin\\LangData\\International\\International.xml", std::ofstream::binary);
		if (os) 
		{
			os << pBuffer2;
			os.close();
		}

		if(pBuffer)
			delete[] pBuffer;
		if(pBuffer2)
			delete[] pBuffer2;


		// execute xml file	 
		// Note: it executes successfully under admin account, therefore, the following call moved to UtsEngine.exe
		//cfg.CfgSetInternationLanguage(L"c:\\crestron\\bin\\LangData\\International\\International.bat");
	}
	
	BaseLogger::WriteFormattedLog(_T("CfgSetInternationLanguage() - OUT Set international language to %s"), A2T(strLangSettings.c_str())  );

	return bRetv;
}