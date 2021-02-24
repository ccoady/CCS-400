////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        AudioCore.h
///
/// Header file for Core Audio configuration functions.
///
/// @author      William Levine
/// @date        02/23/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_AUDIOCORE_H_)
#define _AUDIOCORE_H_

#ifndef UNDER_CE
#include "MixerControl\AlexfMixer.h"	// open source class to work with generic mixer controls.
#include <map>
#include "AudioDefs.h"

typedef std::map<EAudioCtrls, CAlexfMixer*>	mixer_ctrl_map;

class CMixerCtrlMap
{
public:
	CMixerCtrlMap()		{}
	~CMixerCtrlMap();

	static CAlexfMixer *GetMixerCtrlClass(EAudioCtrls eCtrl);

protected:
	static 	mixer_ctrl_map MixerCtrlMap;
};


#endif
#endif