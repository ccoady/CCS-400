#pragma once
#include "audiomanager.h"
class CmPCConfig_DeclSpec CAudioManagerCCS100 : public CAudioManager
{
public:
	CAudioManagerCCS100(void);
	~CAudioManagerCCS100(void);
	void Init(HWND hwnd, IAudioDeviceChangeListener* pListener);
	virtual void LoadAudioDeviceMap();

	// Master or main volume
	virtual bool SetMasterVolumeLevel(const int volume);
	virtual bool SetMasterVolumeLevel(const int leftVolume, const int rightVolume);
	virtual bool GetMasterVolumeLevel(int &leftVolume, int &rightVolume);
	virtual bool SetMasterMute(const bool muteSetting);
    virtual bool GetMasterMute();

	// Mic settings (to be deleted)
	virtual bool GetMicGain(int &leftVolume, int &rightVolume);
	virtual bool SetMicGain(const int leftVolume, const int rightVolume);
    virtual bool SetMicGain(const int volume);
    virtual bool GetMicMute();
	virtual bool SetMicMute(const bool muteSetting);
};

