////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        SecureApi.h
///
/// Wrappers for security functions.
/// These classes are meant to be very loose wrappers around all the various security structures in
/// the Win32 API.
///
/// Some security basics.
/// A given object (folder, device, etc.) has a SECURITY_DESCRIPTOR 
/// A SECURITY_DESCRIPTOR 
///		An owner (SID) 
///     A primary group (SID) 
///     A discretionary ACL (An ACL which specifies the access that various users and groups can obtain)
///     A system ACL (An ACL that controls the generation of audit messages)
///     and Qualifiers for the preceding items 
///
/// ACL - Access Control List
/// ACE - Access Control Entries (i.e. one item in an ACL)
///	SID - Security ID - Each user and group has a unique SID.  There are also SIDs
///       for builtin concepts like "Everyone" and "Logged on User" (the built in 
///       sids are "Well Known" sids).
///
/// The following code retrieves the security descriptor for a given share name on the local 
/// computer and prints out the dacl.
///
///	SHARE_INFO_502 * Info502 = NULL;
///	DWORD status = NetShareGetInfo( NULL, L"public", 502, (BYTE **)&Info502 );
///	if ( status == 0 )  
///	{
///		CSecurityDescriptor sd(Info502->shi502_security_descriptor);
///		if (sd.IsValid())
///		{
///			CAcl acl(sd.GetDacl());
///			int count = acl.GetAceCount();
///			for (int i = 0; i < count; i++)
///			{
///				CFileAce ace = acl.GetAce(i);
///				CSid sid(ace.GetSid());
///				TCHAR szName[MAX_PATH];
///				TCHAR szDomain[MAX_PATH];
///
///				sid.LookupAccountName(szName,szDomain);
///
///				printf(_T("User: %s\\%s "), szDomain, szName);
///
///				if (ace.IsRead())	printf(_T("Read "));
///				if (ace.IsWrite())  printf(_T("Write "));
///
///				if (ace.IsAllowedType())	printf(_T("Allowed "));
///				if (ace.IsDeniedType())		printf(_T("Denied "));
///				if (ace.IsAuditType())		printf(_T("Audit "));
///
///				printf(_T("\n"));
///			}
///
///		}
///
///	}
///
///
/// @author      John Pavlik
/// @date        05/27/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#ifndef _SECUREAPI_H_
#define _SECUREAPI_H_

#ifndef UNDER_CE

#include <aclapi.h>
#include <accctrl.h>


//////////////////////////////////////////////////////////////////////////////
/// Wrapper for Access control entries
/// 
//////////////////////////////////////////////////////////////////////////////
class CAce
{
public:

	CAce(LPVOID pAce)
	{
		m_pAce = pAce;
	}

	virtual ~CAce() { m_pAce = NULL; }

	BYTE		GetType() { return ((ACE_HEADER *)m_pAce)->AceType; }
	BYTE        GetFlags(){ return ((ACE_HEADER *)m_pAce)->AceFlags; }
	WORD        GetSize() { return ((ACE_HEADER *)m_pAce)->AceSize; }
	bool        IsAllowedType() 
	{ 
		return (GetType() == ACCESS_ALLOWED_ACE_TYPE) || (GetType() == ACCESS_ALLOWED_OBJECT_ACE_TYPE);
	}
	bool        IsDeniedType() 
	{ 
		return (GetType() == ACCESS_DENIED_ACE_TYPE) || (GetType() == ACCESS_DENIED_OBJECT_ACE_TYPE);
	}
	bool        IsAuditType() 
	{ 
		return (GetType() == SYSTEM_AUDIT_ACE_TYPE) || (GetType() == SYSTEM_AUDIT_OBJECT_ACE_TYPE);
	}

	ACCESS_MASK GetMask()
	{
		switch (GetType())
		{
		case ACCESS_ALLOWED_ACE_TYPE: return ((ACCESS_ALLOWED_ACE *)m_pAce)->Mask;
		case ACCESS_DENIED_ACE_TYPE:  return ((ACCESS_DENIED_ACE *)m_pAce)->Mask;
		case SYSTEM_AUDIT_ACE_TYPE:   return ((SYSTEM_AUDIT_ACE *)m_pAce)->Mask;
		case ACCESS_ALLOWED_OBJECT_ACE_TYPE: return ((ACCESS_ALLOWED_OBJECT_ACE *)m_pAce)->Mask;
		case ACCESS_DENIED_OBJECT_ACE_TYPE:  return ((ACCESS_DENIED_OBJECT_ACE *)m_pAce)->Mask;
		case SYSTEM_AUDIT_OBJECT_ACE_TYPE:   return ((SYSTEM_AUDIT_OBJECT_ACE *)m_pAce)->Mask;
		default:
			break;
		}
		return 0;
	}

	PSID GetSid()	
	{
		switch (GetType())
		{
		case ACCESS_ALLOWED_ACE_TYPE: return &((ACCESS_ALLOWED_ACE *)m_pAce)->SidStart;
		case ACCESS_DENIED_ACE_TYPE:  return &((ACCESS_DENIED_ACE *)m_pAce)->SidStart;
		case SYSTEM_AUDIT_ACE_TYPE:   return &((SYSTEM_AUDIT_ACE *)m_pAce)->SidStart;
		case ACCESS_ALLOWED_OBJECT_ACE_TYPE: return &((ACCESS_ALLOWED_OBJECT_ACE *)m_pAce)->SidStart;
		case ACCESS_DENIED_OBJECT_ACE_TYPE:  return &((ACCESS_DENIED_OBJECT_ACE *)m_pAce)->SidStart;
		case SYSTEM_AUDIT_OBJECT_ACE_TYPE:   return &((SYSTEM_AUDIT_OBJECT_ACE *)m_pAce)->SidStart;
		default:
			break;
		}
		return 0;
	}

	CAce &operator =(const CAce &that)
	{
		m_pAce = that.m_pAce;
	}

protected:

	LPVOID m_pAce;

};


////////////////////////////////////////////////////////////////////////////////
/// An Ace with file specific rights understanding
class CFileAce : public CAce
{
public:

	CFileAce(LPVOID p) : CAce(p) { }

	enum
	{
		READ_MASK = (FILE_READ_ATTRIBUTES|FILE_READ_DATA|FILE_READ_EA),
		WRITE_MASK = (FILE_APPEND_DATA|FILE_WRITE_ATTRIBUTES|FILE_WRITE_DATA|FILE_WRITE_EA),

		SHARE_FULLCONTROL = 0x001F01FF,
		SHARE_CHANGE	  = 0x001301BF,
		SHARE_READ		  = 0x001200A9
	};

	inline bool IsRead()
	{
		return (GetMask() & READ_MASK ) != 0;
	}

	inline bool IsWrite()
	{
		return (GetMask() & WRITE_MASK ) != 0;
	}

	CFileAce &operator =(const CAce &that)
	{
		(CAce)*this = that;
	}


};




////////////////////////////////////////////////////////////////////////////////
/// Access control List 
class CAcl  
{
public:
	CAcl(PACL pAcl = NULL);
	virtual ~CAcl();
	BOOL InitializeAcl(DWORD nAclLength);
	DWORD SetEntriesInAcl(ULONG cCountOfExplicitEntries, PEXPLICIT_ACCESS pListOfExplicitEntries);

	inline BOOL IsValid() { return ::IsValidAcl(m_pACL); }
	
	inline BOOL AddAce(DWORD dwStartingAceIndex, LPVOID pAceList, DWORD nAceListLength)
		{ return ::AddAce(m_pACL, ACL_REVISION, dwStartingAceIndex, pAceList, nAceListLength);  }
	inline BOOL AddAccessAllowedAce(DWORD dwAccessMask, PSID pSID)
		{ return ::AddAccessAllowedAce(m_pACL, ACL_REVISION, dwAccessMask, pSID); }
	inline BOOL AddAccessDeniedAce(DWORD dwAccessMask, PSID pSID)
		{ return ::AddAccessDeniedAce(m_pACL, ACL_REVISION, dwAccessMask, pSID); }
	inline BOOL AddAuditAccessAce(DWORD dwAccessMask, PSID pSid, BOOL bAuditSuccess, BOOL bAuditFailure)
		{ return ::AddAuditAccessAce(m_pACL, ACL_REVISION, dwAccessMask, pSid, bAuditSuccess, bAuditFailure) ; }

	inline BOOL DeleteAce(DWORD dwAceIndex) { return ::DeleteAce(m_pACL, dwAceIndex); }
	inline BOOL FindFirstFreeAce(LPVOID * pAce) { return ::FindFirstFreeAce(m_pACL, pAce); }

	inline LPVOID GetAce(DWORD dwAceIndex) 
	{ 
		LPVOID pAce;
		if (::GetAce(m_pACL, dwAceIndex, &pAce))
			return pAce;
		return NULL;
	}

	inline int GetAceCount()
	{
		ACL_SIZE_INFORMATION aclSizeInfo;
		if (GetAclInformation( m_pACL, &aclSizeInfo, sizeof(ACL_SIZE_INFORMATION), AclSizeInformation))  
			return aclSizeInfo.AceCount;
		return 0;
	}

	inline operator PACL () { return m_pACL; }

	// Calculates the size needed for an ACL, given an array of
	// SIDs and the size of an ACE type.
	static int GetLengthAcl(int nAceCount, PSID pAceSid[], size_t sizeAce);

protected:
	bool m_fOwn;
	PACL m_pACL;

};



/////////////////////////////////////////////////////////////////////
/// This class is a loose wrapper for managing a SECURITY_DESCRIPTOR
class CSecurityDescriptor  
{
public:
	CSecurityDescriptor();
	CSecurityDescriptor(PSECURITY_DESCRIPTOR);
	virtual ~CSecurityDescriptor();

	inline BOOL IsValid() { return ::IsValidSecurityDescriptor(m_pSD); }

	inline BOOL SetGroup(PSID pGroup, BOOL bGroupDefaulted) { return ::SetSecurityDescriptorGroup(m_pSD, pGroup, bGroupDefaulted); }

	inline BOOL SetOwner(PSID pOwner, BOOL bOwnerDefaulted) { return ::SetSecurityDescriptorOwner(m_pSD, pOwner, bOwnerDefaulted); }

	inline BOOL SetSacl(BOOL bSaclPresent, PACL pSacl, BOOL bSaclDefaulted)
		{ return ::SetSecurityDescriptorSacl(m_pSD, bSaclPresent, pSacl, bSaclDefaulted); }

	inline BOOL SetDacl(BOOL bDaclPresent, PACL pDacl, BOOL bDaclDefaulted)
		{ return ::SetSecurityDescriptorDacl(m_pSD, bDaclPresent, pDacl, bDaclDefaulted); }

	inline BOOL GetGroup(PSID* pGroup, LPBOOL lpGroupDefaulted) { return ::GetSecurityDescriptorGroup(m_pSD, pGroup, lpGroupDefaulted); }

	inline BOOL GetOwner(PSID* pOwner, LPBOOL lpOwnerDefaulted) { return ::GetSecurityDescriptorGroup(m_pSD, pOwner, lpOwnerDefaulted); }

	inline PACL GetSacl(LPBOOL lpbSaclDefaulted = NULL)
	{ 
		PACL pSacl;
		BOOL bSaclPresent;
		BOOL bSaclDefaulted;
		BOOL brc = ::GetSecurityDescriptorSacl(m_pSD, &bSaclPresent, &pSacl, &bSaclDefaulted); 
		if (brc && !bSaclPresent)
			return FALSE;

		if (lpbSaclDefaulted)
			*lpbSaclDefaulted = bSaclDefaulted;

		return pSacl;
	}

	inline PACL GetDacl(LPBOOL lpbDaclDefaulted = NULL)
	{ 
		PACL pDacl;
		BOOL bDaclPresent;
		BOOL bDaclDefaulted;
		BOOL brc = ::GetSecurityDescriptorDacl(m_pSD, &bDaclPresent, &pDacl, &bDaclDefaulted); 
		if (brc && !bDaclPresent)
			return FALSE;

		if (lpbDaclDefaulted)
			*lpbDaclDefaulted = bDaclDefaulted;

		return pDacl;

	}

	inline operator PSECURITY_DESCRIPTOR () { return m_pSD; }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Wrapper to MakeSelfRelativeSD.
/// This creates a self relative SD and then sets its internal memory to use the newly created SD
/// (freeing the original)
///
/// @author      John Pavlik
/// @date        05/26/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	BOOL MakeSelfRelative();


protected:

	bool				 m_fOwn;
	PSECURITY_DESCRIPTOR m_pSD;

};

/// CSid is a loose wrapper around the windows SID object.
/// The object of this class is not to encompass all functionality
/// but rather to hide some of the less useful (i.e. stuff I personally
/// don't care about) but code lengthy functions.
class CSid
{
public:

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor.
/// CSid constructor - very boring
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	CSid(PSID psid = NULL)			{ m_fOwn = psid ? false : true; m_pSid = psid; }
////////////////////////////////////////////////////////////////////////////////////////////////////
/// Destructor.
/// Also boring, free memory if needed.
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	virtual ~CSid() { Free();	     }



////////////////////////////////////////////////////////////////////////////////////////////////////
/// A wrapper around Win32 ::LookupAccountName.
/// We don't really care about all the extra parameters, just get us the SID!
///
/// @param       lpAccountName  Whose SID do you want to look up?
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	BOOL CreateFromAccountName(LPCTSTR lpAccountName, 
							 LPCTSTR lpSystemName = NULL);

////////////////////////////////////////////////////////////////////////////////////////////////////
/// A wrapper around Win32 ::LookupAccountSid.
/// Find out what account the current SID references.  Naming is backwards from
/// windows but more understandable from perspective of a SID object.
///
/// @param       szName  Pointer to MAX_PATH buffer for account name
/// @param       szDomain Pointer to MAX_PATH buffer for domain name
/// @param       lpSystemName   Computer or NULL for local
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	BOOL LookupAccountName(LPTSTR szName, LPTSTR szDomain, LPCTSTR pSystem = NULL);


////////////////////////////////////////////////////////////////////////////////////////////////////
/// Find out the length of the SID data.
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	inline DWORD GetLength() 
	{ 
		return ::GetLengthSid(m_pSid); 
	}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Check the SID for validity.
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	inline BOOL IsValid() 
	{ 
		return ::IsValidSid(m_pSid); 
	}

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Get at the raw pointer in case you need it for something else.
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	inline operator PSID () { return m_pSid; } 

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Check to see if two sids are equal.
///
/// @param       const CSid &
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	inline BOOL operator == (const CSid &sid) { return ::EqualSid(sid.m_pSid, m_pSid); }

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Copy a sid.
///
/// @author      John Pavlik
/// @date        05/25/04
////////////////////////////////////////////////////////////////////////////////////////////////////
	inline CSid & operator =(const CSid &sid) { Copy(sid.m_pSid); return *this; }


/// Some helpers for some often used sids
	static PSID Everyone()
	{
		static PSID	sid = NULL;
		
		if ( sid == NULL )  {
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_WORLD_SID_AUTHORITY;
			AllocateAndInitializeSid( &SIDAuthWorld, 1,
										 SECURITY_WORLD_RID, 0, 0, 0, 0, 0, 0, 0,
										 &sid );
		}
		return sid;
	}

	static PSID LocalAdmin()
	{
		static PSID sid = NULL;

		if ( sid == NULL )  {
			SID_IDENTIFIER_AUTHORITY ntauth = SECURITY_NT_AUTHORITY;
			AllocateAndInitializeSid( &ntauth, 2,
										SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, 
										&sid );
		}
		return sid;
	}

	static PSID System()
	{
		static PSID	sid = NULL;

		if ( sid == NULL )  {
			SID_IDENTIFIER_AUTHORITY SIDAuthWorld = SECURITY_NT_AUTHORITY;
			AllocateAndInitializeSid( &SIDAuthWorld, 1,
										 SECURITY_LOCAL_SYSTEM_RID, 0, 0, 0, 0, 0, 0, 0,
										 &sid );
		}
		return sid;
}

protected:

	void Allocate(DWORD dwSize);
	void Free();
	void Copy(PSID pSid);
	
	bool  m_fOwn;
	PSID  m_pSid;
};


#endif

#endif