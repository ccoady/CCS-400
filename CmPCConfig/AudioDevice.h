#pragma once
#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <tchar.h>
#include <string>
#include <atlstr.h>

// Windows Vista/7 audio api
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

#include "AudioErrorDefs.h"
#include <audioclient.h>
#include "CommonDefs.h"
#include "BaseLogger.h"

using namespace std;
#define MAX_LINE_LEN	4096
#define MAX_KEY_LEN		256
#define CAPTURE_SUBKEYS _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MMDevices\\Audio\\Capture")
#define RENDER_SUBKEYS _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\MMDevices\\Audio\\Render")

#define GUID_DEVICE_NAME_PROP  _T("{b3f8fa53-0004-438e-9003-51a46e139bfc},6")
#define GUID_DEVICE_FNAME_PROP _T("{a45c254e-df1c-4efd-8020-67d146a850e0},2")
#define GUID_LISTEN_CONTROL_PROP _T("{24dbb0fc-9311-4b3d-9cf0-18ff155639d4},1")
#define GUID_OUTPUT_CONNECTION_PROP _T("{24dbb0fc-9311-4b3d-9cf0-18ff155639d4},0")
#define GUID_DEF_COMMUNICATION_PROP _T("{24dbb0fc-9311-4b3d-9cf0-18ff155639d4},1")
#define GUID_PID_PROP				_T("{233164c8-1b2c-4c7d-bc68-b671687a2567},1")
#define GUID_OUTPUT_GUID_PREFIX_PROP _T("{0.0.0.00000000}.")

#define LOG(sz, ...) ATLTRACE(sz L"\n", __VA_ARGS__);
#define ERR(sz, ...) ATLTRACE(L"ERROR: " sz L"\n", __VA_ARGS__);
HRESULT LogProperty(PROPERTYKEY key, const PROPVARIANT &val);
LPCWSTR StringFromState(DWORD state);
LPCWSTR StringFromKey(PROPERTYKEY key, LPCWSTR fallback);
LPCWSTR StringFromWaveFormatTag(WORD wFormatTag);
LPCWSTR StringFromSubFormat(GUID SubFormat);

class CmPCConfig_DeclSpec CAudioDevice
{
    friend HRESULT __LogHostError(HRESULT res, TCHAR *func);

public:
    CAudioDevice(void);
    CAudioDevice(IMMDevice* pDevice);
    virtual ~CAudioDevice(void);
    HRESULT Init();
    void Cleanup();
    
    bool IsInput() { return m_fInput; };
    bool IsOutput() { return !m_fInput; };

    void GetID(tstring& strId);
    tstring GetName() { return m_strName; };

    int GetVolume()		{ return (int)(m_fVolume*100); };
    bool SetVolume( int leftVolume,  int rightVolume);
    bool SetVolume(int nVol);
    

    bool SetMute(bool bMute);
    int GetNumChannels()	{ return m_nChannels; };
    int GetChannelVolume(int nChannel)		{ return (int)m_fChannelVolumes[nChannel]*100; };
    bool GetCurrentChannelVolume(int channel, int &nVolume);
    bool GetMuteState()	{ return m_bMuted; };
    bool GetMute(void);
    
    tstring GetGuid(){return m_strGuid;}
    tstring GetPidPropertyString(){return m_pidPropertyString;}

    HRESULT GetDataDirection(bool *pIsInput);
    
    int GetConnectionCount()		{ return 0; };
    
    void SetToDefaultDevice();
    void GetDeviceEnumerator(tstring& name);
    IMMDevice* GetEndpoint() { return m_pDevice;}
    bool SetActiveSubUnit(TCHAR* tSubUnitName);
    HRESULT Init(TCHAR* tszSubPart);
    HRESULT InitVolumeInfo(TCHAR* tszSubPart);
 
    tstring GetDevicePnpName(tstring strDeviceID);
    void SetLyncDevice(bool fInput=true);
    bool SetLyncInputDeviceRegKeys(HKEY hKey, const tstring strDeviceName, const tstring strPnpName);
    bool SetLyncOutputDeviceRegKeys(HKEY hKey, const tstring strDeviceName, const tstring strPnpName);

    void Enable();
    void Disable();
    void SetDeviceGain(bool bState);
   	void GetDeviceGain(bool& bState);


protected:

    HRESULT InitVolumeInfo();
    HRESULT InitDeviceName();
    HRESULT InitDeviceState();
    bool SetPartVolume(int nVol);
    HRESULT InitializePartPointers(IMMDevice *pEndptDev);
    HRESULT Search(IMMDevice *pEndptDev);
    bool TraversePart(IPart* pPart);
    bool VisitConnector(IConnector *pConnFrom );
    void SetPartMuteAndVolumePtrs(IPart* iPart);
    void LoadPIDProperty();

protected:

    IMMDevice*		        m_pDevice;              // Endpoint
    IAudioEndpointVolume*	m_pEndpointVolume;      // EndpointVolumeInterface
    IAudioMute*             m_pIAudioMute;          // The mute interface for this Subunit.
    IAudioVolumeLevel*      m_IAudioVolumeLevel;    // The volume interface for this Subunit.
    IAudioAutoGainControl*  m_IAudioGC;
    
    tstring			m_strName;
    tstring			m_strDeviceDescriptionName;
    tstring			m_strInterfaceFriendlyName;
    tstring			m_strEnumerationName;
    tstring         m_strGuid;
    tstring			m_guid;
    float			m_fVolume;
    UINT			m_nChannels;
    float			m_fChannelVolumes[16];
    bool			m_bMuted;

    DWORD			m_dwState;
    tstring			m_strStateString;
    bool			m_fInput;   
    tstring			m_pidPropertyString;
    tstring			m_strPnpName;

private:
    CString       m_cstrSubPartName;
};



