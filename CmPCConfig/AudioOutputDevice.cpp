/* Copyright (2012) Crestron Electronics Inc., All Rights Reserved.                       
 * The source code contained or described herein and all documents related to         
 * the source code ("Material") are owned by Crestron Electronics Inc. or its suppliers       
 * or licensors. Title to the Material remains with Crestron Electronics Inc. or its          
 * suppliers and licensors. The Material contains trade secrets and proprietary       
 * and confidential information of Crestron or its suppliers and licensors. The          
 * Material is protected by worldwide copyright and trade secret laws and             
 * treaty provisions. No part of the Material may be used, copied, reproduced,        
 * modified, published, uploaded, posted, transmitted, distributed, or                
 * disclosed in any way without Crestron's prior express written permission.             
 *                                                                                    
 * No license under any patent, copyright, trade secret or other intellectual         
 * property right is granted to or conferred upon you by disclosure or                
 * delivery of the Materials, either expressly, by implication, inducement,           
 * estoppel or otherwise. Any license under such intellectual property rights         
 * must be express and approved by Crestron in writing.               
 */

#include "StdAfx.h"
#include "AudioOutputDevice.h"




const IID IID_IAudioRenderClient = __uuidof(IAudioRenderClient);




CAudioOutputDevice::CAudioOutputDevice(IMMDevice* pDevice)
	:CAudioDevice(pDevice)
	
{
	 
}


CAudioOutputDevice::CAudioOutputDevice(CAudioOutputDevice* pOther)
	:CAudioDevice(pOther->m_pDevice)
	
{	
	m_pDevice->AddRef();
	
}



CAudioOutputDevice::~CAudioOutputDevice(void)
{
    m_pDevice->Release();
}



HRESULT CAudioOutputDevice::Init()
{
	m_fInput = false;


	return CAudioDevice::Init();
}


void CAudioOutputDevice::Cleanup()
{


	CAudioDevice::Cleanup();
}





	
	




