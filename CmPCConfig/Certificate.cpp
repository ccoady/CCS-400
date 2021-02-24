#include "stdafx.h"
#include "Certificate.h"
#include "CExecuteCommand.hpp"

//
// Add a Certificate
//
CmPCConfig_DeclSpec BOOL CfgAddCertificate(TCHAR CertifcateName[], TCHAR StoreName[] )
{
	//CertMgr.exe -add -c -n "MyCert" -r "localMachine" -s "root|my|trustedpublisher"
	TCHAR cmd[1024];
	int sz = TSIZEOF(cmd);
	_stprintf_s(cmd, sz, _T("certmgr -add \"%s\" -s -r localMachine \"%s\""), 
		CertifcateName, StoreName);
	
	CExecuteCommand ex;
	return ex.CExecuteCommand::Execut(cmd);
}

//
// Copy - Copies a certificate with the common name MyCert in the MY system store to a file called NewCert.cer.
//
CmPCConfig_DeclSpec BOOL CfgCopyCertificate(TCHAR CertifcateName[], TCHAR StoreName[], TCHAR CopiedCertName[] )
{
	//CertMgr.exe -add -c -n "MyCert" -r "localMachine" -s "root" "NewMyCert.cer"
	TCHAR cmd[1024];
	int sz = TSIZEOF(cmd);
	_stprintf_s(cmd, sz, _T("certmgr -add -c -n \"%s\" -r localMachine -s \"%s\" \"%s\""), 
		CertifcateName, StoreName, CopiedCertName);

	CExecuteCommand ex;
	return ex.CExecuteCommand::Execut(cmd);
}

//
// Delete Certificate
//
CmPCConfig_DeclSpec BOOL CfgDelCertificate(TCHAR CertifcateName[], TCHAR StoreName[] )
{
	TCHAR cmd[1024];
	int sz = TSIZEOF(cmd);
	_stprintf_s(cmd, sz, _T("certmgr -del \"%s\" -s -r localMachine \"%s\""), 
		CertifcateName, StoreName);
	
	CExecuteCommand ex;
	return ex.CExecuteCommand::Execut(cmd);
}

//
// View certificate 
//
CmPCConfig_DeclSpec BOOL CfgViewCertificate(TCHAR CertifcateName[], TCHAR StoreName[] )
{
	// certmgr -all -r localMachine -s root|my|trustedpublisher
	TCHAR cmd[1024];
	int sz = TSIZEOF(cmd);
	_stprintf_s(cmd, sz, _T("certmgr -n \"%s\" -r localMachine -s \"%s\""), 
		CertifcateName, StoreName);

	CExecuteCommand ex;
	return ex.CExecuteCommand::Execut(cmd);
}


