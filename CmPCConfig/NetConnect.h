#ifndef _NETCONNECT_H_
#define _NETCONNECT_H_

#include <map>
#include <string>
#include "CommonDefs.h"

using std::map;
using std::make_pair;
using std::pair;




class CNetConnect
{
public:

	/////////////////////////////////////////////////
	/// CNetConnect::CNetConnect()
	/// Initialize our object by reading in our
	/// registry settings.  Normally you will want to
	/// call RefreshConnections after this.
	/////////////////////////////////////////////////
	CNetConnect();

	/////////////////////////////////////////////////
	/// CNetConnect::~CNetConnect()
	/// Store registry data and delete our map.
	/////////////////////////////////////////////////
	~CNetConnect();

	/////////////////////////////////////////////////
	/// void CNetConnect::StoreRegistryData()
	/// Flushes the persistent members of our map 
	/// into the registry
	/////////////////////////////////////////////////
	void StoreRegistryData();

	/////////////////////////////////////////////////
	/// void CNetConnect::ReadRegistryData()
	/// Read in any persisted data
	/////////////////////////////////////////////////
	void ReadRegistryData();

	/////////////////////////////////////////////////
	/// void CNetConnect::ReadDataFromRegistry()
	/// Read in registry-resident data
	/////////////////////////////////////////////////
	void ReadDataFromRegistry();

	/////////////////////////////////////////////////
	/// Restores any connections that are in our map
	/// You probably want to call this right after
	/// the constructor.  At any other time, if we've
	/// done a refresh or findwe could end up with 
	/// existing connections which might be problematic
	/////////////////////////////////////////////////
	DWORD RestoreConnection(DWORD dwIndex);

	/////////////////////////////////////////////////
	/// Cleans (deletes) the connection map
	/////////////////////////////////////////////////
	DWORD CleanConnectionMap();

	////////////////////////////////////////////////////////
	/// Makes sure that our connection map is up to date
	/// A side effect of this function is that the current
	/// persistent settings will be flushed to the registry.
	////////////////////////////////////////////////////////
	DWORD RefreshConnections();

	/////////////////////////////////////////////////
	/// Close the given connection
	/// If its a persistent connection lets make
	/// an extra check.
	/////////////////////////////////////////////////
	DWORD RemoveConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName, BOOL fForce, 
								BOOL fPersistentAlso = FALSE);

	/////////////////////////////////////////////////
	/// Add the given connection
	/// If the connection already exists in the map,
	/// its data will be refreshed.
	/////////////////////////////////////////////////
	DWORD AddConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName,
		LPCTSTR pUserName = NULL, LPCTSTR pUserPwd = NULL, 
				BOOL fPersist = FALSE);


	////////////////////////////////////////////////////////////////////
	////////////////////////////////////////////////////////////////////
	BOOL GetConnectionInfo(DWORD dwIndex, LPTSTR pRemoteName, LPTSTR pLocalName,
		LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist);

	////////////////////////////////////////////////////////////////////
	/// In this version, pass in the remote name and we'll fill out the 
	/// rest - returns FALSE if not in the map
	////////////////////////////////////////////////////////////////////
	BOOL GetConnectionInfo(LPCTSTR pRemoteName, LPCTSTR pLocalName,
		LPTSTR pUserName, LPTSTR pUserPwd, BOOL *pfPersist);

	UINT GetDriveType(LPCTSTR pFileName);

	size_t GetNumConnections() { return m_Connections.size(); }


private:

	/////////////////////////////////////////////////
	/// Used internally
	/////////////////////////////////////////////////
	struct NetConnection
	{
		TCHAR szRemoteName[MAX_PATH];
		TCHAR szLocalName[128];
		TCHAR szUserName[128];
		TCHAR szUserPwd[128];
		BOOL  bPersist;
		BOOL  bConnected; // we set this so that we know whether a given drive is connected
	};

	struct OldNetConnection
	{
		TCHAR szRemoteName[MAX_PATH];
		TCHAR szLocalName[20];
		TCHAR szUserName[20];
		TCHAR szUserPwd[20];
		BOOL  bPersist;
		BOOL  bConnected; // we set this so that we know whether a given drive is connected
	};

	/////////////////////////////////////////////////
	/// Enumerates existing connections
	/// Called by RefreshConnections to find what the user 
	/// might have browsed to on their own.
	/// Call with null on the first call.  Mildly
	/// recursive, if you have an object with 
	/// multiple resources you might recurse a level or two at the most.
	/////////////////////////////////////////////////
	DWORD FindConnections(LPNETRESOURCE lpnr = NULL);


	/////////////////////////////////////////////////
	/// Add connections that we find during FindConnections
	/////////////////////////////////////////////////
	DWORD AddConnection(LPNETRESOURCE lpnr);

	/////////////////////////////////////////////////
	/// Adds a connection to the connection map
	/////////////////////////////////////////////////
	DWORD InsertConnection(NetConnection *pConnection);

	/////////////////////////////////////////////////
	/// Actually connect to the resource
	/////////////////////////////////////////////////
	DWORD Connect(NetConnection *pConnection);
	/////////////////////////////////////////////////
	/// Actually disconnect from the resource
	/////////////////////////////////////////////////
	DWORD Disconnect(NetConnection *pConnection, BOOL fForce);

	/////////////////////////////////////////////////
	/// Finds the given connection in the map
	/////////////////////////////////////////////////
	NetConnection *FindConnection(LPCTSTR pRemoteName, LPCTSTR pLocalName);


private:


	// some typdefs
	typedef map<tstring,NetConnection *>	CONNECTION_MAP;
	CONNECTION_MAP m_Connections;


};

#endif