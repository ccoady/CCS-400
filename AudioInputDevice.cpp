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
#include "AudioDevice.h"
#include "AudioInputDevice.h"
#include "AudioOutputDevice.h"


const IID IID_IAudioCaptureClient = __uuidof(IAudioCaptureClient);

CAudioInputDevice::CAudioInputDevice(IMMDevice* pDevice)
:CAudioDevice(pDevice)
	,m_pCaptureClient(NULL)	
{
}

CAudioInputDevice::~CAudioInputDevice(void)
{
}


HRESULT CAudioInputDevice::Init()
{
	m_fInput = true;

	
	return CAudioDevice::Init();
}


void CAudioInputDevice::Cleanup()
{
	SAFE_RELEASE(m_pCaptureClient);

	CAudioDevice::Cleanup();
}
