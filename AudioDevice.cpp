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

#include <initguid.h>



#include <mmdeviceApi.h>
#include <Guiddef.h>
#include <devpkey.h>
#include <propkey.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <ks.h>
#include <ksmedia.h>
#include <Mmdeviceapi.h>
#include <AudioEngineEndPoint.h>
#include <DeviceTopology.h>
#include <EndpointVolume.h> 


#include "AudioDevice.h"

#include "PolicyConfig.h"
#include <mftransform.h>
#include <math.h>
#include <algorithm>    // std::transform
#include <functional>
#include "decibels.h"


//#define DISABLE_SET_VOLUME
//#define DISABLE_SET_MUTE
//#define DISABLE_SET_VOLUME_PART
//#define SKIP_POLYCOM

//#include <Mmdeviceapi.h>
using namespace std;

const IID IID_IDeviceTopology = __uuidof(IDeviceTopology);
const IID IID_IPart = __uuidof(IPart);
const IID IID_IConnector = __uuidof(IConnector);
const IID IID_IAudioInputSelector = __uuidof(IAudioInputSelector);
const IID IID_IAudioClient = __uuidof(IAudioClient);
const IID IID_IAudioAutoGainControl = __uuidof(IAudioAutoGainControl);
const IID IID_IAudioVolumeLevel = __uuidof(IAudioVolumeLevel);



extern TCHAR tcharDbg[128];
int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);


CAudioDevice::CAudioDevice(IMMDevice* pDevice)
    :m_pDevice(pDevice)
    ,m_strName(_T(""))
    ,m_fVolume(-1)
    ,m_dwState(0)
    ,m_strStateString(_T("NULL"))
    ,m_pEndpointVolume(NULL)
    ,m_strGuid(_T(""))
    ,m_guid(_T(""))
    ,m_pidPropertyString(_T(""))
    ,m_pIAudioMute(NULL)
    ,m_IAudioVolumeLevel(NULL)
    ,m_strDeviceDescriptionName(_T(""))
    ,m_strInterfaceFriendlyName(_T(""))
    ,m_strEnumerationName(_T(""))
    ,m_strPnpName(_T(""))
    ,m_IAudioGC(NULL)
    ,m_nChannels(0)
{
    //BaseLogger::WriteLog(_T("Entered CAudioDevice::CAudioDevice"));
    if (pDevice != NULL)
    {
        pDevice->AddRef();
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s FAILED because pDevice param is NULL - tid = (0x%x)"), _AUDIOFUNC_ , ::GetCurrentThreadId());
    }
}

CAudioDevice::~CAudioDevice()	
{
    SAFE_RELEASE(m_pDevice);
    SAFE_RELEASE(m_pIAudioMute);
    SAFE_RELEASE(m_IAudioVolumeLevel);
    SAFE_RELEASE(m_pEndpointVolume);
     SAFE_RELEASE(m_IAudioGC);
}


void CAudioDevice::Cleanup()
{
}


////////////////////////////////////////////////////////////////////////////////
// 
// Function:    bool Init()
//
// Parameters:  None
//
// Returns:     bool	- true on success
//
// Description: Called by the EnumDevices() method of the AudioSystemSnapshot 
//				object.  Queries the current pointer to the Device object
//				for volume and device name info.
//
// Author:		Jason Vourtsis - 10/11
//
////////////////////////////////////////////////////////////////////////////////

HRESULT CAudioDevice::Init()
{
    HRESULT hr = E_FAIL;

    //BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("CAudioDevice::Init ENTERED - tid = (0x%x)"), ::GetCurrentThreadId());
    if (m_pDevice)
    {		
        //BaseLogger::WriteFormattedLog(_T("CAudioDevice::Init NOT null - tid = (0x%x)\n"), ::GetCurrentThreadId());
        //BaseLogger::WriteLog(_T("Entered CAudioDevice::Init"));
        hr = InitDeviceName();
        EXIT_ON_ERROR(hr);

        hr = InitDeviceState();
        EXIT_ON_ERROR(hr);

        hr = InitVolumeInfo();
        EXIT_ON_ERROR(hr);

        LoadPIDProperty();
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tCore Audio ERROR IMMEdia * is NULL!" ), _AUDIOFUNC_ERROR_);
    }
    return hr;

Exit:
    LogHostError(hr, _AUDIOFUNC_);

	return 0;
}

HRESULT CAudioDevice::Init(TCHAR* tszSubPart)
{
    HRESULT hr = E_FAIL;
    m_fInput = TRUE; // because we know it is an input
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(%s):\tIN)"), _AUDIOFUNC_, tszSubPart);
    if (m_pDevice)
    {		
        m_cstrSubPartName = tszSubPart;
        hr = InitDeviceName();
        hr = InitDeviceState();
        LoadPIDProperty();
        InitializePartPointers(m_pDevice);
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tCore Audio ERROR IMMEdia * is NULL!- tid = (0x%x)\n" ), _AUDIOFUNC_ERROR_);
    }
    return hr;
}

void CAudioDevice::GetID(tstring& strId)
{
    TCHAR* id;

    this->m_pDevice->GetId(&id);
    strId = id;
}

HRESULT CAudioDevice::GetDataDirection(bool* pIsInput)
{
    HRESULT hr = E_FAIL;
    DataFlow flow;
    IDeviceTopology *pDeviceTopology = NULL;
    IConnector *pConnFrom = NULL;

    if (m_pDevice)
    {
        // Get the endpoint device's IDeviceTopology interface.
        hr = m_pDevice->Activate(
            IID_IDeviceTopology, CLSCTX_ALL, NULL,
            (void**)&pDeviceTopology);

        EXIT_ON_ERROR(hr);

        // The device topology for an endpoint device always
        // contains just one connector (connector number 0).
        hr = pDeviceTopology->GetConnector(0, &pConnFrom);
        SAFE_RELEASE(pDeviceTopology)
            EXIT_ON_ERROR(hr);

        // Make sure that this is a capture device.
        hr = pConnFrom->GetDataFlow(&flow);
        EXIT_ON_ERROR(hr);

        SAFE_RELEASE(pConnFrom);
        if (flow == Out)
        {
            // Data flows out of this device into the system, thus, an audio input
            *pIsInput = true;
        }
        else
            *pIsInput = false;

        return hr;
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tDEVICE IS NULL"), _AUDIOFUNC_ERROR_);
    }


Exit:
    LogHostError(hr, _T("CAudioDevice::GetDataDirection"));
    SAFE_RELEASE(pDeviceTopology);
    SAFE_RELEASE(pConnFrom);
    return hr;
}



HRESULT CAudioDevice::InitDeviceName()
{
    USES_CONVERSION;
    HRESULT hr = E_FAIL;
    IPropertyStore* pProperties = NULL;
    DWORD tid = GetCurrentThreadId();

    if (m_pDevice)
    {
        // first get the PNP name
        TCHAR* tsId;
        m_pDevice->GetId(&tsId);
        m_strPnpName = GetDevicePnpName(tsId);
        CoTaskMemFree(tsId);

        hr = m_pDevice->OpenPropertyStore(STGM_READ, &pProperties);
        EXIT_ON_ERROR(hr);

        if (S_OK == hr)
        {
            PROPVARIANT varName;
            PropVariantInit(&varName);
            hr = pProperties->GetValue(PKEY_Device_FriendlyName, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                m_strName = W2T(varName.pwszVal);
                //BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("** Created = %s"), m_strName.c_str());
                PropVariantClear(&varName);

            }



            PropVariantInit(&varName);
            hr = pProperties->GetValue(PKEY_Device_DeviceDesc, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                m_strDeviceDescriptionName = W2T(varName.pwszVal);
                PropVariantClear(&varName);
                //return S_OK;
            }


            PropVariantInit(&varName);

            hr = pProperties->GetValue(PKEY_DeviceInterface_FriendlyName, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                m_strInterfaceFriendlyName = W2T(varName.pwszVal);
                PropVariantClear(&varName);
            }

            hr = pProperties->GetValue(PKEY_Device_EnumeratorName, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                m_strEnumerationName = W2T(varName.pwszVal);
                PropVariantClear(&varName);
            }


            bool IsInput = false;
            GetDataDirection(&IsInput);

            // PKEY_AudioEndpoint_GUID
            hr = pProperties->GetValue(PKEY_AudioEndpoint_GUID, &varName);
            EXIT_ON_ERROR(hr);

            if (S_OK == hr)
            {
                tstring guid = W2T(varName.pwszVal);
                std::string destinationString;

                // Convert the source string to lower case
                // storing the result in destination string

                std::transform(guid.begin(),
                    guid.end(),
                    guid.begin(),
                    ::tolower);

                // store just the guid.
                m_guid = guid;

                PropVariantClear(&varName);
                // return S_OK;
                if (!IsInput)
                {
                    m_strGuid = GUID_OUTPUT_GUID_PREFIX_PROP + guid;
                }
                else
                    m_strGuid = guid;
            }
            BaseLogger::WriteFormattedLog(TRACE_AUDIO_L1, _T("0x%8.8x %-10.10s  %-70.70s %-50.50s"), tid,  m_strEnumerationName.c_str(), m_strName.c_str(), m_strDeviceDescriptionName.c_str());
        }
    }
    else
    {
        TCHAR dbgMsg[256] = {_T('\0')};
        wsprintf(dbgMsg, _T("CAudioDevice::InitDeviceName Core Audio ERROR IMMEdia * is NULL!\n" ));
        BaseLogger::WriteLog(dbgMsg);
    }


Exit:
    LogHostError(hr, _T("CAudioDevice::InitDeviceName"));
    SAFE_RELEASE(pProperties);
    return hr;
}



//
// Create the class path-like "PnpName" that Lync writes into the registry
// to decide which audio devices to use.  These will be written into the
// HKEY_USERS\SID\Software\Microsoft\UCCPlatform\Lync\ registry key to 
// override the use of Lync-specific devices
//
// jmv 11/14
//

tstring CAudioDevice::GetDevicePnpName(tstring strDeviceID)
{
    HRESULT hr;
    tstring strPnpName = _T("");

    IPropertyStore *propertyStore;
    hr = m_pDevice->OpenPropertyStore(STGM_READ, &propertyStore);
    //CHK("Unable to open device property store", hr);

    DWORD nPropertyCount = 0;
    propertyStore->GetCount(&nPropertyCount);

    for (DWORD n = 0; n < nPropertyCount; ++n)
    {
        PROPERTYKEY Key;
        PROPVARIANT Value;
        PropVariantInit(&Value);

        hr = propertyStore->GetAt(n, &Key);

        if (S_OK == hr)
        {
            propertyStore->GetValue(Key, &Value);

            if (Value.vt == VT_LPWSTR)
            {
                tstring strValue = Value.pwszVal;

                // Look for "\\?\" in the property
                int nStart = (int)strValue.find(_T("\\\\?\\"));

                if (-1 != nStart)
                {
                    // We found a string that is a class path.  Add our ID to it and return it
                    int nEnd = (int)strValue.rfind(_T("#"));
                    strPnpName = strValue.substr(nStart, nEnd-nStart+1);
					int nEndCaps = (int)strPnpName.rfind(_T("#"), (strPnpName.length() - 2));						// We'll later capitalize all the text just in this section of the string - jmv 4/2016
                    strPnpName += strDeviceID;
                    strPnpName += _T("#");

                    //int nEndCaps = strPnpName.rfind(_T("#"), nEnd-1);

                    if (nEndCaps < strPnpName.length())
                    {
                        for (int i=0; i < nEndCaps; ++i)
                            strPnpName[i] = _totupper(strPnpName[i]);
                    }

                    break;
                }
            }

        }
    }

//Exit:
    SAFE_RELEASE(propertyStore);
    return strPnpName;
}



void CAudioDevice::SetLyncDevice(bool fInput)
{
    HKEY hKey;
    TCHAR tszName[_MAX_PATH];
    DWORD dwIndex = 0;
    DWORD dwNameLen = _MAX_PATH;

    if (m_strPnpName.empty())
        return;

    // Set these values for each user SID that's in the HKEY_USERS registry branch

    LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS, &hKey );
    LONG lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);

    //TCHAR tszValueData[_MAX_PATH];

    while (ERROR_SUCCESS == lResultEnum)
    {
        // Exclude short keys
        if (dwNameLen > 9)
        {
            // Exclude "Classes" keys
            if (NULL == (_tcsstr(tszName, _T("_Classes"))))
            {
                TCHAR tszKey[_MAX_PATH];


                HKEY hKeyLync;

                _stprintf_s(tszKey, _MAX_PATH, _T("%s\\Software\\Microsoft\\UCCPlatform\\Lync"), tszName);
                lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLync );
                if(ERROR_SUCCESS == lResult)
                {
                    if (fInput)
                        SetLyncInputDeviceRegKeys(hKeyLync, m_strName, m_strPnpName);
                    else
                        SetLyncOutputDeviceRegKeys(hKeyLync, m_strName, m_strPnpName);

                    RegCloseKey(hKeyLync);
                }
            }
        }

        dwNameLen = _MAX_PATH;
        ++dwIndex;
        lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
    }		

}

bool CAudioDevice::SetLyncInputDeviceRegKeys(HKEY hKey, const tstring strDeviceName, const tstring strPnpName)
{
    HRESULT hr;

    hr = RegSetValueEx(hKey, _T("PrefAudioCapture"), NULL, REG_SZ, (BYTE*) m_strName.c_str(), (DWORD)m_strName.length() * sizeof(TCHAR));

    if (hr == ERROR_SUCCESS)
        hr = RegSetValueEx(hKey, _T("PrefAudioCapturePnpName"), NULL, REG_SZ, (BYTE*) strPnpName.c_str(), (DWORD)strPnpName.length() * sizeof(TCHAR));

    if (hr == ERROR_SUCCESS)
    {
        DWORD dwValue = 1;
        hr = RegSetValueEx(hKey, _T("PrefAudioIsManual"), NULL, REG_DWORD, (BYTE*) &dwValue, sizeof(DWORD));
    }

    if (hr != ERROR_SUCCESS)
    {
        BaseLogger::WriteLog(_T("SetLyncDeviceRegKeys Failed to set keys"));
        return false;
    }

    return true;
}


bool CAudioDevice::SetLyncOutputDeviceRegKeys(HKEY hKey, const tstring strDeviceName, const tstring strPnpName)
{
    HRESULT hr;

    hr = RegSetValueEx(hKey, _T("PrefAudioRender"), NULL, REG_SZ, (BYTE*) strDeviceName.c_str(), (DWORD)strDeviceName.length() * sizeof(TCHAR));

    if (hr == ERROR_SUCCESS)
        hr = RegSetValueEx(hKey, _T("PrefAudioRenderPnpName"), NULL, REG_SZ, (BYTE*) strPnpName.c_str(), (DWORD)strPnpName.length() * sizeof(TCHAR));

    if (hr == ERROR_SUCCESS)
    {
        DWORD dwValue = 1;
        hr = RegSetValueEx(hKey, _T("PrefAudioIsManual"), NULL, REG_DWORD, (BYTE*) &dwValue, sizeof(DWORD));
    }

    if (hr != ERROR_SUCCESS)
    {
        BaseLogger::WriteLog(_T("SetLyncDeviceRegKeys Failed to set keys"));
        return false;
    }

    return true;
}



HRESULT CAudioDevice::InitDeviceState()
{
    HRESULT hr = E_FAIL;
    if (m_pDevice)
    {
        hr = m_pDevice->GetState(&m_dwState);
        EXIT_ON_ERROR(hr);

        switch(m_dwState)
        {
        case DEVICE_STATE_ACTIVE:
            m_strStateString = _T("ACTIVE");
            break;

        case DEVICE_STATE_DISABLED:
            m_strStateString = _T("DISABLED");
            break;

        case DEVICE_STATE_NOTPRESENT:
            m_strStateString = _T("NOT_PRESENT");
            break;

        case DEVICE_STATE_UNPLUGGED:
            m_strStateString = _T("UNPLUGGED");
            break;
        }
    }
    else
    {
        TCHAR dbgMsg[256] = {_T('\0')};
        wsprintf(dbgMsg, _T("CAudioDevice::InitDeviceState Core Audio ERROR IMMEdia * is NULL!\n" ));
        BaseLogger::WriteLog(dbgMsg);
    }


Exit:
    LogHostError(hr, _AUDIOFUNC_);
    return hr;
}


HRESULT CAudioDevice::InitVolumeInfo()
{
    HRESULT hr = E_FAIL;
    if (m_pDevice)
    {
        try{
            hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);
            EXIT_ON_ERROR(hr);
        }
        catch(...)
        {
            BaseLogger::WriteFormattedLog(_T("CAudioDevice::InitVolumeInfo - Activate - tid = (0x%x)"), ::GetCurrentThreadId());
            EXIT_ON_ERROR(hr);
        }


        try{
            hr = m_pEndpointVolume->GetMasterVolumeLevelScalar(&m_fVolume);
            EXIT_ON_ERROR(hr);
        }
        catch(...)
        {
            BaseLogger::WriteFormattedLog(_T("CAudioDevice::InitVolumeInfo - GetMasterVolumeLevelScalar - tid = (0x%x)"), ::GetCurrentThreadId());
            EXIT_ON_ERROR(hr);
        }


        BOOL fTempMute;
        try{
            hr = m_pEndpointVolume->GetMute(&fTempMute);
            EXIT_ON_ERROR(hr);
        }
        catch(...)
        {
            BaseLogger::WriteFormattedLog(_T("CAudioDevice::InitVolumeInfo - GetMute- tid = (0x%x)"), ::GetCurrentThreadId());
            EXIT_ON_ERROR(hr);
        }
        m_bMuted = (fTempMute == TRUE);

        try{
            hr = m_pEndpointVolume->GetChannelCount(&m_nChannels);
            EXIT_ON_ERROR(hr);
        }
        catch(...){
            BaseLogger::WriteFormattedLog(_T("CAudioDevice::InitVolumeInfo - GetChannelCount- tid = (0x%x)"), ::GetCurrentThreadId());
            EXIT_ON_ERROR(hr);

        }

        for (UINT i=0; i< m_nChannels; ++i)
        {
            try{

                hr = m_pEndpointVolume->GetChannelVolumeLevelScalar(i, &m_fChannelVolumes[i]);
                EXIT_ON_ERROR(hr);

            }
            catch(...){
                BaseLogger::WriteFormattedLog(_T("CAudioDevice::InitVolumeInfo - GetChannelVolumeLevelScalar- tid = (0x%x)"), ::GetCurrentThreadId());
                EXIT_ON_ERROR(hr);

            }
        }

        SAFE_RELEASE(m_pEndpointVolume);

        return hr;			

    }
    else
    {
        TCHAR dbgMsg[256] = {_T('\0')};
        wsprintf(dbgMsg, _T("CAudioDevice::InitVolumeInfo Core Audio ERROR IMMEdia * is NULL! - tid = (0x%x)\n" ), ::GetCurrentThreadId());
        BaseLogger::WriteLog(dbgMsg);
    }

Exit:
    SAFE_RELEASE(m_pEndpointVolume);
    LogHostError(hr, _AUDIOFUNC_);	
    return hr;

}


// Retrieves the specified channel volume.
// Assumption: Device volume has a left and right channel
bool CAudioDevice::GetCurrentChannelVolume(int channel, int &nVolume)
{
    float volume = 0.0;
    HRESULT hr = S_OK;
    try{
        if (m_pDevice)
        {
            hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);
            EXIT_ON_ERROR(hr);
        }
        else
        {
            TCHAR dbgMsg[256] = {_T('\0')};
            wsprintf(dbgMsg, _T("CAudioDevice::GetCurrentChannelVolume Core Audio ERROR IMMEdia * is NULL! - tid = (0x%x)\n" ), ::GetCurrentThreadId());
            BaseLogger::WriteLog(dbgMsg);
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("CAudioDevice::GetCurrentChannelVolume - Activate - tid = (0x%x)"), ::GetCurrentThreadId());
        EXIT_ON_ERROR(hr);
    }


    try{
        if (m_pEndpointVolume){
            hr = m_pEndpointVolume->GetChannelVolumeLevelScalar(channel, &volume);
            EXIT_ON_ERROR(hr);
        }
        else
        {
            TCHAR dbgMsg[256] = {_T('\0')};
            wsprintf(dbgMsg, _T("CAudioDevice::GetCurrentChannelVolume Core Audio ERROR IMMEdia * is NULL! - tid = (0x%x)\n" ), ::GetCurrentThreadId());
            BaseLogger::WriteLog(dbgMsg);
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("CAudioDevice::GetCurrentChannelVolume - GetChannelVolumeLevelScalar - tid = (0x%x)"), ::GetCurrentThreadId());
        EXIT_ON_ERROR(hr);
    }


    float num, frac;
    volume = volume*100;
    frac = modf(volume, &num);
    num +=(frac > 0.5) ? 1:0;
    nVolume = (int) (num);
    //wsprintf(tcharDbg,_T("GetCurrentChannelVolume () returns %d\n"), nVolume);  
    //BaseLogger::WriteLog (tcharDbg);
    SAFE_RELEASE(m_pEndpointVolume);
    return true;

Exit:
    TCHAR dbgMsg[256] = {_T('\0')};
    wsprintf(dbgMsg, _T("CAudioDevice::GetCurrentChannelVolume - tid = (0x%x)\n" ), ::GetCurrentThreadId());
    LogHostError(hr, dbgMsg);
    SAFE_RELEASE(m_pEndpointVolume);
    return false;
}

bool CAudioDevice::SetVolume( int leftVolume,  int rightVolume)
{
#ifndef DISABLE_SET_VOLUME
    float m_fLeftVolume = (float)leftVolume / (float)100.0;
    float m_fRightVolume = (float)rightVolume / (float)100.0;
    HRESULT hr = E_FAIL;

    if (m_pDevice)
    {
        hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);
        EXIT_ON_ERROR(hr);
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s(%d, %d)\tCore Audio ERROR IMMEdia * is NULL! " ), _AUDIOFUNC_ERROR_, leftVolume, rightVolume);
    }

    if (m_cstrSubPartName.IsEmpty())
    {
        if (m_pEndpointVolume)
        {
            for (UINT i = 0; i < m_nChannels; i++)
            {
                hr = m_pEndpointVolume->SetChannelVolumeLevelScalar(i, m_fLeftVolume, NULL);
                if (hr!=S_OK)
                {
                    UINT nChannels = 0;
                    BaseLogger::WriteFormattedLog(_T("%s(l,r):\tchannel#:%d, hr = 0x%8.8x"), _AUDIOFUNC_ERROR_, i, hr);
                    m_pEndpointVolume->GetChannelCount(&nChannels);
                    BaseLogger::WriteFormattedLog(_T("%s(l, r):\tchannel count = %d\n"), nChannels, m_nChannels);
                }
                EXIT_ON_ERROR(hr);
            }
        }
    }
    else
    {
        m_IAudioVolumeLevel->SetLevel(0, m_fLeftVolume , NULL);
    }

    SAFE_RELEASE(m_pEndpointVolume);
    return true;


Exit:
    TCHAR dbgMsg[256] = {_T('\0')};
    if (GetName().length() > 0)
        wsprintf(dbgMsg, _T("%s(%d,%d):\tExit On Error - %s" ),_AUDIOFUNC_ERROR_, leftVolume, rightVolume, this->GetName().c_str());

    if (!this->m_cstrSubPartName.IsEmpty())
        wsprintf(dbgMsg, _T("%s(%d,%d):\tExit On Error Subdevice: %s" ),_AUDIOFUNC_ERROR_, leftVolume, rightVolume, this->m_cstrSubPartName.GetString());

    SAFE_RELEASE(m_pEndpointVolume);
    LogHostError(hr, dbgMsg);
    // BaseLogger::WriteFormattedLog(_T("%s(%d, %d):\tExit On Error "), _AUDIOFUNC_ERROR_, leftVolume, rightVolume);
#endif
    return false;
}

bool CAudioDevice::SetPartVolume(int nVol)
{
#ifndef DISABLE_SET_VOLUME_PART
    float fNewLevel;
    float fActual = (float) 0.0;

    if (!m_cstrSubPartName.IsEmpty())
    {
        //BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(%d) %s - m_pEndP: 0x%8.8x, iVolP: 0x%8.8x, iMuteP: 0x%8.8x"), _AUDIOFUNC_, nVol, m_cstrSubPartName.GetString(), this->m_pDevice, this->m_IAudioVolumeLevel, this->m_pIAudioMute); 
        BaseLogger::WriteFormattedLog(_T("%s : %s, "), _AUDIOFUNC_, m_cstrSubPartName.GetString());
    }
    else
    {
        BaseLogger::WriteLog(_T("This part string name is EMPTY!!!!"));
    }

    fNewLevel = (float)decibels[nVol];
    ATLTRACE(_T("-----------------------------\n"));

    //ATLTRACE(_T("[scalar, db] | [%d, %f] "), nVol, fNewLevel);
    m_IAudioVolumeLevel->SetLevel(0, fNewLevel , NULL);
    m_IAudioVolumeLevel->SetLevel(1, fNewLevel , NULL);

    m_IAudioVolumeLevel->GetLevel(0, &fActual);
    //ATLTRACE(_T("Actual %f\n"), fActual );    
#endif
    return true;
}

bool CAudioDevice::SetVolume(int nVol)
{
#ifndef DISABLE_SET_VOLUME
	// We need to cycle the volume between two values when trying to set it to a specific value.
	// Add or subtract one to the volume level and set that first, then set the value that we want
	// This avoids USB devices entering a state that reports back the correct volume, but the device
	// is actually using the default volume.  - jmv 6/15
	int nFakeVol = nVol - 1;
	if (nFakeVol < 0)
		nFakeVol = 1;
	float fFakeVol = (float)nFakeVol / (float)100.0;

    if (!m_cstrSubPartName.IsEmpty())
    {
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s(%d) on m_pIAudioVol: 0x%8.8x"), _AUDIOFUNC_, nVol, m_IAudioVolumeLevel);
    }

	if (m_IAudioVolumeLevel)
	{
		SetPartVolume(nFakeVol);
		return SetPartVolume(nVol);
	}


    m_fVolume = (float)nVol / (float)100.0;
    HRESULT hr = E_FAIL;
    if (m_pDevice != NULL)
    {
        hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);

        EXIT_ON_ERROR(hr);
    }
    else
    {
        TCHAR dbgMsg[256] = {_T('\0')};
        wsprintf(dbgMsg, _T("CAudioDevice::SetVolume() Core Audio ERROR IMMEdia * is NULL! - tid = (0x%x)\n" ), ::GetCurrentThreadId());
        BaseLogger::WriteLog(dbgMsg);

    }

    if (m_pEndpointVolume)
    {
        hr = m_pEndpointVolume->GetChannelCount(&m_nChannels);
        EXIT_ON_ERROR(hr);

        if (m_nChannels > 1)
        {
            SAFE_RELEASE(m_pEndpointVolume);
			SetVolume(nFakeVol, nFakeVol);
			bool res = SetVolume(nVol, nVol);
        }
        else
        {
			m_pEndpointVolume->SetMasterVolumeLevelScalar(fFakeVol, NULL);
            hr = m_pEndpointVolume->SetMasterVolumeLevelScalar(m_fVolume, NULL);
            EXIT_ON_ERROR(hr);
            SAFE_RELEASE(m_pEndpointVolume);
        }
        return true;
    }

Exit:
    TCHAR dbgMsg[256] = {_T('\0')};
    if (GetName().length() > 0)
        wsprintf(dbgMsg, _T("%s(%d): Exit On Error - tid(%d): %s" ),_AUDIOFUNC_, nVol, ::GetCurrentThreadId(), this->GetName().c_str());

    if (!this->m_cstrSubPartName.IsEmpty())
        wsprintf(dbgMsg, _T("%s(%d): Exit On Error - tid(%d): %s" ),_AUDIOFUNC_, nVol, ::GetCurrentThreadId(), this->m_cstrSubPartName.GetString());

    SAFE_RELEASE(m_pEndpointVolume);
    LogHostError(hr, dbgMsg);	
#endif
    return false;

}



bool CAudioDevice::SetMute(bool bMute)
{
    
    HRESULT hr = E_FAIL;
    int nChannels = 0;

#ifndef DISABLE_SET_MUTE

    if (m_pDevice)
    {
        if (m_pIAudioMute)
        {
			//m_pIAudioMute->SetMute(!bMute, NULL);	// cycle mute setting evey time - jmv 6/15
            m_pIAudioMute->SetMute(bMute, NULL);
            BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("SetMute(%d) on m_pIAudioMute 0x%8.8x"), bMute, m_pIAudioMute);
        }
        else
        {
            hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);
            EXIT_ON_ERROR(hr);
            BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("SetMute(%d) on m_pDevice = 0x%8.8x"), bMute, m_pDevice);
			//hr = m_pEndpointVolume->SetMute(!bMute, NULL);	// cycle mute setting evey time - jmv 6/15
			//EXIT_ON_ERROR(hr);

            //Sleep(13000);

			hr = m_pEndpointVolume->SetMute(bMute, NULL);
            EXIT_ON_ERROR(hr);
            SAFE_RELEASE(m_pEndpointVolume);
        }
        m_bMuted = bMute;
        return true;
    }
    else
    {
        BaseLogger::WriteFormattedLog(_T("%s\tCore Audio ERROR IMMEdia * is NULL!\n" ), _AUDIOFUNC_ERROR_);
    }

Exit:
    TCHAR dbgMsg[256] = {_T('\0')};
    if (GetName().length() > 0)
        wsprintf(dbgMsg, _T("%s(%d): Exit On Error - tid(%d): %s" ),_AUDIOFUNC_ERROR_, bMute, ::GetCurrentThreadId(), this->GetName().c_str());

    if (!this->m_cstrSubPartName.IsEmpty())
        wsprintf(dbgMsg, _T("%s(%d): Exit On Error - tid(%d): %s" ),_AUDIOFUNC_ERROR_, bMute, ::GetCurrentThreadId(), this->m_cstrSubPartName.GetString());
    
    SAFE_RELEASE(m_pEndpointVolume);
    LogHostError(hr, dbgMsg);
#endif
    return false;
}

bool CAudioDevice::GetMute(void) 
{
    HRESULT hr = E_FAIL;
    BOOL bMute = TRUE;
    if (m_pDevice)
    {
        if (m_pIAudioMute)
        {
            m_pIAudioMute->GetMute(&bMute);
            m_bMuted = (BOOL)bMute;
        }
        else
        {
            hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume), CLSCTX_ALL, NULL, (LPVOID *)&m_pEndpointVolume);
            EXIT_ON_ERROR(hr);
            hr = m_pEndpointVolume->GetMute((BOOL*)&m_bMuted);
            EXIT_ON_ERROR(hr);
            SAFE_RELEASE(m_pEndpointVolume);
        }
        return m_bMuted;
    }
    else
    {
        TCHAR dbgMsg[256] = {_T('\0')};
        wsprintf(dbgMsg, _T("CAudioDevice::GetMute() Core Audio ERROR IMMEdia * is NULL! - tid = (0x%x)\n" ), ::GetCurrentThreadId());
        BaseLogger::WriteLog(dbgMsg);

    }
Exit:
    TCHAR dbgMsg[256] = {_T('\0')};
    wsprintf(dbgMsg, _T("CAudioDevice::GetMute  - tid = (0x%x)\n" ), ::GetCurrentThreadId());
    LogHostError(hr, dbgMsg);	
    return false;
}

void CAudioDevice::SetToDefaultDevice()
{
    BaseLogger::WriteFormattedLog(_T("%s:\tthis:\"%s\""), _AUDIOFUNC_INFO_, this->m_strDeviceDescriptionName.c_str());

    IPolicyConfigVista *pPolicyConfig;
    ERole reserved = eConsole;

    try{
        HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), 
            NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
        if (SUCCEEDED(hr))
        {

            TCHAR* id;

            this->m_pDevice->GetId(&id);
            tstring strName = GetName();


            HRESULT hrConsole = pPolicyConfig->SetDefaultEndpoint(id /*devID*/, eConsole);
            HRESULT hrMultimedia = pPolicyConfig->SetDefaultEndpoint(id /*devID*/, eMultimedia);
            HRESULT hrComms = pPolicyConfig->SetDefaultEndpoint(id/*devID*/, eCommunications);

            pPolicyConfig->Release();

            BaseLogger::WriteFormattedLog(_T("%s:\t%s[%s] Results: Con[%d] Mult[%d] Comm[%d]"), _AUDIOFUNC_,strName.c_str(), id, hrConsole, hrMultimedia, hrComms);

            return;
        }
    }
    catch(...)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tException"), _AUDIOFUNC_ERROR_);
    }
    BaseLogger::WriteFormattedLog(_T("%s:\tOUT"), _AUDIOFUNC_);
    
}

void CAudioDevice::GetDeviceEnumerator(tstring& name)
{
    name = m_strEnumerationName;   
}



/* EVERYTHING BELOW THIS LINE needs serious refactoring. The code was
used to explorer device topology APIs and to understand Connectors,
Subunits and Parts. It eventually took form as a recursive implementation
of a specific solutuion (looking for Line In and Microphone subunits within
the Speaker Mix endpoint.

Generalizing this code requires much more thought regarding what type of 
services a product may require. Examples: 

Building an framework for disovering and presenting topolgy of an endpoint
as opposed to all devices on the system; 


*/

////////////////////////////////////////////////////////////////////////////////
//
// Parameters:  
//
// Returns:
//
// Description: Traverses the data path from start to end connector
//              to find the Mic and Line subunits for the speaker 
//				endpoint device.
//              
//				
//              
//
// Author:      Jeff Spruiel
//				12/12/2013
//
////////////////////////////////////////////////////////////////////////////////
HRESULT CAudioDevice::InitializePartPointers(IMMDevice *pEndptDev)
{
    HRESULT hr = S_OK;
    DataFlow flow;
    IDeviceTopology *pDeviceTopology = NULL;
    IConnector *pConnFrom = NULL;


    // BaseLogger::WriteFormattedLog(_T("%s IN"), _AUDIOFUNC_);
    CString csName;
    bool bReturn = false;
    if (pEndptDev == NULL)
    {
        EXIT_ON_ERROR(hr = E_POINTER);
    }

    // Print the EndpointDevice name
    LPWSTR pDevId;
    pEndptDev->GetId(&pDevId);

    // Get the endpoint device's IDeviceTopology interface.
    hr = pEndptDev->Activate(IID_IDeviceTopology, CLSCTX_ALL, NULL,  (void**)&pDeviceTopology);
    EXIT_ON_ERROR(hr);

    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
    SAFE_RELEASE(pDeviceTopology);  
    EXIT_ON_ERROR(hr);

    // Make sure that this is a PLAYBACK device.
    hr = pConnFrom->GetDataFlow(&flow);  
    EXIT_ON_ERROR(hr);
    ConnectorType pCType;
    pConnFrom->GetType(&pCType);
    if (flow != In)
    {
        ATLTRACE(_T("Connector Flow Output\n"));
        // Error -- this is a rendering device.
        EXIT_ON_ERROR(hr = AUDCLNT_E_WRONG_ENDPOINT_TYPE)
    }
    else
    {
        ATLTRACE(_T("Connector Flow Input\n"));
    }

    // Traverses the data path
    // through a device topology starting at the previous input
    // connector and ending at the next device's connector.
    VisitConnector( pConnFrom );

Exit:
    // SAFE_RELEASE(pConnFrom);
    // BaseLogger::WriteFormattedLog(_T("%s OUT\n"), _AUDIOFUNC_);
    return hr;
}

////////////////////////////////////////////////////////////////////////////////
//
// Parameters:  pConnFrom - the connector to evaluate
//
// Returns:
//
// Description: Called when a connector is encountered to evaluate it. It
//              may be the end of the path.
//              
//				
//              
//
// Author:      Jeff Spruiel
//				12/12/2013
//
////////////////////////////////////////////////////////////////////////////////
bool CAudioDevice::VisitConnector(IConnector *pConnFrom )
{
    IConnector *pConnTo		= NULL;
    IPart* pPart			= NULL;
    BOOL bConnected 		= false;
    bool bReturn               = false;
    HRESULT                hr = pConnFrom->IsConnected(&bConnected);
    EXIT_ON_ERROR(hr);

    // Does this connector connect to another device?
    if (bConnected == FALSE)
    {
        // This is the end of the data path that
        // stretches from the endpoint device to the
        // system bus or external bus. Verify that
        // the connection type is Software_IO.
        ConnectorType  connType;
        hr = pConnFrom->GetType(&connType);
        EXIT_ON_ERROR(hr);

        if (connType == Software_IO)
        {
            //  BaseLogger::WriteFormattedLog(_T("%s ConnectionType  is %d "), _AUDIOFUNC_, connType);
            // return true;  this is not good. My current understanding is that traversal should end
            // and return true. There is more work to be done to make this correct. For now it works.
            return false;
        }
        EXIT_ON_ERROR(hr = E_FAIL);
    }

    // Get the connector in the next device topology,
    // which lies on the other side of the connection.
    hr = pConnFrom->GetConnectedTo(&pConnTo);
    EXIT_ON_ERROR(hr);
    SAFE_RELEASE(pConnFrom);


    // Get the connector's IPart interface.
    hr = pConnTo->QueryInterface(IID_IPart, (void**)&pPart);	

    // Start search  parts for the correct subunit type.
    // TBD: Fix this string. Fixing it sucks in more generalizations 
    // to be considered. For now the code looks specifically for 
    // Microphone or Line In.
    bReturn = TraversePart(pPart);

Exit:
    SAFE_RELEASE(pPart);
    SAFE_RELEASE(pConnFrom);
    SAFE_RELEASE(pConnTo);
    return bReturn;
}


/*
Checks the part and other upstream compoonents.
TBD: error checking and input validation.
*/
bool CAudioDevice::TraversePart(IPart* pPart)
{

    IConnector*     pConnFrom       = NULL;
    IPart*          pPartNext      	= NULL;
    IPartsList*     pPartsList		= NULL;
    PartType        parttype        = Connector;   
    LPWSTR          lpsStrPartName  = NULL;

    HRESULT         hr              = S_OK;
    UINT            localId         = 0;
    UINT            cnt             = 0;
    bool            bRet            = false;

    //
    // Follow all downstream links to next part.
    //
    hr = pPart->EnumPartsIncoming(&pPartsList);
    EXIT_ON_ERROR(hr);
    pPartsList->GetCount(&cnt);
    if (cnt > 1)
        BaseLogger::WriteFormattedLog(_T("%s\t(IPArt*:%d) # Parts Incoming = %d"), _AUDIOFUNC_, pPart, cnt);

    // Traverse each incomming parts
    for ( UINT partNum = 0; partNum < cnt; partNum++ )
    {
        hr = pPartsList->GetPart(partNum, &pPartNext);
        hr = pPartNext->GetPartType(&parttype);

        if (parttype == Connector)
        {   
            // We've reached the output connector that
            // lies at the end of this device topology.
            // Continue downstream.
            hr = pPartNext->QueryInterface(IID_IConnector,  (void**)&pConnFrom);    
            EXIT_ON_ERROR(hr);
            SAFE_RELEASE(pPartNext);

            bRet = VisitConnector(pConnFrom);
            SAFE_RELEASE(pConnFrom);

            if (bRet == true)
            {
                BaseLogger::WriteFormattedLog(_T("%s reached termination condition"), _AUDIOFUNC_);
                SAFE_RELEASE(pConnFrom);
                return bRet; // termination condition
            }


            continue;
        }

        //
        //  Set the pointers if the criteria is satisfied
        //
        SetPartMuteAndVolumePtrs(pPartNext);

        //bRet=false;
        if (bRet == true)
        {
            SAFE_RELEASE(pPartNext);
            return bRet;
        }

        bRet = TraversePart(pPartNext);
        SAFE_RELEASE(pPartNext);
    }// endfor

    return bRet;

Exit:
    SAFE_RELEASE(pConnFrom);
    return false;
}





////////////////////////////////////////////////////////////////////////////////
//
// Parameters:  part - the Core Audio part.
//
// Returns:
//
// Description: This pattern match is topology specific and therefore codec
//              specific. This method sets the mute and volume pointers when
//              the two back to back part names for mute and volume are found
//              with names that are contained in the specified subpart name
//              defined in UpxTaskMgr.ini.
/*
Example:
LINE_IN = Speaker Mix | Microphone
When a part containing the word 'mute' is received, the next part linked to it 
will be a volume control, but we don't know if this linked mute and volume belongs
to the Microhone, Line In, HDMI or some other channel. So the second name, in the
UpxTaskMgr, in the above case Microphone, identifies the audio channel. For this
codec and corresponding pin configuration, the name of the volume control will
contain a string that identifies a channel. Again, in this example, we are 
looking for a mute and volume device where the volume name contains Microphone.
*/
//
//
//
//
// Author:      Jeff Spruiel
//              12/12/2013
//
////////////////////////////////////////////////////////////////////////////////

void CAudioDevice::SetPartMuteAndVolumePtrs(IPart* part)
{
    LPWSTR partName = NULL;
    CString cstrPartName ;
    UINT icnt =  0;
    static IPart *previousPart = NULL;

    part->GetControlInterfaceCount(&icnt);
    HRESULT hr;

    part->GetName(&partName);
    cstrPartName = partName;
   // BaseLogger::WriteFormattedLog ( _T("%s(IPart*:%d)\tpart name = %s - IN"), _AUDIOFUNC_, part, partName);

    // Return if both pointer are initialized.
    if ((m_pIAudioMute != NULL) && (m_IAudioVolumeLevel != NULL))
    {
        //BaseLogger::WriteFormattedLog(  _T("%s\tVol+Mute acquired for part: %s\n"), _AUDIOFUNC_, partName);
        SAFE_RELEASE(part);
        return;
    }

    //
    // Our first hit must be a mute, but it should not
    // contain the string "Master"
    //
    if (cstrPartName.Find (_T("Mute"), 0) != -1)
    {
        if (cstrPartName.Find (_T("Master"), 0) != -1)
        {
            SAFE_RELEASE(part);
            return ;
        }

        // save this aside in case the next part that arrives is a 
        // volume control with a part name matching our member variable
        previousPart = part;
    }
    else if (cstrPartName.Find (m_cstrSubPartName.GetBuffer(), 0) != -1)
    {
        BaseLogger::WriteFormattedLog(_T("%s\tActivate Vol and Mute on: %s"), 
            _AUDIOFUNC_, 
            partName); 

        // Assumption: the previous part name was mute.
        hr = previousPart->Activate(CLSCTX_ALL, __uuidof(IAudioMute), (void**) &m_pIAudioMute);
        if (hr != S_OK)
            BaseLogger::WriteFormattedLog(_T("%s:\tfailed Activate for IAudioMute hr = 0x%8.8x for part = %s"), _AUDIOFUNC_ERROR_, hr, cstrPartName.GetBuffer());

        hr = part->Activate(CLSCTX_ALL, __uuidof(IAudioVolumeLevel), (void**) &m_IAudioVolumeLevel);
        if (hr != S_OK){
            BaseLogger::WriteFormattedLog(_T("%s:\tfailed Activate for IAudioVolumeLevel hr = 0x%8.8x for part = %s"), _AUDIOFUNC_ERROR_, hr, cstrPartName.GetBuffer());
        }

        SAFE_RELEASE(previousPart);
        SAFE_RELEASE(part);

    }
    else
    {
        SAFE_RELEASE(previousPart);
        SAFE_RELEASE(part);
    }
}

void CAudioDevice::LoadPIDProperty()
{
    DWORD tid = GetCurrentThreadId();
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s IN"),tid, _T(__FUNCTION__));
    //
    // Check for the presence.
    // Get the GUID
    //
    // open the key and write
    //
    HKEY hKey;
    DWORD dwKeyIdx = 0, dwBuffer = MAX_KEY_LEN;
    //TCHAR  cDeviceGuid[MAX_KEY_LEN];
    //FILETIME Filetime;
    long lRes;
    //LPCTSTR pszKey;
    TCHAR  cKEY[MAX_KEY_LEN];

    wsprintf(cKEY, _T("%s\\%s\\%s"),RENDER_SUBKEYS, m_guid.c_str(), _T("Properties"));

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
        BaseLogger::WriteFormattedLog(_T("CAudioManager::LoadPIDProperty failed AdjustTokenPrivileges."));
        CloseHandle(hToken);
        return;
    }

    if ((lRes = RegOpenKeyEx(HKEY_LOCAL_MACHINE, cKEY, 0, KEY_READ, &hKey)) != ERROR_SUCCESS){

        CloseHandle(hToken);
        //BaseLogger::WriteFormattedLog(_T("CAudioManager::LoadPIDProperty failed RegOpenKeyEx with %d."), lRes);
        return;// FALSE;
    }

    BaseLogger::WriteFormattedLog(TRACE_AUDIO_L1, _T("Reg key loc = %s"), cKEY);

    // Read the pid property string into the member variable.
    // m_pidPropertyString
    DWORD dwBufferLen = MAX_PATH;
    BOOL bSuccess = false;
    TCHAR tcVal[256]={0};
    if ( RegQueryValueEx(hKey,  GUID_PID_PROP , NULL, NULL, (LPBYTE)tcVal, &dwBufferLen) == ERROR_SUCCESS)
    {
        m_pidPropertyString = tcVal;
        //BaseLogger::WriteFormattedLog(_T("CAudioManager::LoadPIDProperty SUCCESS"));
        bSuccess = TRUE;
    }

    // Many are exected to fail
    //if (!bSuccess)
    //    BaseLogger::WriteFormattedLog(_T("CAudioManager::LoadPIDProperty  failed RegSetValueEx = %d"), lRes);


    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s OUT"),tid , _T(__FUNCTION__));
    CloseHandle(hToken);
    RegCloseKey(hKey); 
}


void CAudioDevice::Enable()
{
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t IN"), _AUDIOFUNC_);
    TCHAR  cKEY[MAX_KEY_LEN];

    if (IsInput())
    {
        wsprintf(cKEY, _T("%s\\%s"),CAPTURE_SUBKEYS, m_guid.c_str());
    }
    else
    {
        wsprintf(cKEY, _T("%s\\%s"),RENDER_SUBKEYS, m_guid.c_str());

    }

    CRegKey reg;
    bool bFound = false;
    DWORD dwState = 0x00000001;
    LONG lErr=NO_ERROR;

    lErr = reg.Open(HKEY_LOCAL_MACHINE, cKEY, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER );
    if (lErr == NO_ERROR)
    {	

        if (reg.SetDWORDValue(_T("DeviceState"), dwState) == ERROR_SUCCESS)
        {
            BaseLogger::WriteFormattedLog(_T("%s\tEnable DeviceState for \"%s\""), _AUDIOFUNC_, GetName().c_str());
        }
        else
        {
            BaseLogger::WriteFormattedLog(_T("%s\tSetDWORDValue on \"%s\"."),  _AUDIOFUNC_ERROR_, GetName().c_str());
        }

    }
    else
        BaseLogger::WriteFormattedLog(_T("%s\reg.Open on \"%s\"."),  _AUDIOFUNC_ERROR_, GetName().c_str());
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t OUT"), _AUDIOFUNC_);
}


void CAudioDevice::Disable()
{
    // Get the registry setting
    // Read the render subkeys
    // Append the guid for the audio endpoint
    // Open the registry key
    // Set the device state to 0x100001
    // Query the key to confirm the device state read reflects DISABLED.
    // Get the registry render key
    TCHAR  cKEY[MAX_KEY_LEN];
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t IN"), _AUDIOFUNC_);
    if (IsInput())
    {
        wsprintf(cKEY, _T("%s\\%s"),CAPTURE_SUBKEYS, m_guid.c_str());
    }
    else
    {
        wsprintf(cKEY, _T("%s\\%s"),RENDER_SUBKEYS, m_guid.c_str());

    }

    CRegKey reg;
    bool bFound = false;
    DWORD dwState = 0x10000001;
    LONG lErr=NO_ERROR;

    lErr = reg.Open(HKEY_LOCAL_MACHINE, cKEY, KEY_READ | KEY_SET_VALUE | KEY_ENUMERATE_SUB_KEYS | WRITE_OWNER );
    if (lErr == NO_ERROR)
    {	

        if (reg.SetDWORDValue(_T("DeviceState"), dwState) == ERROR_SUCCESS)
        {

            BaseLogger::WriteFormattedLog(_T("%s\tDisabled DeviceState for \"%s\""), _AUDIOFUNC_, GetName().c_str());
        }    
        else
        {
            BaseLogger::WriteFormattedLog(_T("%s\tSetDWORDValue on \"%s\"."),  _AUDIOFUNC_ERROR_, GetName().c_str());
        }

    }
    else
        BaseLogger::WriteFormattedLog(_T("%s\reg.Open on \"%s\"."),  _AUDIOFUNC_ERROR_, GetName().c_str());

    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t OUT"), _AUDIOFUNC_);
}



void  CAudioDevice::SetDeviceGain(bool bState)
{
    HRESULT hr;
    DataFlow flow;
    IDeviceTopology *pDeviceTopology = NULL;
    IConnector *pConnFrom = NULL;
    IConnector *pConnTo = NULL;
    IPart *pPartPrev = NULL;
    IPart *pPartNext = NULL;
    IAudioAutoGainControl  *pAGC = NULL;

    IMMDevice* pEndptDev = m_pDevice;

    BaseLogger::WriteFormattedLog(_T("%s(%d)"), _AUDIOFUNC_, bState);

    // Get the endpoint device's IDeviceTopology interface.
    hr = pEndptDev->Activate(
        IID_IDeviceTopology, CLSCTX_ALL, NULL,
        (void**)&pDeviceTopology);
    //EXIT_ON_ERROR(hr)


    // The device topology for an endpoint device always
    // contains just one connector (connector number 0).
    hr = pDeviceTopology->GetConnector(0, &pConnFrom);
    // SAFE_RELEASE(pDeviceTopology)
    // EXIT_ON_ERROR(hr)


    // Make sure that this is a capture device.
    hr = pConnFrom->GetDataFlow(&flow);
    // EXIT_ON_ERROR(hr)

    if (flow != Out)
    {
        // Error -- this is a rendering device.
        // EXIT_ON_ERROR(hr = AUDCLNT_E_WRONG_ENDPOINT_TYPE)
    }

    // Outer loop: Each iteration traverses the data path
    // through a device topology starting at the input
    // connector and ending at the output connector.
    while (TRUE)
    {
        BOOL bConnected;
        hr = pConnFrom->IsConnected(&bConnected);
        // EXIT_ON_ERROR(hr)

        // Does this connector connect to another device?
        if (bConnected == FALSE)
        {
            // This is the end of the data path that
            // stretches from the endpoint device to the
            // system bus or external bus. Verify that
            // the connection type is Software_IO.
            ConnectorType  connType;
            hr = pConnFrom->GetType(&connType);
            // EXIT_ON_ERROR(hr)

            if (connType == Software_IO)
            {
                break;  // finished
            }
            // EXIT_ON_ERROR(hr = E_FAIL)
        }

        // Get the connector in the next device topology,
        // which lies on the other side of the connection.
        hr = pConnFrom->GetConnectedTo(&pConnTo);
        //EXIT_ON_ERROR(hr)
        //SAFE_RELEASE(pConnFrom)


        // read this endpoint connector
        hr = pConnTo->QueryInterface(
            IID_IPart, (void**)&pPartPrev);

        // enumerate the parts interface for the AGC
        while (TRUE)
        {
            PartType parttype;
            UINT localId;
            IPartsList *pParts;
            IControlInterface* pControlInterface = 0;
            UINT iControlCount = 0;
            LPWSTR pName;
            GUID pIID;

            hr = pPartPrev->GetName(&pName);
            //  _tprintf(L"Part = %s\n", pName);

            if (!_tcscmp(pName, _T("Microphone Boost")))
            {
                IAudioVolumeLevel *pVol;
                // the part is not a MUX (input selector).
                hr = pPartPrev->Activate(
                    CLSCTX_ALL,
                    IID_IAudioVolumeLevel ,
                    (void**)&pVol);

                if (hr == S_OK)
                {
                    UINT nChCnt;
                    double fNewLevel = bState ? 20.0 : 0.0;

                    pVol->GetChannelCount(&nChCnt);
                    float pfDB;
                    for (UINT cnt=0; cnt< nChCnt; cnt++)
                    {
                        pVol->GetLevel(cnt, &pfDB);
                        wprintf(L"current db = %f\n", pfDB);

                        hr = pVol->SetLevel(cnt, (float)fNewLevel, NULL);
                        //wprintf(L"new db = %f\n", fNewLevel);

                        pVol->GetLevel(cnt, &pfDB);
                        wprintf(L"new db = %f\n", pfDB);
                    }
                }
            }

            CoTaskMemFree (pName);

            pPartPrev->GetControlInterfaceCount(&iControlCount);


            //  _tprintf(L"interface control count = %d\n", iControlCount);
            for (UINT cnt=0; cnt<iControlCount; cnt++){
                pPartPrev->GetControlInterface(cnt, &pControlInterface);
                pControlInterface->GetIID(&pIID);
                hr = pControlInterface->GetName(&pName);
                //  _tprintf(_T("interface control name = %s\n\n"), pName);
                CoTaskMemFree (pName);
            }



            // Follow downstream link to next part.
            hr = pPartPrev->EnumPartsOutgoing(&pParts);
            //EXIT_ON_ERROR(hr)

            hr = pParts->GetPart(0, &pPartNext);
            pParts->Release();
            // EXIT_ON_ERROR(hr)

            hr = pPartNext->GetPartType(&parttype);
            // EXIT_ON_ERROR(hr)

            if (parttype == Connector)
            {
                // We've reached the output connector that
                // lies at the end of this device topology.
                hr = pPartNext->QueryInterface(
                    IID_IConnector,
                    (void**)&pConnFrom);
                // EXIT_ON_ERROR(hr)

                // SAFE_RELEASE(pPartPrev)
                // SAFE_RELEASE(pPartNext)
                break;
            }

            // Failure of the following call means only that
            // the part is not a MUX (input selector).
            hr = pPartNext->Activate(
                CLSCTX_ALL,
                IID_IAudioAutoGainControl ,
                (void**)&pAGC);
            if (hr == S_OK)
            {
                // We found a MUX (input selector), so select
                // the input from our endpoint device.
                hr = pPartPrev->GetLocalId(&localId);
                // EXIT_ON_ERROR(hr)

                BOOL bState;
                hr = pAGC->GetEnabled(&bState);
                // EXIT_ON_ERROR(hr)

                // SAFE_RELEASE(pAGC)
            }

            // SAFE_RELEASE(pPartPrev)
            pPartPrev = pPartNext;
            pPartNext = NULL;
        }
    }
    BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s\t OUT"), _AUDIOFUNC_);
}

void  CAudioDevice::GetDeviceGain(bool& bState)
{
	HRESULT hr;
	DataFlow flow;
	IDeviceTopology *pDeviceTopology = NULL;
	IConnector *pConnFrom = NULL;
	IConnector *pConnTo = NULL;
	IPart *pPartPrev = NULL;
	IPart *pPartNext = NULL;
	IAudioAutoGainControl  *pAGC = NULL;

	IMMDevice* pEndptDev = m_pDevice;

	BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s(%d) IN"), _T(__FUNCTION__), bState);

	// Get the endpoint device's IDeviceTopology interface.
	hr = pEndptDev->Activate(
		IID_IDeviceTopology, CLSCTX_ALL, NULL,
		(void**)&pDeviceTopology);
	//EXIT_ON_ERROR(hr)


	// The device topology for an endpoint device always
	// contains just one connector (connector number 0).
	hr = pDeviceTopology->GetConnector(0, &pConnFrom);
	// SAFE_RELEASE(pDeviceTopology)
	// EXIT_ON_ERROR(hr)


	// Make sure that this is a capture device.
	hr = pConnFrom->GetDataFlow(&flow);
	// EXIT_ON_ERROR(hr)

	if (flow != Out)
	{
		// Error -- this is a rendering device.
		// EXIT_ON_ERROR(hr = AUDCLNT_E_WRONG_ENDPOINT_TYPE)
	}

	// Outer loop: Each iteration traverses the data path
	// through a device topology starting at the input
	// connector and ending at the output connector.
	while (TRUE)
	{
		BOOL bConnected;
		hr = pConnFrom->IsConnected(&bConnected);
		// EXIT_ON_ERROR(hr)

		// Does this connector connect to another device?
		if (bConnected == FALSE)
		{
			// This is the end of the data path that
			// stretches from the endpoint device to the
			// system bus or external bus. Verify that
			// the connection type is Software_IO.
			ConnectorType  connType;
			hr = pConnFrom->GetType(&connType);
			// EXIT_ON_ERROR(hr)

			if (connType == Software_IO)
			{
				break;  // finished
			}
			// EXIT_ON_ERROR(hr = E_FAIL)
		}

		// Get the connector in the next device topology,
		// which lies on the other side of the connection.
		hr = pConnFrom->GetConnectedTo(&pConnTo);
		//EXIT_ON_ERROR(hr)
		//SAFE_RELEASE(pConnFrom)


		// read this endpoint connector
		hr = pConnTo->QueryInterface(
			IID_IPart, (void**)&pPartPrev);

		// enumerate the parts interface for the AGC
		while (TRUE)
		{
			PartType parttype;
			UINT localId;
			IPartsList *pParts;
			IControlInterface* pControlInterface = 0;
			UINT iControlCount = 0;
			LPWSTR pName;
			GUID pIID;

			hr = pPartPrev->GetName(&pName);
			//  _tprintf(L"Part = %s\n", pName);

			if (!_tcscmp(pName, _T("Microphone Boost")))
			{
				IAudioVolumeLevel *pVol;
				// the part is not a MUX (input selector).
				hr = pPartPrev->Activate(
					CLSCTX_ALL,
					IID_IAudioVolumeLevel,
					(void**)&pVol);

				if (hr == S_OK)
				{
					UINT nChCnt;
					double fNewLevel = bState ? 20.0 : 0.0;

					pVol->GetChannelCount(&nChCnt);
					float pfDB;
					for (UINT cnt = 0; cnt< nChCnt; cnt++)
					{
						pVol->GetLevel(cnt, &pfDB);
						wprintf(L"GetLevel current db = %f\n", pfDB);

						// hr = pVol->SetLevel(cnt, fNewLevel, NULL);
						// wprintf(L"new db = %f\n", fNewLevel);

						// pVol->GetLevel(cnt, &pfDB);
						// wprintf(L"new db = %f\n", pfDB);
					}
				}
			}

			CoTaskMemFree(pName);

			pPartPrev->GetControlInterfaceCount(&iControlCount);


			//  _tprintf(L"interface control count = %d\n", iControlCount);
			for (UINT cnt = 0; cnt<iControlCount; cnt++){
				pPartPrev->GetControlInterface(cnt, &pControlInterface);
				pControlInterface->GetIID(&pIID);
				hr = pControlInterface->GetName(&pName);
				//  _tprintf(_T("interface control name = %s\n\n"), pName);
				CoTaskMemFree(pName);
			}



			// Follow downstream link to next part.
			hr = pPartPrev->EnumPartsOutgoing(&pParts);
			//EXIT_ON_ERROR(hr)

			hr = pParts->GetPart(0, &pPartNext);
			pParts->Release();
			// EXIT_ON_ERROR(hr)

			hr = pPartNext->GetPartType(&parttype);
			// EXIT_ON_ERROR(hr)

			if (parttype == Connector)
			{
				// We've reached the output connector that
				// lies at the end of this device topology.
				hr = pPartNext->QueryInterface(
					IID_IConnector,
					(void**)&pConnFrom);
				// EXIT_ON_ERROR(hr)

				// SAFE_RELEASE(pPartPrev)
				// SAFE_RELEASE(pPartNext)
				break;
			}

			// Failure of the following call means only that
			// the part is not a MUX (input selector).
			hr = pPartNext->Activate(
				CLSCTX_ALL,
				IID_IAudioAutoGainControl,
				(void**)&pAGC);
			if (hr == S_OK)
			{
				// We found a MUX (input selector), so select
				// the input from our endpoint device.
				hr = pPartPrev->GetLocalId(&localId);
				// EXIT_ON_ERROR(hr)

				BOOL bState;
				hr = pAGC->GetEnabled(&bState);
				// EXIT_ON_ERROR(hr)

				// SAFE_RELEASE(pAGC)
			}

			// SAFE_RELEASE(pPartPrev)
			pPartPrev = pPartNext;
			pPartNext = NULL;
		}
	}

	BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s() OUT"), _T(__FUNCTION__));
}
