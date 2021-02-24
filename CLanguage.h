///
/// @File: CLanguage.h
///

#pragma once;

#include "CLanguage.hpp"


CmPCConfig_DeclSpec BOOL CfgGetSupportedLanguages(tstring& langues, tstring& tResponse );

CmPCConfig_DeclSpec BOOL CfgGetInstalledLanguages(vector<tstring>& langues, tstring& tResponse );

CmPCConfig_DeclSpec BOOL CfgGetCurrentLanguage(tstring& curLang, tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgGetThreadPreferredUILanguages(tstring& curLang);

CmPCConfig_DeclSpec BOOL CfgSetCurrentLanguage(TCHAR lang[], tstring& tResponse );

CmPCConfig_DeclSpec BOOL CfgSetLanguageForAllUsers(TCHAR tszNewLanguage[], tstring& tResponse );

CmPCConfig_DeclSpec BOOL CfgSetInternationLanguage( string strLangSettings, tstring& tResponse );

