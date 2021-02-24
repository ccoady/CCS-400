#include "CmPCConfig.h"
#include "CAudioInputDevices.hpp"


CmPCConfig_DeclSpec BOOL CfgGetRemoveableAudioInputDevices(vector<tstring>& vInputDevices, tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgSetDefaultAudioInputDevice(tstring tInputDevice,  tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgGetDefaultInputDeviceName(tstring& defaultDeviceName,  tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgTestCAudioInputDevices(tstring& tResponse);
