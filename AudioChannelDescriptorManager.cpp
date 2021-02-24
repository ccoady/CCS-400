#include "StdAfx.h"

#include "AudioManager.h"

#include "CmPCConfigDLL.h"


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
CAudioChannelDescriptorManager::CAudioChannelDescriptorManager(CAudioManager* pAudioMixManager, AudioChannelDescriptor* pAudioChannelDescriptorTable)
	:
	m_AudioMixer (pAudioMixManager),
	m_pTable (pAudioChannelDescriptorTable)
{
	InitializeWrapperClass();
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
CAudioChannelDescriptorManager::~CAudioChannelDescriptorManager()
{}

/*
void CAudioChannelDescriptorManager::SetDeviceToJoinTable(AudioChannelDescriptor* pTable)
{
		m_pTable = pTable;
	InitializeWrapperClass();
}
*/

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
void CAudioChannelDescriptorManager::LoadDevicesPtrs()
{
int                         idx 				= 0;
	ACDescriptorWrapper*               pRecord 	= NULL;
	BOOL                        bFound 			= false;
	CAudioDevice*               pDevice	= NULL;
	ACDescriptorWrapper*               pChannelDescriptor   = NULL;
	DWORD tid = GetCurrentThreadId();
	BaseLogger::WriteFormattedLog (TRACESTACK , _T("0x%8.8x - %s IN"), tid, _AUDIOFUNC_);

	ClearAllPtrs();

	USES_CONVERSION;
	_TCHAR* pstrDeviceName = NULL;

	// Find the device with the matching to which JoinToDeviceTable[idx].name corresponds.

	// Walk list until sentinel or device is reached.
	pChannelDescriptor = GetFirstChannelDescriptor();
	TCHAR* pChannelName = pChannelDescriptor->GetChannelName();

	while ((pChannelName != NULL) && !bFound)
	{
		BaseLogger::WriteFormattedLog(TRACESTACK | TRACE_AUDIO, _T("0x%8.8x - %s - %s "),tid,  _AUDIOFUNC_, pChannelName);
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

		pDevice = m_AudioMixer->GetDeviceByName(tTempNameBuff); 

		pRecord = pChannelDescriptor;		
		if (pDevice != NULL)
		{
			// We may need to create a new CAudioDevice wrapper for this interface.
				pRecord->SetAudioPtr(pDevice);
		
		}else
		{
			BaseLogger::WriteFormattedLog(_T("%s - Error device '%s' not found in enumeration, check configuration file."), _AUDIOFUNC_, pstrDeviceName);
			pRecord->SetAudioPtr(NULL); 
		}
		pChannelDescriptor = GetNextChannelDescriptor();
		pChannelName = NULL;
		if (pChannelDescriptor)
			pChannelName = pChannelDescriptor->GetChannelName();


	}
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetFirstChannelDescriptor 
//
// Parameters:  
//
// Returns:     ACDescriptorWrapper* - retrieves the first element in the 
//              vector of classes of type ACDescriptorWrapper. ACDescriptorWrapper
//              is a wrapper class for the structure AudioChannelDescriptor.
// 
// Description: Retrieve an AudioChannelDescriptor structure.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
ACDescriptorWrapper* CAudioChannelDescriptorManager::GetFirstChannelDescriptor ()
{
	if (m_pAudioChannelVector.size() > 0)
	{
		if ((m_pAudioChannelIterator = m_pAudioChannelVector.begin()) != m_pAudioChannelVector.end())
		{
			return *m_pAudioChannelIterator;
		}
	}
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetNextChannelDescriptor 
//
// Parameters:  
//
// Returns:     ACDescriptorWrapper* - retrieves the next element in the 
//              vector of classes of type ACDescriptorWrapper ACDescriptorWrapper
//              is a wrapper class for the structure.
// 
// Description: Retrieve the next AudioChannelDescriptor or returns null.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
ACDescriptorWrapper* CAudioChannelDescriptorManager::GetNextChannelDescriptor ()
{
    if (m_pAudioChannelIterator == m_pAudioChannelVector.end())
    {
        return NULL;
    }

    m_pAudioChannelIterator++;
    if (m_pAudioChannelIterator != m_pAudioChannelVector.end())
    {
        return *m_pAudioChannelIterator;
    }
    
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetChannelDescriptor
//
// Parameters:  pcChannelNameRequested - The requested audio channel descriptor.
//				
// Returns:     NULL or the descriptor.
// 
// Description: Returns the requested AudioChannelDescriptor or null.
//
// Author:      Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
ACDescriptorWrapper* CAudioChannelDescriptorManager::GetChannelDescriptor(TCHAR* pcChannelNameRequested)
{
	ACDescriptorWrapper* pChannelDescriptor   = NULL;
	BOOL bFound = false;

	pChannelDescriptor =  GetFirstChannelDescriptor();
	TCHAR* pChannelName = pChannelDescriptor->GetChannelName();

	while ((pChannelName != NULL) && !bFound)
	{
		bFound  = !_tcscmp(pChannelName, pcChannelNameRequested);
		if (bFound)
		{
			bFound = true; 
		}
		else
		{
			pChannelDescriptor = GetNextChannelDescriptor();
			if (pChannelDescriptor != NULL)
				pChannelName = pChannelDescriptor->GetChannelName();
		}
	}

	return bFound ? pChannelDescriptor : NULL;
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
void CAudioChannelDescriptorManager::ClearAllPtrs()
{
		ACDescriptorWrapper* pCurrent = NULL;

	for (m_pAudioChannelIterator = m_pAudioChannelVector.begin(); (m_pAudioChannelIterator != m_pAudioChannelVector.end()); m_pAudioChannelIterator++)
	{
		pCurrent = (ACDescriptorWrapper*)*m_pAudioChannelIterator;
		ATLASSERT(pCurrent);
		if (pCurrent)
		{
			pCurrent->SetAudioPtr(NULL);
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
// Function:	InitializeWrapperClass 
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
void CAudioChannelDescriptorManager::InitializeWrapperClass()
{
	AudioChannelDescriptor* pEntry = m_pTable;

	while (pEntry->strChannelName[0] != 0)
	{
		ACDescriptorWrapper* p = new ACDescriptorWrapper();
		p->SetEntry(pEntry );
		m_pAudioChannelVector.push_back(p);
		pEntry++;
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
void CAudioChannelDescriptorManager::PushCachedJoinsToDevices()
{
	ACDescriptorWrapper* pDevToJoin = GetFirstChannelDescriptor();

	while ((pDevToJoin != 0))
	{
		if(pDevToJoin->IsValid())
		{
			pDevToJoin->PushCachedValues();
		}
		pDevToJoin = GetNextChannelDescriptor();
	}
}