/* Copyright (2012) Crestron Electronics Inc., All Rights Reserved.
* The source code contained or described herein and all documents related to
* the source code ("Material") are owned by Crestron Electronics Inc. or its suppliers
* or licensors. Title to the Material remains with Crestron Electronics Inc. or its
* suppliers and licensors. The Material contains trade secrets and proprietary
* and confidential information of Crestron or its suppliers and licensors. The
* Material is protected by worldwide copyright and trade secret laws and
* treaty provisions. No part of the Material may be used, copied, reproduced,
* modified, published, uploaded, posted, transmitted, distributed, or
* disclosed in any way without Crestron's prior express written permission.
*
* No license under any patent, copyright, trade secret or other intellectual
* property right is granted to or conferred upon you by disclosure or
* delivery of the Materials, either expressly, by implication, inducement,
* estoppel or otherwise. Any license under such intellectual property rights
* must be express and approved by Crestron in writing.
*/

#include "stdafx.h"

#include "AudioManager.h"
#include "..\UtsConfigUtil\ConfigFuncUts.h"
#include "system.h"


#include <Mmdeviceapi.h>
#include "DeviceListener.h"
#include <regex>

using namespace std;
TCHAR tcharDbg[128];

//TCHAR* gMicPriority [] = {_T("USB audio CODEC)"), _T("HD Pro Webcam C930)"), _T("HD Pro Webcam C920)")};


CAudioManager* g_this = NULL;


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
CAudioManager::CAudioManager(void):
m_nDevices(0),
    m_pMasterVolume(NULL),
    m_pMic(NULL),
    m_hwnd(NULL),
    m_DeviceChangeNotifier(NULL),
    m_bInitialized(false),
    m_pListener(NULL),
    m_pOutgoingDeviceChangeListener(NULL),
    m_pIncommingDeviceChangeListener(NULL)
{
    m_usbList = new CUsbList();
    BaseLogger::WriteFormattedLog(_T("%s\t"), _AUDIOFUNC_);
    ::InitializeCriticalSection(&m_CriticalSection);
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
CAudioManager::CAudioManager(AudioChannelDescriptor* pAudioChannelDescriptorTable):
m_pTable(pAudioChannelDescriptorTable),
    m_nDevices(0),
    m_pMasterVolume(NULL),
    m_pMic(NULL),
    m_hwnd(NULL),
    m_DeviceChangeNotifier(NULL),
    m_bInitialized(false),
    m_pListener(NULL),
    m_pOutgoingDeviceChangeListener(NULL),
    m_pIncommingDeviceChangeListener(NULL)
{
    m_usbList = new CUsbList();
    ::InitializeCriticalSection(&m_CriticalSection);
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
void CAudioManager::CreateAudioChannelDescriptorManager()
{
    m_pAudioDeviceCfg = new CAudioChannelDescriptorManager(this, m_pTable);
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
CAudioManager::~CAudioManager(void)
{
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s"), _AUDIOFUNC_);
    Destroy();
    ::DeleteCriticalSection(&m_CriticalSection);
    CoUninitialize();
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s"), _AUDIOFUNC_);
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
void CAudioManager::Init(HWND hwnd, IAudioDeviceChangeListener* pListener)
{
    m_hwnd = hwnd;
    g_this = this;

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s"), _AUDIOFUNC_);

    // return if already initialized
    if (m_bInitialized)
        return;

    //__try{
    memset (m_tszFirmwarePath, '0', MAX_PATH *sizeof (TCHAR));
    memset (m_tszIniFilePathName, '0', 2048 *sizeof (TCHAR));

    CfgGetFirmwarePath(m_tszFirmwarePath);
    _stprintf_s(m_tszIniFilePathName, 2048, _T("%s\\%s"), m_tszFirmwarePath, _T("UpxTaskMgr.ini"));
    BaseLogger::WriteLog(TRACESTACK, m_tszIniFilePathName);

    CoInitialize(NULL);
    m_DeviceChangeNotifier = new CMMNotificationClient(NULL);
    m_pListener = pListener;
    m_DeviceChangeNotifier->Init(pListener);
    CreateAudioChannelDescriptorManager();

    // Initialize the device objects
    EnumDevices();
    //
    m_bInitialized = true;
    //}
    //  __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation())){
    //      BaseLogger::WriteFormattedLog(_T("CAudioManager::Init Exception %d"), GetExceptionCode());
    //  }

    // CoUninitialize();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s\tOUT"), _AUDIOFUNC_);
}



////////////////////////////////////////////////////////////////////////////////
// Function:	
//
// Parameters:		HWND hwnd
//					IAudioManagerDeviceChangeListener
// Returns:     
//              
//              
// 
// Description: 
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManager::Init(HWND hwnd, IAudioManagerDeviceChangeListener* pOutgoingDeviceChangeListener)
{
    m_hwnd = hwnd;
    g_this = this;

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);

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
    m_pOutgoingDeviceChangeListener = pOutgoingDeviceChangeListener;
    m_DeviceChangeNotifier->Init(m_pIncommingDeviceChangeListener = this);
    CreateAudioChannelDescriptorManager();

    // Initialize the device objects
    EnumDevices();
    OnEnumDevicesComplete();
    //
    m_bInitialized = true;
    //}
    //  __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation())){
    //      BaseLogger::WriteFormattedLog(_T("CAudioManager::Init Exception %d"), GetExceptionCode());
    //  }

    // CoUninitialize();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
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
void CAudioManager::EnumDevices()
{
    DWORD tid = GetCurrentThreadId();
    // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s IN"), ::GetCurrentThreadId(), _AUDIOFUNC_);
    ::EnterCriticalSection(&m_CriticalSection);
    try{
        m_nDevices = 0;

        Destroy();

        IMMDeviceEnumerator *deviceEnumerator = NULL;
        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&deviceEnumerator);

        if (S_OK == hr)
        {
            IMMDeviceCollection* deviceCollection = NULL;
            hr = deviceEnumerator->EnumAudioEndpoints(eAll, DEVICE_STATE_ACTIVE|DEVICE_STATE_DISABLED, &deviceCollection);

            if (S_OK == hr)
            {
                UINT uiDevices;
                hr = deviceCollection->GetCount(&uiDevices);
                m_nDevices = (int)uiDevices;
                BaseLogger::WriteFormattedLog(_T("%s:\tEnum'd audio device cnt: %d"), _AUDIOFUNC_, m_nDevices);

                IMMDevice* currentDevice = NULL;
                for (UINT i=0; i<uiDevices; i++)
                {
                    deviceCollection->Item(i, &currentDevice);

                    if (currentDevice)
                    {
                        CAudioDevice tempDevice(currentDevice);

                        bool bIsInput;
                        hr = tempDevice.GetDataDirection(&bIsInput);

                        // Our little trick here with creating two objects causes this device to be cleaned up
                        // when our temp AudioDevice is destroyed, unless we AddRef it here.
                        currentDevice->AddRef();

                        if (S_OK == hr)
                        {
                            if (bIsInput)
                            {
                                CAudioInputDevice* pNew = new CAudioInputDevice(currentDevice);
                                pNew->Init();
                                m_vInputList.push_back(pNew);
                                // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s Added input dev = %s"), _AUDIOFUNC_, pNew->GetName().c_str() );
                            }
                            else
                            {
                                CAudioOutputDevice* pNew = new CAudioOutputDevice(currentDevice);
                                pNew->Init();
                                m_vOutputList.push_back(pNew);
                                // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s Added output dev %s"), _AUDIOFUNC_, pNew->GetName().c_str() );
                            }

                        }
                    }
                    else
                    {
                        TCHAR dbgMsg[256] = {_T('\0')};
                        //  BaseLogger::WriteLog(dbgMsg);
                    }

                    // Keep IMMDevice pointers open until the CAudioDevice object destroys it
                }

                deviceCollection->Release();
            }
            else
            {
                BaseLogger::WriteFormattedLog(_T("%s:\tEnumAudioEndpoints FAILED"), _AUDIOFUNC_ERROR_ );
            }

            deviceEnumerator->Release();
        }
        LoadAudioDeviceMap();
        // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("0x%8.8x - %s OUT"), tid,_AUDIOFUNC_);
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tException"), _AUDIOFUNC_ERROR_);
        ::LeaveCriticalSection(&m_CriticalSection);
        return;
    }

    ::LeaveCriticalSection(&m_CriticalSection);
}

void CAudioManager::Re_EnumerateDevices()
{
    BaseLogger::WriteFormattedLog(_T("%s:\tIN"), _AUDIOFUNC_);
    EnumDevices();
    OnEnumDevicesComplete();
    BaseLogger::WriteFormattedLog(_T("%s:\tOUT"), _AUDIOFUNC_);
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
void CAudioManager::LoadAudioDeviceMap()
{

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);
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

            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("HDMIOutputDevice"), NULL, tszHdmiDevice, _MAX_PATH, m_tszIniFilePathName);
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("HDMIOutputAdapter"), NULL, tszHdmiAdapter, _MAX_PATH, m_tszIniFilePathName);
        }

        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tOUT"), _AUDIOFUNC_);
    }
    __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation()))
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tCaught exception\tOUT"), _AUDIOFUNC_ERROR_);
    }
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
CAudioDevice*  CAudioManager::GetInputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter)
{
    USES_CONVERSION;
    CAudioDevice* pCurrent = 0;
    bool bFound = false;

    try{
        if (tszAdapter == NULL )
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tinvalid 2nd name"), _AUDIOFUNC_);
            return NULL;
        }

        if ( tszAdapter[0] == NULL)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tinvalid 2nd name"),_AUDIOFUNC_);
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
            bFound = ScanForName(name, tszDevice, tszAdapter);
        }

        if (bFound)
        {
            return pCurrent;
        }

        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t - audio device not found."),_AUDIOFUNC_WARNING_);
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s\tException"), _AUDIOFUNC_ERROR_);
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
CAudioDevice*  CAudioManager::GetOutputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter)
{
    USES_CONVERSION;
    CAudioDevice* pCurrent = 0;
    bool bFound = false;

    try{
        if (tszAdapter == NULL)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tinvalid 2nd name"), _AUDIOFUNC_WARNING_);
            return NULL;
        }

        if (tszDevice)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tsearch for - %s %s"),_AUDIOFUNC_, tszDevice, tszAdapter);
        }
        else
        {
            BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tsearch for - NULL %s"), _AUDIOFUNC_, tszAdapter);
        }

        vector<CAudioDevice*>::iterator theIterator;
        for (theIterator = m_vOutputList.begin(); (theIterator != m_vOutputList.end()) && !bFound; theIterator++)
        {
            pCurrent = (CAudioDevice*)*theIterator;
            tstring name = pCurrent->GetName();
            bFound = ScanForName(name, tszDevice, tszAdapter);
        }

        if (bFound)
        {
            return pCurrent;
        }
        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\taudio device not found."), _AUDIOFUNC_WARNING_);
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s"), _AUDIOFUNC_ERROR_);
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
bool CAudioManager::ScanForName(const tstring& name, TCHAR* tszDevice, TCHAR* tszAdapter )
{
    bool bFound = false  ;
    try
    {
        basic_string <char>::size_type idxA = 0;
        basic_string <char>::size_type idxB = 0;

        if(tszDevice)
        {
            idxA = name.find ( tszDevice);
            if ( idxA != string::npos )
            {
                idxB = name.find ( tszAdapter , idxA);
                bFound = ( idxB != string::npos ) ? true : false;
            }
        }
        else
        {
            idxB = name.find ( tszAdapter);
            bFound = ( idxB != string::npos ) ? true : false;
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tException"), _AUDIOFUNC_ERROR_);
        return bFound;
    }

    return bFound;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	Destroy
//
// Parameters:  None
//
// Returns:     None
//              
// Description: Deletes pointers to objects from the lists.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
bool CAudioManager::Destroy()
{
    USES_CONVERSION;
    CAudioDevice* pCurrent = 0;
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tIN"), _AUDIOFUNC_);
    try{
        for_each( m_vInputList.begin(), m_vInputList.end(), [](CAudioDevice*& pDevice)
        {
            if (pDevice != 0)
                pDevice->Cleanup();

            delete pDevice;
            pDevice = 0;
        });
        m_vInputList.erase(m_vInputList.begin(), m_vInputList.end());

        for_each( m_vOutputList.begin(), m_vOutputList.end(), [](CAudioDevice*& pDevice)
        {
            if (pDevice != 0)
                pDevice->Cleanup();

            delete pDevice;
            pDevice = 0;
        });	
        m_vOutputList.erase(m_vOutputList.begin(), m_vOutputList.end());
    }
    catch(...){
        BaseLogger::WriteFormattedLog(_T("%s:\tException"), _AUDIOFUNC_ERROR_);
        return false;
    }

    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s:\tOUT"), _AUDIOFUNC_WARNING_);
    return true;
}
/// The device has just un-docked. Break down the class for the external audio card
bool CAudioManager::DeviceUndocked()
{
    return false;
}

/// The device has just docked. Create the class for the external audio card
bool CAudioManager::DeviceDocked()
{
    return false;
}

/// Returns true if audio is currently routed to the external audio device
bool CAudioManager::IsAudioExternal()
{
    return false;
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
void CAudioManager::GetDeviceName(IMMDevice* pCurrentIMMDevice, tstring& string)
{
    USES_CONVERSION;

    HRESULT hr = E_FAIL;
    IPropertyStore* pProperties = NULL;

    if (pCurrentIMMDevice == 0)
    {
        return;
    }

    if (pCurrentIMMDevice)
    {
        hr = pCurrentIMMDevice->OpenPropertyStore(STGM_READ, &pProperties);
        EXIT_ON_ERROR(hr);

        if (S_OK == hr)
        {
            PROPVARIANT varName;
            PropVariantInit(&varName);
            hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                string = W2T(varName.pwszVal);
                PropVariantClear(&varName);
                return;
            }
        }
    }

Exit:
    LogHostError(hr, _AUDIOFUNC_ERROR_);
    SAFE_RELEASE(pProperties);
}


////////////////////////////////////////////////////////////////////////////////
//
// Function:	GetDeviceByName
//
// Parameters:  tcNameParam - the name of the device to return
//
// Returns:     A pointer to the CAudioDevice that matches the specified device name.
//
// Description: Sets a registry key to direct audio to the output device specified
//               in the configuration file.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
CAudioDevice*  CAudioManager::GetDeviceByName(_TCHAR* tcNameParam)
{
    USES_CONVERSION;

    CAudioDevice* pCurrent 	= NULL;
    bool bFound 			= false;

    BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO_L1, _T("%s IN"),_AUDIOFUNC_);

    if ((tcNameParam == NULL) || (tcNameParam[0] ==NULL))
    {
        BaseLogger::WriteFormattedLog(_T("%s Error: name param is NULL"),_AUDIOFUNC_);
        BaseLogger::WriteFormattedLog( TRACESTACK, _T("%s OUT alt 1 "),_AUDIOFUNC_);
        return NULL;
    }

    try{
        vector<CAudioDevice*>::iterator theIterator;
        for (theIterator = m_vOutputList.begin(); (theIterator != m_vOutputList.end()) && !bFound; theIterator++)
        {
            //BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO, _T("%s 1"),_AUDIOFUNC_);
            pCurrent = (CAudioDevice*)*theIterator;
            tstring name = pCurrent->GetName();
            //BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO, _T("%s 2"),_AUDIOFUNC_);

            // compare names
            if (DoStringsMatch(tcNameParam, (TCHAR*)name.c_str()))
            {
                bFound = true;
            }
        }

        if (bFound)
        {
            BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO_L1, _T("%s OUT"),_AUDIOFUNC_);
            return pCurrent;
        }

        // Next process the input list
        for (theIterator = m_vInputList.begin(); (theIterator != m_vInputList.end()) && !bFound; theIterator++)
        {
            //BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO, _T("%s 3"),_AUDIOFUNC_);
            pCurrent = (CAudioDevice*)*theIterator;
            tstring name = pCurrent->GetName();
            //BaseLogger::WriteFormattedLog( TRACESTACK| TRACE_AUDIO, _T("%s 4"),_AUDIOFUNC_);

            // compare names
            if (DoStringsMatch(tcNameParam, (TCHAR*)name.c_str()))
            {
                bFound = true;
            }
        }

        if (bFound)
        {
            BaseLogger::WriteFormattedLog( TRACESTACK, _T("%s OUT"),_AUDIOFUNC_);
            return pCurrent;
        }

        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO_L1, _T("%s:\t%s not found. OUT"), _AUDIOFUNC_WARNING_, tcNameParam);
    }
    catch(exception& e)
    {
        e;
        BaseLogger::WriteFormattedLog(_T("%s:\texception on %s "), _AUDIOFUNC_ERROR_, tcNameParam);
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
void CAudioManager::IsRegStateListening(CAudioInputDevice* pDevice, bool& bResult)
{
    HKEY hKey;
    DWORD dwKeyIdx = 0, dwBuffer = MAX_KEY_LEN;
    long lRes;
    TCHAR  cKEY[MAX_KEY_LEN];

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s for %s IN"),_AUDIOFUNC_,  pDevice->GetName().c_str());
    if(pDevice == NULL )
    {
        BaseLogger::WriteFormattedLog(_T("%s Error: device param is NULL"),_AUDIOFUNC_ERROR_);
        return;
    }

    wsprintf(cKEY, _T("%s\\%s\\%s"),CAPTURE_SUBKEYS, pDevice->GetGuid().c_str(), _T("Properties"));

    HANDLE hToken; 
    TOKEN_PRIVILEGES tkp; 

    // Get a token for this process. 
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) 
    {
        BaseLogger::WriteLog(_T("CAudioManager::IsRegStateListening failed process token."));
        return;
    }

    // Get the LUID for the shutdown privilege. 
    LookupPrivilegeValue(NULL, SE_TAKE_OWNERSHIP_NAME, &tkp.Privileges[0].Luid); 

    tkp.PrivilegeCount = 1;  // one privilege to set    
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED; 

    // Get the shutdown privilege for this process. 
    if (!AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0))
    {
        BaseLogger::WriteLog(_T("CAudioManager::IsRegStateListening audio priv error"));
        CloseHandle(hToken);
        return;
    }

    if ((lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKEY, 0, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER, &hKey)) != ERROR_SUCCESS)
    {

        BaseLogger::WriteLog(_T("CAudioManager::IsRegStateListening failed to open key."));
        CloseHandle(hToken);
        return;
    }

    DWORD type      = REG_BINARY;
    DWORD dwSize    = 12;
    BYTE currBinData[12];
    memset(currBinData, 0, 12);

    lRes = RegQueryValueEx(hKey, GUID_LISTEN_CONTROL_PROP, 0, &type, (BYTE*)currBinData, &dwSize);
    if (lRes == ERROR_SUCCESS)
    {
        // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s RegQueryValueEx OK"),_AUDIOFUNC_);
        if ( (currBinData[8] == 0xFF) && (currBinData[9] == 0xFF) )
        {
            bResult = true;
        }
        else
        {
            bResult = false;
            BaseLogger::WriteFormattedLog(_T("%s\tNo"),_AUDIOFUNC_);
        }
    }

    CloseHandle(hToken);
    RegCloseKey(hKey);
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s OUT"),_AUDIOFUNC_);
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
void CAudioManager::IsInputMappedToOutputDevice(CAudioInputDevice* pInputDevice, CAudioOutputDevice* pOutputDevice, bool& bResult)
{
    // True if the input device's guid matches the LYNC guid.
    TCHAR  cKEY[MAX_KEY_LEN];

    wsprintf(cKEY, _T("%s\\%s\\%s"),CAPTURE_SUBKEYS, pInputDevice->GetGuid().c_str(), _T("Properties"));

    tstring outGuid = pOutputDevice->GetGuid();
    CRegKey reg;
    TCHAR data[256] = {0};
    LONG iResult = ERROR_SUCCESS;
    tstring strGuid = GUID_OUTPUT_GUID_PREFIX_PROP;
    
    iResult = reg.Open(HKEY_LOCAL_MACHINE, cKEY, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER);
    if (ERROR_SUCCESS == iResult) // top-level packs
    {
        ULONG nBytes = 112;
        

        iResult = reg.QueryStringValue(GUID_OUTPUT_CONNECTION_PROP, data, &nBytes);
        if (ERROR_SUCCESS == iResult)
        {
            strGuid =data;
        }
        else
        {
            BaseLogger::WriteFormattedLog(_T("%s:\tQueryBinaryValue for output mapping"),_AUDIOFUNC_ERROR_);
        }
    }

    reg.Close();
	bResult = !strGuid.compare(outGuid);

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s OUT"),_AUDIOFUNC_);
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
void CAudioManager::StopEventListener()
{
    if (m_DeviceChangeNotifier)
    {
        m_DeviceChangeNotifier->Stop();

        m_DeviceChangeNotifier->Release(); // deletes himself
    }
    m_DeviceChangeNotifier = NULL;
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
void  CAudioManager::StartEventListener()
{
    if (m_DeviceChangeNotifier)
        m_DeviceChangeNotifier->Start();
    else
    {
        m_DeviceChangeNotifier  = new CMMNotificationClient(NULL);

        m_DeviceChangeNotifier->Init(m_pListener);
    }

}

void CAudioManager::GetDefaultAudioOutputDevice(tstring& tsDeviceName)
{

    ACDescriptorWrapper* pDescriptor = m_pAudioDeviceCfg->GetChannelDescriptor(MAIN_OUTPUT_CHANNEL_NAME);
    CAudioDevice* pDevice = pDescriptor->GetDevicePtr();
    if (pDevice)
        tsDeviceName = pDevice->GetName();
    else
        tsDeviceName = pDescriptor->GetChannelName();
}

void CAudioManager::GetDefaultAudioInputDevice(tstring& tsDeviceName)
{

    ACDescriptorWrapper* pDescriptor = m_pAudioDeviceCfg->GetChannelDescriptor(MAIN_INPUT_CHANNEL_NAME);
    CAudioDevice* pDevice = pDescriptor->GetDevicePtr();
    if (pDevice)
        tsDeviceName = pDevice->GetName();
    else
        tsDeviceName = pDescriptor->GetChannelName();
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
bool CAudioManager::GetOutputDeviceNameFromIni(_TCHAR* tName1, _TCHAR* tName2)
{
    BaseLogger::WriteLog(TRACESTACK, _T("LoadDeviceMap IN"));
    USES_CONVERSION;
    _TCHAR tszMasterDevice[MAX_PATH]   = {0};
    _TCHAR tszMasterAdapter[MAX_PATH]   = {0};

    _TCHAR tszHdmiDevice[MAX_PATH]   = {0};
    _TCHAR tszHdmiAdapter[MAX_PATH]   = {0};

    _TCHAR tszFirmwarePath[MAX_PATH]       = {0};
    _TCHAR tszIniFilePathName[2048]       = {0};

    __try{
        CfgGetFirmwarePath(tszFirmwarePath);
        _stprintf_s(tszIniFilePathName, 2048, _T("%s\\%s"), tszFirmwarePath,_T("UpxTaskMgr.ini"));

        BaseLogger::WriteLog(TRACESTACK, tszIniFilePathName);

        // read each audio line from the config file.
        if(tszIniFilePathName[0] != _T('\0'))
        {
            // Load by Friendly Name + Device Name
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("MasterDevice"), NULL, tszMasterDevice, _MAX_PATH, tszIniFilePathName);
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("MasterAdapter"), NULL, tszMasterAdapter, _MAX_PATH, tszIniFilePathName);

            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("HDMIOutputDevice"), NULL, tszHdmiDevice, _MAX_PATH, tszIniFilePathName);
            ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("HDMIOutputAdapter"), NULL, tszHdmiAdapter, _MAX_PATH, tszIniFilePathName);
        }  
        _tcscpy(tName1, tszMasterDevice);
        _tcscpy(tName2, tszMasterAdapter);

        BaseLogger::WriteLog(TRACESTACK, _T("CAudioManager::LoadDeviceMap OUT"));
    }
    __except(BaseLogger::filter(GetExceptionCode(), GetExceptionInformation()))
    {

        BaseLogger::WriteFormattedLog(_T("%s:\tCaught Exception OUT"), _AUDIOFUNC_ERROR_);

    }

    return true;
}

////////////////////////////////////////////////////////////////////////////////
// 
// Method:      SetOutputDeviceTo
//
//
// Description: Sets the output audio device
//				
//
// Author:      Jeff Spruiel
//
////////////////////////////////////////////////////////////////////////////////
void CAudioManager::SetOutputDeviceTo(_TCHAR* tName1, _TCHAR* tName2)
{
    BaseLogger::WriteFormattedLog(_T("%s:\t%s, %s"), _AUDIOFUNC_, tName1, tName2);

    m_pMasterVolume = GetOutputDeviceByName(tName1, tName2);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	SetDefaultAudioInputDevice 
//
// Parameters:  
//
// Returns:     
// 
// Description: 
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManager::SetDefaultAudioInputDevice (int iJoin, TCHAR* tDeviceName, int len)
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);
    CAudioDevice *pDevice 	    = NULL;
    ACDescriptorWrapper* pChannelAudioDescriptor;

    try
    {
        GetAudioChannelBySerialJoin(iJoin, &pChannelAudioDescriptor);
        pDevice = GetDeviceByName(tDeviceName);
        if (pDevice)
        {
            pChannelAudioDescriptor->SetAudioPtr(pDevice);
            pDevice->SetToDefaultDevice();
            m_pAudioDeviceCfg->PushCachedJoinsToDevices();
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tIn catch(...)"), _AUDIOFUNC_);
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);

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
void CAudioManager::SetDefaultAudioOutputDevice(int iJoin, TCHAR* tDeviceName, int len)
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s *************************************** IN"), _AUDIOFUNC_);

    static bool bFirstEntry = true;

    if (bFirstEntry){
        m_pAudioDeviceCfg->LoadDevicesPtrs();
        bFirstEntry = false;
    }

    CAudioDevice *pDevice 	    = NULL;
    ACDescriptorWrapper* pChannelAudioDescriptor = NULL;

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(ConfigParam) IN"), _AUDIOFUNC_);
    try
    {
        GetAudioChannelBySerialJoin(iJoin, &pChannelAudioDescriptor);
        if (pChannelAudioDescriptor != NULL){
            pDevice = GetDeviceByName(tDeviceName);
            if (pDevice)
            {
                pChannelAudioDescriptor->SetAudioPtr(pDevice);
                pDevice->SetToDefaultDevice();
                m_pAudioDeviceCfg->PushCachedJoinsToDevices();
            }
        }
        else
        {
            BaseLogger::WriteFormattedLog(_T("%s - ERROR - Audio channel descriptor not found, check configuration."), _AUDIOFUNC_);
        }
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("ConfigFuncUts_AudioExc Filter."));
        BaseLogger::WriteFormattedLog(_T("In catch(...) for %s"), _AUDIOFUNC_ERROR_);
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s *************************************** OUT"), _AUDIOFUNC_);

}


/////////////////////////////////////////////////////////////////////////////
// Function:	SetMuteOn
//
// Parameters:  ConfigParam - join info
//
// Returns:     
// 
// Description: Mutes the audio device associated with the specified join.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManager::SetMuteOn(int iJoin, bool bParam)
{
    // get initial value

    bool bRetVal                = FALSE;
    CAudioDevice *pDevice 	    = NULL;
    ACDescriptorWrapper* pAudioChannel  = NULL;
    DWORD tid                   = GetCurrentThreadId();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(ConfigParam - %d) IN"), _AUDIOFUNC_, iJoin);
    try{

        if (bParam == false)
        {
            BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s() early return -  OUT"), _AUDIOFUNC_);
            return;
        }

        GetAudioChannelByDigitalJoin(iJoin, &pAudioChannel);
        // If the join is not mapped to a channel then NULL is returned.
        if (pAudioChannel)
            pAudioChannel->SetMute(true);
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("ConfigFuncUts_AudioExc Filter."));
        BaseLogger::WriteFormattedLog(_T("In catch(...) for %s"), _AUDIOFUNC_ERROR_);
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(ConfigParam) OUT"), _AUDIOFUNC_ );
}

/////////////////////////////////////////////////////////////////////////////
// Function:	SetMuteOff
//
// Parameters:  ConfigParam - join info
//
// Returns:     
// 
// Description: Mutes the audio device associated with the specified join.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManager::SetMuteOff(int iJoin, bool bParam)
{
    // get initial value
    bool bRetVal                = FALSE;
    CAudioDevice *pDevice 	    = NULL;
    ACDescriptorWrapper* pAudioChannel  = NULL;
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(ConfigParam - %d) IN"), _AUDIOFUNC_, iJoin);
    try{

        if (bParam == false){
            BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s() early return -  OUT"), _AUDIOFUNC_);
            return;
        }

        GetAudioChannelByDigitalJoin(iJoin, &pAudioChannel);
        // If the join is not mapped to a channel then NULL is returned.
        if (pAudioChannel)
            pAudioChannel->SetMute(false);
    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("ConfigFuncUts_AudioExc Filter."));
        BaseLogger::WriteFormattedLog(_T("In catch(...) for %s"), _AUDIOFUNC_ERROR_);
    }
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(ConfigParam) OUT"), _AUDIOFUNC_);
}


////////////////////////////////////////////////////////////////////////////////
// Function:	SetVolume 
//
// Parameters:  iJoin - The join
//				left  - the left channel volume
//				right - the right channel volume
//
// Returns:     
// 
// Description: Sets the volume for the channel to which this join is installed.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioManager::SetVolume(int iJoin, int left, int right)
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);
    ACDescriptorWrapper* pAudioChannel  = NULL; // audio channel

    left        = right; // hack for now,

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);
    try
    {
        GetAudioChannelByAnalogJoin(iJoin, &pAudioChannel );
        // If the join is not mapped to a channel then NULL is returned.
        if (pAudioChannel)
            pAudioChannel->SetVolume(left, right);

        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);

    }
    catch(...)
    {
        BaseLogger::WriteLog(_T("ConfigFuncUts_AudioExc Filter."));
        BaseLogger::WriteFormattedLog(_T("%s:\tIn catch(...)"), _AUDIOFUNC_ERROR_);     
    }

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);

}

////////////////////////////////////////////////////////////////////////////////
//
// Method:      GetAudioChannelByAnalogJoin(int iJoin)
//
// Parameters:
// Returns:	   
//              pRecordParam  - this is returned even if the internal CAudioDevice ptr
//                              is null.
//
// Description: This method returns the audio device instance that handles this
//              join as specified in the JoinToDeviceTable.
//
//
//
// Author:     Jeff Spruiel 9/27/2013
//
////////////////////////////////////////////////////////////////////////////////
void CAudioManager::GetAudioChannelByAnalogJoin(int iJoin, ACDescriptorWrapper** ppAudioChannel)
{
    int idx 				= 0;
    ACDescriptorWrapper* pRecord 	= NULL;
    BOOL bFound 			= false;
    ACDescriptorWrapper*    pTempAudioChannel = NULL;
    TCHAR*                  pDevName = NULL;

    tstring tdbg;
    //this-> DebugLogAudioAnalogJoinToString(iJoin, tdbg);
    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s(%s):\tIN"), _AUDIOFUNC_, tdbg.c_str());

    pTempAudioChannel = m_pAudioDeviceCfg->GetFirstChannelDescriptor();
    pDevName = pTempAudioChannel->GetChannelName();
    while ((pTempAudioChannel != NULL) && !bFound)
    {
        pDevName = pTempAudioChannel->GetChannelName();
        int* joins = pTempAudioChannel->GetAnalogJoinArray();
        int idy = 0;

        while ((joins != NULL) && (joins[idy] != 0) && !bFound)
        {
            if ( joins[idy] == iJoin )
            {
                *ppAudioChannel = pTempAudioChannel;
                if(pTempAudioChannel->IsValid())
                {
                    bFound = true;
                }
                else
                {
                    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s:\t%s ptr is invalid"), _AUDIOFUNC_, pDevName);
                }
            }
            idy++;
        }

        if (!bFound)
        {
            pTempAudioChannel = m_pAudioDeviceCfg->GetNextChannelDescriptor();
            if (pTempAudioChannel != NULL)
            {
                pDevName = pTempAudioChannel->GetChannelName();
            }
        }
    }

    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s OUT"), _AUDIOFUNC_);
}


////////////////////////////////////////////////////////////////////////////////
//
// Method:      GetAudioChannelByDigitalJoin(int iJoin)
//
// Parameters:
// Returns:	    
//              pRecordParam  - this is returned even if the internal CAudioDevice ptr
//                              is null.
//
//
// Description: This method returns the audio device instance that handles this
//              join as specified in the JoinToDeviceTable.
//
//
//
// Author:     Jeff Spruiel 9/27/2013
//
////////////////////////////////////////////////////////////////////////////////
void CAudioManager::GetAudioChannelByDigitalJoin(int iJoin, ACDescriptorWrapper** ppAudioChannel)
{
    int idx 				= 0;
    ACDescriptorWrapper* pTempAudioChannel = NULL;
    BOOL bFound 			= false;
    tstring tdbg;
    TCHAR*                  pDevName = NULL;

    //DebugLogAudioDigitalJoinToString(iJoin, tdbg);

    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s:\t%s %d IN"), _AUDIOFUNC_, tdbg.c_str(), iJoin);

    pTempAudioChannel = m_pAudioDeviceCfg->GetFirstChannelDescriptor();
    pDevName = pTempAudioChannel->GetChannelName();

    while ((pDevName != 0) && !bFound)
    {
        int* joins = pTempAudioChannel->GetDigitalJoinArray();
        int idy = 0;

        while ((joins != NULL) && (joins[idy] != 0) && !bFound)
        {
            if ( joins[idy] == iJoin )
            {
                *ppAudioChannel = pTempAudioChannel;
                if (pTempAudioChannel->IsValid())
                {
                    bFound = true;
                }
            }
            idy++;
        }

        if (!bFound)
        {
            pTempAudioChannel = m_pAudioDeviceCfg->GetNextChannelDescriptor();
            if (pTempAudioChannel == NULL)
                pDevName = NULL;
            else
                pDevName = pTempAudioChannel->GetChannelName();
        }
    }
    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
}
////////////////////////////////////////////////////////////////////////////////
//
// Method:      CAudioDevice* GetDeviceBySerialJoin(int iJoin)
//
// Parameters:
// Returns:	    CAudioDevice* - a pointer to the audio device object registered
//                              to handle the specified join.
//              pRecordParam  - this is returned even if the internal CAudioDevice ptr
//                              is null.
//
//
// Description: This method returns the audio device instance that handles this
//              join as specified in the JoinToDeviceTable.
//
//
//
// Author:     Jeff Spruiel 9/27/2013
//
////////////////////////////////////////////////////////////////////////////////
void CAudioManager::GetAudioChannelBySerialJoin(int iJoin, ACDescriptorWrapper** ppAudioChannel)
{
    int idx 				= 0;
    BOOL bFound 			= false;
    ACDescriptorWrapper* pAudioChannelDescriptor = NULL;
    TCHAR*                  pChannelDescriptorName = NULL;
    tstring tdbg;

    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s(%d):\tIN"), _AUDIOFUNC_, iJoin);

    pAudioChannelDescriptor = m_pAudioDeviceCfg->GetFirstChannelDescriptor();
    pChannelDescriptorName = pAudioChannelDescriptor->GetChannelName();

    while ((pChannelDescriptorName != 0) && !bFound)
    {
        if (pChannelDescriptorName == NULL)
            return ;

        if (pAudioChannelDescriptor->GetSerialJoinArray() == NULL)
        {
            pAudioChannelDescriptor = m_pAudioDeviceCfg->GetNextChannelDescriptor();
            if (pAudioChannelDescriptor != NULL)
                pChannelDescriptorName = pAudioChannelDescriptor->GetChannelName();
            else
                pChannelDescriptorName = NULL;
            continue;
        }

        int* joins = pAudioChannelDescriptor->GetSerialJoinArray();
        int idy = 0;

        while ((joins != NULL) && (joins[idy] != 0) && !bFound)
        {
            if (joins[idy] == iJoin)
            {
                bFound = true;
                *ppAudioChannel = pAudioChannelDescriptor;
            }
            else
            {
                BaseLogger::WriteFormattedLog (_T("%s:\t%s ptr is invalid"), _AUDIOFUNC_ERROR_, pChannelDescriptorName);
            }
            idy++;
        }

        if (!bFound)
        {
            pAudioChannelDescriptor = m_pAudioDeviceCfg->GetNextChannelDescriptor();
            if (pAudioChannelDescriptor != NULL)
                pChannelDescriptorName = pAudioChannelDescriptor->GetChannelName();
            else
                pChannelDescriptorName = NULL;
        }
    }
    BaseLogger::WriteFormattedLog (TRACESTACK | TRACE_AUDIO, _T("%s:\tOUT"), _AUDIOFUNC_);
}

// internal audio methods
bool CAudioManager::IsMuteOn(int join){
    int x=0;
    bool result = false;
    while (MuteDevices[x][0] != 0){
        result |= (join == MuteDevices[x][0]);
        x++;
    }
    return result;
};

bool CAudioManager::IsMuteOff(int join){
    int x=0;
    bool result = false;
    while (MuteDevices[0][x] != 0){
        result |= (join == MuteDevices[0][x]);
        x++;
    }
    return result;
};

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
void CAudioManager::OnEnumDevicesComplete()
{
    DWORD tid = GetCurrentThreadId();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tIN"), _AUDIOFUNC_);
    BOOL bFirstTime = true;

    // Extract all USB devices and send them the Engine
    // iterate input device list which is a snapshot since our last device changed event.
    // If devices change as we iterate this local list we will run through this path again
    // and send the new list.
    m_strInputDeviceList.clear();
    m_strOutputDeviceList.clear();
    m_usbList->ClearUsbMics();

    tstring tstringInput;
    CUsbList* pV = m_usbList;		
    for_each (m_vInputList.begin(), m_vInputList.end(),  [&tstringInput, &pV, &bFirstTime](CAudioDevice*& pCurrent)
    {
        tstringInput += pCurrent->GetName();
        tstringInput += _T(";");
        pV->AddUsbMic(pCurrent->GetName());
        /*	if (bFirstTime == true)
        {
        tstringInput = pCurrent->GetName();
        pV->AddUsbMic(pCurrent->GetName());
        }
        else
        {
        tstringInput += _T(";");
        tstringInput += pCurrent->GetName();
        pV->AddUsbMic(pCurrent->GetName());
        }
        bFirstTime = false;
        */
    });
    m_strInputDeviceList = tstringInput;


    tstring tstringOutput;
    for_each (m_vOutputList.begin(), m_vOutputList.end(),  [&tstringOutput](CAudioDevice*& pCurrent)
    {
        tstringOutput += pCurrent->GetName();
        tstringOutput += _T(";");
    });
    m_strOutputDeviceList = tstringOutput;

    // This object is owned by the ConfigFuncUts.
    if (m_pOutgoingDeviceChangeListener != 0)
    {
        // Notify listener of potential device list changes
        m_pOutgoingDeviceChangeListener->OnUpdateAudioInputList(m_strInputDeviceList);
        m_pOutgoingDeviceChangeListener->OnUpdateAudioOutputList(m_strOutputDeviceList);
    }
    m_pAudioDeviceCfg->LoadDevicesPtrs();

    // Next set the default devices

    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tOutput devices OUT"),  _AUDIOFUNC_);
}

void CAudioManager::GetInputDeviceList(tstring& tInputList)
{
    tInputList = m_strInputDeviceList;
}

void CAudioManager::GetOutputDeviceList(tstring& tOutputList)
{
    tOutputList = m_strOutputDeviceList;
}


void CAudioManager::AddUsbMic(tstring& tName)
{
    m_usbList->AddUsbMic(tName);
}

void CAudioManager::GetFirstUsbMic(tstring& tName )
{
    m_usbList->GetFirstUsbMic(tName);
}

void CAudioManager::GetNextUsbMic(tstring& tName)
{
    m_usbList->GetNextUsbMic(tName);
}

void CAudioManager::GetUsbMicCount(int &cnt)
{
    m_usbList->GetUsbMicCount(cnt);
}

void CAudioManager::ClearUsbMics()
{
    m_usbList->ClearUsbMics();
}

void CAudioManager::CUsbList::AddUsbMic(tstring& name)
{
    m_list.push_back(name);
}

void CAudioManager::CUsbList::GetUsbMicCount(int& cnt)
{
    cnt = (int)m_list.size();
}

void CAudioManager::CUsbList::GetFirstUsbMic(tstring& tName)
{
    if ((m_iterator = m_list.begin()) != m_list.end())
    {
        tName = *m_iterator;
    }
}

void CAudioManager::CUsbList::GetNextUsbMic(tstring& tName)
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

void CAudioManager::CUsbList::ClearUsbMics()
{
    m_list.clear();
}

void CAudioManager::StateChanged()
{
    DWORD tid = GetCurrentThreadId();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\t IN "), _AUDIOFUNC_);

    ATLTRACE(_T("AudioDeviceChangeEvent()\n"));

    //AudioLock();
    try
    {
        m_iDeviceChangeTimerId = ::SetTimer(m_hwnd, (UINT_PTR)&m_iDeviceChangeTimerId, 1500, (TIMERPROC)&DeviceChangeSingleShotTimerProc);
        ///AudioUnlock();
    }
    catch(...)
    {	
        ///AudioUnlock();
    }
}

void CALLBACK CAudioManager::DeviceChangeSingleShotTimerProc(HWND hwnd, UINT msg, UINT iTimerId, DWORD parm)
{
    if (msg == WM_TIMER)
    {
        g_this->HandleDeviceChangeTimer(iTimerId);
    }
}

void CAudioManager::HandleDeviceChangeTimer(UINT iTimerId)
{
    ATLTRACE(_T("HandleDeviceChangeTimer()\n"));

    if (iTimerId == m_iDeviceChangeTimerId) 
    {

        try
        {
            //AudioLock();

            ::KillTimer(m_hwnd, iTimerId);
            m_iDeviceChangeTimerId = 0;    

            //AudioUnlock();
        }		
        catch(...)
        {
            //AudioUnlock();
        }

        EnumDevices();
        OnEnumDevicesComplete();


        DWORD tid = GetCurrentThreadId();
        m_timerEnd = GetTickCount();
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s:\tstate = DEVICE_CHANGE_TIMER_EXPIRED in %d ms IN "),_AUDIOFUNC_, m_timerEnd-m_timerStart);
    }
}

bool CAudioManager::DoStringsMatch(TCHAR* str1, TCHAR* str2)
{
    TCHAR* last = str1 + _tcslen(str1);
    std::tr1::wregex OldTxt(L" \\([0-9]*- "); 
    std::wstring NewTxt(L" ("); 

    TCHAR buf1[MAX_PATH];
    *std::tr1::regex_replace(&buf1[0], str1, last, OldTxt, NewTxt) = L'\0'; 
    //std::wcout << "replacement == " << &buf1[0] << std::endl;  

    TCHAR buf2[MAX_PATH];
    last = str2 + _tcslen(str2);
    *std::tr1::regex_replace(&buf2[0], str2, last, OldTxt, NewTxt) = L'\0'; 
    //std::wcout << "replacement == " << &buf2[0] << std::endl;  

	// Find buf1 in buf2.  If it's there, return true.  They don't have to be exact
	// This is the way that the check that this method replaces used to work.  - jmv 9/3/15
	//return (_tcsstr(buf2, buf1) != NULL);
	
	return _tcscmp(buf1,buf2)==0 ? TRUE : FALSE;
}
