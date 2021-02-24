#ifndef _AudioMixerManagerWifi_H
#define _AudioMixerManagerWifi_H

// /////////////////////////////////////////////////////////////////////////////////////
/** 
 	\file AudioMixerManagerWifi.h
	\brief Manages multiple audio mixer devices
*/
// /////////////////////////////////////////////////////////////////////////////////////

#include <windows.h>
#include <mmsystem.h>

#include <vector>

#include "AudioMixerManager.h"
#include "UPXPanelDef.h"


using namespace std;

class CWinAudioMixer;


// /////////////////////////////////////////////////////////////////////////////////////
//  IMPLEMENTATION: CAudioMixerManagerWifi
// /////////////////////////////////////////////////////////////////////////////////////

//
//	For panels with Realtek HD Audio chipsets, there is more than one mixer.  One for
//	outputs and one for inputs.  This class will look specifically for the input mixer
//	and connect to it as well as the default output mixer.  We then redirect the input
//	functions to the new input-only mixer.
//
//	10/2010 - jmv
//



/// Implements the IAudioMixerManager interface for managing the Windows audio mixer
class CAudioMixerManagerWifi : public CAudioMixerManager {
  public:
  	//ctors
    CAudioMixerManagerWifi();

    virtual ~CAudioMixerManagerWifi();

    virtual bool Init(LPCTSTR szInternalDeviceName, bool &bUnitDocked);

	/// Shuts down an instance of this class and cleans up the mess
    virtual bool Destroy();

	/// Returns a pointer to the current CWinAudioMixer object for audio input (should always be the internal audio device)
    virtual CWinAudioMixer *GetCurrentAudioMixerMicInput();
	
	///////////////////////////////////////////////////////////////////////////////////
	//	OVERRIDES:	These input functions are modified to point to the correct mixer
	///////////////////////////////////////////////////////////////////////////////////
	

	/// Set the mic gain level
    virtual bool SetMicGain(const int leftVolume, const int rightVolume);

	/// Set the mic gain level
    virtual bool SetMicGain(const int volume);

	/// Get the mic gain level
    virtual bool GetMicGain(int &leftVolume, int &rightVolume);
	
	/// Get the bounds for the mic	
	virtual bool GetMicBounds(int &min, int &max);
	
	/// Set the mic mute
    virtual bool SetMicMute(const bool muteSetting);

	/// Get the mic mute
    virtual bool GetMicMute();
	
	/// Set the mic boost setting (not all sound cards support this).
    virtual bool SetMicBoost(const bool setting);

	/// Get the mic boost setting (not all sound cards support this).
    virtual bool GetMicBoost();
	
	/// Set true to use stereo format for a mic input
    virtual bool SetMicStereo(const bool setting);

	/// Get the state of the mic stereo format flag
    virtual bool GetMicStereo();
	
	/// Set the line input gain level (stereo)
    virtual bool SetLineInputGain(const int leftVolume, const int rightVolume);

	/// Set the line gain level (mono)
    virtual bool SetLineInputGain(const int volume);

	/// Get the line input gain level
    virtual bool GetLineInputGain(int &leftVolume, int &rightVolume);
	
	/// Get the bounds for the line input	
	virtual bool GetLineInputBounds(int &min, int &max);
	
	/// Set the line input mute
    virtual bool SetLineInputMute(const bool muteSetting);

	/// Get the line input mute state
    virtual bool GetLineInputMute();	


  protected:
	/// The CWinAudioMixer instance which will handle the output audio device (if it is different from input)
	CWinAudioMixer		*m_mixerInput;

	// ///////////////////////////////////////////////////////////////////////////////
	/**
	* \brief Enumerate through all audio mixers currently present on this system. 
	* \return The number of audio mixers present
	* \param[out] deviceMatched Will be set true if the specific device is found, false otherwise.
    *
	*  We enumerate through all devices looking for the target device specified to the Init method.
	* \note The caller should check the m_nMixerDevices value to make sure there is at least one audio mixer in the sytem.
	*/
	// ///////////////////////////////////////////////////////////////////////////////
	UINT EnumerateAudioMixers();
	
};
#endif

