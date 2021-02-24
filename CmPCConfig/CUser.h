#pragma once;

#include "CmPCConfig.h"

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgGetResponseString(tResponseBuf, ResponseBufLen/*in&out*/);
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgGetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen);

CmPCConfig_DeclSpec BOOL CfgShowAllUsers();

CmPCConfig_DeclSpec BOOL CfgAddUser(LPWSTR name, LPWSTR password, DWORD privillage);

CmPCConfig_DeclSpec BOOL CfgDeleteUser(LPWSTR name);


