#include "stdafx.h"
#include "CAudioOutputDevices.h"
#include "system.h"

CmPCConfig_DeclSpec BOOL CfgGetRemoveableAudioOutputDevices(vector<tstring>& vOutputDevices, tstring& tResponse)
{
	tResponse.clear();
	vOutputDevices.clear();
	BOOL bRetv = FALSE;

	CAudioOutputDevices cfg;
	bRetv = cfg.GetAudioOutputDevices(vOutputDevices, tResponse);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetDefaultAudioOutputDevice(tstring tOutputDevice, tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;
	tstring tID;
	CAudioOutputDevices cfg;
  
    if( cfg.GetOutputDeviceID(tOutputDevice, tID) )
	{
        cfg.SetAsDefaultDevice(tID.c_str() );

		// Set the device in the registry for Lync
		tstring strPnpName = _T("");
		cfg.GetOutputDevicePnpName(tOutputDevice, strPnpName);
		if (strPnpName != _T(""))
        {
			cfg.SetLyncOutputDevice(tOutputDevice, strPnpName);

            // The assumption is when we get here, the calls have
            // succeeded. Error checking needs to be more robust
            // for methods called from this method. JAS 10/28/2015
            bRetv = true;
        }
	}

	return bRetv;
}


CmPCConfig_DeclSpec BOOL CfgGetDefaultOutputDeviceName(tstring& defaultDeviceName,  tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;

	CAudioOutputDevices cfg;
	bRetv = cfg.GetDefaultDeviceName(defaultDeviceName);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgTestCAudioOutputDevices(tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;

	CAudioOutputDevices cfg;
	bRetv = cfg.test();

	return bRetv;
}
