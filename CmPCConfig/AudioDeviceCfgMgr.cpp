#include "StdAfx.h"

#include "AudioDeviceCfgMgr.h"
#include "CmPCConfigDLL.h"


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
DeviceToJoin* CAudioDeviceCfgMgr::GetChannelDescriptor(TCHAR* pcChannelNameRequested)
{
	DeviceToJoin* pChannelDescriptor   = NULL;
	BOOL bFound = false;

	pChannelDescriptor =  GetFirstDevice();
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
			pChannelDescriptor = GetNextDevice();
			if (pChannelDescriptor != NULL)
				pChannelName = pChannelDescriptor->GetChannelName();
		}
	}

	return bFound ? pChannelDescriptor : NULL;
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
DeviceToJoin* CAudioDeviceCfgMgr::GetFirstDevice ()
{
	if (m_pDevList.size() > 0)
	{
		if ((m_pDevIterator = m_pDevList.begin()) != m_pDevList.end())
		{
			return *m_pDevIterator;
		}
	}
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	GetNextDevice 
//
// Parameters:  
//
// Returns:     DeviceToJoin* - retrieves the next element in the 
//              vector of classes of type DeviceToJoin; or null is returned. DeviceToJoin
//              is a wrapper class for the structure.
// 
// Description: Retrieve the next DeviceToJoinX or returns null.
//
// Author:       Jeff Spruiel
//////////////////////////////////////////////////////////////////////////////
DeviceToJoin* CAudioDeviceCfgMgr::GetNextDevice ()
{
    if (m_pDevIterator == m_pDevList.end())
    {
        return NULL;
    }

    m_pDevIterator++;
    if (m_pDevIterator != m_pDevList.end())
    {
        return *m_pDevIterator;
    }
    
    return NULL;
}
