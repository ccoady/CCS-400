#include "stdafx.h"
#include "CAudioInputDevices.h"


CmPCConfig_DeclSpec BOOL CfgGetRemoveableAudioInputDevices(vector<tstring>& vInputDevices, tstring& tResponse)
{
	tResponse.clear();
	vInputDevices.clear();
	BOOL bRetv = FALSE;

	CAudioInputDevices cfg;
	bRetv = cfg.GetAudioInputDevices(vInputDevices, tResponse);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetDefaultAudioInputDevice(tstring tInputDevice, tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;

	tstring tID;
	CAudioInputDevices cfg;
	if( cfg.GetInputDeviceID(tInputDevice, tID) )
	{
		cfg.SetAsDefaultDevice(tID.c_str() );

		// Set the device in the registry for Lync
		tstring strPnpName = _T("");
		cfg.GetInputDevicePnpName(tInputDevice, strPnpName);
		if (strPnpName != _T(""))
        {
			cfg.SetLyncInputDevice(tInputDevice, strPnpName);
            
            // The assumption is when we get here, the calls have
            // succeeded. Error checking needs to be more robust
            // for methods called from this method. JAS 10/28/2014
            bRetv = TRUE;
        }
	}

	return bRetv;
}


CmPCConfig_DeclSpec BOOL CfgGetDefaultInputDeviceName(tstring& defaultDeviceName,  tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;

	CAudioInputDevices cfg;
	bRetv = cfg.GetDefaultDeviceName(defaultDeviceName);

	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgTestCAudioInputDevices(tstring& tResponse)
{
	tResponse.clear();
	BOOL bRetv = FALSE;

	CAudioInputDevices cfg;
	bRetv = cfg.test();

	return bRetv;
}