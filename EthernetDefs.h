////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        EthernetDefs.h
///
/// Header file which defines structs, enums, and defs for ethernet related functions.
///
/// @author      William Levine
/// @date        02/04/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_ETHERNETDEFS_H_)
#define _ETHERNETDEFS_H_

#define MAX_IP_ADDRESS_LEN	20
#define MAX_NAME_SERVERS	16

/// This enumerates the possible ethernet adaptors logically.  These will be mapped by adaptor name 
///	(e.g. Intel Pro 100+) using registry values under the keys 
///	HKEY_LOCAL_MACHINE\SOFTWARE\Crestron Electronics Inc.\UPX\Network\Adaptors\InternalPrivate and
/// HKEY_LOCAL_MACHINE\SOFTWARE\Crestron Electronics Inc.\UPX\Network\Adaptors\InternalPublic.
enum EEthernetAdaptors
{
	EEnetAdaptor_Null = 0,
	EEnetAdaptor_InternalPrivate = 1,	///< The built-in adaptor used to connect to the coldfire board.
	
	EEnetAdaptor_PublicStart = 4,
	EEnetAdaptor_InternalPublic = EEnetAdaptor_PublicStart,	
	EEnetAdaptor_ExtendedA,
	EEnetAdaptor_ExtendedB,
	EEnetAdaptor_Max
};

/// Flags which specify which items to set using the structure provided and the function
///	CfgEthernetSetIPSettings
enum EDeviceIPSettings
{
	EDevIP_EnableDHCP = 0x0001,
	EDevIP_IPAddress = 0x0002,
	EDevIP_SubnetMask = 0x0004,
	EDevIP_NameServers = 0x0008,
	EDevIP_DefaultGateway = 0x0010,
	
	EDevIP_DHCPServer = 0x0100,
	EDevIP_DHCPIPAddress = 0x0200,
	EDevIP_DHCPSubnetMask = 0x0400,
	EDevIP_DHCPNameServes = 0x0800,
	EDevIP_DHCPDefaultGateway = 0x1000
};

/////////////////////////////////////////////////////////////////////
/// Flags for specifying the ethernet link
///
/// Intel based : Intel(R) PRO/100 VE Network Connection
/// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002bE10318}\0002\Ndi\params\SpeedDuplex\enum]
///
///	VIA based :
/// [HKEY_LOCAL_MACHINE\SYSTEM\CurrentControlSet\Control\Class\{4D36E972-E325-11CE-BFC1-08002bE10318}\0000\Ndi\params\ConnectionType\enum]
///
/// *******************************
/// *** HIGHLY DEVICE DEPENDENT ***
/// *******************************
enum EEthernetSpeedLink
{
	EEnetSpeedLink_AutoDetect						= 0,
	
	EEnetSpeedLink_NICIntel_10Mbps_HalfDuplex		= 1,
	EEnetSpeedLink_NICIntel_10Mbps_FullDuplex		= 2,
	EEnetSpeedLink_NICIntel_100Mbps_HalfDuplex		= 3,
	EEnetSpeedLink_NICIntel_100Mbps_FullDuplex		= 4,

	EEnetSpeedLink_NICVia_100Mbps_HalfDuplex		= 1,
	EEnetSpeedLink_NICVia_100Mbps_FullDuplex		= 2,
	EEnetSpeedLink_NICVia_10Mbps_HalfDuplex			= 3,
	EEnetSpeedLink_NICVia_10Mbps_FullDuplex			= 4,

	EEnetSpeedLink_Unkown							= (int)-1
};

/////////////////////////////////////////////////////////////////////
// STRUCTURE for Link Speed TABLE entries
typedef struct _EEthernetSpeedLinkEntry
{
	const EEthernetSpeedLink	type;
	const _TCHAR*				name;

	_EEthernetSpeedLinkEntry(const EEthernetSpeedLink _type, const _TCHAR* _name) : type(_type), name(_name) {};

	static int findEntry(_EEthernetSpeedLinkEntry* pTable, const _TCHAR* tszName)
	{
		if (pTable == NULL || tszName == NULL || tszName[0] == 0)
			return -1;

		// search the speed link table
		for (int i=0 ; pTable[i].name != NULL ; i++)
		{
			if (_tcsicmp(tszName, pTable[i].name) == NULL)
				return i;
		}

		return -1;
	}

	static int findEntry(_EEthernetSpeedLinkEntry* pTable, const EEthernetSpeedLink enetLink)
	{
		if (pTable == NULL)
			return -1;

		// search the speed link table
		for (int i=0 ; pTable[i].name != NULL ; i++)
		{
			if (enetLink == pTable[i].type)
				return i;
		}

		return -1;
	}
} EEthernetSpeedLinkEntry;


/// Structure that can specify various ip settings for the device.  This is used in several functions
/// along with the EDeviceIPSettings enumeration to set/get the settings listed.
struct SDeviceIPSettings
{
	SDeviceIPSettings()	{memset(this, 0, sizeof(SDeviceIPSettings));}

	DWORD	dwEnableDHCP;

	struct 
	{
		// the following values are valid when DHCP is disabled
		_TCHAR	szIPAddress[MAX_IP_ADDRESS_LEN];
		_TCHAR	szSubnetMask[MAX_IP_ADDRESS_LEN];
		_TCHAR	szNameServers[MAX_NAME_SERVERS][MAX_IP_ADDRESS_LEN];
		_TCHAR	szDefaultGateway[MAX_IP_ADDRESS_LEN];
	}static_ip;

	struct
	{
		// the following values are valid when DHCP is enabled
		_TCHAR	szDHCPServer[MAX_IP_ADDRESS_LEN];
		_TCHAR	szDHCPIPAddress[MAX_IP_ADDRESS_LEN];
		_TCHAR	szDHCPSubnetMask[MAX_IP_ADDRESS_LEN];
		_TCHAR	szDHCPNameServers[MAX_NAME_SERVERS][MAX_IP_ADDRESS_LEN];
		_TCHAR	szDHCPDefaultGateway[MAX_IP_ADDRESS_LEN];
	}dhcp;
};


#endif