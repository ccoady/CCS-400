// /////////////////////////////////////////////////////////////////////////////////////
/** 
 	\file AudioMixerManager.cpp
	\brief Contains the implementation of the CAudioMixerManager class
	
*/
// /////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include <windows.h>
#include <mmsystem.h>

#include "WinAudioMixer.h"
#include "AudioMixerManager.h"

/// Constructor
CAudioMixerManager::CAudioMixerManager() {
	m_mixerInternal = NULL;
	m_mixerExternal = NULL;
	m_currentOutputExternal = false;
	m_szInternalMixerName[0] = _TCHAR('\0');
}

/// Destructor
CAudioMixerManager::~CAudioMixerManager() {
	Destroy();
}
bool CAudioMixerManager::InitOnlyOneMixer(LPCTSTR szDevName, bool &bUnitDocked) 
{
	MIXERCAPS 	mixcaps;
	MMRESULT 	mmr = 0;
	bool        bDeviceMatched = false;
	UINT count = mixerGetNumDevs();

	if (count < 1)
		return 0;

	for (UINT i = 0; i < count; i++) 
	{
		ZeroMemory(&mixcaps, sizeof(mixcaps)); 
		mmr = mixerGetDevCaps(i, &mixcaps, sizeof(mixcaps));
		ATLASSERT(MMSYSERR_NOERROR == mmr);		//Should never fail
		if (MMSYSERR_NOERROR == mmr) 
		{
			if (!_tcsncicmp(mixcaps.szPname, szDevName, MAXPNAMELEN)) 
			{
				//We have a match, this must be the internal device, but only init if we don't already have an instance
				if (NULL == m_mixerInternal) 
				{
					m_szInternalMixerName;;
					_tcsncpy(m_szInternalMixerName, szDevName, MAXPNAMELEN);
					m_mixerInternal = new CWinAudioMixer();
					if (!m_mixerInternal->Init(m_szInternalMixerName, bDeviceMatched)) 
					{
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance\r\n");
						bDeviceMatched = false;
						break;
					}
					else
					{
						bDeviceMatched = true;
						break;
					}
				}
			}

		}
	}

	return bDeviceMatched;
}

bool CAudioMixerManager::Init(LPCTSTR szInternalDeviceName, bool &bUnitDocked) {
	bUnitDocked = false;
	UINT numDevs = 0;
	
	if (_tcsnlen(szInternalDeviceName, MAXPNAMELEN) > 0) {
		//A device name was passed in, lets use it to find our target device
		_tcsncpy(m_szInternalMixerName, szInternalDeviceName, MAXPNAMELEN);
		numDevs = EnumerateAudioMixers();
	}
	
	if (NULL == m_mixerInternal) {
		//No internal device found, just grab the first we find as the internal mixer
		numDevs = EnumerateFirstAudioMixer();
	}
	
	if (numDevs > 1) {
		//More than one device is present
		if (m_mixerInternal && m_mixerExternal) {
			//We've initialized both internal & external devices, assume that we are docked.
			bUnitDocked = true;
		}
	}
	
	return true;	
}

bool CAudioMixerManager::Destroy() {
	if (m_mixerExternal) {
		m_mixerExternal->Destroy();
		delete m_mixerExternal;
		m_mixerExternal = NULL;
	}
	
	if (m_mixerInternal) {
		m_mixerInternal->Destroy();
		delete m_mixerInternal;
		m_mixerInternal = NULL;
	}
	
	return true;
}

UINT CAudioMixerManager::GetAudioMixerCount() {
	return mixerGetNumDevs();
}

bool CAudioMixerManager::DeviceUndocked() {
	if (m_mixerExternal) {
		m_mixerExternal->Destroy();
		delete m_mixerExternal;
		m_mixerExternal = NULL;
	}
	
	m_currentOutputExternal = false;
	
	return true;
}

bool CAudioMixerManager::DeviceDocked() {
	//We just re-enumerate. This method will only set the instances that have not already been set. So, only the external device should appear here  
	EnumerateAudioMixers();
	
	return true;
}

bool CAudioMixerManager::IsAudioExternal() {
	return m_currentOutputExternal;
}

CWinAudioMixer * CAudioMixerManager::GetCurrentAudioMixerOutput() {
	if (m_currentOutputExternal && m_mixerExternal) {
		return m_mixerExternal;
	}
	
	return m_mixerInternal;
}

CWinAudioMixer * CAudioMixerManager::GetCurrentAudioMixerMicInput() {
	return m_mixerInternal;	//Always return the internal device for the internal mics
}

// ***** The following methods will all call into CWinAudioMixer to do the actual work
LPCTSTR CAudioMixerManager::GetMixerDeviceName() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMixerDeviceName(); 
	}
	else {
		return _T("");
	}
}

bool CAudioMixerManager::SetMasterVolumeLevel(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMasterVolumeLevel(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetMasterVolumeLevel(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMasterVolumeLevel(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMasterVolumeLevel(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMasterVolumeLevel(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMasterVolumeBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMasterVolumeBounds(min, max); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetMasterMute(const bool muteSetting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMasterMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMasterMute() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMasterMute(); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetWaveOutVolumeLevel(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetWaveOutVolumeLevel(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetWaveOutVolumeLevel(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetWaveOutVolumeLevel(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetWaveOutVolumeLevel(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetWaveOutVolumeLevel(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetWaveOutBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetWaveOutBounds(min, max); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetWaveOutMute(const bool muteSetting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetWaveOutMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetWaveOutMute() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetWaveOutMute(); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManager::SetMicGain(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetMicGain(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicGain(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMicGain(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMicBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicBounds(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManager::SetMicMute(const bool muteSetting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMicMute() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicMute(); 
	}
	else {
		return false;
	}
}
	
bool CAudioMixerManager::SetMicBoost(const bool setting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicBoost(setting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMicBoost() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicBoost(); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetMicStereo(const bool setting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicStereo(setting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetMicStereo() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicStereo(); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetLineInputGain(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetLineInputGain(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputGain(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetLineInputGain(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetLineInputBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputBounds(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManager::SetLineInputMute(const bool muteSetting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetLineInputMute() {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputMute(); 
	}
	else {
		return false;
	}
}

// ***** Tone controls
bool CAudioMixerManager::SetToneTreble(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetToneTreble(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetToneTreble(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetToneTreble(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetToneTreble(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetToneTreble(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetToneTrebleBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetToneTrebleBounds(min, max); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetToneBass(const int leftVolume, const int rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetToneBass(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetToneBass(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetToneBass(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetToneBass(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetToneBass(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetToneBassBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetToneBassBounds(min, max); 
	}
	else {
		return false;
	}
}


/// Enumerate through all our mixers and find the one that interests us
UINT CAudioMixerManager::EnumerateAudioMixers() {
	UINT count = mixerGetNumDevs();
	if (count < 1)
		return 0;

	MIXERCAPS 	mixcaps;
	MMRESULT 	mmr = 0;
	bool		bDeviceMatched = false;
	
	for (UINT i = 0; i < count; i++) {
		ZeroMemory(&mixcaps, sizeof(mixcaps)); 
		mmr = mixerGetDevCaps(i, &mixcaps, sizeof(mixcaps));
		ATLASSERT(MMSYSERR_NOERROR == mmr);		//Should never fail
		if (MMSYSERR_NOERROR == mmr) {
			if (!_tcsncicmp(mixcaps.szPname, m_szInternalMixerName, MAXPNAMELEN)) {
				//We have a match, this must be the internal device, but only init if we don't already have an instance
				if (NULL == m_mixerInternal) {
					m_mixerInternal = new CWinAudioMixer();
					if (!m_mixerInternal->Init(m_szInternalMixerName, bDeviceMatched)) {
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance\r\n");
					}
				}
			}
			else {
				//No match, assume this is the external device, but only init if we don't already have an instance
				//This means that if we have more than one "external" device, we will only init the first one
				if (NULL == m_mixerExternal) {
					m_mixerExternal = new CWinAudioMixer();
					if (!m_mixerExternal->Init(mixcaps.szPname, bDeviceMatched)) {
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance\r\n");
					}
					m_currentOutputExternal = true;	//TODO: We need to find out if we should automatically switch here
				}
			}
		}
	}
	
	return count;
}

/// Just grab the first mixer that we see
UINT CAudioMixerManager::EnumerateFirstAudioMixer() {
	UINT count = mixerGetNumDevs();
	if (count < 1)
		return 0;

	MIXERCAPS 	mixcaps;
	MMRESULT 	mmr = 0;
	bool		bDeviceMatched = false;
	
	for (UINT i = 0; i < count; i++) {
		ZeroMemory(&mixcaps, sizeof(mixcaps)); 
		mmr = mixerGetDevCaps(i, &mixcaps, sizeof(mixcaps));
		ATLASSERT(MMSYSERR_NOERROR == mmr);		//Should never fail
		if (MMSYSERR_NOERROR == mmr) {
			//We found a mixer, lets assume this is our one and only audio mixer
			if (NULL == m_mixerInternal) {
				m_mixerInternal = new CWinAudioMixer();
				if (!m_mixerInternal->Init(mixcaps.szPname, bDeviceMatched)) {
					ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance\r\n");
				}
				m_currentOutputExternal = false;	//Since no device name was given, we can't tell internal from external
			}
		}
	}
	
	return count;
}


bool CAudioMixerManager::GetRecLineInVolumeLevel(int &leftVolume, int &rightVolume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetRecLineInVolumeLevel(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::GetRecLineInVolumeBounds(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetRecLineInVolumeBounds(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManager::SetRecLineInVolumeLevel(int &min, int &max) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetRecLineInVolumeLevel(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManager::SetRecLineInVolumeLevel(const int volume) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetRecLineInVolumeLevel(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SelectInputOnMux(const TCHAR *szInputName) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		
		return mixerIF->SelectInputOnMux(szInputName);
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetDigitalOutputMute(const bool muteSetting) {
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetDigitalOutputMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManager::SetStereoMixVol(const int iValue) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);

	if (mixerIF) 
	{
		return mixerIF->SetStereoMixVol(iValue); 
	}

	return false;
}