#pragma once
#include "stdafx.h"

#include "audiomanager.h"
#include <Mmdeviceapi.h>
#include "IAudioDeviceChangeListener.h"

// Stores mic gain settings for the Analog Mic
#define MIC_SETTINGS_INI _T("MicSettings.ini")

class CmPCConfig_DeclSpec CAudioManagerCCS200 : public CAudioManager
{
	
public:
    static const int LINE_LEVEL = 0;
    static const int MIC_LEVEL  = 1;

    CAudioManagerCCS200(void);
    virtual 		~CAudioManagerCCS200(void);
	// Initializes the base class with the message window for posting events
	// and the listener which receives USB device change notifications.
    virtual void Init(HWND hwnd, IAudioDeviceChangeListener* pListener);

	// Called by the framework ;-) after EnumDevices() completes.
    virtual 		void OnEnumDevicesComplete();
    void            SetListenOffOnAllUsbDevices();
    void            MapUSBMicsToLync();
    tstring& GetUsbInputDeviceNamesList();
    tstring& GetOutputDeviceNamesList();

    void AddUsbMic(tstring& tName);
    void GetFirstUsbMic(tstring& tName );
    void GetNextUsbMic(tstring& tName);
    void GetUsbMicCount(int &);
	void ClearUsbMics();

	bool SetVolumeLevel (CAudioDevice* pDevice, const int volume);
    bool GetVolumeLevel (CAudioDevice* pDevice, int &leftVolume, int &rightVolume);
    bool SetMute    (CAudioDevice* pDevice, const bool muteSetting);
    bool GetMute    (CAudioDevice* pDevice);

	void MapOutputDevice(CAudioInputDevice* pDevice, CAudioOutputDevice* pOutput);
    void SetListenState(CAudioInputDevice* pDevice, bool state);
    CAudioInputDevice* CreateInputSubDevice(IMMDevice* pMMDevice, TCHAR* tSubPartName);
	void SetDeviceToJoinTable(DeviceToJoinX* pTable){m_pDeviceToJoinTable = pTable;}

    void ReadMicGainFromIniFile(tstring& szGainLevel);
    void WriteMicGainToIniFile(const tstring& szGainLevel);

    CAudioDevice*  GetInputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter);
    CAudioDevice*  GetOutputDeviceByName(_TCHAR* tszDevice, _TCHAR* tszAdapter);
    class CUsbList {
    public:
        void AddUsbMic(tstring& tName);
        void GetFirstUsbMic(tstring& tName );
        void GetNextUsbMic(tstring& tName);
        void GetUsbMicCount(int &);
        void ClearUsbMics();
    private:
        vector<tstring>	m_list;
        vector<tstring>::iterator m_iterator;
    };

    CUsbList* m_usbList;

	// Not used
	CAudioInputDevice* m_pCurrentListeningUsbDevice;

	// The device to which input devices are sent and mixed.
	CAudioDevice*   m_pMixedOutputDevice;

	// Current list of USB input devices read from the Enumeration.
//	tstring 		m_strUsbList;
//	tstring			m_strOutputDeviceList;
//    vector<tstring>	m_strUsbList2;
//    vector<tstring>::iterator m_strUsbList2Iterator;

private:
		DeviceToJoinX* m_pDeviceToJoinTable;

};

