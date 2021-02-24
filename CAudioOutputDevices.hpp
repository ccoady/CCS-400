//
// This console app enumerates and mutes all audio capture endpoints using the
// IAudioEndpointVolume::SetMute() method of the Vista/Win7 Core Audio API
// 
// Copyright (c) Fotios Basagiannis. All rights reserved
//
// EndpointVolumeChanger.cpp : Endpoint Volume Changing sample application.
//
#pragma once;

#include "CExecuteCommand.hpp"
#include <conio.h>
#include <string>
#include <vector>
#include <Strsafe.h>

#include <FunctionDiscoveryKeys.h>
//#include <EndpointVolume.h>

#include "PolicyConfig.h"
#include <regex>

#include "baselogger.h"

//#define CHK(msg, hr)		if (FAILED(hr)){ printf("%s: %x\n", msg, hr); goto Exit;}
#define CHK(msg, hr)		if (FAILED(hr)){ USES_CONVERSION; \
	BaseLogger::WriteFormattedLog( _T("ERROR: CAudioOutputDevices, %s: hr %x\n"), A2T(msg), hr); \
	goto Exit;}



class CAudioOutputDevices
{
public: 

class CDevice
{
public: tstring tFriendlyName;
public:	tstring tID;
public: tstring tPnpName;
};




	CAudioOutputDevices()
	{
		Init();

	}
	
	~CAudioOutputDevices()
	{
		CleanUp();
	}

	BOOL GetAudioOutputDevices( vector<tstring>& vOutputDevices, tstring& tResponse)
	{
		tResponse.clear();
		vOutputDevices.clear();

		for(int i=0; i<m_vOutputDevices.size(); i++)
		{
			tResponse += m_vOutputDevices.at(i)->tFriendlyName;
			tResponse += _T("\n");

			int idx = (int)m_vOutputDevices.at(i)->tFriendlyName.find(_T("Realtek Digital Output (Realtek High Definition Audio)") );
			if(idx < 0)
				vOutputDevices.push_back(m_vOutputDevices.at(i)->tFriendlyName );
		}

		return m_vOutputDevices.size()>0 ? TRUE : FALSE;
	}

protected: 
	BOOL Init()
	{
		int deviceCount = EnumAudioOutputDevices();
		printf( "\n*** Output devices found: %d \n", deviceCount);

		// log names of all the audio Output devices
		CDevice* pDevice;		
		deviceCount = (int)m_vOutputDevices.size();
		BaseLogger::WriteFormattedLog( _T("CAudioOutputDevices: Audio Output devices count %d"), deviceCount );
		for(int i=0; i< deviceCount; i++)
		{
			pDevice = m_vOutputDevices.at(i);
			BaseLogger::WriteFormattedLog( _T("CAudioOutputDevices: %d Audio Output device '%s' "), i, pDevice->tFriendlyName.c_str() );
		}

		return m_vOutputDevices.size()>0 ? TRUE : FALSE;
	}
	BOOL CleanUp()
	{
		for(int i=0; i<m_vOutputDevices.size(); i++)
			if( m_vOutputDevices.at(i) )
				delete m_vOutputDevices.at(i);
		return TRUE;
	}

	std::vector <CDevice*> m_vOutputDevices;
	template <class T> void SafeRelease(T **ppT)
	{
		if (*ppT)
		{
			(*ppT)->Release();
			*ppT = NULL;
		}
	}



//
// Create the class path-like "PnpName" that Lync writes into the registry
// to decide which audio devices to use.  These will be written into the
// HKEY_USERS\SID\Software\Microsoft\UCCPlatform\Lync\ registry key to 
// override the use of Lync-specific devices
//
// jmv 11/14
//

tstring GetDevicePnpName(IMMDevice* pDevice, tstring strDeviceID)
{
	HRESULT hr;
	tstring strPnpName = _T("");

	IPropertyStore *propertyStore;
	hr = pDevice->OpenPropertyStore(STGM_READ, &propertyStore);
	CHK("Unable to open device property store", hr);

	DWORD nPropertyCount = 0;
	propertyStore->GetCount(&nPropertyCount);

	for (DWORD n = 0; n < nPropertyCount; ++n)
	{
		PROPERTYKEY Key;
		PROPVARIANT Value;
		PropVariantInit(&Value);

		hr = propertyStore->GetAt(n, &Key);

		if (S_OK == hr)
		{
			propertyStore->GetValue(Key, &Value);
			
			if (Value.vt == VT_LPWSTR)
			{
				tstring strValue = Value.pwszVal;

				// Look for "\\?\" in the property
				int nStart = (int)strValue.find(_T("\\\\?\\"));

				if (-1 != nStart)
				{
					// We found a string that is a class path.  Add our ID to it and return it
					int nEnd = (int)strValue.rfind(_T("#"));
					strPnpName = strValue.substr(nStart, nEnd-nStart+1);
					strPnpName += strDeviceID;
					strPnpName += _T("#");

					int nEndCaps = (int)strValue.rfind(_T("#"), nEnd-1);

					if (nEndCaps < strPnpName.length())
					{
						for (int i=0; i < nEndCaps; ++i)
							strPnpName[i] = _totupper(strPnpName[i]);
					}
		
					break;
				}
			}

		}
	}

Exit:
	SafeRelease(&propertyStore);
	return strPnpName;
}




	//
	//  Retrieves the device friendly name for a particular device in a device collection.  
	//  The returned string was allocated using malloc() so it should be freed using free();
	//
	LPWSTR GetDeviceName(IMMDeviceCollection *DeviceCollection, UINT DeviceIndex)
	{
		IMMDevice *device;
		LPWSTR deviceId;
		HRESULT hr;
		wchar_t *returnValue = NULL;
		tstring strPnpName = _T("");

		hr = DeviceCollection->Item(DeviceIndex, &device);
		CHK("Unable to get device ", hr);

		hr = device->GetId(&deviceId);
		CHK("Unable to get device id", hr);

		// Get the Pnp name here while we still have the device pointer
		strPnpName = GetDevicePnpName(device, deviceId);

		IPropertyStore *propertyStore;
		hr = device->OpenPropertyStore(STGM_READ, &propertyStore);
		SafeRelease(&device);
		CHK("Unable to open device property store", hr);

		PROPVARIANT friendlyName;
		PropVariantInit(&friendlyName);
		hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
		SafeRelease(&propertyStore);
		CHK("Unable to retrieve friendly name for device", hr);


		wchar_t deviceName[128];
		hr = StringCbPrintf(deviceName, sizeof(deviceName), L"%s (%s)", friendlyName.vt != VT_LPWSTR ? L"Unknown" : friendlyName.pwszVal, deviceId);
		CHK("Unable to format friendly name for device", hr);


		CDevice* pDevice = new CDevice;
		pDevice->tFriendlyName = friendlyName.pwszVal;
		pDevice->tID = deviceId;
		pDevice->tPnpName = strPnpName;
		m_vOutputDevices.push_back(pDevice);

		PropVariantClear(&friendlyName);
		CoTaskMemFree(deviceId);

		returnValue = _wcsdup(deviceName);
		if (returnValue == NULL)
			printf("Unable to allocate buffer for return\n");

Exit:

		return returnValue;
	}

	int EnumAudioOutputDevices()
	{
		HRESULT hr;
		IMMDeviceEnumerator *deviceEnumerator = NULL;
		IMMDeviceCollection *deviceCollection = NULL;
		UINT deviceCount = 0;
		IMMDevice *device = NULL;

		//HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		CHK("Unable to initialize COM ", hr);

		//We initialize the device enumerator here
		hr = CoCreateInstance( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator) );
		CHK("Unable to instantiate device enumerator ", hr);

		//Here we enumerate the audio endpoints of interest (in this case audio capture endpoints)
		//into our device collection. We use "eCapture" for audio capture endpoints, "eRender" for 
		//audio output endpoints and "eAll" for all audio endpoints 
		hr = deviceEnumerator->EnumAudioEndpoints( eRender, DEVICE_STATE_ACTIVE, &deviceCollection );
		CHK("Unable to retrieve device collection ", hr);

		hr = deviceCollection->GetCount(&deviceCount);
		CHK("Unable to get device collection Count ", hr);

		//
		//This loop goes over each audio endpoint in our device collection,
		//gets and diplays its friendly name and then tries to mute it
		//
		for (UINT i = 0 ; i < deviceCount ; i += 1)
		{
			LPWSTR deviceName;

			//Here we use the GetDeviceName() function provided with the sample 
			//(see source code zip)
			deviceName = GetDeviceName(deviceCollection, i); //Get device friendly name
			if (deviceName == NULL) goto Exit;

			//printf("Device to be muted has index: %d and name: %S\n", i, deviceName);
			printf("Device index: %d and name: %S\n", i, deviceName);

			free(deviceName); //this needs to be done because name is stored in a heap allocated buffer

			/*

			device = NULL;

			//Put device ref into device var
			hr = deviceCollection->Item(i, &device);
			if (FAILED(hr))
			{
			printf("Unable to retrieve device %d: %x\n", i, hr);
			goto Exit;
			}



			//This is the Core Audio interface of interest
			IAudioEndpointVolume *endpointVolume = NULL;

			//We activate it here
			hr = device->Activate( __uuidof(IAudioEndpointVolume), CLSCTX_INPROC_SERVER, NULL, reinterpret_cast<void **>(&endpointVolume) );
			if (FAILED(hr))
			{
			printf("Unable to activate endpoint volume on output device: %x\n", hr);
			goto Exit;
			}


			hr = endpointVolume->SetMute(TRUE, NULL); //Try to mute endpoint here
			if (FAILED(hr))
			{
			printf("Unable to set mute state on endpoint: %x\n", hr);
			goto Exit;
			}
			else
			printf("Endpoint muted successfully!\n");
			*/

		}


Exit: //Core Audio and COM clean up here
		if(deviceCollection) SafeRelease(&deviceCollection);
		if(deviceEnumerator) SafeRelease(&deviceEnumerator);
		if(device) SafeRelease(&device);
		CoUninitialize();
		return deviceCount;
	}

	public: BOOL GetOutputDeviceID(tstring tName, tstring &tID)
	{
		tID.clear();
		BOOL bRetv = FALSE;

		for(int i=0; i<m_vOutputDevices.size(); i++)
		{
			CDevice* pDevice = m_vOutputDevices.at(i);
			int idx = (int)tName.find(pDevice->tFriendlyName);
			if( idx < 0 )
				continue; // try the nex device
			else
			{
				bRetv = TRUE;
				tID = pDevice->tID;
				break;
			}

		}

		// still have not got device id. Lets guess
		/*if(bRetv==FALSE)
			bRetv = SearchOutputDeviceID(L"^Microphone.*USB audio CODEC", tID);*/

		return bRetv;
	}


	public: BOOL GetOutputDevicePnpName(tstring tName, tstring &tPnpName)
	{
		tPnpName.clear();
		BOOL bRetv = FALSE;

		for(int i=0; i<m_vOutputDevices.size(); i++)
		{
			CDevice* pDevice = m_vOutputDevices.at(i);
			int idx = (int)tName.find(pDevice->tFriendlyName);
			if( idx < 0 )
				continue; // try the nex device
			else
			{
				bRetv = TRUE;
				tPnpName = pDevice->tPnpName;
				break;
			}

		}

		return bRetv;
	}


public: 
	BOOL SearchOutputDeviceID(TCHAR wExp[], tstring &tID) // Gets Id of first "Microphone.*audio CODEC"
	{
		BOOL bRetv = FALSE;

		//std::string str = "Microphone (2- USB audio CODEC)";
		//std::tr1::regex rx("^Microphone.*audio CODEC");
		//bRetv = regex_search(str.begin(), str.end(), rx);
		std::wstring wStr;
		for(int i=0; i<m_vOutputDevices.size(); i++)

		{
			CDevice* pDevice = m_vOutputDevices.at(i);
			if(pDevice) wStr = pDevice->tFriendlyName;

			std::tr1::wregex rx( wExp );
			bRetv = regex_search(wStr.begin(), wStr.end(), rx);

			if( bRetv==FALSE )
				continue; // try the nex device
			else
			{
				tID = pDevice->tID;
				break;
			}
		}

		return bRetv;
	}

	void SetAsDefaultDevice(const wchar_t tID[])
	{
		//TCHAR tID[MAX_PATH] = {0};
		HRESULT hr;
		IPolicyConfigVista *pPolicyConfig;
		ERole reserved = eConsole;

		//hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		CHK("Unable to initialize COM: ", hr);

		hr = CoCreateInstance(__uuidof(CPolicyConfigVistaClient), NULL, CLSCTX_ALL, __uuidof(IPolicyConfigVista), (LPVOID *)&pPolicyConfig);
		if (SUCCEEDED(hr))
		{
			hr = pPolicyConfig->SetDefaultEndpoint(tID, eConsole);
			hr = pPolicyConfig->SetDefaultEndpoint(tID, eMultimedia);
			hr = pPolicyConfig->SetDefaultEndpoint(tID, eCommunications);

			pPolicyConfig->Release();
		}
		else
			CHK("Unable to create the PolicyConfig instant: ", hr);

Exit:
		CoUninitialize();
	}

	BOOL GetDefaultDeviceName(tstring& defaultDeviceName)
	{
		HRESULT hr;
		IMMDeviceEnumerator *deviceEnumerator = NULL;
		IMMDevice* pDefaultDevice = 0;
		IPropertyStore *propertyStore = 0;
		PROPVARIANT friendlyName;

		//HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED); //  A GUI application should use COINIT_APARTMENTTHREADED instead of COINIT_MULTITHREADED.
		CHK("Unable to initialize COM: ", hr);

		//We initialize the device enumerator here
		hr = CoCreateInstance( __uuidof(MMDeviceEnumerator), NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&deviceEnumerator) );
		CHK("Unable to instantiate device enumerator: ", hr);

		hr = deviceEnumerator->GetDefaultAudioEndpoint( eCapture, eCommunications, &pDefaultDevice);
		CHK("Unable to get Default Audio Endpoint: ", hr);

		hr = pDefaultDevice->OpenPropertyStore(STGM_READ, &propertyStore);
		CHK("Unable to get Default Audio Endpoint: ", hr);


		PropVariantInit(&friendlyName);
		hr = propertyStore->GetValue(PKEY_Device_FriendlyName, &friendlyName);
		CHK("Unable to get friendly name", hr);
		defaultDeviceName = friendlyName.pwszVal;

Exit: //Core Audio and COM clean up here
		if(deviceEnumerator) SafeRelease(&deviceEnumerator);
		if(pDefaultDevice) SafeRelease(&pDefaultDevice);
		if(propertyStore) SafeRelease(&propertyStore);
		CoUninitialize();

		return TRUE;
	}




	bool SetLyncDeviceRegKeys(HKEY hKey, const tstring strDeviceName, const tstring strPnpName)
	{
		HRESULT hr;

		hr = RegSetValueEx(hKey, _T("PrefAudioRender"), NULL, REG_SZ, (BYTE*) strDeviceName.c_str(), (DWORD)strDeviceName.length() * sizeof(TCHAR));

		if (hr == ERROR_SUCCESS)
			hr = RegSetValueEx(hKey, _T("PrefAudioRenderPnpName"), NULL, REG_SZ, (BYTE*) strPnpName.c_str(), (DWORD)strPnpName.length() * sizeof(TCHAR));

		if (hr == ERROR_SUCCESS)
		{
			DWORD dwValue = 1;
			hr = RegSetValueEx(hKey, _T("PrefAudioIsManual"), NULL, REG_DWORD, (BYTE*) &dwValue, sizeof(DWORD));
		}

		if (hr != ERROR_SUCCESS)
		{
			BaseLogger::WriteLog(_T("SetLyncDeviceRegKeys Failed to set keys"));
			return false;
		}

		return true;
	}


	void SetLyncOutputDevice(const tstring strDeviceName, const tstring strPnpName)
	{
		HKEY hKey;
		TCHAR tszName[_MAX_PATH];
		DWORD dwIndex = 0;
		DWORD dwNameLen = _MAX_PATH;
		
		// Set these values for each user SID that's in the HKEY_USERS registry branch

		LONG lResult = RegOpenKeyEx( HKEY_USERS, _T(""), 0, KEY_ALL_ACCESS, &hKey );
		LONG lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);

// JAS:UNREFERENCED        TCHAR tszValueData[_MAX_PATH];
        
		while (ERROR_SUCCESS == lResultEnum)
		{
			// Exclude short keys
			if (dwNameLen > 9)
			{
				// Exclude "Classes" keys
				if (NULL == (_tcsstr(tszName, _T("_Classes"))))
				{
					TCHAR tszKey[_MAX_PATH];

                    
					HKEY hKeyLync;
                    
					_stprintf_s(tszKey, _MAX_PATH, _T("%s\\Software\\Microsoft\\UCCPlatform\\Lync"), tszName);
					lResult = RegOpenKeyEx( HKEY_USERS, tszKey, 0, KEY_ALL_ACCESS, &hKeyLync );
					if(ERROR_SUCCESS == lResult)
					{
						SetLyncDeviceRegKeys(hKeyLync, strDeviceName, strPnpName);
						RegCloseKey(hKeyLync);
					}
				}
			}

			dwNameLen = _MAX_PATH;
			++dwIndex;
			lResultEnum = RegEnumKeyEx(hKey, dwIndex, tszName, &dwNameLen, NULL, NULL, NULL, NULL);
		}		

	}




	public: int test( )
		{
			int deviceCount;
			CDevice *pDevice;
			int i = -1;

			deviceCount = EnumAudioOutputDevices();
			printf( "\n*** Output devices found: %d \n", deviceCount);

			if(deviceCount > 0)
			{
				while(i != 'X' )
				{
					_cputs( "SetAsDefaultDevice, type a number:  " );
					i = toupper( _getch() );	
					_putch( i );
					_cputs( "\r\n");

					if(i>='0' && i< (48+deviceCount))
					{
						pDevice = m_vOutputDevices.at(i);
						SetAsDefaultDevice( pDevice->tID.c_str() );
					}
					else
						_cputs( "ERROR: Not a valid device number \r\n"  );
				}
			}

			return 0;

		}
};
