#pragma once;

#include "CmPCConfig.h"
#include "CDomain.hpp"

using namespace std;

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgJoinPanelToDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE. 
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgJoinPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRemovePanelFromDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRemovePanelFromDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenamePanelInDomain("panelNewName", "Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenamePanelInDomain(TCHAR panelNewName[], TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenamePanelInWorkgroup("panelNewName");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenamePanelInWorkgroup(TCHAR panelNewName[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgRenameWorkgroup("WorkGroupNewName");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgRenameWorkgroup(TCHAR WorkGroupNewName[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgGetResponseString(tResponseBuf, ResponseBufLen/*in&out*/);
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgGetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgJoinPanelToDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgAddPanelToDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgQueryPanelsInDomain("Contoso.com", "AlexD", "pass@word1");
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgQueryPanelsInDomain(TCHAR domainName[], TCHAR userD[], TCHAR passwordD[], tstring& tResponse);

//---------------------------------------------------------------------------
// USAGE: CDeviceNamesInfo info; CfgGetDeviceNamesInfo_Obsolete(info);
CmPCConfig_DeclSpec BOOL CfgGetDeviceNamesInfo_Obsolete(CDeviceNamesInfo &info, tstring &tResponse);

CmPCConfig_DeclSpec HRESULT CfgNetJoinDomain(_TCHAR Domain[], _TCHAR AccountOU[], _TCHAR Account[], _TCHAR Password[],  DWORD joinOption, tstring& tResponse);

CmPCConfig_DeclSpec HRESULT CfgNetUnJoinDomain(_TCHAR Account[], _TCHAR Password[],  DWORD joinOption, tstring& tResponse);

CmPCConfig_DeclSpec HRESULT CfgRestoreHostName(tstring& tResponse);
CmPCConfig_DeclSpec tstring CfgGetWorkgroupName();
CmPCConfig_DeclSpec tstring CfgGetDomainName();
CmPCConfig_DeclSpec tstring CfgGetPrimaryDnsSuffix();
CmPCConfig_DeclSpec BOOL CfgSetPrimaryDnsSuffix(tstring suffix, tstring& tResponse );
CmPCConfig_DeclSpec BOOL CfgChangeHostname(TCHAR newHostName[], tstring& tResponse);
CmPCConfig_DeclSpec BOOL CfgUpdateManifest();
CmPCConfig_DeclSpec BOOL CfgUpdateManifest(TCHAR szOverride[]);
CmPCConfig_DeclSpec BOOL CfgGetCaptureDevieInfo(BOOL bWriteIni, tstring& tResponse);
CmPCConfig_DeclSpec BOOL CfgGetSoundBar(BOOL bWriteIni, tstring& tResponse, bool g);
CmPCConfig_DeclSpec BOOL CfgGetHuddlyIQ(BOOL bWriteIni, tstring& tResponsse, bool g);
enum HUDDLY{HUDDLY_START, HUDDLY_START2, HUDDLY_START5, HUDDLY_STOP, HUDDLY_INFO, HUDDLY_AUTOFRAMRE_OFF, HUDDLY_AUTOFRAMRE_ON};
CmPCConfig_DeclSpec BOOL CfgHandlHuddlyIQ(HUDDLY action, tstring& tResponse);
CmPCConfig_DeclSpec BOOL CfgIsSkypeMode();
CmPCConfig_DeclSpec bool CfgShouldRunHuddlyIQ();
CmPCConfig_DeclSpec tstring GetFullPath(TCHAR varName[], TCHAR fileName[]);
CmPCConfig_DeclSpec tstring GetFullPath_(TCHAR varName[], TCHAR fileName[]);

