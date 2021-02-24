#include "stdafx.h"

#include "AudioManager.h"

////////////////////////////////////////////////////////////////////////////////
// Function:	
//
// Parameters:  
//
// Returns:     
//              
//              
// 
// Description: 
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void ACDescriptorWrapper::SetVolume (int left, int right)
{
	if (IsValid())
	{
		m_pRecord->pDevice->SetVolume(left, right);
	}
	CacheVolume(left, right);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	
//
// Parameters:  
//
// Returns:     
//              
//              
// 
// Description: 
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void ACDescriptorWrapper::SetMute (bool bMuteVal)
{
	if (IsValid())
	{
		bool bMuted = m_pRecord->pDevice->GetMute();
        if (bMuted != bMuteVal)
        {
            m_pRecord->pDevice->SetMute(bMuteVal);
        }
	}
	CacheMute(bMuteVal);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	
//
// Parameters:  
//
// Returns:     
//              
//              
// 
// Description: 
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void ACDescriptorWrapper::PushCachedValues()
{
	DWORD tid = GetCurrentThreadId();
	int nChannels = m_pRecord->pDevice->GetNumChannels();
	if (nChannels > 1)
	{
		BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s setVol() l=%d, r=%d"), 
			tid,
			_AUDIOFUNC_,
			m_pRecord->iLastVolumeLeft,
			m_pRecord->iLastVolumeRight);

		m_pRecord->pDevice->SetVolume(m_pRecord->iLastVolumeLeft, m_pRecord->iLastVolumeRight);
	}
	{
		m_pRecord->pDevice->SetVolume(m_pRecord->iLastVolumeRight);
	}

	m_pRecord->pDevice->SetMute(m_pRecord->bSetMute);
}



