#pragma once

#include "CmPCConfig.h"


typedef std::basic_string <TCHAR> tstring;




class CNetworkAuthentication
{
public:
	CNetworkAuthentication();
	virtual ~CNetworkAuthentication(void);



	bool IsEnabled();
	bool Enable();
	bool Disable();
	bool SetCertificate(tstring& tstrCert);




private:

	tstring GetProfileFilename();

	
};



//
// Config Func handler functions
//
CmPCConfig_DeclSpec BOOL CfgSetNetworkAuth(bool bEnable);
CmPCConfig_DeclSpec BOOL CfgSetNetworkAuthCert(tstring& tstrCert);