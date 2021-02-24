

#include "StdAfx.h"
#include "AudioManager.h"
#include "DeviceListener.h"


#include "..\UtsConfigUtil\ConfigFuncUts.h"

static TCHAR tcharDbg[128];

//int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
CMMNotificationClient::CMMNotificationClient(CAudioManager *pAudioMixerManager) :
m_cRef(1),
    m_pEnumerator(NULL),
    m_pAudioMixerManager(pAudioMixerManager)
{
}


void CMMNotificationClient::Init(IAudioDeviceChangeListener* pListener)
{
    try{
         BaseLogger::WriteFormattedLog(_T("%s:(IAudioDeviceChangeListener:0x%8.8x"), _AUDIOFUNC_, pListener);
        m_pListener = pListener;

        HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&m_pEnumerator);
        if (hr != S_OK)
        {
            BaseLogger::WriteFormattedLog(_T("%s:\tCoCreateInstance failed "), _AUDIOFUNC_ERROR_);
            return;
        }

        hr = m_pEnumerator->RegisterEndpointNotificationCallback(this);
        if (hr != S_OK)
            BaseLogger::WriteFormattedLog(_T("%s:\tRegisterEndpointNotificationCallback failed"), _AUDIOFUNC_ERROR_);
    }

    catch(exception& e) //BaseLogger::filter(GetExceptionCode(), GetExceptionInformation()))
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tException: %s"), _AUDIOFUNC_ERROR_, e.what());
    }
}


void CMMNotificationClient::Stop()
{
    if (m_pEnumerator)
    {
        HRESULT hr = m_pEnumerator->UnregisterEndpointNotificationCallback(this);
        SAFE_RELEASE(m_pEnumerator)
    }
}

void CMMNotificationClient::Start()
{
    HRESULT hr = CoCreateInstance(__uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IMMDeviceEnumerator), (LPVOID *)&m_pEnumerator);
    if (hr != S_OK)
    {
        BaseLogger::WriteFormattedLog(_T("%s:\tCoCreateInstance failed "), _AUDIOFUNC_);
        return;
    }


    hr = m_pEnumerator->RegisterEndpointNotificationCallback(this);
    if (hr != S_OK)
        BaseLogger::WriteFormattedLog(_T("%s:\t RegisterEndpointNotificationCallback failed"), _AUDIOFUNC_ERROR_);

    // If the previous call fails, we should then start a timer to signal when the Audio Service is available!!!!
    // This would resolve any service start dependency issue.
}



CMMNotificationClient::~CMMNotificationClient()
{
    if (m_pEnumerator)
    {
        HRESULT hr = m_pEnumerator->UnregisterEndpointNotificationCallback(this);
        SAFE_RELEASE(m_pEnumerator)
    }
}

ULONG STDMETHODCALLTYPE CMMNotificationClient::AddRef()
{
    return InterlockedIncrement(&m_cRef);
}

ULONG STDMETHODCALLTYPE CMMNotificationClient::Release()
{
    ULONG ulRef = InterlockedDecrement(&m_cRef);
    if (0 == ulRef)
    {
        delete this;
    }
    return ulRef;
}

HRESULT STDMETHODCALLTYPE CMMNotificationClient::QueryInterface(
    REFIID riid, VOID **ppvInterface)
{
    if (IID_IUnknown == riid)
    {
        AddRef();
        *ppvInterface = (IUnknown*)this;
    }
    else if (__uuidof(IMMNotificationClient) == riid)
    {
        AddRef();
        *ppvInterface = (IMMNotificationClient*)this;
    }
    else
    {
        *ppvInterface = NULL;
        return E_NOINTERFACE;
    }
    return S_OK;
}

//
// Notifies the client that the default audio endpoint device for a particular role has changed.
//
HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDefaultDeviceChanged(
    EDataFlow flow, ERole role,
    LPCWSTR pwstrDeviceId)
{
    return S_OK;
}

//
// Indicates that a new audio endpoint device has been added.
//
HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceAdded(LPCWSTR pwstrDeviceId)
{
    // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("CMMNotificationClient::OnDeviceAdded() - tid = (0x%x)"), ::GetCurrentThreadId());
    //   wsprintf(tcharDbg, _T(" CMMNotificationClient::OnDeviceAdded() - tid = (0x%x)"), ::GetCurrentThreadId());
    //  BaseLogger::WriteLog(tcharDbg);
    //   LogFriendlyName(pwstrDeviceId);
    return S_OK;
};

//
// Indicates that the state of an audio endpoint device has changed.
//
HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceRemoved(LPCWSTR pwstrDeviceId)
{
    // BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("CMMNotificationClient::OnDeviceRemoved() - tid = (0x%x)"), ::GetCurrentThreadId());
    //      wsprintf(tcharDbg, _T(" CMMNotificationClient::OnDeviceRemoved() - tid = (0x%x)"), ::GetCurrentThreadId());
    // BaseLogger::WriteLog(tcharDbg);
    //  LogFriendlyName(pwstrDeviceId);

    return S_OK;
}


//
// Indicates that the state of an audio endpoint device has changed.
//
HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnDeviceStateChanged(LPCWSTR pwstrDeviceId, DWORD dwNewState)
{
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("0x%8.8x - CMMNotificationClient::OnDeviceStateChanged() ******** IN"), ::GetCurrentThreadId());

    TCHAR  *pszState = _T("?????");

    LogFriendlyName(pwstrDeviceId);

    switch (dwNewState)
    {
    case DEVICE_STATE_ACTIVE:
        pszState = _T("ACTIVE");
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s:\t ACTIVE "), _AUDIOFUNC_);
        break;                                                     
    case DEVICE_STATE_DISABLED:                                    
        pszState = _T("DISABLED");                                 
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s:\t DISABLED"),_AUDIOFUNC_);
        break;                                                     
    case DEVICE_STATE_NOTPRESENT:                                  
        pszState = _T("NOTPRESENT");                               
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s:\t NOTPRESENT"), _AUDIOFUNC_);
        break;                                                     
    case DEVICE_STATE_UNPLUGGED:                                   
        pszState = _T("UNPLUGGED");                                
        BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s:\t UNPLUGGED"), _AUDIOFUNC_);
        break;
    }

          void* pVoid = NULL;
        CoInitialize(pVoid);
		if (m_pListener)
			m_pListener->StateChanged();
		else
			BaseLogger::WriteFormattedLog(_T("%s:\tlistener is NULL"), _AUDIOFUNC_INFO_);
        CoUninitialize();
    

    wsprintf(tcharDbg, _T("-  -->New device state is DEVICE_STATE_%s (0x%8.8x)\n"), pszState, dwNewState);
    BaseLogger::WriteLog(TRACESTACK | TRACE_AUDIO,tcharDbg);
    BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("%s:\tOUT\n"), _AUDIOFUNC_);

    return S_OK;
}

//
// Indicates that the value of a property belonging to an audio endpoint device has changed.
//
HRESULT STDMETHODCALLTYPE CMMNotificationClient::OnPropertyValueChanged(
    LPCWSTR pwstrDeviceId,
    const PROPERTYKEY key)
{
    //BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("0x%8.8x - CMMNotificationClient::OnPropertyValueChanged()"), ::GetCurrentThreadId());
    //BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO,_T("0x%8.8x - CMMNotificationClient::OnPropertyValueChanged() for device = %s"), ::GetCurrentThreadId(), pwstrDeviceId);
    return S_OK; // SHORT CIRCUIT
}

//
// Prints the friendly name from the endpoint ID.
// 
HRESULT CMMNotificationClient::LogFriendlyName(LPCWSTR pwstrId)
{
    HRESULT hr = S_OK;
    IMMDevice *pDevice = NULL;
    IPropertyStore *pProps = NULL;
    PROPVARIANT varString;

    CoInitialize(NULL);
    PropVariantInit(&varString);

    if (m_pEnumerator == NULL)
    {
        // Get enumerator for audio endpoint devices.
        hr = CoCreateInstance(__uuidof(MMDeviceEnumerator),
            NULL, CLSCTX_INPROC_SERVER,
            __uuidof(IMMDeviceEnumerator),
            (void**)&m_pEnumerator);
    }
    if (hr == S_OK)
    {
        hr = m_pEnumerator->GetDevice(pwstrId, &pDevice);
    }
    if (hr == S_OK)
    {
        hr = pDevice->OpenPropertyStore(STGM_READ, &pProps);
    }
    if (hr == S_OK)
    {
        // Get the endpoint device's friendly-name property.
        hr = pProps->GetValue(PKEY_Device_FriendlyName, &varString);
    }
    wsprintf(tcharDbg, _T("%s:\tDevice name: %s "),_AUDIOFUNC_, varString.pwszVal );
    BaseLogger::WriteLog(TRACESTACK | TRACE_AUDIO, tcharDbg);
    PropVariantClear(&varString);

    SAFE_RELEASE(pProps)
        SAFE_RELEASE(pDevice)
        CoUninitialize();
    return hr;
}

