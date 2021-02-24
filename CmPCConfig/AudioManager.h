#pragma once
#include "stdafx.h"

// Windows Vista/7 audio api
#include <functiondiscoverykeys.h>
#include "EndpointVolume.h"
#include <vector>

#include "AudioInputDevice.h"
#include "AudioOutputDevice.h"
#include "BaseLogger.h"
#include "DeviceListener.h"
#include "IAudioDeviceChangeListener.h"
#include "IAudioManagerDeviceChangeListener.h"


#define MAIN_INPUT_CHANNEL_NAME   _T("MICROPHONE")
#define MAIN_OUTPUT_CHANNEL_NAME  _T("SPEAKERS")

#ifdef UNIT_TESTING
#define SETTINGS_FILE            _T(".\\debug\\UpxTaskMgr.ini")
#endif

using namespace std;
#define MAXPNAMELEN 32

#define MAX_JOINS  10
typedef int JOIN_ARRAY[MAX_JOINS];

// Forward declarations.
class CMMNotificationClient;
class CAudioChannelDescriptorManager;
class ACDescriptorWrapper;


////////////////////////////////////////////////////////////////////////////////////
//
// Class Name: DeviceToJoinX
//
// Description:  A plan old structure containing fields pertinent to an audio
//               channel.
//
// Depricated.
//
////////////////////////////////////////////////////////////////////////////////////

typedef struct _DeviceToJoinX
{
	TCHAR    		strChannelName[256];	// channel name
	TCHAR    		strDefDeviceName[256];	// default device name
	CAudioDevice* 	pDevice;	// device to be filled in during init
	int 			*digJoins;
	int 			*anaJoins;
	int 			*serJoins;
	// Applied when pDevice is not null
	bool            bSetMute;    // default muted
	int             iLastVolumeLeft ;  // default 50
	int             iLastVolumeRight;
}DeviceToJoinX;

////////////////////////////////////////////////////////////////////////////////////
// Class Name: AudioChannelDescriptor
//
// Description:  A plan old structure containing fields pertinent to an audio
//               channel. The channel name field is a user defined name chosen to
//			     match the role to which this channel serves. The default device 
//               name is used to specify the default device for the target platform
//               and the default device name may not be the same as the audio device
//				 pointer that gets filled in. 
//
////////////////////////////////////////////////////////////////////////////////////
typedef struct _AudioChannelDescriptor
{
	TCHAR    		strChannelName[256];	// channel name
	TCHAR    		strDefDeviceName[256];	// default device name
	CAudioDevice* 	pDevice;	// device to be filled
	int 			*digJoins;
	int 			*anaJoins;
	int 			*serJoins;
	// Applied when pDevice is not null
	bool            bSetMute;    // default muted and cache
	int             iLastVolumeLeft ;  // default 50 and cache
	int             iLastVolumeRight;  // ditto
}AudioChannelDescriptor;




//////////////////////////////////////////////////////////////////////
//
// Name: Class CAudioManager
//
// Description: This class is the main interface to the Audio implementation.
//              It monitors and responds to device change events from Windows.
//              It provides operations to set properties 
//              on audio devices.
//
/////////////////////////////////////////////////////////////////////
class CmPCConfig_DeclSpec CAudioManager : public IAudioDeviceChangeListener
{
	friend CMMNotificationClient;

public:
	CAudioManager(void);
	CAudioManager(AudioChannelDescriptor* pAudioChannelDescriptorTable);
	virtual ~CAudioManager(void);

	virtual void Init(HWND,  IAudioDeviceChangeListener* pListener);

	virtual void Init(HWND hwnd, IAudioManagerDeviceChangeListener* pListener);


	void GetDeviceName(IMMDevice* currentIMMDevice, tstring& string);

	// Enumerates all audion endpoints in the system
	// and store them in lists.
	void EnumDevices();
    void Re_EnumerateDevices();

	// Reads the config file and sets the volume, wave, line, mic volumes
	virtual void LoadAudioDeviceMap();

	// Called by the base class after enumerate devices have completed.
	virtual void OnEnumDevicesComplete();

	/// Shuts down an instance of this class and cleans up the mess
	virtual bool Destroy();
	/// The device has just un-docked. Break down the class for the external audio card

	virtual bool DeviceUndocked();
	/// The device has just docked. Create the class for the external audio card

	virtual bool DeviceDocked();

	/// Returns true if audio is currently routed to the external audio device
	virtual bool IsAudioExternal(); 

	bool m_bInitialized; // for subsequent calls to init

	/// Set the master mute.
	virtual bool SetMasterMute(const bool muteSetting){return false;}

	/// Get the master mute.
	virtual bool GetMasterMute(){return false;}

	/// Set the mic gain level
	virtual bool SetMicGain(const int leftVolume, const int rightVolume){return false;}

	/// Set the mic gain level
	virtual bool SetMicGain(const int volume){return false;}
	/// Get the mic mute
	virtual bool GetMicMute(){return false;}
	virtual bool SetMicMute(const bool muteSetting){return false;}

	void IsRegStateListening(CAudioInputDevice* pDevice, bool& state);
	void IsInputMappedToOutputDevice(CAudioInputDevice* pInputDevice, CAudioOutputDevice* pOutputDevice, bool& bResult);


	CAudioDevice*  GetDeviceByName(_TCHAR* tcNameParam);
	virtual CAudioDevice* GetInputDeviceByName(_TCHAR* tszEndpointName, _TCHAR* tszEndpointNameB);
	virtual CAudioDevice* GetOutputDeviceByName(_TCHAR* tszEndpointName, _TCHAR* tszEndpointNameB);
	bool ScanForName(const tstring& name, TCHAR* tszEndpointName, TCHAR* tszEndpointNameB );



	void StopEventListener();
	void  StartEventListener();

	bool GetOutputDeviceNameFromIni(_TCHAR* tName1, _TCHAR* tName2);
	void SetOutputDeviceTo(_TCHAR* tName1, _TCHAR* tName2);

	HWND GetHwnd(){return m_hwnd;};

	// CCS100
	virtual bool SetMasterVolumeLevel(const int volume){return 0;}
	virtual bool SetMasterVolumeLevel(const int leftVolume, const int rightVolume){return 0;}
	virtual bool GetMasterVolumeLevel(int &leftVolume, int &rightVolume){return 0;}
	virtual bool GetMicGain(int &leftVolume, int &rightVolume){return 0;}

	virtual void SetDefaultAudioInputDevice  (int iJoin, TCHAR* tDeviceName, int len);
	virtual void SetDefaultAudioOutputDevice (int iJoin, TCHAR* tDeviceName, int len);
	virtual void GetDefaultAudioOutputDevice (tstring& tsDeviceName );
	virtual void GetDefaultAudioInputDevice(tstring& tsDeviceName);

	virtual void SetMuteOn(int iJoin, bool bParam);
	virtual void SetMuteOff(int iJoin, bool bParam);
	virtual void SetVolume(int iJoin, int left, int right);

	// Override to create create an AudioChannelDescriptorManager to parametize it with
	// you override AudioDescriptorTable.
	virtual void CreateAudioChannelDescriptorManager();

	// Called to retrieve the latest input and output device list.
	void GetOutputDeviceList(tstring&);
	void GetInputDeviceList(tstring&);

	// Override the base implementation to specialize what happens when a
	// device status change event occurs.
	virtual void StateChanged();



	static void CALLBACK DeviceChangeSingleShotTimerProc(HWND, UINT, UINT iTimerId, DWORD);
	void HandleDeviceChangeTimer(UINT iTimerId);
    bool DoStringsMatch(TCHAR* str1, TCHAR* str2);

private:
	void GetAudioChannelByAnalogJoin  (int iJoin, ACDescriptorWrapper** pAudioChannel);
	void GetAudioChannelByDigitalJoin (int iJoin, ACDescriptorWrapper** pAudioChannel);
	void GetAudioChannelBySerialJoin  (int iJoin, ACDescriptorWrapper** ppAudioChannel);
	bool IsMuteOn(int join);
	bool IsMuteOff(int join);

protected:
	/// The name of our internal audio mixer. Anything else is considered to be the "external" mixer
	_TCHAR		    m_szInternalMixerName[MAXPNAMELEN];
	_TCHAR 		    m_tszFirmwarePath[MAX_PATH];
	_TCHAR 		    m_tszIniFilePathName[2048];

	vector<CAudioDevice*>	m_vInputList;
	vector<CAudioDevice*>	m_vOutputList;

	CRITICAL_SECTION	m_CriticalSection;

	CMMNotificationClient*				m_DeviceChangeNotifier;
	IAudioDeviceChangeListener*			m_pListener; // ConfigFunc deprecated
	IAudioManagerDeviceChangeListener*	m_pOutgoingDeviceChangeListener; // 
	IAudioDeviceChangeListener*			m_pIncommingDeviceChangeListener; // Receives from Windows via CMMNot'

	CAudioDevice*           m_pMasterVolume;
	CAudioDevice*             m_pMic;
	HWND m_hwnd;
	AudioChannelDescriptor* m_pTable;

	CAudioChannelDescriptorManager*		m_pAudioDeviceCfg;
public:

	int m_nDevices;


	class CUsbList {
	public:
		void AddUsbMic(tstring& tName);
		void GetFirstUsbMic(tstring& tName );
		void GetNextUsbMic(tstring& tName);
		void GetUsbMicCount(int &);
		void ClearUsbMics();
		vector<tstring>	m_list;

	private:
		vector<tstring>::iterator m_iterator;
	};

//private:
	void AddUsbMic(tstring& tName);
	void GetFirstUsbMic(tstring& tName );
	void GetNextUsbMic(tstring& tName);
	void GetUsbMicCount(int &);
	void ClearUsbMics();


	CUsbList* m_usbList;
	tstring m_strOutputDeviceList;
	tstring m_strInputDeviceList;
	UINT64 m_iDeviceChangeTimerId;
	DWORD m_timerStart;
    DWORD m_timerEnd;
};



//////////////////////////////////////////////////////////////////////
//
// Name:        Class CAudioChannelDescriptorManager
//
// Description: Manages loading, refreshing and enumerating of the 
//              table of AudioChannelDescriptor instances. Provides
//              operations to set volume and mute on audio devices.
//
//////////////////////////////////////////////////////////////////////
class CmPCConfig_DeclSpec CAudioChannelDescriptorManager
{
public:
	CAudioChannelDescriptorManager(){};

	CAudioChannelDescriptorManager(CAudioManager* pAudioMixManager, 
				       AudioChannelDescriptor* pAudioChannelDescriptorTable);

	~CAudioChannelDescriptorManager();

//	virtual void SetDeviceToJoinTable(AudioChannelDescriptor* pTable);
	virtual void LoadDevicesPtrs();
	ACDescriptorWrapper* GetFirstChannelDescriptor ();
	ACDescriptorWrapper* GetNextChannelDescriptor ();
	ACDescriptorWrapper* GetChannelDescriptor(TCHAR*);
	void ClearAllPtrs();
	void PushCachedJoinsToDevices();

// Fields for this class
protected:
	CAudioManager* m_AudioMixer;
	void InitializeWrapperClass();
private:
	AudioChannelDescriptor* m_pTable;
	vector<ACDescriptorWrapper*>	m_pAudioChannelVector;
	vector<ACDescriptorWrapper*>::iterator m_pAudioChannelIterator;
};


///////////////////////////////////////////////////////////////////////////////////////
//
// Name:        Class ACDescriptorWrapper
//
// Description: A wrapper class for an AudioChannelDescriptor structure providing
//				setter/getter operations on the underlying AudioChannelDescriptor structure. An
//              instance exists for each AudioChannelDescriptor in the table of 
//				AudioChannelDescriptors.
//
///////////////////////////////////////////////////////////////////////////////////////
class CmPCConfig_DeclSpec ACDescriptorWrapper
{
public:
	ACDescriptorWrapper(){}
	~ACDescriptorWrapper(){}

	TCHAR* GetChannelName (){return m_pRecord->strChannelName;}
	TCHAR* GetDeviceName (){return m_pRecord->strDefDeviceName;}
	CAudioDevice* GetDevicePtr(){return m_pRecord->pDevice;}
	void SetAudioPtr(CAudioDevice* pDevice){m_pRecord->pDevice = pDevice;}
	int* GetDigitalJoinArray(){return m_pRecord->digJoins;}
	int* GetAnalogJoinArray(){return m_pRecord->anaJoins;}
	int* GetSerialJoinArray(){return m_pRecord->serJoins;}
	bool IsValid(){return (m_pRecord->pDevice != NULL) ? true : false;}
	void CacheVolume(int l, int r){	m_pRecord->iLastVolumeLeft = l;
	m_pRecord->iLastVolumeRight = r;}
	void SetVolume(int iLeft, int iRight);
	void SetMute(bool bMuteVal);
	void CacheMute(bool bMute){m_pRecord->bSetMute = bMute;}
	void PushCachedValues();
	void SetEntry(AudioChannelDescriptor* pEntry){m_pRecord = pEntry;}

// Fields in this class
private:
	// Pointer to the underlying structure.
	AudioChannelDescriptor*  m_pRecord;
};

