
#pragma once
#include "CmPCConfig.h"
//#include <vector>
//using namespace std;





CmPCConfig_DeclSpec void CfgLyncFixUserSettings();
CmPCConfig_DeclSpec void CfgLyncInit();
CmPCConfig_DeclSpec void CfgLyncSetDisplaySettings(int nTotal, int nControl, int nLeft, int nRight, TCHAR* tszControl, TCHAR* tszLeft, TCHAR* tszRight);
CmPCConfig_DeclSpec void CfgLyncSwapLargeDisplays();
CmPCConfig_DeclSpec void CfgLyncShowUI(bool bShow);
CmPCConfig_DeclSpec void CfgSetLyncVideoCropping();
CmPCConfig_DeclSpec bool CfgLyncImportCert(const TCHAR* tszFile, BOOL bRootCA , tstring &tstrResult);
CmPCConfig_DeclSpec void CfgLyncSetEWSCredentials(const TCHAR* tszUsername, const TCHAR* tszAddress, const TCHAR* tszPassword);
CmPCConfig_DeclSpec void CfgLyncSetEWSServers(const TCHAR* tszInternal, const TCHAR* tszExternal);
CmPCConfig_DeclSpec bool CfgLyncImportPersonalCert(const TCHAR* tszFile, const TCHAR* tszPassword, tstring &tstrResult);
CmPCConfig_DeclSpec void CfgLyncSetSKUData(const TCHAR* tszKey, const TCHAR* tszValue);
CmPCConfig_DeclSpec void CfgLyncClearLyncLogs();
CmPCConfig_DeclSpec void CfgLyncSetLyncDataBackupMax(DWORD dwMax);
CmPCConfig_DeclSpec bool GetSfBVersion(tstring& tsVersion);

