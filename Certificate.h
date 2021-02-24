#pragma once;

#include "CmPCConfig.h"

//---------------------------------------------------------------------------
// USAGE: bRetv = CfgGetResponseString(tResponseBuf, ResponseBufLen/*in&out*/);
// return value is TRUE or FALSE
// if return is FALSE, usse CfgGetResponseString() to get reason of failure
CmPCConfig_DeclSpec BOOL CfgGetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen);

CmPCConfig_DeclSpec BOOL CfgAddCertificate(TCHAR CertifcateName[], TCHAR StoreName[] );

CmPCConfig_DeclSpec BOOL CfgCopyCertificate(TCHAR CertifcateName[], TCHAR StoreName[], TCHAR CopiedCertName[] );

CmPCConfig_DeclSpec BOOL CfgDelCertificate(TCHAR CertifcateName[], TCHAR StoreName[] );

CmPCConfig_DeclSpec BOOL CfgViewCertificate(TCHAR CertifcateName[], TCHAR StoreName[] );

