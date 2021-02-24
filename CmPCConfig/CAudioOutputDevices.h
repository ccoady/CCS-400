#include "CmPCConfig.h"
#include "CAudioOutputDevices.hpp"


CmPCConfig_DeclSpec BOOL CfgGetRemoveableAudioOutputDevices(vector<tstring>& vOutputDevices, tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgSetDefaultAudioOutputDevice(tstring tOutputDevice,  tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgGetDefaultOutputDeviceName(tstring& defaultDeviceName,  tstring& tResponse);

CmPCConfig_DeclSpec BOOL CfgTestCAudioOutputDevices(tstring& tResponse);
