
//////////////////////////////////////////////////////////////////////////////////
//
// File:		NetworkAuthentication.cpp
//
// Description:	Class and Cfg functions to handle wired network authentication 
//				with 802.1x protocol.
//
// Author:		Jason Vourtsis
//
//////////////////////////////////////////////////////////////////////////////////




#include "StdAfx.h"
#include <fstream>

#include "NetworkAuthentication.h"

//#include "UtsEngine.h"


//#include "UpxPanelDef.h"
#include "CExecuteCommand.hpp"


#define INTERFACE_NAME		_T("\"Local Area Connection\"")
#define PROFILE_FILE_NAME	_T("Local Area Connection.xml")




CNetworkAuthentication::CNetworkAuthentication()
//: m_pEngine(pEngine)
{
}

CNetworkAuthentication::~CNetworkAuthentication(void)
{
}




////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Return a boolean indicating if wired network authentication is
//					turned on for the system.  This must include the service
//					running and the profile set to include authentication.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
bool CNetworkAuthentication::IsEnabled()
{
	CExecuteCommand ex;
	bool bCmdStatus;
	tstring strResult;
	
	ex.Execut(_T("sc query dot3svc"), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::IsEnabled() Execute: 'sc query dot3svc' %d %s"), bCmdStatus, strResult);

	if (bCmdStatus)
	{
		int nFind = (int)strResult.find(_T("STATE"));

		if (-1 != nFind)
		{
			nFind = (int)strResult.find(_T(":"), nFind);
			tstring strStatus = strResult.substr(nFind+2, 1);

			// Status code "4" is RUNNING.  All others are not.
			if (strStatus != _T("4"))
				return false;

			// Check for authentication being enabled even if service is on.
			tstring strCommand = _T("netsh lan show profile interface=");
			strCommand += INTERFACE_NAME;
			
			ex.Execut(strCommand.c_str(), strResult, bCmdStatus);
			BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::IsEnabled() Execute: '%s' %d %s"), strCommand, bCmdStatus, strResult);

			if (bCmdStatus)
			{
				nFind = (int)strResult.find(_T("802.1x"));
				
				if (-1 != nFind)
				{
					nFind = (int)strResult.find(_T(":"), nFind);
					tstring strAuthStatus = strResult.substr(nFind+2, 7);

					if (strAuthStatus == _T("Enabled"))
						return true;
				}
			}
		}
	}

	return false;
}




////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Turn on network authentication by forcing the service on,
//					setting it to automatically start, and applying our saved
//					profile information from an xml file in the D:\Settings dir
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
bool CNetworkAuthentication::Enable()
{
	bool bCmdStatus;
	tstring strResult;
	
	CExecuteCommand ex;

	//m_pEngine->OnErrorMsg(_T("Set Network Authentication: ON"), E_NOTICE);
	BaseLogger::WriteLog(_T("Set Network Authentication: ON"));

	ex.Execut(_T("net start dot3svc"), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Enable() Execute: 'net start dot3svc' %d %s"), bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("successfully"))))
	{
		if (-1 == strResult.find(_T("The requested service has already been started.")))
		{
			BaseLogger::WriteLog(_T("Error starting dot3svc"));			
			return false;
		}
	}

	ex.Execut(_T("sc config dot3svc start= auto"), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Enable() Execute: 'sc config dot3svc start= auto' %d %s"), bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("SUCCESS"))))
	{
		BaseLogger::WriteLog(_T("Error configuring auto start for service: dot3svc"));
		return false;
	}

	tstring strCommand = _T("netsh lan add profile filename=\"");
	strCommand += GetProfileFilename();
	strCommand += _T("\" interface=");
	strCommand += INTERFACE_NAME;
	
	
	ex.Execut(strCommand.c_str(), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Enable() Execute: '%s' %d %s"), strCommand, bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("successfully"))))
	{
		BaseLogger::WriteLog(_T("Error enabling 802.1X"));
		return false;
	}


	return true;
}



////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Turn network authentication off by setting the value in the
//					profile, stopping the service and setting it to manual start.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
bool CNetworkAuthentication::Disable()
{
	bool bCmdStatus;
	tstring strResult;
	
	CExecuteCommand ex;

	BaseLogger::WriteLog(_T("Set Network Authentication: OFF"));

	tstring strCommand = _T("netsh lan set profileparameter interface=");
	strCommand += INTERFACE_NAME;
	strCommand += _T(" oneXEnabled=no");
	
	ex.Execut(strCommand.c_str(), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Disable() Execute: '%s' %d %s"), strCommand, bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("successfully"))))
	{
		BaseLogger::WriteLog(_T("Error disabling 802.1X"));
		return false;
	}
	
	Sleep(500);

	ex.Execut(_T("net stop dot3svc"), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Disable() Execute: 'net stop dot3svc' %d %s"), strCommand, bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("successfully"))))
	{
		BaseLogger::WriteLog(_T("Error stopping dot3svc"));
		return false;
	}

	ex.Execut(_T("sc config dot3svc start= demand"), strResult, bCmdStatus);
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CNetworkAuthentication::Disable() Execute: 'sc config dot3svc start= demand' %d %s"), bCmdStatus, strResult);

	if ((!bCmdStatus) || (-1 == strResult.find(_T("SUCCESS"))))
	{
		BaseLogger::WriteLog(_T("Error configuring manual start for service: dot3svc"));
		return false;
	}	


	return true;
}




////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Find our pre-saved profile file and return the path to it.
//					The default is stored in the \bin directory.  Our working copy
//					is in the \Settings directory.  If the file is not in Settings
//					copy it from bin and leave it there.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
tstring CNetworkAuthentication::GetProfileFilename()
{
	tstring strSettingsFile = _T("D:\\settings\\");
	tstring strBinFile = _T("C:\\crestron\\bin\\");
	
	// Build the filename for the Bin directory
	TCHAR  FirmwareDirectory[MAX_PATH];
	if(GetEnvironmentVariable(_T("CRESTRON_FIRMWARE"),FirmwareDirectory,TSIZEOF(FirmwareDirectory)) !=  0)
	{
		strBinFile = FirmwareDirectory;
		strBinFile += _T("\\");		
	}

	strBinFile += PROFILE_FILE_NAME;

	// Build the filename for the Settings directory
	TCHAR  SettingsDirectory[MAX_PATH];
	if(GetEnvironmentVariable(_T("CRESTRON_SETTINGS"), SettingsDirectory, TSIZEOF(SettingsDirectory)) !=  0)
	{
		strSettingsFile = SettingsDirectory;
		strSettingsFile += _T("\\");		
	}

	strSettingsFile += PROFILE_FILE_NAME;


	// If our settings file is there, don't mess with it
	if (::PathFileExists(strSettingsFile.c_str()))
		return strSettingsFile;


	if (!::PathFileExists(strBinFile.c_str()))
	{
		BaseLogger::WriteFormattedLog(0, _T("Error: Could not find Auth profile file: %s"), strBinFile.c_str());
		return _T("");
	}

	// Copy our file from the bin directory to the settings dir and return it
	::CopyFile(strBinFile.c_str(), strSettingsFile.c_str(), FALSE);

	return strSettingsFile;

}







////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Read our working profile file and replace the <TrustedRootCA>
//					value that is in it with the certificate thumbprint value that
//					was sent to us in the serial join.  Save the new value in our
//					Working xml file.  After a change here, we re-call Enable()
//					to force a refresh of the system profile.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
bool CNetworkAuthentication::SetCertificate(tstring& tstrCert)
{
	// Keep all file operations in ascii for this
	USES_CONVERSION;

	tstring strFilename = GetProfileFilename();
	BaseLogger::WriteFormattedLog(0, _T("Setting Network Auth Certificate '%s' in file %s"), tstrCert.c_str(), strFilename.c_str());

	std::vector<std::string> lineList;
	
	//
	// Read our profile file into memory
	//
	ifstream f(strFilename.c_str());
	if (f.is_open())
	{
		std::string line;
		
		while (getline(f, line))
		{
			lineList.push_back(line);
		}

		f.close();
	}
	else
	{
		BaseLogger::WriteFormattedLog(0, _T("Error: Could not open Auth profile file: %s"), strFilename.c_str());
		return false;
	}


	//
	// Replace our thumbprint with the new one
	//
	bool bReplaced = false;

	for (int i=0; i<lineList.size(); ++i)
	{
		int nStart = (int)lineList[i].find("<TrustedRootCA>");
		if (-1 != nStart)
		{
			std::string strRemove = "</TrustedRootCA>";
			int nEnd = (int)lineList[i].find(strRemove);
			if (-1 != nEnd)
			{
				nEnd += (int)strRemove.size();
				//strRemove = lineList[i].substr(nStart, nEnd-nStart);

				std::string strAdd = "<TrustedRootCA>";
				strAdd += T2A(tstrCert.c_str());
				strAdd += "</TrustedRootCA>";

				lineList[i].replace(nStart, nEnd-nStart, strAdd);			
				bReplaced = true;
				break;
			}
		}
	}


	//
	// Save our new version of the file over the old
	//
	if (bReplaced)
	{
		ofstream f(strFilename.c_str());
		if (f.is_open())
		{
			for (int i=0; i<lineList.size(); ++i)
			{
				f.write(lineList[i].c_str(), lineList[i].size());
				f.write("\n", 1);
			}

			f.close();
		}
		else
		{
			BaseLogger::WriteFormattedLog(0, _T("Error: Could not open Auth profile file: %s"), strFilename.c_str());
			return false;
		}
	}
	else
	{
		BaseLogger::WriteFormattedLog(0, _T("Error: Could not find TrustedRootCA section in Auth profile file: %s"), strFilename.c_str());
		return false;
	}

	//
	// See if we need to re-apply auth settings
	//
	if (IsEnabled())
	{
		BaseLogger::WriteFormattedLog(0, _T("Re-enabling Network Authorization with new certificate: %s"), tstrCert.c_str());
		Enable();
	}


	return true;
}









////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Config func handler exposed in CmPCConfig.dll for handling 
//					Network Auth on and off.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec BOOL CfgSetNetworkAuth(bool bEnable)
{
	CNetworkAuthentication auth;
	bool bRes = false;

	if (bEnable)
	{
		if (!auth.IsEnabled())
			bRes = auth.Enable();
	}
	else
	{
		if (auth.IsEnabled())
			bRes = auth.Disable();
	}

	return bRes ? TRUE:FALSE;
}




////////////////////////////////////////////////////////////////////////////////
// Parameters:	
//			        
// Returns:
//
// Description:		Config func handler exposed in CmPCConfig.dll for handling 
//					Network Auth certificate setting.
//
// Author:      	Jason Vourtsis
////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec BOOL CfgSetNetworkAuthCert(tstring& tstrCert)
{
	CNetworkAuthentication auth;
	bool bRes;

	bRes = auth.SetCertificate(tstrCert);

	return bRes ? TRUE:FALSE;
}




