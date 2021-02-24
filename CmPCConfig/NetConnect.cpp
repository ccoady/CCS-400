#include "stdafx.h"
#include "netconnect.h"
#include "encrypt.h"
#include "system.h"


#define gszConnections _T("SOFTWARE\\Crestron Electronics Inc.\\UPX\\NetData")
#define gszDataFileName _T("\\~upxnd.dat")


tstring MakeMapKey(LPCTSTR pRemoteName, LPCTSTR pLocalName)
{
	tstring strKey = pRemoteName;
	if (pLocalName)
		strKey += pLocalName;
	_tcsupr(const_cast<TCHAR *>(strKey.c_str()));

	return strKey;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CNetConnect::CNetConnect()
{
	ReadRegistryData();
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CNetConnect::~CNetConnect()
{
	StoreRegistryData();
	CleanConnectionMap();
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CNetConnect::StoreRegistryData()
{
	TCHAR szDataFileName[MAX_PATH];
	CfgGetSettingsPath(szDataFileName);
	lstrcat(szDataFileName,gszDataFileName);
	
	HANDLE hFile = ::CreateFile(szDataFileName,GENERIC_WRITE,0,NULL,CREATE_ALWAYS,0,0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesWritten = 0;
		DWORD dwDataSize = sizeof(NetConnection);
		DWORD dwNumConnections = 0;
		CONNECTION_MAP::iterator itMap;
		for (itMap = m_Connections.begin();
			itMap != m_Connections.end();
			itMap++)
		{
			NetConnection *pConnection = (*itMap).second;
			if (pConnection->bPersist)
				dwNumConnections++;
		}


		::WriteFile(hFile,&dwDataSize,sizeof(DWORD),&dwBytesWritten,NULL);
		::WriteFile(hFile,&dwNumConnections,sizeof(DWORD),&dwBytesWritten,NULL);

		for (itMap = m_Connections.begin();
			itMap != m_Connections.end();
			itMap++)
		{
			NetConnection *pConnection = (*itMap).second;
			if (pConnection->bPersist)
			{
				NetConnection data;
				memcpy(&data,pConnection,sizeof(data));
				Encrypt((TCHAR *)&data,sizeof(data));
				::WriteFile(hFile,&data,sizeof(data),&dwBytesWritten,NULL);
			}
		}

		::CloseHandle(hFile);
	}

}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
void CNetConnect::ReadRegistryData()
{
	TCHAR szDataFileName[MAX_PATH];
	CfgGetSettingsPath(szDataFileName);
	lstrcat(szDataFileName,gszDataFileName);
	
	HANDLE hFile = ::CreateFile(szDataFileName,GENERIC_READ,0,NULL,OPEN_EXISTING,0,0);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD dwBytesRead;
		DWORD dwDataSize;
		DWORD dwNumConnections;
		if (::ReadFile(hFile,&dwDataSize,sizeof(DWORD),&dwBytesRead,NULL))
		{
			//ATLASSERT(dwBytesRead == sizeof(DWORD));
			//ATLASSERT(dwDataSize == sizeof(NetConnection));
			if ((dwBytesRead == sizeof(DWORD)) && (dwDataSize == sizeof(NetConnection)))
			{
				if (::ReadFile(hFile,&dwNumConnections,sizeof(DWORD),&dwBytesRead,NULL))
				{
					//ATLASSERT(dwBytesRead == sizeof(DWORD));
					if(dwBytesRead == sizeof(DWORD))
					{
						for (int i = 0; i < (int) dwNumConnections; i++)
						{
							NetConnection *pConnection = new NetConnection;
							memset(pConnection,0,sizeof(NetConnection));
							if (::ReadFile(hFile,pConnection,sizeof(NetConnection),&dwBytesRead,NULL))
							{
								/// decrypt the data
								Encrypt((TCHAR *)pConnection,sizeof(NetConnection));
								pConnection->bPersist = true;
								pConnection->bConnected = FALSE;
								InsertConnection(pConnection);
							}
							else
							{
								delete pConnection, pConnection = NULL;
								ReadDataFromRegistry();
							}
						}
					}
					else
					{
						ReadDataFromRegistry();
					}
				}
				else
				{
					ReadDataFromRegistry();
				}
			}
			else
			{
				ReadDataFromRegistry();
			}
		}
		::CloseHandle(hFile);
	}
	else
	{
		ReadDataFromRegistry();
	}
	RegDeleteKey(HKEY_LOCAL_MACHINE,gszConnections);
}

////////////////////////////////////////////////////////////////////////////////
//	Module:		ReadDataFromRegistry(void)
//					Executed when ~upxnd.dat file is corrupted
//					Reads "NetConnection" data from registry and
//					sets NetConnection object values
//	Author:		Michael Rinaldi, 09/30/2009
////////////////////////////////////////////////////////////////////////////////
void CNetConnect::ReadDataFromRegistry(void)
{
		UINT id = 1;
		CRegKey reg;
		if (NO_ERROR == reg.Open(HKEY_LOCAL_MACHINE,gszConnections))
		{
			do
			{
				TCHAR szValueName[20];
				wsprintf(szValueName,_T("%d"),id);

				NetConnection *pConnection = new NetConnection;
				memset(pConnection,0,sizeof(NetConnection));

				OldNetConnection olddata;
				memset(&olddata,0,sizeof(OldNetConnection));

				DWORD dwType = REG_BINARY;
				DWORD cbData = sizeof(NetConnection);
				if (NO_ERROR == RegQueryValueEx((HKEY)reg,szValueName,NULL,&dwType,
								NULL,&cbData))
				{
					if (cbData > sizeof(OldNetConnection))
					{
						if (NO_ERROR == RegQueryValueEx((HKEY)reg,szValueName,NULL,&dwType,
										(BYTE *)pConnection,&cbData))
						{
							/// decrypt the data
							Encrypt((TCHAR *)pConnection,sizeof(NetConnection));
							memset(pConnection->szUserName,0,sizeof(pConnection->szUserName));
							memset(pConnection->szUserPwd,0,sizeof(pConnection->szUserPwd));

							pConnection->bPersist = TRUE;
							pConnection->bConnected = FALSE;
							InsertConnection(pConnection);
							id++;
						}
					}
					else
					{
						cbData = sizeof(NetConnection);
						if (NO_ERROR == RegQueryValueEx((HKEY)reg,szValueName,NULL,&dwType,
										(BYTE *)pConnection,&cbData))
						{
							/// decrypt the data
							Encrypt((TCHAR *)&olddata,sizeof(OldNetConnection));
							_tcscpy(pConnection->szUserName,olddata.szUserName);
							_tcscpy(pConnection->szUserPwd,olddata.szUserPwd);
							pConnection->bPersist = true;
							pConnection->bConnected = FALSE;
							InsertConnection(pConnection);
						}
					}
				}
				else 
				{
					delete pConnection, pConnection = NULL;
					break;
				}

				id++;

			} while (true);
		}
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::RestoreConnection(DWORD dwIndex)
{
	int i;
	DWORD dwResult = NO_ERROR;
	CONNECTION_MAP::iterator itMap;

	ATLASSERT(dwIndex < m_Connections.size());
	for (itMap = m_Connections.begin(), i = 0; 
		 (i < (int) dwIndex) && (itMap != m_Connections.end()); itMap++, i++)
		;

	if (itMap != m_Connections.end())
	{
		NetConnection *pConnection = (*itMap).second;
		if (!pConnection->bConnected)
		{
			DWORD dwConnectionResult = Connect(pConnection);
			if (dwConnectionResult != NO_ERROR)
			{
				dwResult = dwConnectionResult;
			}
			else
				pConnection->bConnected = TRUE;
		}
	}

	return dwResult;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::CleanConnectionMap()
{
	CONNECTION_MAP::iterator itMap;
	for (itMap = m_Connections.begin();
		itMap != m_Connections.end();
		itMap++)
	{
		NetConnection *pConnection = (*itMap).second;
		delete pConnection;
	}

	m_Connections.erase(m_Connections.begin(), m_Connections.end());
	return NO_ERROR;
}


////////////////////////////////////////////////////////////////////
/// Woefully inefficient but avoids dealing with memory allocation
/// issues.  Also, how big can this list possibly be?  (famous last words)
////////////////////////////////////////////////////////////////////
BOOL CNetConnect::GetConnectionInfo(DWORD dwIndex, LPTSTR pRemoteName, LPTSTR pLocalName,
	LPTSTR pUserName, LPTSTR pUserPwd, BOOL* pfPersist)
{
	if (dwIndex >= m_Connections.size())
		return FALSE;

	CONNECTION_MAP::iterator itMap = m_Connections.begin();
	for (int i = 0; i < (int)dwIndex; i++)
	{
		ATLASSERT(itMap != m_Connections.end());
		itMap++;
	}

	if (itMap != m_Connections.end())
	{
		NetConnection *pConnection = (*itMap).second;
		_tcscpy(pRemoteName,pConnection->szRemoteName);
		_tcscpy(pLocalName,pConnection->szLocalName);
		_tcscpy(pUserName,pConnection->szUserName);
		_tcscpy(pUserPwd,pConnection->szUserPwd);
		*pfPersist = pConnection->bPersist;

		return TRUE;
	}

	return FALSE;
}


////////////////////////////////////////////////////////////////////
/// In this version, pass in the remote name and we'll fill out the 
/// rest - returns FALSE if not in the map
////////////////////////////////////////////////////////////////////
BOOL CNetConnect::GetConnectionInfo(LPCTSTR pRemoteName, LPCTSTR pLocalName,
	LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist)
{
	NetConnection *pConnection = FindConnection(pRemoteName, pLocalName);
	if (!pConnection)
		return FALSE;

	_tcscpy(pUserName,pConnection->szUserName);
	_tcscpy(pUserPwd,pConnection->szUserPwd);
	*pfPersist = pConnection->bPersist;

	return TRUE;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::RefreshConnections()
{
	StoreRegistryData();

	CONNECTION_MAP::iterator itMap;
	for (itMap = m_Connections.begin();
		itMap != m_Connections.end();
		itMap++)
	{
		NetConnection *pConnection = (*itMap).second;
		delete pConnection;
	}

	m_Connections.erase(m_Connections.begin(), m_Connections.end());
	ReadRegistryData();

	FindConnections();

	return NO_ERROR;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::FindConnections(LPNETRESOURCE lpnr)
{
	DWORD dwResult, dwResultEnum;
	HANDLE hEnum;
	DWORD cbBuffer = 16384;      // 16K is a good size
	DWORD cEntries = -1;         // enumerate all possible entries
	LPNETRESOURCE lpnrLocal;     // pointer to enumerated structures
	DWORD i;
	//
	// Call the WNetOpenEnum function to begin the enumeration.
	//
	dwResult = WNetOpenEnum(RESOURCE_CONNECTED, // all network resources
		RESOURCETYPE_ANY,   // all resources
		0,        // enumerate all resources
		lpnr,     // NULL first time the function is called
		&hEnum);  // handle to the resource
	
	if (dwResult != NO_ERROR)
		return dwResult;

	//
	// Call the GlobalAlloc function to allocate resources.
	//
	lpnrLocal = (LPNETRESOURCE) GlobalAlloc(GPTR, cbBuffer);
	
	do
	{  
		//
		// Initialize the buffer.
		//
		ZeroMemory(lpnrLocal, cbBuffer);
		//
		// Call the WNetEnumResource function to continue
		//  the enumeration.
		//
		dwResultEnum = WNetEnumResource(hEnum,      // resource handle
			&cEntries,  // defined locally as -1
			lpnrLocal,  // LPNETRESOURCE
			&cbBuffer); // buffer size
		//
		// If the call succeeds, loop through the structures.
		//
		if (dwResultEnum == NO_ERROR)
		{
			for(i = 0; i < cEntries; i++)
			{

				// If the NETRESOURCE structure represents a container resource, 
				//  call the EnumerateFunc function recursively.
				
				if(RESOURCEUSAGE_CONTAINER == (lpnrLocal[i].dwUsage
					& RESOURCEUSAGE_CONTAINER))
				{
					dwResult = FindConnections(&lpnrLocal[i]);
					if (!dwResult != NO_ERROR)
						break;
				}


				dwResult = AddConnection(&lpnrLocal[i]);
				if (dwResult != NO_ERROR)
				{
					break;
				}
			}
		}
		// Process errors.
		//
		else if (dwResultEnum != ERROR_NO_MORE_ITEMS)
		{
			dwResult = dwResultEnum;
			break;
		}
	} while ((dwResult == NO_ERROR) && (dwResultEnum != ERROR_NO_MORE_ITEMS));


	//
	// Call the GlobalFree function to free the memory.
	//
	GlobalFree((HGLOBAL)lpnrLocal);

	//
	// Call WNetCloseEnum to end the enumeration.
	//
	WNetCloseEnum(hEnum);
	
	return dwResult;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::RemoveConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName, BOOL fForce, 
											BOOL fPersistentAlso /*= FALSE*/)
{

	CONNECTION_MAP::iterator itMap;
	tstring strKey = MakeMapKey(pRemoteName,pLocalName);
	
	itMap = m_Connections.find(strKey);
	if (itMap != m_Connections.end())
	{
		NetConnection *pConnection = (*itMap).second;
		if (!pConnection->bPersist || fPersistentAlso)
		{
			DWORD dwResult = Disconnect(pConnection, fForce);

			if (dwResult != ERROR_OPEN_FILES)
			{
				m_Connections.erase(itMap);
				delete pConnection;
			}
			return dwResult;
		}

		return NO_ERROR;

	}

	return NO_ERROR;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::AddConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName,
								 LPCTSTR pUserName, LPCTSTR pUserPwd, BOOL fPersist)
{
	DWORD dwResult = NO_ERROR;
	NetConnection *pOldConnection = FindConnection(pRemoteName, pLocalName);

	NetConnection *pNewConnection = new NetConnection;
	memset(pNewConnection,0,sizeof(NetConnection));

	// either way, store the new data
	_tcscpy(pNewConnection->szRemoteName, pRemoteName);

	if (pLocalName)
		_tcscpy(pNewConnection->szLocalName, pLocalName);
	else 
		_tcscpy(pNewConnection->szLocalName, _T(""));
	if (pUserName)
		_tcscpy(pNewConnection->szUserName, pUserName);
	else 
		_tcscpy(pNewConnection->szUserName, _T(""));
	if (pUserPwd)
		_tcscpy(pNewConnection->szUserPwd, pUserPwd);
	else 
		_tcscpy(pNewConnection->szUserPwd, _T(""));
	pNewConnection->bPersist = fPersist;

	if (pOldConnection)
		Disconnect(pOldConnection,TRUE);

	dwResult = Connect(pNewConnection);

	if (dwResult == NO_ERROR)
	{
		if (pOldConnection)
		{
			/// copy data into old structure location
			memcpy(pOldConnection,pNewConnection,sizeof(NetConnection));
			delete pNewConnection; 
		}
		else
			InsertConnection(pNewConnection);
	}
	else
		delete pNewConnection;

	return dwResult;
}

// <winerror.h>
DWORD CNetConnect::Connect(NetConnection *pConnection)
{
	NETRESOURCE nr = {0};
	nr.dwType = RESOURCETYPE_DISK;
	nr.lpRemoteName = pConnection->szRemoteName[0] ? pConnection->szRemoteName : NULL;
	nr.lpLocalName = pConnection->szLocalName[0] ? pConnection->szLocalName : NULL;
	DWORD dwFlags = pConnection->szUserPwd[0] ? 0 : CONNECT_INTERACTIVE; // if no password allow prompt
	HWND hWndOwner = GetActiveWindow();

	int retryCount = 0;
	DWORD dwResult = WNetAddConnection3(hWndOwner,&nr,
			pConnection->szUserPwd[0] ? pConnection->szUserPwd : NULL,
			pConnection->szUserName[0] ? pConnection->szUserName : NULL, dwFlags);
	if ((dwResult == ERROR_LOGON_FAILURE) || (dwResult == ERROR_NO_SUCH_LOGON_SESSION))
	{
		TCHAR szTempUserName[MAX_PATH];
		if (pConnection->szUserName[0])
		{
			if (!_tcschr(pConnection->szUserName,'\\') && !_tcschr(pConnection->szUserName,'/'))
			{
				if (pConnection->szRemoteName[0] == '\\' && pConnection->szRemoteName[1] == '\\')
				{
					_tcscpy(szTempUserName,&pConnection->szRemoteName[2]);
					TCHAR *p = _tcschr(szTempUserName,'\\');
					if (!p)
					{
						p = _tcschr(szTempUserName,'/');
						if (!p)
							p = &szTempUserName[_tcslen(szTempUserName)];
					}

					*p++ = '\\';
					_tcscpy(p,pConnection->szUserName);
				}
				else
				{
					TCHAR *p = _tcsstr(pConnection->szRemoteName,_T(":\\\\"));
					if (!p)
						p = _tcsstr(pConnection->szRemoteName,_T("://"));

					if (p)
					{
						p += 3; // skip the colon and double slash
						_tcscpy(szTempUserName,p);

						p = _tcschr(szTempUserName,'\\');
						if (!p)
						{
							p = _tcschr(szTempUserName,'/');
							if (!p)
								p = &szTempUserName[_tcslen(szTempUserName)];
						}

						*p++ = '\\';
						_tcscpy(p,pConnection->szUserName);


					}


				}
			}

		}
		dwResult = WNetAddConnection3(hWndOwner,&nr,
					pConnection->szUserPwd[0] ? pConnection->szUserPwd : NULL,
					szTempUserName, dwFlags);


	}

	if (dwResult == NO_ERROR)
		pConnection->bConnected = TRUE;

	return dwResult;
}

DWORD CNetConnect::Disconnect(NetConnection *pConnection, BOOL fForce)
{
#ifdef _DEBUG
	if (0 == _tcsicmp(pConnection->szRemoteName,_T("\\\\j-pavlik\\upxdebug")))
		return NO_ERROR;
#endif

	DWORD dwResult = NO_ERROR;
	if (pConnection->szLocalName[0])
		dwResult = WNetCancelConnection(pConnection->szLocalName,fForce);
	else
		dwResult = WNetCancelConnection(pConnection->szRemoteName,fForce);

	if (dwResult == NO_ERROR)
		pConnection->bConnected = FALSE;

	return dwResult;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::AddConnection(LPNETRESOURCE lpnr)
{
	DWORD dwResult = NO_ERROR;
	BOOL fNew = false;
	NetConnection *pConnection = FindConnection(lpnr->lpRemoteName,lpnr->lpLocalName);
	if (!pConnection)
	{
		fNew = true;
		pConnection = new NetConnection;
		memset(pConnection,0,sizeof(NetConnection));
	}

	// either way, store the new data
	_tcscpy(pConnection->szRemoteName, lpnr->lpRemoteName);

	if (lpnr->lpLocalName)
		_tcscpy(pConnection->szLocalName, lpnr->lpLocalName);
	else 
		_tcscpy(pConnection->szLocalName, _T(""));

	pConnection->bConnected = TRUE;

	if (fNew)
		InsertConnection(pConnection);

	return dwResult;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
DWORD CNetConnect::InsertConnection(NetConnection *pConnection)
{
	ATLASSERT(!FindConnection(pConnection->szRemoteName,pConnection->szLocalName));

	tstring strKey = MakeMapKey(pConnection->szRemoteName,pConnection->szLocalName);
	m_Connections.insert( pair<tstring, NetConnection *>(strKey, pConnection));
	return NO_ERROR;
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
CNetConnect::NetConnection *CNetConnect::FindConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName)
{
	CONNECTION_MAP::iterator itMap;
	tstring strKey = MakeMapKey(pRemoteName,pLocalName);
	
	itMap = m_Connections.find(strKey);
	if (itMap == m_Connections.end())
	{
		return NULL;
	}
	return (*itMap).second;
}



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
UINT CNetConnect::GetDriveType(LPCTSTR pFileName)
{
	TCHAR szTempFileName[MAX_PATH];
	if (pFileName[1] == ':') // drive letter
	{
		TCHAR szPath[4] = { pFileName[0],':','\\','\0' };
		return ::GetDriveType(szPath);
	}

	// check for URL (webdav redirector)
	const TCHAR *p = _tcsstr(pFileName,_T(":\\\\"));
	if (!p)
		p = _tcsstr(pFileName,_T("://"));

	if (p)
	{
		p += 3; // skip the colon and double slash
		_tcscpy(szTempFileName,_T("\\\\"));
		_tcscat(szTempFileName,p);
		pFileName = szTempFileName;
	}


	if (pFileName[0] != '\\' || pFileName[1] != '\\')
		return DRIVE_UNKNOWN;

	CONNECTION_MAP::iterator itMap;
	for (itMap = m_Connections.begin();
		itMap != m_Connections.end();
		itMap++)
	{
		NetConnection *pConnection = (*itMap).second;
		if (pConnection->bConnected)
		{
			if (0 == _tcsnicmp(pConnection->szRemoteName,
						pFileName,_tcslen(pConnection->szRemoteName)))
				return DRIVE_REMOTE;
		}
	}

	return DRIVE_NO_ROOT_DIR;
}
