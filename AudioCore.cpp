////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        AudioCore.cpp
///
/// Implementation file for Core Audio configuration functions.
///
/// @author      William Levine
/// @date        02/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "AudioCore.h"
#include "AudioDefs.h"
#include <utility>

#ifndef UNDER_CE

using std::map;
using std::make_pair;
using std::pair;

// definition of static class member;
mixer_ctrl_map CMixerCtrlMap::MixerCtrlMap;

// we declare a single global instance of the class so the destrctor will be called when the dll is closed
CMixerCtrlMap  GlobalInstanceToCallDestructor;
	
// all the mixer info for our various controls
int MixerCtrlInfo[][4] = {
	{EAudioCtrl_MasterVolume, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_VOLUME},
	{EAudioCtrl_MasterMute, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_MUTE},
	{EAudioCtrl_WaveVolume, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MIXERCONTROL_CONTROLTYPE_VOLUME},
	{EAudioCtrl_WaveMute, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT, MIXERCONTROL_CONTROLTYPE_MUTE},

	{EAudioCtrl_LineInVolume, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME},
	{EAudioCtrl_LineInMute, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_LINE, MIXERCONTROL_CONTROLTYPE_MUTE},
	{EAudioCtrl_MicVolume, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, MIXERCONTROL_CONTROLTYPE_VOLUME},
	{EAudioCtrl_MicMute, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, MIXERCONTROL_CONTROLTYPE_MUTE},
	//{EAudioCtrl_MicMute, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, MIXERCONTROL_CONTROLTYPE_MUX},

	// the following control are untested as they do not work on the development machine.
	//		this may not be the proper way to go about this.
	{EAudioCtrl_BassBoost, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_BASS},
	{EAudioCtrl_TrebleBoost, MIXERLINE_COMPONENTTYPE_DST_SPEAKERS, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_TREBLE},
	{EAudioCtrl_MicBoost, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, MIXERCONTROL_CONTROLTYPE_ONOFF},
	{EAudioCtrl_MicStero, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE, MIXERCONTROL_CONTROLTYPE_LOUDNESS},

	{EAudioCtrl_LineInRecVol, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, MIXERLINE_COMPONENTTYPE_SRC_LINE, MIXERCONTROL_CONTROLTYPE_VOLUME},
	{EAudioCtrl_LineInRecSel, MIXERLINE_COMPONENTTYPE_DST_WAVEIN, NO_SOURCE, MIXERCONTROL_CONTROLTYPE_MUX},

	{0}	};

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Destructor deallocates all allocated Mixer control classes.
///
/// @author      William Levine
/// @date        02/26/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CMixerCtrlMap::~CMixerCtrlMap()
{
mixer_ctrl_map::iterator it = MixerCtrlMap.begin();

	while (it != MixerCtrlMap.end())
	{
		delete (*it).second;
		it++;
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Checks the map to see if a class exists for the control specified.  If it does not, it is
/// allocated, and put into the map.  The pointer wither way is returned for use.
///
/// @param       eCtrl        
///
/// @author      William Levine
/// @date        02/26/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CAlexfMixer *CMixerCtrlMap::GetMixerCtrlClass(EAudioCtrls eCtrl)
{
mixer_ctrl_map::iterator it;
CAlexfMixer *pMixer = NULL;

	it = MixerCtrlMap.find(eCtrl);
	
	if ( (it == MixerCtrlMap.end()) || (it->second == NULL) )
	{
	int iInfoIdx = 0;
		
		while ((MixerCtrlInfo[iInfoIdx][0]) && (MixerCtrlInfo[iInfoIdx][0] != eCtrl))
			iInfoIdx++;

		if (MixerCtrlInfo[iInfoIdx][0])
		{
			pMixer = new CAlexfMixer(MixerCtrlInfo[iInfoIdx][1], MixerCtrlInfo[iInfoIdx][2], MixerCtrlInfo[iInfoIdx][3]);
			
			if (it == MixerCtrlMap.end())
				MixerCtrlMap.insert(make_pair(eCtrl, pMixer->IsOk() ? pMixer : NULL));
			else
				it->second = pMixer->IsOk() ? pMixer : NULL;
		}
	}
	else
		pMixer = (*it).second;

	return pMixer;
}


#endif