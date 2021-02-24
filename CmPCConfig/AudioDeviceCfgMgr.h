#pragma once
#include "..\UtsConfigUtil\AudioConfDef.h"
#include "AudioManager.h"

#include <vector>


using namespace std;

class CAudioDevice;


static int MuteDevices[][2] =
{
    {AUDIO_ANALOG_OUT_ON,       AUDIO_ANALOG_OUT_OFF},
    {AUDIO_LINE_LYNC_INPUT_ON,  AUDIO_LINE_LYNC_INPUT_OFF},
    {AUDIO_LINE_IN_ON,          AUDIO_LINE_IN_OFF},
    {AUDIO_HDMI_01_INPUT_ON,    AUDIO_HDMI_01_INPUT_OFF},
    {AUDIO_HDMI_LYNC_INPUT_ON,  AUDIO_HDMI_LYNC_INPUT_OFF},
    {AUDIO_MIC_ON,              AUDIO_MIC_OFF},
    {AUDIO_USB_INPUT_ON,        AUDIO_USB_INPUT_OFF},
    {0,0}
};


/**
	A wrapper class around the DeviceToJoinX plain old structure.
	Description: 
*/
class CmPCConfig_DeclSpec DeviceToJoin
{
public:
    DeviceToJoin();
    ~DeviceToJoin();

    TCHAR* GetChannelName (){return m_pRecord->strChannelName;}
	TCHAR* GetDeviceName (){return m_pRecord->strDefDeviceName;}
    CAudioDevice* GetDevicePtr() {return m_pRecord->pDevice;}
    void SetAudioPtr(CAudioDevice* pDevice);
    int* GetDigitalJoinArray() {return m_pRecord->digJoins;}
    int* GetAnalogJoinArray() {return m_pRecord->anaJoins;}
    int* GetSerialJoinArray(){ return m_pRecord->serJoins;}
    bool HasDigitalJoin();
    bool HasAnalogJoin();
    bool HasSerialJoin();
    bool IsValid(){return (m_pRecord->pDevice != NULL) ? true : false;}
    void CacheVolume(int l, int r);
    void CacheMute(bool);
    void PushCachedValues();
    void SetEntry(DeviceToJoinX* pEntry);

protected:


private:
    DeviceToJoinX*  m_pRecord;
};


/**
	Name: Class CAudioDeviceCfgMgr

	Description: This class initializes and manages access to the static list of predefined devices.
	The product dependent configuration details are specified in the UpxTaskMgr.ini file
	and this class knows how to read the product specific ini section.

	- Reads the devices from the UpxTaskMgr.ini
	- Instantiates the Windows 7 Core Audio devices
	- Sets the pointer for each DeviceToJoin audio context
*/
class CmPCConfig_DeclSpec CAudioDeviceCfgMgr
{
    CAudioManager* m_AudioMixer;

public:
    CAudioDeviceCfgMgr(){};
    CAudioDeviceCfgMgr(CAudioManager* pAudioMixManager){m_AudioMixer = pAudioMixManager;}
    ~CAudioDeviceCfgMgr(){};

	virtual void SetDeviceToJoinTable(DeviceToJoinX* pTable){};
	virtual void LoadDevicesPtrs(){};

	DeviceToJoin* GetFirstDevice ();//{return NULL;};
	DeviceToJoin* GetNextDevice ();//{return NULL;};
	DeviceToJoin* GetChannelDescriptor(TCHAR*);
	
	void ClearAllPtrs(){};
	void SetVolume(int left, int right){};
	void SetMute(bool){};
	void PushCachedJoinsToDevices(){};

    // APIs to support device EndPoint Parts.
	bool ConfigDeviceHasSubPart(TCHAR* tCName){return false;}
	void ConfigDeviceGetEndpointName(TCHAR* tEndpointName){};
	void ConfigDeviceGetSubPartName(TCHAR* tSubPartName){};
	void ConfigDeviceGetEndpointAndSubPartName(TCHAR* ptchName, TCHAR** ptEndpointName,  TCHAR** ptSubPartName){};

protected:
	void InitializeWrapperClass(){};

  DeviceToJoinX* m_pDevJoinTable;

  vector<DeviceToJoin*>	m_pDevList;

  vector<DeviceToJoin*>::iterator m_pDevIterator;

};

