////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Registry.h
///
/// Header file for general registry helper functions.
///
/// @author      William Levine
/// @date        01/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_REGISTRY_H_)
#define _REGISTRY_H_

CmPCConfig_DeclSpec HRESULT GetRegistryString(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, _TCHAR *pszResult, int buffLen);
CmPCConfig_DeclSpec HRESULT SetRegistryString(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD dwRegType, const _TCHAR *pszValue, bool bConvertToWcs = FALSE);

CmPCConfig_DeclSpec HRESULT GetRegistryDWORD(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD *pdwResult);
CmPCConfig_DeclSpec HRESULT SetRegistryDWORD(HKEY hRootHandle, const _TCHAR *pszSubKey, const _TCHAR *pszValueName, DWORD dwValue);

CmPCConfig_DeclSpec HRESULT UnLoadOfflineHive(TCHAR *pcszFilePath, HKEY hNewRootKey, bool bSave);
CmPCConfig_DeclSpec HRESULT LoadOfflineHive(TCHAR *pcszFilePath, HKEY *phNewRootKey);

#endif