#include "stdafx.h"
#include "AudioManagerCCS100.h"
#include "system.h"

CAudioManagerCCS100::CAudioManagerCCS100(void)
{
}


CAudioManagerCCS100::~CAudioManagerCCS100(void)
{
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
void CAudioManagerCCS100::LoadAudioDeviceMap()
{

    BaseLogger::WriteLog(TRACESTACK | TRACE_AUDIO, _T("CAudioManager::LoadDeviceMap IN"));
    USES_CONVERSION;
    _TCHAR tszMasterDevice[MAX_PATH]   = {0};
    _TCHAR tszMasterAdapter[MAX_PATH]   = {0};

    _TCHAR tszHdmiDevice[MAX_PATH]   = {0};
    _TCHAR tszHdmiAdapter[MAX_PATH]   = {0};

    __try{
        // read each audio line from the config file.
        if(m_tszIniFilePathName[0] != _T('\0'))
        {
            // Load by Friendly Name + Device Name
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("MasterDevice"), NULL, tszMasterDevice, _MAX_PATH, m_tszIniFilePathName);
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("MasterAdapter"), NULL, tszMasterAdapter, _MAX_PATH, m_tszIniFilePathName);
        }

		// Initialize pointers
        m_pMasterVolume = GetOutputDeviceByName(tszMasterDevice, tszMasterAdapter);
		BaseLogger::WriteFormattedLog(TRACE_AUDIO, _T("Master Volume device set to 0x%8.8x"), m_pMasterVolume);
        
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManager::LoadDeviceMap OUT"));
    }
    __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation()))
	{
        BaseLogger::WriteLog(_T("CAudioManager::LoadAudioDeviceMap Caught Exception OUT"));
    }
}

/// Set the master output volume level. 
bool CAudioManagerCCS100::SetMasterVolumeLevel(const int volume)
{
    USES_CONVERSION;

    try{
        if (!m_bInitialized)
        {
            BaseLogger::WriteFormattedLog(_T("%s - not initialized"), __WFUNCTION__);
            return false;
        }

        if (m_pMasterVolume) {
            return m_pMasterVolume->SetVolume(volume);
        }
        BaseLogger::WriteLog( _T("SetMasterVolumeLevel(vol) error"));
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::SetMasterVolumeLevel(vol) Exception"));
        return false;
    }
    return false;

}


void CAudioManagerCCS100::Init(HWND hwnd, IAudioDeviceChangeListener* pListener)
{
    m_hwnd = hwnd;
   // g_this = this;

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("CAudioManager() ::Init IN - tid = (0x%x)"), ::GetCurrentThreadId());

    // return if already initialized
    if (m_bInitialized)
        return;

    //__try{
        memset (m_tszFirmwarePath, '0', MAX_PATH *sizeof (TCHAR));
        memset (m_tszIniFilePathName, '0', 2048 *sizeof (TCHAR));

		CfgGetFirmwarePath(m_tszFirmwarePath);
        _stprintf_s(m_tszIniFilePathName, 2048, _T("%s\\%s ThreadID %d"), m_tszFirmwarePath,_T("UpxTaskMgr.ini"), ::GetCurrentThreadId());
        BaseLogger::WriteLog(TRACESTACK, m_tszIniFilePathName);

        CoInitialize(NULL);
        m_DeviceChangeNotifier = new CMMNotificationClient(NULL);
        m_pListener = pListener;
        m_DeviceChangeNotifier->Init(pListener);
		//CreateAudioChannelDescriptorManager();

        // Initialize the device objects
        EnumDevices();
        //
        m_bInitialized = true;
    //}
  //  __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation())){
  //      BaseLogger::WriteFormattedLog(_T("CAudioManager::Init Exception %d"), GetExceptionCode());
  //  }

    // CoUninitialize();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("CAudioManager::Init(,,,) OUT - tid = (0x%x)"), ::GetCurrentThreadId());
}

/// Set the master output volume level. 
bool CAudioManagerCCS100::SetMasterVolumeLevel(const int leftVolume, const int rightVolume)
{
    USES_CONVERSION;

    try{
        if (!m_bInitialized)
            return false;

        if (m_pMasterVolume) 
        {
            return m_pMasterVolume->SetVolume(leftVolume, rightVolume);
        }
        BaseLogger::WriteLog(_T("SetMasterVolumeLevel(l, v) error"));
    }

    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::SetMasterVolumeLevel Exception"));
        return false;
    }
    return false;

}

/// Get the master output volume.	
bool CAudioManagerCCS100::GetMasterVolumeLevel(int &leftVolume, int &rightVolume)
{
    try{
        if (!m_bInitialized)
        {
            BaseLogger::WriteFormattedLog(_T("%s - not initialized"), __WFUNCTION__);
             return false;;
        }

        if (m_pMasterVolume) 
        {
            if (m_pMasterVolume->GetCurrentChannelVolume(0, leftVolume) && 
                m_pMasterVolume->GetCurrentChannelVolume(1, rightVolume))
            {
                return true;
            }
        }
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::GetMasterVolumeLevel error"));
    }

    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::GetMasterVolumeLevel Exception"));
        return false;
    }
    return false;
}

/// Get the mic gain level
bool CAudioManagerCCS100::GetMicGain(int &leftVolume, int &rightVolume)
{
    ::EnterCriticalSection(&m_CriticalSection);

    try{
        if (!m_bInitialized)
             return false;

        if (m_pMic == 0) 
        {
            BaseLogger::WriteLog( _T("GetMicGain MIC ERROR"));
            ::LeaveCriticalSection(&m_CriticalSection);
            return false;
        }

        m_pMic->GetCurrentChannelVolume(0, leftVolume);
        if (m_pMic->GetNumChannels() < 2)
        {
            rightVolume = leftVolume;
            ::LeaveCriticalSection(&m_CriticalSection);
            return true;
        }
        else m_pMic->GetCurrentChannelVolume(1, rightVolume);
        {
            ::LeaveCriticalSection(&m_CriticalSection);
            return true;
        }
    }
    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::GetMicGain(l, r)"));
        ::LeaveCriticalSection(&m_CriticalSection);
        return  false;

    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return false;
}



/// Get the mic mute
bool CAudioManagerCCS100::GetMicMute()
{
    bool retVal = false;

    ::EnterCriticalSection(&m_CriticalSection);
    try{
        if (!m_bInitialized)
             return false;

        if (m_pMic == 0) 
        {
            BaseLogger::WriteLog(_T("MIC NOT FOUND!!!"));
            ::LeaveCriticalSection(&m_CriticalSection);
            return false;
        } 
        retVal = m_pMic->GetMute();
    }
    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::GetMicMute()"));
        ::LeaveCriticalSection(&m_CriticalSection);
        return retVal;
    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return retVal;
}

/// Set the mic gain level
bool CAudioManagerCCS100::SetMicGain(const int volume)
{
    USES_CONVERSION;
    bool retVal = false;
    try{
        ::EnterCriticalSection(&m_CriticalSection);
        if (!m_bInitialized)
             return false;

        if (m_pMic == 0)
        {
            BaseLogger::WriteLog( _T("SetMicGain MIC ERROR"));
            return false;
        }

        if (m_pMic->GetNumChannels() > 1)
        {
            retVal = m_pMic->SetVolume(volume, volume);
        }
        else
        {
            retVal = m_pMic->SetVolume(volume);
        }
    }  
    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::SetMicGain(l, r)"));
        ::LeaveCriticalSection(&m_CriticalSection);
        return  retVal;

    }

    ::LeaveCriticalSection(&m_CriticalSection);
    return retVal;
}


/// Set the mic gain level
bool CAudioManagerCCS100::SetMicGain(const int leftVolume, const int rightVolume)
{
    bool retVal = false;

    ::EnterCriticalSection(&m_CriticalSection);

    try{
        if (!m_bInitialized)
             return false;

        if (m_pMic == 0)
        {
            return false;
        }

        if (m_pMic->GetNumChannels() < 2)
        {
            retVal = m_pMic->SetVolume(leftVolume);
        }else
        {
            retVal = m_pMic->SetVolume(leftVolume, rightVolume);
        }
    }
    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::SetMicGain(l, r)"));
        ::LeaveCriticalSection(&m_CriticalSection);
        return  retVal;

    }
    ::LeaveCriticalSection(&m_CriticalSection);
    return  retVal;
}

/// Set the mic mute
bool CAudioManagerCCS100::SetMicMute(const bool muteSetting)
{
    bool retVal = false;
    USES_CONVERSION;
    // ATLASSERT(m_pMic);

    try{
        if (!m_bInitialized)
             return false;

        ::EnterCriticalSection(&m_CriticalSection);
        if (m_pMic == 0) 
        {
            BaseLogger::WriteLog(_T("MIC NOT FOUND!!!"));

            ::LeaveCriticalSection(&m_CriticalSection);
            return false;
        }
        retVal = m_pMic->SetMute(muteSetting);
        // check

        ::LeaveCriticalSection(&m_CriticalSection);
    }
    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::SetMicMute()"));
        return retVal;
    }

    return retVal;
}


/// Get the master mute.
bool CAudioManagerCCS100::GetMasterMute()
{
    bool bReturn = false;
    try{
        if (!m_bInitialized)
             return false;


        if (m_pMasterVolume)
            bReturn = m_pMasterVolume->GetMute();
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::GetMasterMute"));
        return bReturn;
    }
    return bReturn;
}

/// Set the master mute.
bool CAudioManagerCCS100::SetMasterMute(const bool muteSetting)
{
	BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::SetMasterMute"));

    bool bReturn = false;
    try{
        if (!m_bInitialized)
             return false;

        if (m_pMasterVolume)											// In MS, this pointer never gets set.  Here it does.  Why?
		{
			BaseLogger::WriteFormattedLog(TRACE_AUDIO, _T("Setting Mute (%d) on Master Volume device 0x%8.8x"), muteSetting, m_pMasterVolume);
            bReturn = m_pMasterVolume->SetMute(muteSetting);
		}
        else
        {
            BaseLogger::WriteLog(_T("SetMasterMute ERROR"));
        }
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::GetMasterVolumeBounds"));
        return false;
    }
    return bReturn;
}