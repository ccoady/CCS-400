////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        System.h
///
/// Header file for System related configuration functions.
///
/// @author      William Levine
/// @date        05/6/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_SYSTEM_H_)
#define _SYSTEM_H_

#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgRegistryCommitChanges(const _TCHAR *pstOutputScriptPathname);
CmPCConfig_DeclSpec HRESULT CfgSetBootDUAScriptFile(const _TCHAR *pstWorkingPath, const _TCHAR *pstScriptFilename);
CmPCConfig_DeclSpec HRESULT CfgSetNextBootImage(bool bBootToBootImage = TRUE );

CmPCConfig_DeclSpec HRESULT CfgGetRuntimeImageSystemPath(_TCHAR *pstPath);
CmPCConfig_DeclSpec HRESULT CfgGetBootImageSystemPath(_TCHAR *pstPath);

CmPCConfig_DeclSpec HRESULT CfgGetDriveAccesability(_TCHAR tchDriveLetter);
CmPCConfig_DeclSpec HRESULT CfgRunTouchScreenCalibration(bool fConfig);
CmPCConfig_DeclSpec HRESULT CfgEnableTouchPktDebug(bool bEnable);

CmPCConfig_DeclSpec HRESULT CfgSetBootmonParams(LPCTSTR pMsg, int timeout);
CmPCConfig_DeclSpec HRESULT CfgDoSysInstall(bool fDelSettings);
#endif


CmPCConfig_DeclSpec HRESULT CfgGetSettingsPath(_TCHAR *pstPath);
CmPCConfig_DeclSpec HRESULT CfgGetPrivateStoragePath(_TCHAR *pstPath);
CmPCConfig_DeclSpec HRESULT CfgGetUserPath(_TCHAR *pstPath);
CmPCConfig_DeclSpec HRESULT CfgGetFirmwarePath(_TCHAR *pstPath);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Clears all files in the specified path.
///
/// @param       pstPath     path to clear
/// @param       fRecursive  if recursive is true than we also remove directories
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgClearFolder(_TCHAR *pstPath, bool fRecursive = true);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Clears our "special" folders.
/// For example, pass CSIDL_COOKIES to clear cookies
///
/// @param       specialFolderSee SHGetFolderPath for CSIDL options
/// @param       fRecursive  if true, recursively removes subfolders and their contents
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE
CmPCConfig_DeclSpec HRESULT CfgClearSpecialFolder(int specialFolder, bool fRecursive = true);
#endif


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Clear the temporary internet file cache.
/// This then deletes the related special folder as well
///
/// @author      John Pavlik
/// @date        06/30/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef UNDER_CE

#define CLEAR_CACHE_COOKIES   0x0001
#define CLEAR_CACHE_HISTORY	  0x0002
#define CLEAR_CACHE_TEMPFILES 0x0004
CmPCConfig_DeclSpec HRESULT CfgClearUrlCache(UINT cacheType);

#endif


#endif