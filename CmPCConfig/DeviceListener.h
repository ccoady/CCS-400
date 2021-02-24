#pragma once
#include "stdafx.h"

// Windows Vista/7 audio api
#include <functiondiscoverykeys.h>
#include "EndpointVolume.h"
#include <vector>
#include "IAudioDeviceChangeListener.h"

class CAudioManager;
#define SAFE_RELEASE(punk)  \
    if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

class CMMNotificationClient : public IMMNotificationClient
{
    IAudioDeviceChangeListener*     m_pListener;
    CAudioManager* m_pAudioMixerManager;
    LONG m_cRef;
    IMMDeviceEnumerator *m_pEnumerator;

    // Private function to print device-friendly name
    HRESULT LogFriendlyName(LPCWSTR  pwstrId);

public:
    CMMNotificationClient(CAudioManager *pAudioMixerManager);
    ~CMMNotificationClient();

    void Init(IAudioDeviceChangeListener* pListener);
    // IUnknown methods -- AddRef, Release, and QueryInterface

    ULONG STDMETHODCALLTYPE AddRef();

    ULONG STDMETHODCALLTYPE Release();
    HRESULT STDMETHODCALLTYPE QueryInterface(
        REFIID riid, VOID **ppvInterface);

    // Callback methods for device-event notifications.
    HRESULT STDMETHODCALLTYPE OnDefaultDeviceChanged(
        EDataFlow flow, ERole role,
        LPCWSTR pwstrDeviceId);

    HRESULT STDMETHODCALLTYPE OnDeviceAdded(LPCWSTR pwstrDeviceId);

    HRESULT STDMETHODCALLTYPE OnDeviceRemoved(LPCWSTR pwstrDeviceId);

    HRESULT STDMETHODCALLTYPE OnDeviceStateChanged(
        LPCWSTR pwstrDeviceId,
        DWORD dwNewState);

    HRESULT STDMETHODCALLTYPE OnPropertyValueChanged(
        LPCWSTR pwstrDeviceId,
        const PROPERTYKEY key);

    void Stop();
    void Start();

};