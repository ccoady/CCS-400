#pragma once
#include "CommonDefs.h"

class IAudioManagerDeviceChangeListener 
{
public:
	virtual void OnUpdateAudioInputList   (tstring& strInputList) = 0;
	virtual void OnUpdateAudioOutputList (tstring& strOutputList) = 0;
};