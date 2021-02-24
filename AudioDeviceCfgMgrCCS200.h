#pragma once

#include "AudioManagerCCS200.h"
#include "AudioDeviceCfgMgr.h"
#include "..\UtsConfigUtil\AudioConfDef.h"
#include <vector>


using namespace std;
#if 0
class CAudioDevice;

#define MAX_JOINS  10
typedef int JOIN_ARRAY[MAX_JOINS];


/**
	A wrapper class around the DeviceToJoinX plain old structure.
	Description: 
*/
class CmPCConfig_DeclSpec DeviceToJoin
{
public:
    DeviceToJoin();
    ~DeviceToJoin();

    TCHAR* GetName(){return m_pRecord->name;}
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
#endif

/**
	Name: Class CAudioDeviceCfgMgr

	Description: This class initializes and manages access to the static list of predefined devices.
	The product dependent configuration details are specified in the UpxTaskMgr.ini file
	and this class knows how to read the product specific ini section.

	- Reads the devices from the UpxTaskMgr.ini
	- Instantiates the Windows 7 Core Audio devices
	- Sets the pointer for each DeviceToJoin audio context
*/
class CmPCConfig_DeclSpec CAudioDeviceCfgMgrCCS200 : public CAudioDeviceCfgMgr
{
    CAudioManager* m_AudioMixer;

public:
    CAudioDeviceCfgMgrCCS200(){};
    CAudioDeviceCfgMgrCCS200(CAudioManager* pAudioMixManager){m_AudioMixer = pAudioMixManager;}
    ~CAudioDeviceCfgMgrCCS200(){};

	virtual void SetDeviceToJoinTable(DeviceToJoinX* pTable);
	virtual void LoadDevicesPtrs();

    //DeviceToJoin* GetFirstDevice ();
    //DeviceToJoin* GetNextDevice ();

	
    void ClearAllPtrs();
    void SetVolume(int left, int right);
    void SetMute(bool);
    void PushCachedJoinsToDevices();

    // APIs to support device EndPoint Parts.
    bool ConfigDeviceHasSubPart(TCHAR* tCName);
    void ConfigDeviceGetEndpointName(TCHAR* tEndpointName);
    void ConfigDeviceGetSubPartName(TCHAR* tSubPartName);
    void ConfigDeviceGetEndpointAndSubPartName(TCHAR* ptchName, TCHAR** ptEndpointName,  TCHAR** ptSubPartName);


  void InitializeWrapperClass();

  //DeviceToJoinX* m_pDevJoinTable;

 // vector<DeviceToJoin*>	m_pDevList;

 // vector<DeviceToJoin*>::iterator m_pDevIterator;

};

