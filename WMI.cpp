// CWMI.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"
#include "WMI.h"

#ifndef UNDER_CE
#include "CmPCConfigDLL.h"
#include "SystemPatch.h"

#define _WIN32_DCOM
#include <iostream>
using namespace std;
#include <windows.h>
#include <Wbemidl.h>
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")
#include "comutil.h"

#endif

#define WMI_DEBUG			TRUE
#if WMI_DEBUG
#	define  WMI_TRACE		ATLTRACE
#else
#	if (_MSC_VER >= 1400) // VS2005 Compiler
		#define WMI_TRACE		__noop
#	else // (_MSC_VER < 1400)
		#define WMI_TRACE		((void)(0))
#	endif // (_MSC_VER >= 1400)
#endif

//*****************************************************************************
// constructor(s)/destructor(s)
//*****************************************************************************
CWMI::CWMI()
: m_bSetIPAddress(false)
, m_bSetIPMask(false)
, m_bSetDefRouter(false)
{
}

CWMI::~CWMI()
{
	Cleanup();
}

#ifndef UNDER_CE
//*****************************************************************************
//
// Method:      void Init()
//
// Parameters:  none
//
// Returns:		void
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
void CWMI::Init()
{
	HRESULT hr;

	// initialize the COM library
	if ((hr = CoInitialize(0)) != S_OK)
		WMI_TRACE(_T("CoInitializeEx() failed in CWMI\r\n"));

	if (GetAdapterInfoViaWmi() == TRUE)
		m_bAdapterInfoRetrieved = true;
	else
		m_bAdapterInfoRetrieved = false;
}

//*****************************************************************************
//
// Method:      void Cleanup()
//
// Parameters:  none
//
// Returns:		void
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
void CWMI::Cleanup()
{
	ClearAdapterList();

	CoUninitialize();
}

//*****************************************************************************
//
// Method:      HRESULT PrepareAdapterInfoList()
//
// Parameters:  none
//
// Returns:		HANDLE
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/21/10
//
//*****************************************************************************
HRESULT CWMI::PrepareAdapterInfoList()
{
	if (IsAdapterInfoListFlag() == false)
	{
		if (GetAdapterInfoViaWmi() == FALSE)
			return S_FALSE;
		SetAdapterInfoListFlag(true);
	}
	
	return S_OK;
}
//*****************************************************************************
//
// Method:      int GetAdapterInfoViaWmi()
//
// Parameters:  void
//
// Returns:		BOOL
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
BOOL CWMI::GetAdapterInfoViaWmi(void)
{
	if (FindPhysicalAdaptersViaWmi())
		return QueryPhysicalAdapterAttribViaWmi();
	else
		return FALSE;
}

//*****************************************************************************
//
// Method:      BOOL RefreshAdapterInfo()
//
// Parameters:  void
//
// Returns:		BOOL
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 1/20/11
//
//*****************************************************************************
void CWMI::RefreshAdapterInfo(void)
{
	Cleanup();

	Init();
}


//*****************************************************************************
//
// Method:      int FindPhysicalAdaptersViaWmi()
//
// Parameters:  
//
// Returns:		void
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
BOOL CWMI::FindPhysicalAdaptersViaWmi()
{
	try
	{
		BOOL bRet = FALSE;
		HRESULT hr;

		IWbemLocator *pLocator = NULL;
		IWbemServices *pService = NULL;
		IEnumWbemClassObject *pEnum = NULL;

		IWbemClassObject *pClass = NULL;
		IWbemClassObject *pMethod = NULL;
		IWbemClassObject *pInInst = NULL;
		IWbemClassObject *pOutInst = NULL;

		// create WbemLocatorInstance
		if (CoCreateInstance(CLSID_WbemAdministrativeLocator,
							 NULL,
							 CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER,
							 IID_IUnknown,
							 (void**)&pLocator) != S_OK)
		{
			return FALSE;
		}

		// using the WbemLocator, connect to the server
		BSTR bstrNamespace = (L"root\\cimv2");
		if (pLocator->ConnectServer(bstrNamespace,
									NULL,
									NULL,
									NULL,
									0,
									NULL,
									NULL,
									&pService) != S_OK)
		{
			pLocator->Release();
			return FALSE;
		}

		// set security levels on the proxy
		hr = CoSetProxyBlanket(pService,
							   RPC_C_AUTHN_WINNT,
							   RPC_C_AUTHZ_NONE,
							   NULL,
							   RPC_C_AUTHN_LEVEL_CALL,
							   RPC_C_IMP_LEVEL_IMPERSONATE,
							   NULL,
							   EOAC_NONE);
		if (FAILED(hr))
		{
			pService->Release();
			pLocator->Release();
			return FALSE;
		}
		
		// use IWbemServices pointer to make requests of m_WMI
		hr = pService->ExecQuery(bstr_t("WQL"),
								 bstr_t("SELECT * FROM  Win32_NetworkAdapter"),
								 WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
								 NULL,
								 &pEnum);
		if (FAILED(hr))
		{
			pService->Release();
			pLocator->Release();
			return FALSE;
		}

		// get the data
		IWbemClassObject *pclsObj = NULL;
		VARIANT vtProp, vtAdapterTypeID, vtManufacturer;
		CIMTYPE vtType = 0;
		ULONG ulReturn;

		while (pEnum)
		{
			ulReturn = 0;
			HRESULT hr = pEnum->Next(5000, 1, &pclsObj, &ulReturn);
			if (hr == WBEM_S_TIMEDOUT)
			{
				bRet = FALSE;
				break;
			}

			if (SUCCEEDED(hr) && ulReturn)
			{
				hr = pclsObj->Get(L"AdapterTypeID", 0, &vtAdapterTypeID, &vtType, 0);
				if (SUCCEEDED(hr) && vtAdapterTypeID.vt != VT_NULL)
				{
					hr = pclsObj->Get(L"Manufacturer", 0, &vtManufacturer, &vtType, 0);
					if (SUCCEEDED(hr) && vtManufacturer.vt != VT_NULL)
					{
						// need to expand if there are non-Microsoft virtual adapters
						if (	(vtAdapterTypeID.ulVal == 0x00 || vtAdapterTypeID.ulVal == 0x09)
							&&	(memcmp(_T("Microsoft"), (LPCTSTR)_bstr_t(vtManufacturer), sizeof((LPCTSTR)_bstr_t(vtManufacturer)))))
						{
							// found a physical adapter

							ETHERNETADAPTERINFO * pInfo = new ETHERNETADAPTERINFO;
							if (pInfo == NULL)
							{
								// failed to allocate memory
								VariantClear(&vtAdapterTypeID);
								VariantClear(&vtManufacturer);
								break;
							}

							// store index number of the network adapter
							hr = pclsObj->Get(L"Index", 0, &vtProp, 0, 0);
							pInfo->dwIndex = vtProp.ulVal;
							VariantClear(&vtProp);

							// adapter type id
							pInfo->dwAdapterTypeID = vtAdapterTypeID.ulVal;

							// adapter manufacturer
							_stprintf_s(pInfo->m_tsManufacturer, MAX_PATH, _T("%s"), vtManufacturer.bstrVal );

							m_AdapterList.push_back(pInfo);
							bRet = TRUE;
						}
					}
				}
				VariantClear(&vtAdapterTypeID);
				VariantClear(&vtManufacturer);
			}
			else
				break;
		}

		// cleanup
		if (pService != NULL)
			pService->Release();

		if (pLocator != NULL)
			pLocator->Release();

		if (pEnum != NULL)
			pEnum->Release();

		if (pclsObj != NULL)
			pclsObj->Release();

		return bRet;
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
}

//*****************************************************************************
//
// Method:      int QueryPhysicalAdapterAttribViaWmi()
//
// Parameters:  void
//
// Returns:		BOOL
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
BOOL CWMI::QueryPhysicalAdapterAttribViaWmi(void)
{
	try
	{
		BOOL bRet = FALSE;
		HRESULT hr;

		IWbemLocator *pLocator = NULL;
		IWbemServices *pNamespace = 0;
		IWbemClassObject *pClass = NULL;
		IWbemClassObject *pMethod = NULL;
		IWbemClassObject *pInInst = NULL;
		IWbemClassObject *pOutInst = NULL;

		// strings needed later
		BSTR className = SysAllocString(L"Win32_NetworkAdapterConfiguration");
		BSTR methodName = SysAllocString(L"EnableStatic");
		BSTR namespacePath = SysAllocString(L"root\\cimv2");

		// create WbemLocatorInstance
		hr = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER,
								IID_IWbemLocator, (LPVOID *)&pLocator);

		// using the WbemLocator, connect ot the namespace
		hr = pLocator->ConnectServer(namespacePath, NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);

		hr = CoSetProxyBlanket(pNamespace,						// Indicates the proxy to set 
								RPC_C_AUTHN_WINNT,				// RPC_C_AUTHN_xxx 
								RPC_C_AUTHZ_NONE,				// RPC_C_AUTHZ_xxx 
								NULL,							// Server principal name 
								RPC_C_AUTHN_LEVEL_CALL,			// RPC_C_AUTHN_LEVEL_xxx 
								RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx 
								NULL,							// client identity 
								EOAC_NONE);						// proxy capabilities 

		if (FAILED(hr))
		{
			ATLTRACE(_T("Failed 'CoSetProxyBlanket()'\r\n"));

			pNamespace->Release();
			pLocator->Release();
			return FALSE;	// program has failed
		}

		// get the class object
		hr = pNamespace->GetObject(className, 0, NULL, &pClass, NULL);
		if (FAILED(hr))
		{
			ATLTRACE(_T("Failed 'GetObject()'\r\n"));

			pNamespace->Release();
			pLocator->Release();
			return FALSE;
		}

		// get the method we want to use in this class
		hr = pClass->GetMethod(methodName, 0, &pMethod, NULL);
		if (FAILED(hr))
		{
			ATLTRACE(_T("Failed 'GetMethod()'\r\n"));

			pNamespace->Release();
			pLocator->Release();
			return FALSE;
		}

		// spawn an instance of the method so we can set it up (parameters)
		hr = pMethod->SpawnInstance(0, &pInInst);
		if (FAILED(hr))
		{
			ATLTRACE(_T("Failed 'SpawnInstance()'\r\n"));

			pNamespace->Release();
			pLocator->Release();
			return FALSE;
		}

		// enumerate the instances of the NetworkAdapter to get at the __PATH property
		IEnumWbemClassObject* pEnum;
		hr = pNamespace->CreateInstanceEnum(className,
											WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
											NULL, &pEnum);
		if (FAILED(hr))
		{
			ATLTRACE(_T("Failed 'CreateInstanceEnum()'\r\n"));

			pNamespace->Release();
			pLocator->Release();
			return FALSE;
		}

		// get the instance
		ULONG numRet;
		BSTR PropName = NULL;;
		VARIANT vtDesc, vtDhcp, vtIPAddress, vtSubnetMask, vtDefaultGateway,
				vtDNSServer, vtWINSPrimaryServer, vtWINSSecondaryServer;

		IWbemClassObject *pclsObj = NULL;
		SAFEARRAY *pSA = NULL;
		CComBSTR bstr;
		BSTR *pbstr;
		long lIndex;

		while (hr != WBEM_S_FALSE)
		{
			hr = pEnum->Next(5000, (ULONG)1, &pclsObj, &numRet);
			if (hr == WBEM_S_TIMEDOUT)
			{
				bRet = FALSE;
				break;
			}

			if (SUCCEEDED(hr) && numRet == 0)
			{
				// successfully got data, there just isn't anything there
				break;
			}

			VARIANT vtIndex;
			CIMTYPE vtType = 0;

			hr = pclsObj->Get(L"Index", 0, &vtIndex, &vtType, 0);
			if (SUCCEEDED(hr) && vtIndex.vt != VT_NULL)
			{
				EthernetAdapterList::iterator iter;
				iter = m_AdapterList.begin();
				while (iter != m_AdapterList.end())
				{
					ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
					if (pInfo->dwIndex == vtIndex.ulVal)
					{
						lIndex = pInfo->dwIndex;

						//*********************************************************
						// found the matching adapter
						//*********************************************************

						//*********************************************************
						// Description
						//*********************************************************
						
						// initialize default description string
						_stprintf_s(pInfo->m_tsDesc, MAX_ADAPTER_DESCRIPTION_LENGTH, _T(""));
						PropName = SysAllocString(L"Description");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtDesc, 0, 0);
							SysFreeString(PropName);
							if (SUCCEEDED(hr))
							{
								if (vtDesc.vt != VT_NULL)
									_stprintf_s(pInfo->m_tsDesc, MAX_ADAPTER_DESCRIPTION_LENGTH, _T("%s"),  vtDesc.bstrVal );
							}
						}

						//*********************************************************
						// DHCP
						//*********************************************************
						PropName = SysAllocString(L"DHCPEnabled");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtDhcp, 0, 0);
							SysFreeString(PropName);
							if (SUCCEEDED(hr))
							{
								pInfo->m_dwDHCP = (DWORD)vtDhcp.boolVal;
								pInfo->m_dwNewDHCP = (DWORD)vtDhcp.boolVal;
							}
						}

						//*********************************************************
						// IP address
						//*********************************************************
						_stprintf_s(pInfo->m_tsIpAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"IPAddress");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtIPAddress, 0, 0);
							SysFreeString(PropName);

							if (vtIPAddress.vt != VT_NULL && SUCCEEDED(hr))
							{
								pSA = vtIPAddress.parray;
								hr = SafeArrayAccessData(pSA, (void HUGEP**)&pbstr);
								if (SUCCEEDED(hr))
								{
									_stprintf_s(pInfo->m_tsIpAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									_stprintf_s(pInfo->m_tsNewIpAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									hr = SafeArrayUnaccessData(pSA);
								}
							}
						}

						//*********************************************************
						// IP Subnet Mask
						//*********************************************************
						_stprintf_s(pInfo->m_tsIpMask, MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"IPSubnet");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtSubnetMask, 0, 0);
							SysFreeString(PropName);

							if (vtSubnetMask.vt != VT_NULL && SUCCEEDED(hr))
							{
								pSA = vtSubnetMask.parray;
								hr = SafeArrayAccessData(pSA, (void HUGEP**)&pbstr);
								if (SUCCEEDED(hr))
								{
									_stprintf_s(pInfo->m_tsIpMask, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									_stprintf_s(pInfo->m_tsNewIpMask, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									hr = SafeArrayUnaccessData(pSA);
								}
							}
						}

						//*********************************************************
						// Primary/Secondary DNS address
						//*********************************************************
						_stprintf_s(pInfo->m_tsDnsAddr[0], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						_stprintf_s(pInfo->m_tsDnsAddr[1], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"DNSServerSearchOrder");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtDNSServer, 0, 0);
							SysFreeString(PropName);

							// DNS addresses
							if (vtDNSServer.vt != VT_NULL && SUCCEEDED(hr))
							{
								LONG lLow = 0, lUp = 0, lCnt = 0;
								::SafeArrayGetLBound(vtDNSServer.parray, 1, &lLow);
								::SafeArrayGetUBound(vtDNSServer.parray, 1, &lUp);
								for (LONG i = lLow; (i <= lUp && lCnt < 2); i++)
								{
									BSTR bsDnsServerIPAddress;
									if (::SafeArrayGetElement(vtDNSServer.parray, &i, &bsDnsServerIPAddress) == S_OK)
									{
										_stprintf_s(pInfo->m_tsDnsAddr[lCnt], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(bsDnsServerIPAddress));
										_stprintf_s(pInfo->m_tsNewDnsAddr[lCnt], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(bsDnsServerIPAddress));
										lCnt++;
									}
								}
							}
						}

						//*********************************************************
						// Primary WINS address
						//*********************************************************
						_stprintf_s(pInfo->m_tsWinsAddr[0], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"WINSPrimaryServer");
						if (PropName != NULL)
						{
							hr = pclsObj->Get(PropName, 0, &vtWINSPrimaryServer, 0, 0);
							SysFreeString(PropName);
							if (vtWINSPrimaryServer.vt != VT_NULL && SUCCEEDED(hr))
							{
								_stprintf_s(pInfo->m_tsWinsAddr[0], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(vtWINSPrimaryServer));
								_stprintf_s(pInfo->m_tsNewWinsAddr[0], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(vtWINSPrimaryServer));
							}
						}

						//*********************************************************
						// Secondary WINS address
						//*********************************************************
						_stprintf_s(pInfo->m_tsWinsAddr[1], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"WINSSecondaryServer");
						hr = pclsObj->Get(PropName, 0, &vtWINSSecondaryServer, 0, 0);
						SysFreeString(PropName);
						if (vtWINSSecondaryServer.vt != VT_NULL && SUCCEEDED(hr))
						{
							_stprintf_s(pInfo->m_tsWinsAddr[1], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(vtWINSSecondaryServer));
							_stprintf_s(pInfo->m_tsNewWinsAddr[1], MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(vtWINSSecondaryServer));
						}

						//*********************************************************
						// Default router/gateway IP address
						//*********************************************************
						_stprintf_s(pInfo->m_tsGatewayAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
						PropName = SysAllocString(L"DefaultIPGateway");
						hr = pclsObj->Get(PropName, 0, &vtDefaultGateway, 0, 0);
						SysFreeString(PropName);
						if (SUCCEEDED(hr))
						{
							lIndex = 0;

							if (vtDefaultGateway.vt != VT_NULL)
							{
								pSA = vtDefaultGateway.parray;
								hr = SafeArrayAccessData(pSA, (void HUGEP**)&pbstr);
								if (SUCCEEDED(hr))
								{
									_stprintf_s(pInfo->m_tsGatewayAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									_stprintf_s(pInfo->m_tsNewGatewayAddr, MAX_ADAPTER_DESCRIPTION_LENGTH, _bstr_t(*pbstr));
									hr = SafeArrayUnaccessData(pSA);
								}
							}
						}
						bRet = TRUE;
					}
					iter++;
				}
			}
		}

		// free up resources
		if (PropName != NULL)
			SysFreeString(PropName);

		if (pEnum != NULL)
			pEnum->Release();

		if (pclsObj != NULL)
			pclsObj->Release();

		if (className != NULL)
			SysFreeString(className);

		if (namespacePath != NULL)
			SysFreeString(namespacePath);

		if (pLocator != NULL)
			pLocator->Release();

		if (pClass != NULL)
			pClass->Release();

		if (pMethod != NULL)
			pMethod->Release();

		if (pNamespace != NULL)
			pNamespace->Release();

		if (pInInst != NULL)
			pInInst->Release();

		return bRet;
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
}

//*****************************************************************************
//
// Method:      BOOL SetWin32_NetworkAdapterConfigurationViaWmi()
//
// Parameters:  
//
// Returns:		void
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
BOOL CWMI::SetWin32_NetworkAdapterConfigurationViaWmi(ETHERNETADAPTERINFO *pAdapterInfo)
{
	try
	{
		HRESULT hr;
		BOOL bRet = TRUE;

		BSTR bstrNamespacePath = SysAllocString(L"root\\cimv2");

		IWbemLocator *pLocator = NULL;
		IWbemServices *pServices = NULL;

		// create WbemLocatorInstance
		hr = CoCreateInstance(CLSID_WbemLocator,
							  0,
							  CLSCTX_INPROC_SERVER,
							  IID_IWbemLocator,
							  (LPVOID*)&pLocator);
		if (FAILED(hr))
		{
			SysFreeString(bstrNamespacePath);
			return FALSE;
		}

		// connect to the service
		hr = pLocator->ConnectServer(bstrNamespacePath,	// object path of WMI namespace
									 NULL,				// user name. NULL = current user
									 NULL,				// user password. NULL = current user
									 NULL,				// locale. Null indicates current
									 0,					// security flags
									 NULL,				// authority
									 NULL,				// context object
									 &pServices);		// pointer to IWbemServices proxy
		if (FAILED(hr))
		{
			SysFreeString(bstrNamespacePath);
			pLocator->Release();
			return FALSE;
		}

		hr = CoSetProxyBlanket(pServices,					// Indicates the proxy to set
							   RPC_C_AUTHN_WINNT,			// RPC_C_AUTHN_xxx
							   RPC_C_AUTHZ_NONE,			// RPC_C_AUTHZ_xxx
							   NULL,						// Server principal name
							   RPC_C_AUTHN_LEVEL_CALL,		// RPC_C_AUTHN_LEVEL_xxx
							   RPC_C_IMP_LEVEL_IMPERSONATE,	// RPC_C_IMP_LEVEL_xxx
							   NULL,						// client identity
							   EOAC_NONE);					// proxy capabilities
		if (FAILED(hr))
		{
			SysFreeString(bstrNamespacePath);
			pServices->Release();
			pLocator->Release();
			return FALSE;
		}
		
		// grab class required to work on Win32_NetworkAdapterConfiguration
		IWbemClassObject *pClass = NULL;
		BSTR bstrClassPath = SysAllocString(L"Win32_NetworkAdapterConfiguration");
		hr = pServices->GetObject(bstrClassPath, 0, NULL, &pClass, NULL);
		if (FAILED(hr))
		{
			SysFreeString(bstrClassPath);
			SysFreeString(bstrNamespacePath);
			pServices->Release();
			pLocator->Release();
			return FALSE;
		}
		SysFreeString(bstrClassPath);

		if (WBEM_S_NO_ERROR == hr)
		{
			// grab pointers to the input parameter class of the two methods we are going to call
			BSTR bstrMethod_EnableStatic            = SysAllocString(L"EnableStatic");
			BSTR bstrMethod_SetGateways             = SysAllocString(L"SetGateways");

			HRESULT	hrEnableStatic, hrSetGateways;

			IWbemClassObject *pInClass_EnableStatic            = NULL;
			IWbemClassObject *pInClass_SetGateways             = NULL;

			if (	(pClass->GetMethod(bstrMethod_EnableStatic, 0, &pInClass_EnableStatic, NULL) == WBEM_S_NO_ERROR)
				&&	(pClass->GetMethod(bstrMethod_SetGateways, 0, &pInClass_SetGateways, NULL) == WBEM_S_NO_ERROR))
			{
				// spawn instances of the input parameter classes so that we can stuff our parameters in
				IWbemClassObject * pInInst_EnableStatic = NULL;
				IWbemClassObject * pInInst_SetGateways  = NULL;

				if (	(pInClass_EnableStatic->SpawnInstance(0, &pInInst_EnableStatic) == WBEM_S_NO_ERROR)
					&&	(pInClass_SetGateways->SpawnInstance(0, &pInInst_SetGateways) == WBEM_S_NO_ERROR))
				{
					//*************************************************************
					// pack desired parameters into the input class instances
					//*************************************************************

					// insert into safe arrays, allocating memory as we do so
					// (destroying the safe array will destroy the allocated memory)
					long lIndex[] = {0};

//					BSTR bstrIp          = SysAllocString(_bstr_t(pAdapterInfo->m_tsNewIpAddr));
//					BSTR bstrIpMask      = SysAllocString(_bstr_t(pAdapterInfo->m_tsNewIpMask));
//					BSTR bstrGatewayAddr = SysAllocString(_bstr_t(pAdapterInfo->m_tsNewGatewayAddr));

					BSTR bstrIp          = SysAllocString(_bstr_t(_T("172.30.185.123")));
					BSTR bstrIpMask      = SysAllocString(_bstr_t(_T("255.255.240.0")));
					BSTR bstrGatewayAddr = SysAllocString(_bstr_t(_T("172.30.176.1")));

					//*************************************************************
					// IP address
					//*************************************************************
					SAFEARRAY *psaIpList = SafeArrayCreateVector(VT_BSTR, 0, 1);
					SafeArrayPutElement(psaIpList, lIndex, bstrIp);

					//*************************************************************
					// subnet mask
					//*************************************************************
					SAFEARRAY *psaMaskList = SafeArrayCreateVector(VT_BSTR, 0, 1);
					SafeArrayPutElement(psaMaskList, lIndex, bstrIpMask);

					//*************************************************************
					// default gateway
					//*************************************************************

#define DESIRED_GW_METRIC	1					// default value

					unsigned short usMetric = DESIRED_GW_METRIC;

					SAFEARRAY *psaGatewayList = SafeArrayCreateVector(VT_BSTR, 0, 1);
					SafeArrayPutElement(psaGatewayList, lIndex, bstrGatewayAddr);

					SAFEARRAY *psaMetricList = SafeArrayCreateVector(VT_UI1, 0, 1);
					SafeArrayPutElement(psaMetricList, lIndex, &usMetric);

					//************************************************************
					// now wrap each safe array in a VARIANT so that it can be
					// passed to COM function
					//*************************************************************

					//*************************************************************
					// IP address
					//*************************************************************
					VARIANT vtEnableStaticArg1;
					VariantInit(&vtEnableStaticArg1);
					vtEnableStaticArg1.vt = VT_ARRAY | VT_BSTR;
					vtEnableStaticArg1.parray = psaIpList;

					//*************************************************************
					// subnet mask
					//*************************************************************
					VARIANT vtEnableStaticArg2;
					VariantInit(&vtEnableStaticArg2);
					vtEnableStaticArg2.vt = VT_ARRAY |VT_BSTR;
					vtEnableStaticArg2.parray = psaMaskList;

					//*************************************************************
					// default router
					//*************************************************************
					VARIANT vtSetGatewaysArg1;
					VariantInit(&vtSetGatewaysArg1);
					vtSetGatewaysArg1.vt = VT_ARRAY | VT_BSTR;
					vtSetGatewaysArg1.parray = psaGatewayList;

					VARIANT vtSetGatewaysArg2;
					VariantInit(&vtSetGatewaysArg2);
					vtSetGatewaysArg2.vt = VT_ARRAY | VT_UI1;
					vtSetGatewaysArg2.parray = psaMetricList;

					HRESULT hr1, hr2, hr3, hr4/*, hr5, hr6, hr7*/;
					if (	((hr1 = pInInst_EnableStatic->Put(L"IPAddress", 0, &vtEnableStaticArg1, 0)) == WBEM_S_NO_ERROR)
						&&	((hr2 = pInInst_EnableStatic->Put(L"SubNetMask", 0, &vtEnableStaticArg2, 0)) == WBEM_S_NO_ERROR)
						&&	((hr3 = pInInst_SetGateways->Put(L"DefaultIPGateway", 0, &vtSetGatewaysArg1, 0)) == WBEM_S_NO_ERROR)
						&&	((hr4 = pInInst_SetGateways->Put(L"GatewayCostMetric", 0, &vtSetGatewaysArg2, 0)) == WBEM_S_NO_ERROR))
					{
						// first build the object path that specifies which
						// network adapter we are executing a method on
						char indexString[10];
						_itoa(pAdapterInfo->dwIndex, indexString, 10);

						char cInstanceString[100];
						wchar_t wcInstanceString[100];

						strcpy(cInstanceString, "Win32_NetworkAdapterConfiguration.Index='");
						strcat(cInstanceString, indexString);
						strcat(cInstanceString, "'");
						mbstowcs(wcInstanceString, cInstanceString, 100);
										
						BSTR bstrInstancePath = SysAllocString(wcInstanceString);

						// Now call the methods
						IWbemClassObject * pOutInst = NULL;

						//*********************************************************
						// IP Address
						//*********************************************************
						hrEnableStatic = pServices->ExecMethod(bstrInstancePath,
															   bstrMethod_EnableStatic,
															   0,
															   NULL,
															   pInInst_EnableStatic,
															   &pOutInst,
															   NULL);
						if (hrEnableStatic != WBEM_S_NO_ERROR)
						{
							ATLTRACE(_T("EnableStatic - err(0x%0x)\r\n"), hrEnableStatic);
							bRet = FALSE;
						}

						//*********************************************************
						// default router/gateway
						//*********************************************************
						hrSetGateways = pServices->ExecMethod(bstrInstancePath,
															  bstrMethod_SetGateways,
															  0,
															  NULL,
															  pInInst_SetGateways,
															  NULL,
															  NULL);
						if (hrSetGateways != WBEM_S_NO_ERROR)
						{
							ATLTRACE(_T("SetGateways - err(0x%0x)\r\n"), hrSetGateways);
							bRet = FALSE;
						}

						SysFreeString(bstrInstancePath);
					}
					else
					{
	#if _DEBUG
						if (hr4 == WBEM_S_NO_ERROR)
							ATLTRACE(_T("Failed 'GatewayCostMetric' Put - err(0x%0x)\r\n"), hr4);
						if (hr3 == WBEM_S_NO_ERROR)
							ATLTRACE(_T("Failed 'DefaultIPGateway' Put - err(0x%0x)\r\n"), hr3);
						if (hr2 == WBEM_S_NO_ERROR)
							ATLTRACE(_T("Failed 'SubNetMask' Put - err(0x%0x)\r\n"), hr2);
						if (hr1 == WBEM_S_NO_ERROR)
							ATLTRACE(_T("Failed 'IPAddress' Put - err(0x%0x)\r\n"), hr1);
	#endif // _DEBUG
						bRet = FALSE;
					}

					// clear the variants
					VariantClear(&vtEnableStaticArg1);
					VariantClear(&vtEnableStaticArg2);
					VariantClear(&vtSetGatewaysArg1);
					VariantClear(&vtSetGatewaysArg2);

					// destroy the safe arrays
					SafeArrayDestroy(psaIpList);
					psaIpList = NULL;

					SafeArrayDestroy(psaMaskList);
					psaMaskList = NULL;

					SafeArrayDestroy(psaGatewayList);
					psaGatewayList = NULL;

					// destroy the BSTR pointers
					SysFreeString(bstrIp);
					SysFreeString(bstrIpMask);
					SysFreeString(bstrGatewayAddr);
				}

				// free up the instances that we spawned
				if (pInInst_EnableStatic)
				{
					pInInst_EnableStatic->Release();
					pInInst_EnableStatic = NULL;
				}

				if (pInInst_SetGateways)
				{
					pInInst_SetGateways->Release();
					pInInst_SetGateways = NULL;
				}
			}

			// Free up methods input parameters class pointers
			if (pInClass_EnableStatic)
			{
				pInClass_EnableStatic->Release();
				pInClass_EnableStatic = NULL;
			}

			if (pInClass_SetGateways)
			{
				pInClass_SetGateways->Release();
				pInClass_SetGateways = NULL;
			}

			SysFreeString(bstrMethod_EnableStatic);
			SysFreeString(bstrMethod_SetGateways);
		}

		// variable cleanup
		if (pClass)
		{
			pClass->Release();
			pClass = NULL;
		}

		return bRet;
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
}

//*****************************************************************************
//
// Method:      void ClearAdapterList()
//
// Parameters:  
//
// Returns:		void
//
// Scope:       public
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
void CWMI::ClearAdapterList()
{
	try
	{
		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
			iter++;
			delete pInfo;
			pInfo = NULL;
		}

		m_AdapterList.erase(m_AdapterList.begin(), m_AdapterList.end());
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}
}

//*****************************************************************************
//
// Method:      void ConvertIPAddressStringToLong(const _TCHAR*)
//
// Parameters:  [in] _TCHAR * - ;
//
// Returns:		unsigned long
//
// Scope:       protected
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
BOOL CWMI::ExecuteNetsh(_TCHAR * tszCmdLine)
{
	try
	{
		BOOL bRet = FALSE;

		STARTUPINFO si;
		memset(&si, 0, sizeof(si));
		si.cb = sizeof(STARTUPINFO);
		si.lpReserved = NULL;
		si.dwFlags = 0x01;	// STARTF_USESHOWWINDOW
		si.wShowWindow = SW_HIDE;

		PROCESS_INFORMATION pi;

		if (CreateProcess(NULL, tszCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
		{
			WaitForSingleObject(pi.hProcess, INFINITE);

			DWORD dwExit = (DWORD)-1;
			if (GetExitCodeProcess(pi.hProcess,&dwExit))
			{
				bRet = dwExit == 0 ? TRUE : FALSE;
			}
		}

		CloseHandle(pi.hThread);
		CloseHandle(pi.hProcess);

		return bRet;
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return FALSE;
	}
}

//*****************************************************************************
//
// Method:      SetStaticIPAddress(ETHERNETADAPTERINFO *)
//
// Parameters:  [in] * pInfo - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 6/24/10
//
//*****************************************************************************
HRESULT CWMI::SetStaticIPAddress(ETHERNETADAPTERINFO *pInfo)
{
	_TCHAR tsCmdLine[MAX_PATH] = {0};

	try
	{
		_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip set address local static %s %s %s 1"),
				pInfo->m_tsNewIpAddr, pInfo->m_tsNewIpMask, pInfo->m_tsNewGatewayAddr);

		if (ExecuteNetsh(tsCmdLine) == FALSE)
		{
			return S_FALSE;
		}
		return S_OK;
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
		return S_FALSE;
	}
}


//*****************************************************************************
//
// Method:      HRESULT SetDNSAddress(ETHERNETADAPTERINFO *)
//
// Parameters:  [in] pInfo  - ;
//
// Returns:		HRESULT - S_OK: successfully set the DNS IP addresses;
//                        S_FALSE: failed to set the DNS IP addresses;
//
// Scope:       private
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
HRESULT CWMI::SetDNSAddress(ETHERNETADAPTERINFO *pInfo)
{
	unsigned long ulPrevDnsAddr, ulCurrDnsAddr;

	try{
		for (WORD wServer = 0; wServer < 2; wServer++)	// wServer = 0: 'primary'
														//           1: 'secondary'
		{
			// get the IP address of the local adapter
			EthernetAdapterList::iterator iter;
			iter = m_AdapterList.begin();
			while (iter != m_AdapterList.end())
			{
				ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
				ulPrevDnsAddr = ConvertIPAddressStringToLong(pInfo->m_tsDnsAddr[wServer]);
				break;
			}

			// create the command string
			_TCHAR tsCmdLine[MAX_PATH];
			ulCurrDnsAddr = ConvertIPAddressStringToLong(pInfo->m_tsNewDnsAddr[wServer]);
			if (ulPrevDnsAddr != ulCurrDnsAddr)
			{
				if (ulPrevDnsAddr != 0)
				{
					// delete primary DNS
					EthernetAdapterList::iterator iter;
					iter = m_AdapterList.begin();
					while (iter != m_AdapterList.end())
					{
						ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
						_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip delete dns \"Local Area Connection\" "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsDnsAddr[wServer]);
		
						if (ExecuteNetsh(tsCmdLine) == FALSE)
						{
							//SendString(E_MSG_LEVEL_ERROR, _T("Could not delete %s DNS server IP address"),
							//		wServer ? _T("secondary") : _T("primary"));
							return S_FALSE;
						}
						ulPrevDnsAddr = 0;
						_stprintf_s(pInfo->m_tsDnsAddr[wServer], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));

						if (wServer == 0)
						{
							// if the primary was just deleted, the secondary must also be deleted now
							// because the OS will automagically make the secondary address the primary
							if (ConvertIPAddressStringToLong(pInfo->m_tsDnsAddr[wServer + 1]) != 0)
							{
								_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip delete dns \"Local Area Connection\" "));
								_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsDnsAddr[wServer + 1]);
		
								if (ExecuteNetsh(tsCmdLine) == FALSE)
								{
									//SendString(E_MSG_LEVEL_ERROR, _T("Could not delete %s DNS server IP address"),
									//		wServer + 1 ? _T("secondary") : _T("primary"));
									return FALSE;
								}
								_stprintf_s(pInfo->m_tsDnsAddr[wServer + 1], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
							}
						}
						break;
					}
				}

				if (ulPrevDnsAddr == 0 && ulCurrDnsAddr != 0)
				{
					if (wServer == 0)
					{
						_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip set dns \"Local Area Connection\" static "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsNewDnsAddr[wServer]);
						_tcscat_s(tsCmdLine, MAX_PATH, _T(" primary"));	// the primary dns has a index of '1'
					}
					else
					{
						_stprintf_s(tsCmdLine, _MAX_PATH, _T("netsh interface ip add dns \"Local Area Connection\" "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsNewDnsAddr[wServer]);
						_tcscat_s(tsCmdLine, MAX_PATH, _T(" index=2"));	// the secondary dns has a index of '1'
					}

					if (ExecuteNetsh(tsCmdLine) == FALSE)
					{
						//SendString(E_MSG_LEVEL_ERROR, _T("Could not set %s DNS server IP address"),
						//			wServer ? _T("secondary") : _T("primary"));
						return S_FALSE;
					}
					_stprintf_s(pInfo->m_tsDnsAddr[wServer], MAX_ADAPTER_DESCRIPTION_LENGTH, pInfo->m_tsNewDnsAddr[wServer]);
				}
			}
		}
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_OK;
}

//*****************************************************************************
//
// Method:      HRESULT SetWINSAddress(ETHERNETADAPTERINFO *)
//
// Parameters:  [in] pInfo - ;
//
// Returns:		HRESULT - S_OK: successfully set the WINS IP addresses;
//                        S_FALSE: failed to set the WINS IP addresses;
//
// Scope:       protected
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
HRESULT CWMI::SetWINSAddress(ETHERNETADAPTERINFO *pInfo)
{
	unsigned long ulPrevWinsAddr, ulCurrWinsAddr;

	try
	{
		for (WORD wServer = 0; wServer < 2; wServer++)	// wServer = 0: 'primary'
														//           1: 'secondary'
		{
			// get the IP address of the local adapter
			EthernetAdapterList::iterator iter;
			iter = m_AdapterList.begin();
			while (iter != m_AdapterList.end())
			{
				ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
				ulPrevWinsAddr = ConvertIPAddressStringToLong(pInfo->m_tsWinsAddr[wServer]);
				break;
			}

			// create the command string
			_TCHAR tsCmdLine[MAX_PATH];
			ulCurrWinsAddr = ConvertIPAddressStringToLong(pInfo->m_tsNewWinsAddr[wServer]);
			if (ulPrevWinsAddr != ulCurrWinsAddr)
			{
				if (ulPrevWinsAddr != 0)
				{
					// delete WINS
					EthernetAdapterList::iterator iter;
					iter = m_AdapterList.begin();
					while (iter != m_AdapterList.end())
					{
						ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
						_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip delete wins \"Local Area Connection\" "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsWinsAddr[wServer]);

						if (ExecuteNetsh(tsCmdLine) == FALSE)
						{
							//SendString(E_MSG_LEVEL_ERROR, _T("Could not delete %s WINS server IP address"),
							//			wServer ? _T("secondary") : _T("primary"));
							return S_FALSE;
						}
						ulPrevWinsAddr = 0;
						_stprintf_s(pInfo->m_tsWinsAddr[wServer], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));

						if (wServer == 0)
						{
							// if the primary was just deleted, the secondary must also be deleted now
							// because the OS will automagically make the secondary address the primary
							if (ConvertIPAddressStringToLong(pInfo->m_tsWinsAddr[wServer + 1]) != 0)
							{
								_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip delete wins \"Local Area Connection\" "));
								_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsWinsAddr[wServer + 1]);

								if (ExecuteNetsh(tsCmdLine) == FALSE)
								{
									//SendString(E_MSG_LEVEL_ERROR, _T("Could not delete %s WINS server IP address"),
									//			wServer + 1 ? _T("secondary") : _T("primary"));
									return S_FALSE;
								}
								ulPrevWinsAddr = 0;
								_stprintf_s(pInfo->m_tsWinsAddr[wServer + 1], MAX_ADAPTER_DESCRIPTION_LENGTH, _T("0.0.0.0"));
							}
						}
						break;
					}
				}

				if (ulPrevWinsAddr == 0 && ulCurrWinsAddr != 0)
				{
					if (wServer == 0)
					{
						_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip set wins \"Local Area Connection\" static "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsNewWinsAddr[wServer]);
						_tcscat_s(tsCmdLine, MAX_PATH, _T(" primary"));
					}
					else
					{
						_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip add wins \"Local Area Connection\" "));
						_tcscat_s(tsCmdLine, MAX_PATH, pInfo->m_tsNewWinsAddr[wServer]);
						_tcscat_s(tsCmdLine, MAX_PATH, _T(" index=2"));
					}

					if (ExecuteNetsh(tsCmdLine) == FALSE)
					{
						//SendString(E_MSG_LEVEL_ERROR, _T("Could not set %s WINS server IP address"),
						//			wServer ? _T("secondary") : _T("primary"));
						return S_FALSE;
					}
					_stprintf_s(pInfo->m_tsWinsAddr[wServer], MAX_ADAPTER_DESCRIPTION_LENGTH, pInfo->m_tsNewWinsAddr[wServer]);
				}
			}
		}
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_OK;
}

//*****************************************************************************
//
// Method:      void ConvertIPAddressStringToLong(const _TCHAR*)
//
// Parameters:  [in] _TCHAR * - ;
//
// Returns:		unsigned long
//
// Scope:       protected
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
unsigned long CWMI::ConvertIPAddressStringToLong(const _TCHAR *kptsIpText)
{
	unsigned long ulIpAddr = 0;
	int iDigit1 = 0, iDigit2 = 0, iDigit3 = 0, iDigit4 = 0;
	
	if (_stscanf_s(kptsIpText, _T("%d.%d.%d.%d"), &iDigit1, &iDigit2, &iDigit3, &iDigit4) != 4)
		ulIpAddr = 0;
	else
	{
		ulIpAddr = (iDigit1 << 24) | (iDigit2 << 16)  | (iDigit3 << 8) | (iDigit4);
	}

	return ulIpAddr;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterDHCPStatus(DWORD *)
//
// Parameters:  [in] _TCHAR * - ;
//
// Returns:		HRESULT - ;
//
// Scope:       protected
//
// Description: 
//
// Assumptions: 
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterDHCPStatus(DWORD * dwDHCP)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			// there is an adapter in the list
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
			*dwDHCP = pInfo->m_dwDHCP;
			return S_OK;
//			iter++;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterDHCPStatus()
//
// Parameters:  [in] DWORD - ;
//
// Returns:		HRESULT
//
// Scope:       protected
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/18/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterDHCPStatus(DWORD dwDHCP)
{
#if !MULTIPLE_NETWORK_ADAPTERS
	try
	{
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO*)*iter;
			pInfo->m_dwNewDHCP = dwDHCP;
			return S_OK;
	//		iter++;
		}
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

#endif	// MULTIPLE_NETWORK_ADAPTERS

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPAddress()
//
// Parameters:  [in] _TCHAR * - ;
//
// Returns:		HRESULT - ;
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPAddress(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsIpAddr, TSIZEOF(pInfo->m_tsIpAddr));
			return S_OK;
//			iter++;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPAddress()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPAddress(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewIpAddr, tsAddr, TSIZEOF(pInfo->m_tsNewIpAddr));

			m_bSetIPAddress = true;
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPMask()
//
// Parameters:  [in] _TCHAR * - ;
//
// Returns:		HRESULT - ;
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPMask(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsIpMask, TSIZEOF(pInfo->m_tsIpMask));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPMask()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPMask(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewIpMask, tsAddr, TSIZEOF(pInfo->m_tsNewIpMask));

			m_bSetIPMask = true;
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPPDNS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPPDNS(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsDnsAddr[0], TSIZEOF(pInfo->m_tsDnsAddr[0]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPPDNS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPPDNS(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewDnsAddr[0], tsAddr, TSIZEOF(pInfo->m_tsNewDnsAddr[0]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPPWINS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPPWINS(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsWinsAddr[0], TSIZEOF(pInfo->m_tsWinsAddr[0]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPPWINS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPPWINS(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewWinsAddr[0], tsAddr, TSIZEOF(pInfo->m_tsNewWinsAddr[0]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPSDNS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPSDNS(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsDnsAddr[1], TSIZEOF(pInfo->m_tsDnsAddr[1]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPSDNS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPSDNS(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewDnsAddr[1], tsAddr, TSIZEOF(pInfo->m_tsNewDnsAddr[1]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterIPSWINS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterIPSWINS(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsWinsAddr[1], TSIZEOF(pInfo->m_tsWinsAddr[1]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterIPSWINS()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterIPSWINS(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewWinsAddr[1], tsAddr, TSIZEOF(pInfo->m_tsNewWinsAddr[1]));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetEnetAdapterDefRouter()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetEnetAdapterDefRouter(_TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(tsAddr, pInfo->m_tsGatewayAddr, TSIZEOF(pInfo->m_tsGatewayAddr));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      SetEnetAdapterDefRouter()
//
// Parameters:  [in] const _TCHAR * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::SetEnetAdapterDefRouter(const _TCHAR * tsAddr)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = (ETHERNETADAPTERINFO *)*iter;
			_tcsncpy(pInfo->m_tsNewGatewayAddr, tsAddr, TSIZEOF(pInfo->m_tsNewGatewayAddr));
			m_bSetDefRouter = true;
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      GetNewEnetAdapterSettings()
//
// Parameters:  [in] ETHERNETADAPTERINFO * - ;
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/20/10
//
//*****************************************************************************
HRESULT CWMI::GetNewEnetAdapterSettings(ETHERNETADAPTERINFO * pInfo)
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			memcpy(pInfo, *iter, sizeof(ETHERNETADAPTERINFO));
			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}

//*****************************************************************************
//
// Method:      RebootSystem()
//
// Parameters:  void
//
// Returns:		HRESULT
//
// Scope:       
//
// Description: 
//
// Assumptions: none
//
// Author:      Andrew Cham - 5/21/10
//
//*****************************************************************************
HRESULT CWMI::RebootSystem()
{
	try
	{
#if !MULTIPLE_NETWORK_ADAPTERS
		if (PrepareAdapterInfoList() == S_FALSE)
			return S_FALSE;

		EthernetAdapterList::iterator iter;
		iter = m_AdapterList.begin();
		while (iter != m_AdapterList.end())
		{
			ETHERNETADAPTERINFO * pInfo = *iter;
			if (pInfo->m_dwNewDHCP != 0)
			{
				// user netsh to enable DHCP
				_TCHAR tsCmdLine[MAX_PATH] = {0};
				_stprintf_s(tsCmdLine, MAX_PATH, _T("netsh interface ip set address \"Local Area Connection\" source=dhcp"));

				// enable DHCP on the panel
				if (ExecuteNetsh(tsCmdLine) == FALSE)
					return S_FALSE;
			}
			else
			{

				_TCHAR tsDbgMsg[MAX_PATH] = {0};

				// configure panel with the new IP configuration.
				//SetWin32_NetworkAdapterConfigurationViaWmi(pInfo);
				if (SetStaticIPAddress(pInfo) == S_OK)
				{
					if (SetDNSAddress(pInfo) == S_OK)
					{
						if (SetWINSAddress(pInfo) == S_FALSE)
							return S_FALSE;
					}
					else
						return S_FALSE;
				}
				else
					return S_FALSE;
			}

			return S_OK;
		}
#endif // MULTIPLE_NETWORK_ADAPTERS
	}
	catch(...)
	{
//		MessageBox(NULL, _T(__FUNCTION__), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
	}

	return S_FALSE;
}
#endif // UNDER_CE
