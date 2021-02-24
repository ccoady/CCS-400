#include "StdAfx.h"

#include "AudioDeviceCfgMgrCCS200.h"
#include "CmPCConfigDLL.h"



DeviceToJoin::DeviceToJoin()
{

}

DeviceToJoin::~DeviceToJoin()
{

}
bool DeviceToJoin::HasDigitalJoin()
{
	return false;
}

bool DeviceToJoin::HasAnalogJoin()
{
	return false;
}

bool DeviceToJoin::HasSerialJoin()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////
// Function:	CacheMute
//
// Parameters:  bMute - the mute setting
//
// Returns:     
// 
// Description: Stores the most recent setting sent from the Engine
//              via SetVolume. The cache is explicitly flushed to the underlying
//              device.
//              
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void DeviceToJoin::CacheMute(bool bMute)
{
	m_pRecord->bSetMute = bMute;
}

/////////////////////////////////////////////////////////////////////////////
// Function:	CacheVolume
//
// Parameters:  l,r - left and right volume level
//
// Returns:     
// 
// Description: Stores the most recent setting sent from the Engine
//              via SetVolume. The cache is explicitly flushed to the underlying
//              device.
//              
//              
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void DeviceToJoin::CacheVolume(int l, int r)
{
	m_pRecord->iLastVolumeLeft = l;
	m_pRecord->iLastVolumeRight = r;
}

/////////////////////////////////////////////////////////////////////////////
// Function:	PushCachedValues
//
// Parameters:  
//
// Returns:     
// 
// Description: Pushes the cached values in this object to the IMMediaPtr to 
//              set the volume and mute setting.
//              
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void DeviceToJoin::PushCachedValues()
{
	DWORD tid = GetCurrentThreadId();
	int nChannels = m_pRecord->pDevice->GetNumChannels();
	if (nChannels > 1)
	{
		BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s setVol() l=%d, r=%d"), 
			tid,
			_AUDIOFUNC_,
			m_pRecord->iLastVolumeLeft,
			m_pRecord->iLastVolumeRight);

		m_pRecord->pDevice->SetVolume(m_pRecord->iLastVolumeLeft, m_pRecord->iLastVolumeRight);
		m_pRecord->pDevice->GetCurrentChannelVolume(0, m_pRecord->iLastVolumeLeft);
		m_pRecord->pDevice->GetCurrentChannelVolume(1, m_pRecord->iLastVolumeRight);
	}
    else
	{
		m_pRecord->pDevice->SetVolume(m_pRecord->iLastVolumeRight);
	}

	m_pRecord->pDevice->SetMute(m_pRecord->bSetMute);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	SetAudioPtr
//
// Parameters:  pDevice - Stores the underlying pointer to be used by this 
//                          instance.
//
// Returns:     
// 
// Description: Sets the CAudioDevice* which contains the IMMediaPtr used for
//              setting volume and mute. This pointer must be refereshed each
//              time audio endpoint devices change.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void  DeviceToJoin::SetAudioPtr(CAudioDevice* pDevice)
{
	m_pRecord->pDevice = pDevice;
}

void  DeviceToJoin::SetEntry(DeviceToJoinX* pRecord)
{
	m_pRecord = pRecord;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	SetDeviceToJoinTable
//
// Parameters:  pTable - Device to Join table
//
// Returns:     void
// 
// Description: Store the table into a wrapper. Table is parsed.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioDeviceCfgMgrCCS200::SetDeviceToJoinTable(DeviceToJoinX* pTable)
{
	m_pDevJoinTable = pTable;
	InitializeWrapperClass();
}

////////////////////////////////////////////////////////////////////////////////
// Function:	ClearAllPtrs
//
// Parameters:  
//
// Returns:     
// 
// Description: Sets the pointer field to NULL in each DeviceToJoinX structure.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioDeviceCfgMgrCCS200::ClearAllPtrs()
{
	DeviceToJoin* pCurrent = NULL;

	for (m_pDevIterator = m_pDevList.begin(); (m_pDevIterator != m_pDevList.end()); m_pDevIterator++)
	{
		pCurrent = (DeviceToJoin*)*m_pDevIterator;
		ATLASSERT(pCurrent);
		if (pCurrent)
		{
			pCurrent->SetAudioPtr(NULL);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////
// Function:	LoadDevicesPtrs
//
// Parameters:  
//
// Returns:     
// 
// Description: For each audio device info, find the corresponding
//              device from the enumeration managed by AudioMixerMgr 
//              and initialize the device info ptr field.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioDeviceCfgMgrCCS200::LoadDevicesPtrs()
{
	int                         idx 				= 0;
	DeviceToJoin*               pRecord 	= NULL;
	BOOL                        bFound 			= false;
	CAudioDevice*               pDevice	= NULL;
	DeviceToJoin*               pChannelDescriptor   = NULL;
	BaseLogger::WriteFormattedLog (TRACESTACK , _T("%s:\tIN"), _AUDIOFUNC_);

	ClearAllPtrs();

	USES_CONVERSION;
	_TCHAR* pstrDeviceName = NULL;

	// Find the device with the matching to which JoinToDeviceTable[idx].name corresponds.

	// Walk list until sentinel or device is reached.
	pChannelDescriptor = GetFirstDevice();
	TCHAR* pChannelName = pChannelDescriptor->GetChannelName();

	while ((pChannelName != NULL) && !bFound)
	{
		BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("%s\t\"%s\""), _AUDIOFUNC_, pChannelName);
		pstrDeviceName = pChannelDescriptor->GetDeviceName();

		//
		// If subpart in name. Also make a copy of the name because
		// it is modified by ConfigDeviceGetEndpointAndSubPartName.
		// If we don't do this the names of the devices for subparts
		// we get mangeled.
		// 
		TCHAR  tTempNameBuff[256] = {NULL};
		TCHAR* tEndpointName = NULL;
		TCHAR* tSubPartName  = NULL;

		_tcscpy(tTempNameBuff, pstrDeviceName);
		bool bHasSubPart = ConfigDeviceHasSubPart((TCHAR*)tTempNameBuff);
		if ( bHasSubPart )
		{
			ConfigDeviceGetEndpointAndSubPartName(tTempNameBuff, &tEndpointName, &tSubPartName);
			pDevice = m_AudioMixer->GetDeviceByName(tEndpointName);
		}
		else
			pDevice = m_AudioMixer->GetDeviceByName(tTempNameBuff); 

		pRecord = pChannelDescriptor;
        // If the device is in the enumeration get its
        // IMMDevicePtr and update the CAudioDevice->m_pDevicePtr.
		if (pDevice != NULL)
		{
			// We may need to create a new CAudioDevice wrapper for this interface.
			if (bHasSubPart)
			{
				IMMDevice* pMMDevice = pDevice->GetEndpoint();
				//if (pDevice->IsInput() == false)
				//{
				CAudioInputDevice* pTempInput = (reinterpret_cast<CAudioManagerCCS200*>(m_AudioMixer))->CreateInputSubDevice(pMMDevice, tSubPartName);
				pRecord->SetAudioPtr(pTempInput);
				//}
			}
			else
            {
                BaseLogger::WriteFormattedLog(_T("%s:\tMapped\t<\"%-16s\" -> \"%s\">"), 
                                                _AUDIOFUNC_, 
                                                pRecord->GetChannelName(),
                                                pDevice->GetName().c_str());
				pRecord->SetAudioPtr(pDevice);
			}
		}
        else 
        {
            // If the device is not found in the enumeration, then set
            // the CAudioDevice* to NULL.
		
			BaseLogger::WriteFormattedLog(_T("%s:\tError device \"%s\" not found in enumeration, check configuration file."), _AUDIOFUNC_, pstrDeviceName);
			pRecord->SetAudioPtr(NULL); 
		}
		pChannelDescriptor = GetNextDevice();
		pChannelName = NULL;
		if (pChannelDescriptor)
			pChannelName = pChannelDescriptor->GetChannelName();


	}

	BaseLogger::WriteFormattedLog (TRACESTACK , _T("%s Out"), _AUDIOFUNC_);
}


////////////////////////////////////////////////////////////////////////////////
// 
// Method:      ConfigDeviceGetEndpointAndSubPartName 
//
// Parameters:	ptName - input string
//              ptEndpointName - a ptr that gets set to the first string.
//              ptSubPartName  - a ptr that gets set to the second string.
//
// Returns:     void  
//
// Description: Split a string delimited by | into two separate strings.
//              
//
// Author:      Jeff Spruiel 12/12/2013
//
////////////////////////////////////////////////////////////////////////////////
void CAudioDeviceCfgMgrCCS200::ConfigDeviceGetEndpointAndSubPartName(TCHAR* ptName, TCHAR** ptEndpointName,  TCHAR** ptSubPartName)
{
	*ptEndpointName = ptName;  
	int             i       = 0;      // Outer parse loop
	TCHAR*          pCursor = ptName; // initialize cursor to start of string
	bool            bDone   = false; // denotes completed parsing
	int             len     = (int)_tcsnlen(ptName, 250); 

	// Just return with the endpoint name
	if (!ConfigDeviceHasSubPart(ptName))
	{
		return;
	}

	// set the endpoint and subunit name
	while (i++ < len && !bDone)
	{
		if (*pCursor == L'|')
		{
			*ptSubPartName = pCursor+1;
			while(1)
			{
				if (*(pCursor-1) == L' ') // check one step back
				{
					pCursor--;
				}else
					break;
			}
			// NULL terminate the first string
			*pCursor = NULL;

			// Trim front of subpart; 2nd str
			pCursor = *ptSubPartName;
			while(*pCursor == L' ')
			{
				pCursor++;
			}
			*ptSubPartName = pCursor;
			bDone = true;
			break;
		}
		pCursor++;
	}
}

////////////////////////////////////////////////////////////////////////////////
// Function:	ConfigDeviceHasSubPart 
//
// Parameters:  tcName - name of the endpoint device from which to retrieve
//                      the subpart. This name is read from the UpxTaskMgr.ini
//                      file.
//
// Returns:     true if the name is formatted to convey is has subparts.
// 
// Description: Determines whether the specified endpoint device has the delimiter
//              '|' to signify a device with subparts. IE, Speaker Mixer|Mic, or Speaker Mix | LineIn
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
bool CAudioDeviceCfgMgrCCS200::ConfigDeviceHasSubPart(TCHAR* tcName)
{
	CString name = tcName;

	if (name.Find(_T("|")) != -1)
		return true;
	return false;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetFirstDevice 
//
// Parameters:  
//
// Returns:     DeviceToJoin* - retrieves the first element in the 
//              vector of classes of type DeviceToJoin. DeviceToJoin
//              is a wrapper class for the structure.
// 
// Description: Retrieve an DeviceToJoinX structure.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
void CAudioDeviceCfgMgrCCS200::InitializeWrapperClass()
{
	DeviceToJoinX* pEntry = m_pDevJoinTable;

	while (pEntry->strChannelName[0] != 0)
	{
		DeviceToJoin* p = new DeviceToJoin();
		p->SetEntry(pEntry );
		m_pDevList.push_back(p);
		pEntry++;
	}
}

void CAudioDeviceCfgMgrCCS200::PushCachedJoinsToDevices()
{
	DeviceToJoin* pDevToJoin = GetFirstDevice();

	while ((pDevToJoin != 0))
	{
		if(pDevToJoin->IsValid())
		{
			pDevToJoin->PushCachedValues();
		}
		pDevToJoin = GetNextDevice();
	}
}