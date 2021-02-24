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

//

#include "stdafx.h"
//#include <iostream>
//#include <sstream>
#include <vector>
#include <tchar.h>
#include <string>
#include "../../CmPCConfig/NetConnect.h"
#include "../../CmPCConfig/AudioDevice.h"
#include "../../CmPCConfig/AudioMixerManager7.h"

using namespace std;

void UnitTest_GetMasterVolumeLevel(CAudioMixerManager* pMixer);
void UnitTest_GetDeviceByName(CAudioMixerManager* pMixer);
void UnitTest_SetMasterVolumeLevel2Channel(CAudioMixerManager* pMixer);
void UnitTest_SetMasterVolumeLevel(CAudioMixerManager* pMixer);
void UnitTest_SetMasterMute(CAudioMixerManager* pMixer);
void UnitTest_GetMasterMute(CAudioMixerManager* pMixer);
void UnitTest_SetMicGain2Channel(CAudioMixerManager* pMixer);
void UnitTest_SetMicGain(CAudioMixerManager* pMixer);
void UnitTest_GetMicGain2Channel(CAudioMixerManager* pMixer);
void UnitTest_SetMicMute(CAudioMixerManager* pMixer);
void UnitTest_GetMicMute(CAudioMixerManager* pMixer);
void UnitTest_SetLineInputGain2Channel(CAudioMixerManager* pMixer);
void UnitTest_SetLineInputGain(CAudioMixerManager* pMixer);
void UnitTest_GetLineInputGain2Channel(CAudioMixerManager* pMixer);
void UnitTest_SetLineInputMute(CAudioMixerManager* pMixer);
void UnitTest_GetLineInputMute(CAudioMixerManager* pMixer);
void SoundCheck(CAudioMixerManager* pMixer);

int _tmain(int argc, _TCHAR* argv[])
{
    ::CoInitialize(NULL);
    CAudioMixerManager objUnderTest;

     objUnderTest.Init();

    UnitTest_GetMasterVolumeLevel(&objUnderTest);
    UnitTest_GetDeviceByName(&objUnderTest);
    UnitTest_SetMasterVolumeLevel2Channel(&objUnderTest);
    UnitTest_SetMasterVolumeLevel(&objUnderTest);
    UnitTest_SetMasterMute(&objUnderTest);
    UnitTest_GetMasterMute(&objUnderTest);
//    UnitTest_SetMicGain2Channel(&objUnderTest);
    UnitTest_SetMicGain(&objUnderTest);
//    UnitTest_GetMicGain2Channel(&objUnderTest);
    UnitTest_SetMicMute(&objUnderTest);
    UnitTest_GetMicMute(&objUnderTest);
    UnitTest_SetLineInputGain2Channel(&objUnderTest);
    UnitTest_SetLineInputGain(&objUnderTest);
    UnitTest_GetLineInputGain2Channel(&objUnderTest);
    UnitTest_SetLineInputMute(&objUnderTest);
    UnitTest_GetLineInputMute(&objUnderTest);

    SoundCheck(&objUnderTest);

    ::CoUninitialize();
    return 0;
}

void SoundCheck(CAudioMixerManager* pMixer)
{
    ATLASSERT (pMixer->SetMasterMute(false));
    ATLASSERT (pMixer->SetMasterVolumeLevel(100, 100));
    ATLASSERT(pMixer->SetMicGain(40));
    ATLASSERT(pMixer->SetMicMute(false));
    ATLASSERT(pMixer->SetLineInputGain(100));
    ATLASSERT(pMixer->SetLineInputMute(false));

    pMixer->SetMasterVolumeLevel(0,0);
    for (int i = 0; i < 100; i+=10)
    {
        pMixer->SetMasterVolumeLevel(i);
        Sleep(1000);
    }

    // left
    pMixer->SetMasterVolumeLevel(0,0);
    for (int i = 0; i < 100; i+=10)
    {
        pMixer->SetMasterVolumeLevel(i, 0);
        Sleep(1000);
    }

    // right
    pMixer->SetMasterVolumeLevel(0,0);
    for (int i = 0; i < 100; i+=10)
    {
        pMixer->SetMasterVolumeLevel(0, i);
        Sleep(1000);
    }

    pMixer->SetMasterVolumeLevel(80);

}

void UnitTest_GetMasterVolumeLevel(CAudioMixerManager* mixer)
{
    bool bResult = false;
    // Some silly numbers
    int left = -5555;
    int right = -5555;

    bResult = mixer->GetMasterVolumeLevel(left, right);

    ATLASSERT(bResult == true);
    ATLASSERT(left != -5555);
    ATLASSERT(right != -5555);


    cout << " GetMasterVolumeLevel left = " << left << "right = " << right << endl;
    cout << " GetMasterVolumeLevel PASSED" << endl;
}

void UnitTest_GetDeviceByName(CAudioMixerManager* pMixer)
{    
    USES_CONVERSION;

    #define AUDIOLINE_TO_ENDPOINT_MAP _T("AudioLineToEndpointMap")

    _TCHAR tszFilename[MAX_PATH]    = {0};
    _TCHAR tszMasterVol[MAX_PATH]   = {0};
    _TCHAR tszLineInLine[MAX_PATH]  = {0};
    _TCHAR tszMicLine[MAX_PATH]     = {0};


    ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("EAudioLine_Master"), NULL, tszMasterVol, _MAX_PATH, _T(".\\UpxTaskMgr.ini"));
    ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("EAudioLine_LineIn"), NULL, tszLineInLine, _MAX_PATH, _T(".\\UpxTaskMgr.ini"));
    ::GetPrivateProfileString(AUDIOLINE_TO_ENDPOINT_MAP, _T("EAudioLine_Mic"), NULL, tszMicLine, _MAX_PATH, _T(".\\UpxTaskMgr.ini"));
    
    cout << "Master volume = " << (char*) T2CA(tszMasterVol) << endl;
    cout << "Line In = " << (char*) T2CA(tszLineInLine) << endl;
    cout << "Mic volume = " << (char*) T2CA(tszMicLine) << endl;

    //ATLASSERT (pMixer->GetDeviceByName(0) == 0);
    ATLASSERT (pMixer->GetDeviceByName(tszMasterVol) != 0);
    ATLASSERT (pMixer->GetDeviceByName(tszLineInLine) != 0);
    ATLASSERT (pMixer->GetDeviceByName(tszMicLine) != 0);
    cout << " GetDeviceByName PASSED" << endl;
}

void UnitTest_SetMasterVolumeLevel2Channel(CAudioMixerManager* pMixer)
{
    ATLASSERT (pMixer->SetMasterVolumeLevel(20, 30));

    int left = -5555;
    int right = -5555;
    ATLASSERT (pMixer->GetMasterVolumeLevel(left, right));
    ATLASSERT (left == 20);
    ATLASSERT (right == 30);
    cout << " SetMasterVolumeLevel(l,r) PASSED" << endl;
}

void UnitTest_SetMasterVolumeLevel(CAudioMixerManager* pMixer)
{
    int left = -5555;
    int right = -5555;

    ATLASSERT (pMixer->SetMasterVolumeLevel(40));
    ATLASSERT (pMixer->GetMasterVolumeLevel(left, right));
    ATLASSERT (right == 40);
    cout << " SetMasterVolumeLevel PASSED" << endl;
}

void UnitTest_SetMasterMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetMasterMute();

    ATLASSERT (pMixer->SetMasterMute(!bInitial ));
    ATLASSERT (pMixer->GetMasterMute() == !bInitial );

    pMixer->SetMasterMute(bInitial);

    cout << " UnitTest_SetMasterMute PASSED" << endl;
}

void UnitTest_GetMasterMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetMasterMute();

    ATLASSERT (pMixer->SetMasterMute(!bInitial ));
    ATLASSERT (pMixer->GetMasterMute() == !bInitial );

    pMixer->SetMasterMute(bInitial);

    cout << " UnitTest_SetMasterMute PASSED" << endl;
}

void UnitTest_SetMicGain2Channel(CAudioMixerManager* pMixer)
{
    int initialLeft, initialRight;
    int left, right;

    ATLASSERT(pMixer->GetMicGain(initialLeft, initialRight));

    ATLASSERT(pMixer->SetMicGain(35, 15));
    ATLASSERT(pMixer->GetMicGain(left, right));

    cout << "Left, right " << left  << ", " << right << endl;

    ATLASSERT (left == 35);
    ATLASSERT (right == 15);
    ATLASSERT(pMixer->SetMicGain(initialLeft, initialRight));
    cout << " SetMicGain(l,r) PASSED" << endl;
}

void UnitTest_GetMicGain2Channel(CAudioMixerManager* pMixer)
{
    int initialLeft, initialRight;
    int left, right;

    ATLASSERT (pMixer->GetMicGain(initialLeft, initialRight));
    ATLASSERT (pMixer->SetMicGain(5, 90));
    ATLASSERT (pMixer->GetMicGain(left, right));
    ATLASSERT (left == 5);
    ATLASSERT (right == 90);
    ATLASSERT (pMixer->SetMicGain(initialLeft, initialRight));

    cout << "GetMicGain(l, r) PASSED" << endl;
}


void UnitTest_SetMicGain(CAudioMixerManager* pMixer)
{
    int initialLeft, initialRight;
    int left, right;

    ATLASSERT(pMixer->GetMicGain(initialLeft, initialRight));

    ATLASSERT(pMixer->SetMicGain(40));
    ATLASSERT(pMixer->GetMicGain(left, right));
    ATLASSERT (left == 40);
    ATLASSERT (right == 40);

    // reset
    pMixer->SetMicGain(initialLeft, initialRight);
    cout << " SetMicGain(g) PASSED" << endl;
}


void UnitTest_SetMicMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetMicMute();

    ATLASSERT (pMixer->SetMicMute(!bInitial ));
    ATLASSERT (pMixer->GetMicMute() == !bInitial );

    pMixer->SetMasterMute(bInitial);

    cout << " SetMicMute PASSED" << endl;
}


void UnitTest_GetMicMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetMicMute();

    ATLASSERT (pMixer->SetMicMute(!bInitial ));
    ATLASSERT (pMixer->GetMicMute() == !bInitial );

    pMixer->SetMicMute(bInitial);

    cout << " GetMicMute PASSED" << endl;
}


void UnitTest_SetLineInputGain2Channel(CAudioMixerManager* pMixer)
{
    int initialLeft, initialRight;
    int left, right;
    pMixer->GetLineInputGain(initialLeft, initialRight);

    pMixer->SetLineInputGain(15, 35);
    pMixer->GetLineInputGain(left, right);

    ATLASSERT (left == 15);
    ATLASSERT (right == 35);
    pMixer->SetLineInputGain(initialLeft, initialRight);
    cout << " SetLineInputGain(l,r) PASSED" << endl;
}


void UnitTest_SetLineInputGain(CAudioMixerManager* pMixer)
{
    int initialLeft, initialRight;
    int left, right;
    pMixer->GetLineInputGain(initialLeft, initialRight);

    pMixer->SetLineInputGain(40);
    pMixer->GetLineInputGain(left, right);

    ATLASSERT (left == 40);
    pMixer->SetLineInputGain(initialLeft);

    cout << " SetLineInputGain(g) PASSED" << endl;
}


void UnitTest_GetLineInputGain2Channel(CAudioMixerManager* pMixer)
{
    int left = -555;
    int right = -555;

    ATLASSERT (pMixer->GetLineInputGain(left, right));
    ATLASSERT (left != -555);
    ATLASSERT (right != -555);
    cout << " GetLineInputGain(l, r) PASSED" << endl;
}


void UnitTest_SetLineInputMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetLineInputMute();

    ATLASSERT (pMixer->SetLineInputMute(!bInitial ));
    ATLASSERT (pMixer->GetLineInputMute() == !bInitial );

    pMixer->SetLineInputMute(bInitial);

    cout << " SetLineInputMute PASSED" << endl;
}


void UnitTest_GetLineInputMute(CAudioMixerManager* pMixer)
{
    // get initial value
    bool bInitial = pMixer->GetLineInputMute();

    ATLASSERT (pMixer->SetLineInputMute(!bInitial ));
    ATLASSERT (pMixer->GetLineInputMute() == !bInitial );

    pMixer->SetMicMute(bInitial);

    cout << " GetLineInputMute PASSED" << endl;
}

