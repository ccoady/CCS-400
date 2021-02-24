//////////////////////////////////////////////////////////////////////
// AlexfMixer.h: interface for the CAlexfMixer class.
// CAlexfMixer - simple mixer control wrapper
// Copyright (C) Alexander Fedorov 1999
// You may do whatever you want with this code, as long as you include this
// copyright notice in your implementation files.
// If you wish to add new classes to this collection, feel free to do so.
// But please send me your code so that I can update the collection.
// Comments and bug reports: lamer2000@usa.net
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ALEXFMIXER_H__4FC2F321_07F3_11D3_9864_CB4A97E7D6CA__INCLUDED_)
#define AFX_ALEXFMIXER_H__4FC2F321_07F3_11D3_9864_CB4A97E7D6CA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <mmsystem.h>

// Thanks to Langis Pitre
#define NO_SOURCE ((MIXERLINE_COMPONENTTYPE_SRC_LAST + 1))

class CAlexfMixer
{
protected:
	HMIXER m_HMixer;
	MMRESULT mmr;
	DWORD m_dwChannels;
	BOOL m_bSuccess;

	MIXERCONTROL	m_MixerControl;

	void ZeroAll();
public:
	BOOL IsOk() {return m_bSuccess;};
	BOOL On();
	BOOL Off();
	
	DWORD GetControlValue();
	BOOL SetControlValue(DWORD dw);

	bool GetControlValues(DWORD *pdwChannel1, DWORD *pdwChannel2);
	BOOL SetControlValues(DWORD dwChannel1, DWORD dwChannel2);
	BOOL SetRecordSelectMic();

	const MIXERCONTROL *GetMixerControlInfo(void)	{return &m_MixerControl;}

	CAlexfMixer(DWORD DstType, DWORD SrcType, DWORD ControlType);
	CAlexfMixer(HWND hwnd, DWORD DstType, DWORD SrcType, DWORD ControlType);
	virtual ~CAlexfMixer();
};

#endif // !defined(AFX_ALEXFMIXER_H__4FC2F321_07F3_11D3_9864_CB4A97E7D6CA__INCLUDED_)
