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
#include "AudioMixerManagerWifi.h"

/// Constructor
CAudioMixerManagerWifi::CAudioMixerManagerWifi() 
{	
	m_mixerInput = NULL;
}

/// Destructor
CAudioMixerManagerWifi::~CAudioMixerManagerWifi() 
{
	Destroy();
}

bool CAudioMixerManagerWifi::Init(LPCTSTR szInternalDeviceName, bool &bUnitDocked) 
{
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

bool CAudioMixerManagerWifi::Destroy() 
{
	CAudioMixerManager::Destroy();
	
	if (m_mixerInput) {
		m_mixerInput->Destroy();
		delete m_mixerInput;
		m_mixerInput = NULL;
	}
	
	return true;
}


CWinAudioMixer * CAudioMixerManagerWifi::GetCurrentAudioMixerMicInput() 
{
	if (m_mixerInput)
		return m_mixerInput;	// If we have a specific input mixer, return that

	return m_mixerInternal;		//Otherwise, return the internal device for the internal mics
}



bool CAudioMixerManagerWifi::SetMicGain(const int leftVolume, const int rightVolume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::SetMicGain(const int volume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicGain(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetMicGain(int &leftVolume, int &rightVolume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetMicBounds(int &min, int &max) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicBounds(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManagerWifi::SetMicMute(const bool muteSetting) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetMicMute() 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicMute(); 
	}
	else {
		return false;
	}
}
	
bool CAudioMixerManagerWifi::SetMicBoost(const bool setting) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicBoost(setting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetMicBoost() 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerOutput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicBoost(); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::SetMicStereo(const bool setting) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetMicStereo(setting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetMicStereo() 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetMicStereo(); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::SetLineInputGain(const int leftVolume, const int rightVolume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::SetLineInputGain(const int volume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputGain(volume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetLineInputGain(int &leftVolume, int &rightVolume) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputGain(leftVolume, rightVolume); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetLineInputBounds(int &min, int &max) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputBounds(min, max); 
	}
	else {
		return false;
	}
}


bool CAudioMixerManagerWifi::SetLineInputMute(const bool muteSetting) 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->SetLineInputMute(muteSetting); 
	}
	else {
		return false;
	}
}

bool CAudioMixerManagerWifi::GetLineInputMute() 
{
	CWinAudioMixer		*mixerIF = GetCurrentAudioMixerMicInput();
	ATLASSERT(mixerIF);
	
	if (mixerIF) {
		return mixerIF->GetLineInputMute(); 
	}
	else {
		return false;
	}
}

/// Enumerate through all our mixers and find the one that interests us
UINT CAudioMixerManagerWifi::EnumerateAudioMixers() 
{
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
			if (!_tcsncicmp(mixcaps.szPname, m_szInternalMixerName, MAXPNAMELEN)) 
			{
				//We have a match, this must be the internal device, but only init if we don't already have an instance
				if (NULL == m_mixerInternal) 
				{
					m_mixerInternal = new CWinAudioMixer();
					if (!m_mixerInternal->Init(m_szInternalMixerName, bDeviceMatched)) 
					{
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance - Internal\r\n");
					}
				}
			}
			else if (!_tcsncicmp(mixcaps.szPname, _T("Realtek HD Audio Input"), MAXPNAMELEN))
			{
				// For the 8XGA panel, we've got a separate mixer for the input and output
				if (NULL == m_mixerInput)
				{
					m_mixerInput = new CWinAudioMixer();
					if (!m_mixerInput->Init(_T("Realtek HD Audio Input"), bDeviceMatched))
					{
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance - Input\r\n");
					}
				}
			}
			else 
			{
				//No match, assume this is the external device, but only init if we don't already have an instance
				//This means that if we have more than one "external" device, we will only init the first one
				if (NULL == m_mixerExternal) 
				{
					m_mixerExternal = new CWinAudioMixer();
					if (!m_mixerExternal->Init(mixcaps.szPname, bDeviceMatched)) 
					{
						ATLTRACE("CAudioMixerManager::EnumerateAudioMixers - Error creating CWinAudioMixer instance - External\r\n");
					}
					m_currentOutputExternal = true;	//TODO: We need to find out if we should automatically switch here
				}
			}
		}
	}
	
	return count;
}