#include "stdafx.h"
#include "secureApi.h"

#ifndef UNDER_CE

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//  ACL FUNCTIONS
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// CAcl
CAcl::CAcl(PACL pacl)
{
	m_fOwn = false;
	m_pACL = pacl;
}

// Allocates and initializes an empty ACL.
// Use the helper function GetLengthAcl to calculate the size needed
// eg:		GetLengthAcl(3, pSids, sizeof(ACCESS_DENIED_ACE));
BOOL CAcl::InitializeAcl(DWORD nAclLength)
{ 
	if (m_pACL && m_fOwn)
		::LocalFree(m_pACL), m_fOwn = false;

	m_pACL = (PACL) LocalAlloc(LMEM_FIXED, nAclLength);

	if(!m_pACL)
		return FALSE;

	m_fOwn = true;

	return ::InitializeAcl(m_pACL, nAclLength, ACL_REVISION); 
}

CAcl::~CAcl()
{
	if (m_fOwn)
		::LocalFree(m_pACL);
}

// Re-allocates an ACL and merges it with an array of EXPLICIT_ACCESS entries
DWORD CAcl::SetEntriesInAcl(ULONG cCountOfExplicitEntries, PEXPLICIT_ACCESS pListOfExplicitEntries)
{
	PACL paclNew;
	DWORD dwRes = ::SetEntriesInAcl(cCountOfExplicitEntries, pListOfExplicitEntries, m_pACL, &paclNew);

	if(dwRes == ERROR_SUCCESS)
	{
		::LocalFree(m_pACL);
		m_pACL = paclNew;
	}

	return dwRes;
}




// Calculates the size needed for an ACL, given
// an array of SIDs and the size of an ACE type.
int CAcl::GetLengthAcl(int nAceCount, PSID pAceSid[], size_t sizeAce)
{
	int cbAcl = sizeof (ACL);
	for (int i = 0 ; i < nAceCount ; i++) 
	{
		// subtract ACE.SidStart from the size
		int cbAce = int(sizeAce - sizeof(DWORD) );
		// add this ACE's SID length
		cbAce += ::GetLengthSid(pAceSid[i]);
		// add the length of each ACE to the total ACL length
		cbAcl += cbAce;
	}

	return cbAcl;
}





////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//  Security Descriptor FUNCTIONS
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////




// CSecurityDescriptor
CSecurityDescriptor::CSecurityDescriptor()
{
	m_fOwn = true;
	m_pSD = (PSECURITY_DESCRIPTOR) new BYTE[SECURITY_DESCRIPTOR_MIN_LENGTH];
	
	if(m_pSD)
		::InitializeSecurityDescriptor(m_pSD, SECURITY_DESCRIPTOR_REVISION);
}

CSecurityDescriptor::CSecurityDescriptor(PSECURITY_DESCRIPTOR pSD)
{
	m_fOwn = false;
	m_pSD = pSD;
}

CSecurityDescriptor::~CSecurityDescriptor()
{
	if (m_fOwn)
		delete m_pSD, m_pSD = NULL;
}

BOOL CSecurityDescriptor::MakeSelfRelative()
{
	// convert to relative format
	DWORD nb = 0;
	MakeSelfRelativeSD(m_pSD, NULL, &nb);

	PSECURITY_DESCRIPTOR sdRel = (PSECURITY_DESCRIPTOR) new BYTE[nb];;
	if (!MakeSelfRelativeSD(m_pSD, sdRel, &nb))
		return FALSE;
	if (!IsValidSecurityDescriptor(sdRel))
		return FALSE;

	delete m_pSD, m_pSD = NULL;
	m_pSD = sdRel;

	return TRUE;
}



PSECURITY_DESCRIPTOR UpdateSD( PSECURITY_DESCRIPTOR OldSD, PSECURITY_DESCRIPTOR NewSD, SECURITY_INFORMATION ri )
{
	BOOL    present;
	BOOL    defaulted;
	PACL	pacl;
	PSID	psid;

	PSECURITY_DESCRIPTOR	sd = LocalAlloc( LPTR, SECURITY_DESCRIPTOR_MIN_LENGTH );
	InitializeSecurityDescriptor( sd, SECURITY_DESCRIPTOR_REVISION );

	if ( OldSD == NULL )
		OldSD = sd;	// use empty descriptor

	//
	// Get SACL
	//
	if ( ! GetSecurityDescriptorSacl( ri & SACL_SECURITY_INFORMATION ? NewSD : OldSD, &present, &pacl, &defaulted ) )
		return NULL;
	SetSecurityDescriptorSacl( sd, present, pacl, defaulted );

	//     
	// Get DACL     
	//      
	if ( ! GetSecurityDescriptorDacl( ri & DACL_SECURITY_INFORMATION ? NewSD : OldSD, &present, &pacl, &defaulted ) )         
		return NULL;      
	SetSecurityDescriptorDacl( sd, present, pacl, defaulted );

	//     
	// Get Owner     
	//
	if ( ! GetSecurityDescriptorOwner( ri & OWNER_SECURITY_INFORMATION ? NewSD : OldSD, &psid, &defaulted ) )         
		return NULL;      
	SetSecurityDescriptorOwner( sd, psid, defaulted );

	//     
	// Get Group     
	//      
	if ( ! GetSecurityDescriptorGroup( ri & GROUP_SECURITY_INFORMATION ? NewSD : OldSD, &psid, &defaulted ) )         
		return NULL;      
	SetSecurityDescriptorGroup( sd, psid, defaulted );

	return sd;
}  



////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//  SID FUNCTIONS
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



BOOL CSid::CreateFromAccountName(LPCTSTR lpAccountName, 
							 LPCTSTR lpSystemName /* = NULL */)
{
	DWORD        cbSid = 0;
	DWORD		 cbDomainName = 0;
	TCHAR        szDomain[MAX_PATH];
	SID_NAME_USE nameUse;
	BOOL brc = true;
	
	brc = ::LookupAccountName(lpSystemName,lpAccountName,
				NULL, &cbSid,  NULL, &cbDomainName, &nameUse);
	Allocate(cbSid);
	cbDomainName = MAX_PATH;
	brc = ::LookupAccountName(lpSystemName,lpAccountName,
				m_pSid, &cbSid,  szDomain, &cbDomainName, &nameUse);
	return brc;
}


void CSid::Copy(PSID pSid)
{
	Free();

	m_pSid = pSid;
}

void CSid::Free()
{
	if (m_pSid && m_fOwn)
		delete [] m_pSid, m_pSid = NULL;
}

// Allocates memory for a SID 
void CSid::Allocate(DWORD dwSize)
{
	Free();

	m_pSid = new BYTE[dwSize];
}


BOOL CSid::LookupAccountName(LPTSTR szName, LPTSTR szDomain, LPCTSTR pSystem /* = NULL */)
{

	SID_NAME_USE use;
	DWORD szNameSize = MAX_PATH;
	DWORD szDomainSize = MAX_PATH;
	return ::LookupAccountSid(pSystem, m_pSid, szName,	&szNameSize,
									szDomain, &szDomainSize, &use);

}


#endif