////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        Network.h
///
/// Functions to help with network connections and such.
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _NETWORK_H_
#define _NETWORK_H_

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Clears all active network connections.
/// Enumerates the active connections using WNet??
///
/// @param       fForce If True will force connections closed even if they have open files.
///                     You can first call the function with false and then if we return
///                     HRESULT_FROM_WIN32(ERROR_OPEN_FILES) you can prompt the user and then
///                     call this method with Force set to true
/// @return We translate the Win32 errors into HRESULTS.
///         You can pass the return value into CfgNetworkGetErrorText to create
///			a user presentable value. 
///			ERROR_BAD_PROFILE The user profile is in an incorrect format. 
///         ERROR_CANNOT_OPEN_PROFILE The system is unable to open the user profile to process persistent connections. 
///         ERROR_DEVICE_IN_USE The device is in use by an active process and cannot be disconnected. 
///         ERROR_EXTENDED_ERROR A network-specific error occurred. To obtain a description of the error, call the WNetGetLastError function. 
///         ERROR_NOT_CONNECTED The name specified by the lpName parameter is not a redirected device, or the system is not currently connected to the device specified by the parameter. 
///         ERROR_OPEN_FILES There are open files, and the fForce parameter is FALSE. 
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkClearConnections(bool fForce);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Retrieve an approprate error message given an HRESULT.
/// This takes the return value and will either format a system message or call WNetGetErrorText to
/// determine an extended error.
///
/// @param       hResult     The HRESULT value to translate
/// @param       pErrorText  A pointer to a TCHAR *.  This parameter must not be NULL.
/// @param       buffLen     The length of pErrorText in TCHARs
///                          NETWORK_ERROR_BUFFER_SIZE is the suggested length
/// @return      S_OK or E_FAIL
///
/// @author      John Pavlik
/// @date        05/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#define CFG_NETWORK_ERROR_BUFFER_SIZE 1024
CmPCConfig_DeclSpec HRESULT CfgNetworkGetErrorText(HRESULT hResult, _TCHAR *pErrorText, int buffLen);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Set the account name of the administrator account.
/// We make the assumption that there is only a single administrator group account on the computer
///
/// @param       pUserName   
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetAdminName(const _TCHAR *pUserName);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Retrieve the admin account name.
/// We make the assumption that there is only a single administrator group account on the computer
///
/// @param       pUserName   Must not be NULL, Buffer should be CFG_USER_NAME_LENGTH 
///                          (User Name Length) TCHARs in length
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#define CFG_USER_NAME_LENGTH (21)
CmPCConfig_DeclSpec HRESULT CfgNetworkGetAdminName(_TCHAR *pUserName);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Sets the users password.
///
/// @param       pNewPassword  pointer to the new password
/// @param       pOldPassword  Currently not really used.
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetUserPassword(const _TCHAR *pUserName, 
					const _TCHAR *pNewPassword, const _TCHAR *pOldPassword);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Used to retrieve the list of shared folders.
/// Allows the user to iterate through shares but only retrieves folder based shares (no printers)
/// and doesn't return the system shares ($C, $ADMIN, etc.)
///
/// @param       pShareName  Must not be NULL, Buffer should NNLEN in length.
/// @param       pPath       Must not be NULL, Buffer should MAX_PATH in length.
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkEnumShareOpen(DWORD *pdwCookie);
CmPCConfig_DeclSpec HRESULT CfgNetworkGetUserShare(_TCHAR *pShareName,
					_TCHAR *pPath, DWORD dwCookie);
CmPCConfig_DeclSpec HRESULT CfgNetworkEnumShareClose(DWORD dwCookie);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Adds a new shared folder.
///
/// @param       _TCHAR *pShareName
/// @param       _TCHAR *pPath
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkAddUserShare(_TCHAR *pShareName,
					_TCHAR *pPath, int nMaxUsers = 10);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Removes the given shared folder.
///
/// @param       _TCHAR *pShareName
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkDelUserShare(_TCHAR *pShareName);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Retrieve the user rights for the given share
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkGetShareRights(_TCHAR *pShare,
													BOOL *pfReadOnly);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Set the user rights for the given share
///
/// @author      John Pavlik
/// @date        05/24/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetShareRights(_TCHAR *pShare,
													BOOL fReadOnly);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Open and close the connection info object
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkConnectionsOpen(HANDLE *hConnectionHandle);
CmPCConfig_DeclSpec HRESULT CfgNetworkConnectionsClose(HANDLE hConnectionHandle);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Enumerate existing network connections
///
///
/// @param  dwCookie    - Set to NULL before the first call and then 
///                    
/// @param  pRemoteName - Pointer to buffer of at least MAX_PATH
/// @param  pLocalName - Pointer to buffer of at least CFG_NETWORK_NAME_LEN
/// @param  pUserName - Pointer to buffer of at least CFG_NETWORK_NAME_LEN
/// @param  pUserPwd - Pointer to buffer of at least CFG_NETWORK_NAME_LEN
/// @param  pfPersist - Pointer to BOOL value

/// @author      John Pavlik
/// @date        09/18/04
/// @note  Foolishly read the docs and said "oh, usernames can only be 20 characters"
///        Of course if you need to put domain\username that's going to be bigger.
///        Quit messing around and make it 128 - jp 10/19/2004
////////////////////////////////////////////////////////////////////////////////////////////////////
#define CFG_NETWORK_NAME_LEN 128
CmPCConfig_DeclSpec HRESULT CfgNetworkGetConnectionInfo(HANDLE hConnectionHandle, 
			    DWORD dwIndex,
				LPTSTR pRemoteName, LPTSTR pLocalName,
				LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist);
/// In this version pass in the remote and local name and we'll fill out the rest
CmPCConfig_DeclSpec HRESULT CfgNetworkGetConnectionInfo(HANDLE hConnectionHandle, 
				LPCTSTR pRemoteName, LPCTSTR pLocalName,
				LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Set information about an existing network connection
///
/// @param  pRemoteName - Pointer to buffer of at most MAX_PATH
/// @param  pLocalName - Pointer to buffer of at most CFG_NETWORK_NAME_LEN
/// @param  pUserName - Pointer to buffer of at most CFG_NETWORK_NAME_LEN
/// @param  pUserPwd - Pointer to buffer of at most CFG_NETWORK_NAME_LEN
/// @param  pfPersist - Pointer to BOOL value whether this should be persisted

/// @author      John Pavlik
/// @date        09/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkSetConnectionInfo(
				HANDLE hConnectionHandle, 
				LPCTSTR pRemoteName, LPCTSTR pLocalName,
				LPCTSTR pUserName, LPCTSTR pUserPwd, BOOL fPersist);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Close an existing network connection - If Persistent it will also be removed from our list
///
/// @param  pRemoteName - Pointer to buffer of at most MAX_PATH
/// @param  fForce If True will force connections closed even if they have open files.
///                     You can first call the function with false and then if we return
///                     HRESULT_FROM_WIN32(ERROR_OPEN_FILES) you can prompt the user and then
///                     call this method with Force set to true
/// @return We translate the Win32 errors into HRESULTS.
///         You can pass the return value into CfgNetworkGetErrorText to create
///			a user presentable value. 
///			ERROR_BAD_PROFILE The user profile is in an incorrect format. 
///         ERROR_CANNOT_OPEN_PROFILE The system is unable to open the user profile to process persistent connections. 
///         ERROR_DEVICE_IN_USE The device is in use by an active process and cannot be disconnected. 
///         ERROR_EXTENDED_ERROR A network-specific error occurred. To obtain a description of the error, call the WNetGetLastError function. 
///         ERROR_NOT_CONNECTED The name specified by the lpName parameter is not a redirected device, or the system is not currently connected to the device specified by the parameter. 
///         ERROR_OPEN_FILES There are open files, and the fForce parameter is FALSE. 

/// @author      John Pavlik
/// @date        09/18/04
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkRemoveConnection(
				HANDLE hConnectionHandle, 
				LPCTSTR pRemoteName, LPCTSTR pLocalName, BOOL fForce);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Reconnect to previously remembered connections
/// Returns S_FALSE when there are no more connections to be restored.
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkRestoreConnection(HANDLE hConnectionHandle, DWORD dwIndex);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Checks to see if the filename passed in on a valid drive.
/// 
////////////////////////////////////////////////////////////////////////////////////////////////////
CmPCConfig_DeclSpec HRESULT CfgNetworkValidateDrive(LPCTSTR pFileName);

#endif