//////////////////////////////////////////////////////////////////////
// AlexfMixer.cpp: implementation of the CAlexfMixer class.
// CAlexfMixer - simple mixer control wrapper
// Copyright (C) Alexander Fedorov 1999
// You may do whatever you want with this code, as long as you include this
// copyright notice in your implementation files.
// If you wish to add new classes to this collection, feel free to do so.
// But please send me your code so that I can update the collection.
// Comments and bug reports: lamer2000@usa.net
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "AlexfMixer.h"
#include <limits.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

void CAlexfMixer::ZeroAll()
{
	m_HMixer = NULL;
	mmr = MMSYSERR_NOERROR;
	m_dwChannels = 0;
	m_bSuccess = FALSE;
}

CAlexfMixer::CAlexfMixer(DWORD DstType, DWORD SrcType, DWORD ControlType)
{
	ZeroAll();
	if(mixerGetNumDevs() < 1) return;
	mmr = mixerOpen(&m_HMixer, 0, 0, 0L, CALLBACK_NULL);
	if (mmr != MMSYSERR_NOERROR) return;
// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
// DstType
	mxl.dwComponentType = DstType;
	if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)!= MMSYSERR_NOERROR) 
	return;
// SrcType
	if( SrcType != NO_SOURCE )
	{
		UINT nconn = mxl.cConnections;
		DWORD DstIndex = mxl.dwDestination;
		for( UINT j = 0; j < nconn; j++ )
		{
			mxl.cbStruct = sizeof( MIXERLINE );
			mxl.dwSource = j;
			mxl.dwDestination = DstIndex;
			
			if(mixerGetLineInfo( ( HMIXEROBJ )m_HMixer,
				&mxl, MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR) 
				return;
			
			if( mxl.dwComponentType == SrcType ) 
				break;
		}
	}
// get dwControlID
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = ControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &m_MixerControl;
	if (mixerGetLineControls((HMIXEROBJ)m_HMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR)
		return;
	m_dwChannels = mxl.cChannels;
	m_bSuccess = TRUE;
}

CAlexfMixer::CAlexfMixer(HWND hwnd, DWORD DstType, DWORD SrcType, DWORD ControlType)
{
	ZeroAll();
	if(mixerGetNumDevs() < 1) return;
	mmr = mixerOpen(&m_HMixer, 0, (DWORD)hwnd, 0L, CALLBACK_WINDOW);
	if (mmr != MMSYSERR_NOERROR) return;
// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
// DstType
	mxl.dwComponentType = DstType;
	if (mixerGetLineInfo((HMIXEROBJ)m_HMixer, &mxl, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)
	!= MMSYSERR_NOERROR) return;
// SrcType
	if( SrcType != NO_SOURCE )
	{
		UINT nconn = mxl.cConnections;
		DWORD DstIndex = mxl.dwDestination;
		for( UINT j = 0; j < nconn; j++ )
		{
			mxl.cbStruct = sizeof( MIXERLINE );
			mxl.dwSource = j;
			mxl.dwDestination = DstIndex;
			if(mixerGetLineInfo( ( HMIXEROBJ )m_HMixer,
			&mxl, MIXER_GETLINEINFOF_SOURCE ) != MMSYSERR_NOERROR) return;
			if( mxl.dwComponentType == SrcType ) break;
		}
	}
// get dwControlID
	MIXERLINECONTROLS mxlc;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = ControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &m_MixerControl;
	if (mixerGetLineControls((HMIXEROBJ)m_HMixer, &mxlc, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) return;
	m_dwChannels = mxl.cChannels;
	m_bSuccess = TRUE;
	
}

CAlexfMixer::~CAlexfMixer()
{
	if (m_HMixer) mixerClose(m_HMixer);
}

////////////////////////////////////////

DWORD CAlexfMixer::GetControlValue()
{
	if (!m_bSuccess) return 0;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u[2];

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_MixerControl.dwControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = &mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return 0;
	m_bSuccess = TRUE;
	return mxcd_u[0].dwValue;
}

bool CAlexfMixer::GetControlValues(DWORD *pdwChannel1, DWORD *pdwChannel2)
{
	if (!m_bSuccess) return FALSE;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u[2];

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_MixerControl.dwControlID;
	mxcd.cChannels = 2;
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = &mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return FALSE;
	m_bSuccess = TRUE;
	
	*pdwChannel1 = mxcd_u[0].dwValue;
	*pdwChannel2 = mxcd_u[1].dwValue;

	return TRUE;
}

BOOL CAlexfMixer::SetControlValue(DWORD dw)
{
	if (!m_bSuccess) return m_bSuccess;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u;

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_MixerControl.dwControlID;
	mxcd.cChannels = 1;	
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = &mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	
	mxcd_u.dwValue = dw;

	mmr = mixerSetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	m_bSuccess = TRUE;
	return m_bSuccess;
}

BOOL CAlexfMixer::SetControlValues(DWORD dwChannel1, DWORD dwChannel2)
{
	if (!m_bSuccess) return m_bSuccess;
	m_bSuccess = FALSE;
	MIXERCONTROLDETAILS mxcd;
	MIXERCONTROLDETAILS_UNSIGNED mxcd_u[2];

	mxcd.cbStruct = sizeof(mxcd);
	mxcd.dwControlID = m_MixerControl.dwControlID;
	mxcd.cChannels = 2;	
	mxcd.cMultipleItems = 0;
	mxcd.cbDetails = sizeof(mxcd_u);
	mxcd.paDetails = mxcd_u;
	mmr = mixerGetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	
	mxcd_u[0].dwValue = dwChannel1;
	mxcd_u[1].dwValue = dwChannel2;

	mmr = mixerSetControlDetails((HMIXEROBJ)m_HMixer, &mxcd, 0L);
	if (MMSYSERR_NOERROR != mmr) return m_bSuccess;
	m_bSuccess = TRUE;
	return m_bSuccess;
}

BOOL CAlexfMixer::On()
{
	return SetControlValue(0);
}

BOOL CAlexfMixer::Off()
{
	return SetControlValue(1);
}

//////////////////////////////////////////////
//
// Set Recording Control source be Microphone
//
//////////////////////////////////////////////
BOOL CAlexfMixer::SetRecordSelectMic()
{
	BOOL bRet = FALSE;
	DWORD dwMicIndex = ULONG_MAX;
	DWORD dwIndex = 0;
	if (m_HMixer == NULL)
	{
		return bRet;
	}
	// get dwLineID
	MIXERLINE mxl;
	mxl.cbStruct = sizeof(MIXERLINE);
	mxl.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
	if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_HMixer),
						   &mxl,
						   MIXER_OBJECTF_HMIXER |
						   MIXER_GETLINEINFOF_COMPONENTTYPE)
		!= MMSYSERR_NOERROR)
	{
		return bRet;
	}

	// get dwControlID
	MIXERCONTROL mxc;
	MIXERLINECONTROLS mxlc;
	DWORD dwControlType = MIXERCONTROL_CONTROLTYPE_MIXER;
	mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
	mxlc.dwLineID = mxl.dwLineID;
	mxlc.dwControlType = dwControlType;
	mxlc.cControls = 1;
	mxlc.cbmxctrl = sizeof(MIXERCONTROL);
	mxlc.pamxctrl = &mxc;
	if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_HMixer),
							   &mxlc,
							   MIXER_OBJECTF_HMIXER |
							   MIXER_GETLINECONTROLSF_ONEBYTYPE)
		!= MMSYSERR_NOERROR)
	{
		// if no mixer, try MUX
		dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
		mxlc.cbStruct = sizeof(MIXERLINECONTROLS);
		mxlc.dwLineID = mxl.dwLineID;
		mxlc.dwControlType = dwControlType;
		mxlc.cControls = 1;
		mxlc.cbmxctrl = sizeof(MIXERCONTROL);
		mxlc.pamxctrl = &mxc;
		if (::mixerGetLineControls(reinterpret_cast<HMIXEROBJ>(m_HMixer),
								   &mxlc,
								   MIXER_OBJECTF_HMIXER |
								   MIXER_GETLINECONTROLSF_ONEBYTYPE)
			!= MMSYSERR_NOERROR)
		{
			return bRet;
		}
	}

	DWORD dwSelectControlID = mxc.dwControlID;
	DWORD dwMultipleItems = mxc.cMultipleItems;

	if (dwMultipleItems == 0)
	{
		return bRet;
	}

	// get the index of the Microphone Select control
	MIXERCONTROLDETAILS_LISTTEXT *pmxcdSelectValue = new MIXERCONTROLDETAILS_LISTTEXT[dwMultipleItems];
	MIXERCONTROLDETAILS mxcd;
	mxcd.cbStruct = sizeof(MIXERCONTROLDETAILS);
	mxcd.dwControlID = dwSelectControlID;
	mxcd.cChannels = 1;
	mxcd.cMultipleItems = dwMultipleItems;
	mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
	mxcd.paDetails = pmxcdSelectValue;
	if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_HMixer),
								 &mxcd,
								 MIXER_OBJECTF_HMIXER |
								 MIXER_GETCONTROLDETAILSF_LISTTEXT)
		== MMSYSERR_NOERROR)
	{
		// determine which controls the Microphone source line
		for (dwIndex = 0; dwIndex < dwMultipleItems; dwIndex++)
		{
			// get the line information
			MIXERLINE mxl;
			mxl.cbStruct = sizeof(MIXERLINE);
			mxl.dwLineID = pmxcdSelectValue[dwIndex].dwParam1;
			if (::mixerGetLineInfo(reinterpret_cast<HMIXEROBJ>(m_HMixer),
								   &mxl,
								   MIXER_OBJECTF_HMIXER |
								   MIXER_GETLINEINFOF_LINEID)
				== MMSYSERR_NOERROR &&
				mxl.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE)
			{
				// found microphone index.
				dwMicIndex = dwIndex;
				break;
			}
		}

		if (dwMicIndex >= dwMultipleItems)
		{
			// fail to find it using line IDs, some mixer drivers have
			// different meaning for MIXERCONTROLDETAILS_LISTTEXT.dwParam1.
			// let's try comparing the item names.
			for (dwIndex = 0; dwIndex < dwMultipleItems; dwIndex++)
			{
				if (::lstrcmp(pmxcdSelectValue[dwIndex].szName,
							  _T("Microphone")) == 0)
				{
					// found microphone index.
					dwMicIndex = dwIndex;
					break;
				}
			}
		}

		if(dwMicIndex < dwMultipleItems)
		{
			
			// get all the fValues
			MIXERCONTROLDETAILS_BOOLEAN *pmxcdMicValue = new MIXERCONTROLDETAILS_BOOLEAN[dwMultipleItems];
			mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
			mxcd.paDetails = pmxcdMicValue;
			if (::mixerGetControlDetails(reinterpret_cast<HMIXEROBJ>(m_HMixer),
										 &mxcd,
										 MIXER_OBJECTF_HMIXER |
										 MIXER_GETCONTROLDETAILSF_VALUE)
				== MMSYSERR_NOERROR)
			{
				// set the Microphone value
				if(pmxcdMicValue[dwMicIndex].fValue == 0)
				{
					for(dwIndex = 0; dwIndex < dwMultipleItems; dwIndex++)
					{
						pmxcdMicValue[dwIndex].fValue = 0;
					}
					pmxcdMicValue[dwMicIndex].fValue = 1;

					mxcd.paDetails = pmxcdMicValue;
					if (::mixerSetControlDetails(reinterpret_cast<HMIXEROBJ>(m_HMixer),
												 &mxcd,
												 MIXER_OBJECTF_HMIXER |
												 MIXER_SETCONTROLDETAILSF_VALUE)
						== MMSYSERR_NOERROR)
					{
						bRet = TRUE;
					}
				}
			}

			delete []pmxcdMicValue;
		}
	}

	delete []pmxcdSelectValue;

	return bRet;
}
