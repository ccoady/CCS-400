#include "stdafx.h"
#define TEST_APP
#include "AudioManagerCCS200.h"
#include "System.h"

//DeviceToJoinX* pJoinToDeviceTable =  NULL;
#include <fstream>


CAudioManagerCCS200::CAudioManagerCCS200(void):
m_pMixedOutputDevice(NULL),
    m_pCurrentListeningUsbDevice(NULL)
{
    m_usbList = new CUsbList();
}


CAudioManagerCCS200::~CAudioManagerCCS200(void)
{
}



void CAudioManagerCCS200::Init(HWND hwnd, IAudioDeviceChangeListener* pListener)
{
    m_hwnd = hwnd;

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO , _T("%s\tIN"), _AUDIOFUNC_);

    // return if already initialized
    if (m_bInitialized)
        return;

    //  __try{
    CoInitialize(NULL);      
    //m_DeviceChangeNotifier = new CMMNotificationClient(NULL);
    //m_DeviceChangeNotifier->Init(pListener);
    m_pListener = pListener;
    EnumDevices();

    OnEnumDevicesComplete();

    m_bInitialized = true;
    //   }
    //    __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation())){

    //        BaseLogger::WriteLog(_T("CAudioManager::Init Exception"));
    //   }
    BaseLogger::WriteFormattedLog(TRACESTACK| TRACE_AUDIO, _T("CAudioManager::Init(,,,) COMPLETED"));
}

////////////////////////////////////////////////////////////////////////////////
// Function:	virtual OnEnumDevicesComplete
// Parameters:
//            
//            
// 
// Description:  This method is called at the end of device intitialization and
//				enumeration. This is where specialized processing of the list occurs.
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManagerCCS200::OnEnumDevicesComplete()
{
    DWORD tid = GetCurrentThreadId();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s IN"), tid, _AUDIOFUNC_);
    int cntFirst = 0;

    // Extract all USB devices and send them the Engine
    // iterate input device list which is a snapshot since our last device changed event.
    // If devices change as we iterate this local list we will run through this path again
    // and send the new list.
    m_strInputDeviceList.clear();
    m_usbList->ClearUsbMics();
    vector<CAudioDevice*>::iterator theIterator;
    for (theIterator = m_vInputList.begin(); (theIterator != m_vInputList.end()); theIterator++)
    {
        CAudioDevice* pCurrent = (CAudioDevice*)*theIterator;

        // check if this is a USB device.
        tstring strEnumName;
        pCurrent->GetDeviceEnumerator(strEnumName);

        if (strEnumName.compare(_T("USB")) == 0)
        {

            if (cntFirst == 0)
            {
                m_strInputDeviceList = pCurrent->GetName();
                m_usbList->AddUsbMic(pCurrent->GetName());
            }
            else
            {
                m_strInputDeviceList += _T(";");
                m_strInputDeviceList += pCurrent->GetName();
                m_usbList->AddUsbMic(pCurrent->GetName());
            }
            ++cntFirst;
        }
    }

    DeviceToJoinX* pItem = m_pDeviceToJoinTable;
    BOOL bFound = false;
    //	DeviceToJoin* pAudioChannelDescriptor = m_pAudioDeviceCfgMgr->GetChannelDescriptor(_T("ANALOG_OUT"));
    while (pItem->strChannelName[0] != NULL && !bFound)
    {
        bFound  = !_tcscmp(pItem->strChannelName, _T("ANALOG_OUT"));
        if (bFound)
        {
            bFound = true; 
        }
        else
        {
            pItem++;
        }
    }

    m_strOutputDeviceList.clear();
    m_strOutputDeviceList = pItem->strDefDeviceName;

    //	m_strOutputDeviceList = pAudioChannelDescriptor->GetDeviceName();
    for (theIterator = m_vOutputList.begin(); (theIterator != m_vOutputList.end()); theIterator++)
    {
        CAudioDevice* pCurrent = (CAudioDevice*)*theIterator;

        // Ignore if this is a Crestron device with PID_2912
        if (pCurrent->GetPidPropertyString().find(_T("pid_2912")) != -1)
            continue;

        // check if this is a USB device.
        tstring strEnumName;
        pCurrent->GetDeviceEnumerator(strEnumName);


        if (strEnumName.compare(_T("USB")) == 0)
        {
            m_strOutputDeviceList += _T(";");
            m_strOutputDeviceList += pCurrent->GetName();
        }
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s - Output devices OUT"), tid, _AUDIOFUNC_);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	virtual SetListenOffOnAllUsbDevices
// Parameters:
//            
//            
// 
// Description:  This method is called at the end of device intitialization and
//				enumeration. This is where specialized processing of the list occurs.
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManagerCCS200::SetListenOffOnAllUsbDevices()
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"),_AUDIOFUNC_);

    try
    {
        // Extract all USB devices and send them the Engine
        // iterate input device list which is a snapshot since our last device changed event.
        // If devices change as we iterate this local list we will run through this path again
        // and send the new list.
        // m_strInputDeviceList.clear();
        vector<CAudioDevice*>::iterator theIterator;
        for (theIterator = m_vInputList.begin(); (theIterator != m_vInputList.end()); theIterator++)
        {
            CAudioDevice* pCurrent = (CAudioDevice*)*theIterator;

            // check if this is a USB device.
            tstring strEnumName;
            pCurrent->GetDeviceEnumerator(strEnumName);
            if (pCurrent == NULL)
            {
                BaseLogger::WriteFormattedLog(_T("%s:\tThis should never occur."), _AUDIOFUNC_ERROR_);
                break;
            }

            if (strEnumName.compare(_T("USB")) == 0)
            {
                //  basic_string <char>::size_type index = pCurrent->GetName().find(_T("Mic"));
                //  if (index != tstring::npos)
                //  {
                SetListenState(static_cast<CAudioInputDevice*>(pCurrent), false);
                //  }
            }
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tcaught exception OUT"), _AUDIOFUNC_ERROR_);
    }

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
}

void CAudioManagerCCS200::MapUSBMicsToLync()
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);


    DeviceToJoinX* pItem = m_pDeviceToJoinTable;
    BOOL bFound = false;
    //	DeviceToJoin* pAudioChannelDescriptor = m_pAudioDeviceCfgMgr->GetChannelDescriptor(_T("ANALOG_OUT"));
    while (pItem->strChannelName[0] != NULL && !bFound)
    {
        bFound  = !_tcscmp(pItem->strChannelName, _T("LYNC_MIX"));
        if (bFound)
        {
            bFound = true; 
        }
        else
        {
            pItem++;
        }
    }

    CAudioOutputDevice* pLyncMixDevice  = static_cast<CAudioOutputDevice*>( GetDeviceByName (pItem->strDefDeviceName));
    if (pLyncMixDevice  == NULL){
        BaseLogger::WriteFormattedLog(_T("%s:\tError NULL local playback output device, check the config file"),
            _AUDIOFUNC_);
        return;
    }


    // Extract all USB devices and send them the Engine
    // iterate input device list which is a snapshot since our last device changed event.
    // If devices change as we iterate this local list we will run through this path again
    // and send the new list.
    // m_strInputDeviceList.clear();
    vector<CAudioDevice*>::iterator theIterator;
    for (theIterator = m_vInputList.begin(); (theIterator != m_vInputList.end()); theIterator++)
    {
        CAudioInputDevice* pCurrent = (CAudioInputDevice*)*theIterator;


        // check if this is a USB device.
        tstring strEnumName;

        pCurrent->GetDeviceEnumerator(strEnumName);
        if (!strEnumName.empty())
        {
            if (strEnumName.compare(_T("USB")) == 0)
            {
                MapOutputDevice(dynamic_cast<CAudioInputDevice*>(pCurrent), pLyncMixDevice);
            }
        }
    }

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetUsbInputDeviceNamesList
// Parameters:
// returns:     tstring containing a ; delimited list of USB input device
//              friendly names.
//            
//            
// 
// Description:  
//				 
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
tstring& CAudioManagerCCS200::GetUsbInputDeviceNamesList()
{
    return m_strInputDeviceList;
}

tstring& CAudioManagerCCS200::GetOutputDeviceNamesList()
{
    return m_strOutputDeviceList;
}

void CAudioManagerCCS200::AddUsbMic(tstring& tName)
{
    m_usbList->AddUsbMic(tName);
}

void CAudioManagerCCS200::GetFirstUsbMic(tstring& tName )
{
    m_usbList->GetFirstUsbMic(tName);
}

void CAudioManagerCCS200::GetNextUsbMic(tstring& tName)
{
    m_usbList->GetNextUsbMic(tName);
}

void CAudioManagerCCS200::GetUsbMicCount(int &cnt)
{
    m_usbList->GetUsbMicCount(cnt);
}

void CAudioManagerCCS200::ClearUsbMics()
{
    m_usbList->ClearUsbMics();
}

void CAudioManagerCCS200::CUsbList::AddUsbMic(tstring& name)
{
    m_list.push_back(name);
}

void CAudioManagerCCS200::CUsbList::GetUsbMicCount(int& cnt)
{
    cnt = (int)m_list.size();
}

void CAudioManagerCCS200::CUsbList::GetFirstUsbMic(tstring& tName)
{
    if ((m_iterator = m_list.begin()) != m_list.end())
    {
        tName = *m_iterator;
    }
}

void CAudioManagerCCS200::CUsbList::GetNextUsbMic(tstring& tName)
{
    if (m_iterator == m_list.end())
    {
        return;
    }

    m_iterator++;
    if (m_iterator != m_list.end())
    {
        tName = *m_iterator;
    }
}

void CAudioManagerCCS200::CUsbList::ClearUsbMics()
{
    m_list.clear();
}

bool CAudioManagerCCS200::SetMute(CAudioDevice* pDevice, const bool muteSetting)
{
    bool bReturn = false;

    try{
        if (!m_bInitialized)
            return false;

        if (pDevice)
            bReturn = pDevice->SetMute(muteSetting);
        else
        {
            BaseLogger::WriteLog(_T("SetMute ERROR"));
        }
    }

    catch(...)
    {
        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManagerCCS100::SetMute(l, r)"));
        return false;
    }
    return bReturn;
}

bool CAudioManagerCCS200::SetVolumeLevel(CAudioDevice* pDevice, const int volume)
{
    try{
        if (!m_bInitialized)
            return false;

        if (pDevice) {
            return pDevice->SetVolume(volume);
        }
        BaseLogger::WriteLog( _T("SetVolumeLevel(vol) error"));
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManager::SetVolumeLevel(vol) Exception"));
        return false;
    }
    return false;
}

bool CAudioManagerCCS200::GetVolumeLevel(CAudioDevice* pDevice, int &leftVolume, int &rightVolume)
{
    try{
        if (!m_bInitialized)
            return false;;

        if (pDevice)
        {
            if (pDevice->GetCurrentChannelVolume(0, leftVolume) &&
                pDevice->GetCurrentChannelVolume(1, rightVolume))
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

bool CAudioManagerCCS200::GetMute(CAudioDevice* pDevice)
{
    bool bReturn = false;
    try{
        if (!m_bInitialized)
            return false;

        if (pDevice)
            bReturn = pDevice->GetMute();
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("CAudioManagerCCS100::GetMute"));
        return bReturn;
    }
    return bReturn;
}


////////////////////////////////////////////////////////////////////////////////
// 
// Method:      void CreateInputSubDevice()
//
// Parameters:	None 
//               
//
// Description: 
//              
//              
//				
//
// Author:     Jeff Spruiel 12/12/2013
//
////////////////////////////////////////////////////////////////////////////////
CAudioInputDevice* CAudioManagerCCS200::CreateInputSubDevice(IMMDevice* pMMDevice, TCHAR* tSubPartName)
{
    CAudioInputDevice* pInput = NULL;

    pInput = new CAudioInputDevice(pMMDevice);
    ((CAudioDevice*)pInput)->Init(tSubPartName);

    return pInput;
}


////////////////////////////////////////////////////////////////////////////////
// Function:	SetListenState
// Parameters:  pDevice - a pointer to the USB input device class.
//				state   - true to turn on, false to turn off.
// returns:     
//            
//            
// 
// Description: Sets a registry key to enable or disable the listen state.
//               
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManagerCCS200::SetListenState(CAudioInputDevice* pDevice, bool state)
{
    HKEY hKey;
    DWORD dwKeyIdx = 0, dwBuffer = MAX_KEY_LEN;


    long lRes;

    TCHAR  cKEY[MAX_KEY_LEN];

    try{
        BaseLogger::WriteFormattedLog(TRACEALL, _T("%s(%d):\t IN"),_AUDIOFUNC_, state);

        if(pDevice == NULL )
        {
            BaseLogger::WriteFormattedLog(_T("%s(\" \"):\tDevice param is BLANK; returning."),_AUDIOFUNC_INFO_);
            return;
        }

        BaseLogger::WriteFormattedLog(_T("%s(%d):\t\"%s\"\tIN"),_AUDIOFUNC_, state, pDevice->GetName().c_str());
        wsprintf(cKEY, _T("%s\\%s\\%s"),CAPTURE_SUBKEYS, pDevice->GetGuid().c_str(), _T("Properties"));


        HANDLE hToken; 
        TOKEN_PRIVILEGES tkp; 

        // Get a token for this process. 
        if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        {
            BaseLogger::WriteFormattedLog(_T("%s:\tfailed process token."), _AUDIOFUNC_ERROR_);
            return;
        }

        // Get the LUID for the shutdown privilege. 
        LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, &tkp.Privileges[0].Luid); 

        tkp.PrivilegeCount = 1;  // one privilege to set    
        tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

        // Get the shutdown privilege for this process. 
        if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
        {
            BaseLogger::WriteFormattedLog(_T("%s:\taudio priv error"), _AUDIOFUNC_ERROR_);
            CloseHandle(hToken);
            return;
        }

        if ((lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKEY, 0, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER, &hKey)) != ERROR_SUCCESS)
        {

            BaseLogger::WriteFormattedLog(_T("%s:\tfailed to open key."), _AUDIOFUNC_ERROR_);
            CloseHandle(hToken);
            return;
        }

        DWORD type      = REG_BINARY;
        DWORD dwSize    = 12;
        BYTE currBinData[12];
        memset(currBinData, 0, 12);
        if ( state == true)
        {
            // if already set, then don't set it again because the device will not work. This is a corner case
            // for boot up of firmware as well as the user submitting the same device.
            lRes = RegQueryValueEx(hKey, GUID_LISTEN_CONTROL_PROP, 0, &type, (BYTE*)currBinData, &dwSize);
            if (lRes == ERROR_SUCCESS)
            {
                // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s RegQueryValueEx OK"),_AUDIOFUNC_);
                if ( (currBinData[8] != 0xFF) && (currBinData[9] != 0xFF) )
                {
                    currBinData[0] = 0x0B;
                    currBinData[8] = 0xFF;
                    currBinData[9] = 0xFF;
                    lRes = RegSetValueEx( hKey, GUID_LISTEN_CONTROL_PROP, 0, REG_BINARY, (BYTE*)currBinData, 12 );
                    if (lRes != ERROR_SUCCESS)
                        BaseLogger::WriteFormattedLog( _T("%s:\tRegSetValueEx 1 code = 0x%8.8x"),_AUDIOFUNC_ERROR_, lRes);
                }
                else
                {
                    BaseLogger::WriteFormattedLog(_T("%s:\tListen on this device already set to the requested value"),_AUDIOFUNC_);
                }
            }
            else
            {
                BaseLogger::WriteFormattedLog(_T("%s:\tRegQueryValueEx 3 code = 0x%8.8x."), _AUDIOFUNC_WARNING_, lRes);
                currBinData[0] = 0x0B;
                currBinData[8] = 0xFF;
                currBinData[9] = 0xFF;
                lRes = RegSetValueEx( hKey, GUID_LISTEN_CONTROL_PROP, 0, REG_BINARY, (BYTE*)currBinData, 12 );
                if (lRes != ERROR_SUCCESS)
                    BaseLogger::WriteFormattedLog( _T("%s:\tRegSetValueEx 4 code = 0x%8.8x"),_AUDIOFUNC_ERROR_, lRes);

            }
        }
        else
        {
            lRes = RegQueryValueEx(hKey, GUID_LISTEN_CONTROL_PROP, 0, &type, (BYTE*)currBinData, &dwSize);
            if (lRes == ERROR_SUCCESS)
            {
                if ( (currBinData[8] != 0x00) && (currBinData[9] != 0x00) )
                {
                    currBinData[0] = 0x0B;
                    currBinData[8] = 0x00;
                    currBinData[9] = 0x00;
                    lRes = RegSetValueEx( hKey, GUID_LISTEN_CONTROL_PROP, 0, REG_BINARY, (BYTE*)currBinData, 12 );
                    if (lRes != ERROR_SUCCESS)
                        BaseLogger::WriteFormattedLog(_T("%s  failed 5 code = 0x%8.8x"),_AUDIOFUNC_ERROR_, lRes);
                }
                else
                    BaseLogger::WriteFormattedLog(_T("%s:\tListen on this device already set to the requested value"),_AUDIOFUNC_);
            }
            else
            {
                currBinData[0] = 0x0B;
                currBinData[8] = 0x00;
                currBinData[9] = 0x00;
                lRes = RegSetValueEx( hKey, GUID_LISTEN_CONTROL_PROP, 0, REG_BINARY, (BYTE*)currBinData, 12 );
                if (lRes != ERROR_SUCCESS)
                    BaseLogger::WriteFormattedLog( _T("%s:\tRegSetValueEx 6 code = 0x%8.8x"),_AUDIOFUNC_ERROR_, lRes);
            }
        }       

        CloseHandle(hToken);
        RegCloseKey(hKey);
    }

    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tcaught exception\tOUT"), _AUDIOFUNC_ERROR_);
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	MapOutputDevice
// Parameters:  pDevice - a pointer to the USB input device class.
//              pOutput - a pointer to the output device.
// returns:     
//            
//            
// 
// Description: Sets a registry key to direct audio to the output device specified
//              in the configuration file.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManagerCCS200::MapOutputDevice(CAudioInputDevice* pDevice, CAudioOutputDevice* pOutput)
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s\tIN"),_AUDIOFUNC_);
    //
    // Check for the presence.
    // Get the GUID
    //
    // open the key and write
    //
    HKEY hKey;
    DWORD dwKeyIdx = 0, dwBuffer = MAX_KEY_LEN;


    long lRes;

    TCHAR  cKEY[MAX_KEY_LEN];

    wsprintf(cKEY, _T("%s\\%s\\%s"),CAPTURE_SUBKEYS, pDevice->GetGuid().c_str(), _T("Properties"));

    if (pDevice == NULL)
    {
        BaseLogger::WriteFormattedLog(_T("%s Error: Input device param is NULL"),_AUDIOFUNC_ERROR_);
        return;
    }

    if (pOutput == NULL)
    {
        BaseLogger::WriteFormattedLog(_T("%s Error: Output device param is NULL"),_AUDIOFUNC_ERROR_);
        return;
    }

    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 

    // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
        return;


    // Get the LUID for the shutdown privilege. 
    LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

    // Get the shutdown privilege for this process. 
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        BaseLogger::WriteFormattedLog(_T("CAudioManager::MapOutputDevice failed AdjustTokenPrivileges."));
        CloseHandle(hToken);
        return;
    }

    if ((lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKEY, 0, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER, &hKey)) != ERROR_SUCCESS){

        CloseHandle(hToken);
        BaseLogger::WriteFormattedLog(_T("CAudioManager::MapOutputDevice failed RegOpenKeyEx with %d."), lRes);
        return;// FALSE;
    }


    BaseLogger::WriteFormattedLog(TRACE_AUDIO_L1, _T("Reg key loc = %s"), cKEY);
    BaseLogger::WriteFormattedLog(TRACE_AUDIO_L1, _T("\n%s=%s"), GUID_OUTPUT_CONNECTION_PROP,  pOutput->GetGuid().c_str());

    lRes = RegSetValueEx( 
        hKey,
        GUID_OUTPUT_CONNECTION_PROP,
        0, 
        REG_SZ, 
        (BYTE *)  pOutput->GetGuid().c_str(),
        112 );
    if (lRes != ERROR_SUCCESS)
        BaseLogger::WriteFormattedLog(_T("%s:\tfailed RegSetValueEx = %d"), _AUDIOFUNC_ERROR_, lRes);


    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
    CloseHandle(hToken);
    RegCloseKey(hKey); 
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
CAudioDevice*  CAudioManagerCCS200::GetInputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter)
{
    USES_CONVERSION;
    CAudioDevice* pCurrent = 0;
    bool bFound = false;

    try{
        if (tszAdapter == NULL)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\t invalid 2nd name"), _AUDIOFUNC_);
            return NULL;
        }

        if (tszDevice)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tsearch for - %s %s"), _AUDIOFUNC_, tszDevice, tszAdapter);
        }
        else
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tsearch for - NULL %s"), _AUDIOFUNC_, tszAdapter);
        }

        vector<CAudioDevice*>::iterator theIterator;
        for (theIterator = m_vInputList.begin(); (theIterator != m_vInputList.end()) && !bFound; theIterator++)
        {
            pCurrent = (CAudioDevice*)*theIterator;
            tstring name = pCurrent->GetName();
            if (DoStringsMatch(tszAdapter, (TCHAR*)name.c_str()))
            {
                bFound = true;
            }
        }

        if (bFound)
        {
            return pCurrent;
        }

        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\taudio device not found."), _AUDIOFUNC_);
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tException"), _AUDIOFUNC_ERROR_);
        return NULL;
    }
    return NULL;
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
CAudioDevice*  CAudioManagerCCS200::GetOutputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter)
{
    USES_CONVERSION;
    CAudioDevice* pCurrent = 0;
    bool bFound = false;

    try{
        if (tszAdapter == NULL)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tinvalid 2nd name"), _AUDIOFUNC_);
            return NULL;
        }

        if (tszDevice)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tsearch for - %s %s"), _AUDIOFUNC_, tszDevice, tszAdapter);
        }
        else
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\t search for - NULL %s"), _AUDIOFUNC_, tszAdapter);
        }

        vector<CAudioDevice*>::iterator theIterator;
        for (theIterator = m_vOutputList.begin(); (theIterator != m_vOutputList.end()) && !bFound; theIterator++)
        {
            pCurrent = (CAudioDevice*)*theIterator;
            tstring name = pCurrent->GetName();
            if (DoStringsMatch(tszAdapter, (TCHAR*)name.c_str()))
            {
                bFound = true;
            }
        }

        if (bFound)
        {
            return pCurrent;
        }
        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%sd:\t - audio device not found."), _AUDIOFUNC_);
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s"), _AUDIOFUNC_ERROR_);
        return NULL;
    }
    return NULL;
}


void CAudioManagerCCS200::ReadMicGainFromIniFile(tstring& szGainLevel)
{
    USES_CONVERSION;

     // ini file
    TCHAR tsIni[_MAX_PATH] = { 0 };
    TCHAR* szAppName = _T("MicSettings");

    memset (m_tszFirmwarePath, '0', MAX_PATH *sizeof (TCHAR));

    CfgGetFirmwarePath(m_tszFirmwarePath);

    _stprintf(tsIni, _T("%s\\%s"), m_tszFirmwarePath, MIC_SETTINGS_INI);


    TCHAR szKey[_MAX_PATH] = _T("AnalogMicGain") ;
    TCHAR tValue[_MAX_PATH] = {0};
 

    DWORD err = ::GetPrivateProfileString(szAppName, szKey, _T("MicLevel"), tValue, _MAX_PATH, tsIni );

    szGainLevel = tValue;
    //BaseLogger::WriteFormattedLog(_T("%s:\tGain level = %s\n"), _AUDIOFUNC_, tValue);
}

void CAudioManagerCCS200::WriteMicGainToIniFile(const tstring& szGainLevel)
{
    USES_CONVERSION;

     // ini file
    TCHAR tsIni[_MAX_PATH] = { 0 };
    TCHAR* szAppName = _T("MicSettings");

    _stprintf(tsIni, _T("%s\\%s"), m_tszFirmwarePath, MIC_SETTINGS_INI);

    TCHAR szKey[_MAX_PATH] = _T("AnalogMicGain") ;
 
    WritePrivateProfileString (szAppName,  szKey, szGainLevel.c_str(), tsIni);
    //BaseLogger::WriteFormattedLog(_T("%s:\tAnalogMicGain level = %s\n"), _AUDIOFUNC_, szGainLevel.c_str());
}
