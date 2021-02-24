////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        AudioDefs.h
///
/// Header file for definitions common to external and core functions.
///
/// @author      William Levine
/// @date        02/26/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_AUDIODEFS_H_)
#define _AUDIODEFS_H_



#ifndef AUDIO_MIC_BOOST_ON
	#define AUDIO_MIC_BOOST_ON	17339
	#define AUDIO_MIC_BOOST_OFF 17340
#endif

enum EAudioCtrls
{
	EAudioCtrl_Null = 0,

	EAudioCtrl_MasterVolume = 1,
	EAudioCtrl_MasterMute,		//	AUDIO LOCAL and DGE-1 Analog Output MASTER VOLUME ON/OFF
	EAudioCtrl_BassBoost,
	EAudioCtrl_TrebleBoost,

	EAudioCtrl_WaveVolume,
	EAudioCtrl_WaveMute,
	
	EAudioCtrl_LineInVolume,	//	also for DGE-1 analog input
	EAudioCtrl_LineInMute,		//	also for DGE-1 analog input
	
	EAudioCtrl_MicVolume,
	EAudioCtrl_MicMute,
	EAudioCtrl_MicBoost,
	EAudioCtrl_MicStero,

	EAudioCtrl_LineInRecVol,
	EAudioCtrl_LineInRecSel,
	EAudioCtrl_StereoMixSel,
	EAudioCtrl_StereoMixVol,
	EAudioCtrl_DigitalOutput,
//#if	_DGE1
//	EAudioCtrl_InputMixer,			//	DGE-1 Audio Analog INPUT Mixer/playback - needed for future?
//#endif
};

enum EAudioLines
{
	EAudioLine_Null = 0,
	EAudioLine_Master = 1,	//	also for DGE-1 Analog Output
	EAudioLine_Wave,
	EAudioLine_LineIn,		//	also for DGE-1 Analog Input Line-in
	EAudioLine_Mic,
	EAudioLine_LineInRec,
};

#define AUDIOLINE_TO_ENDPOINT_MAP _T("AudioLineToEndpointMap")

#endif
