#pragma once

#pragma warning(disable : 4251) // disable warning because we don't allow create PATCH_MAP from outside

#include "NetConnect.h"
#include <map>
#include <list>
#include <vector>

const int PACK_DESCRIPTION_LEN	= 1024;
#define KEY_PACKS		_T("Software\\Crestron Electronics Inc.\\Packs") //We can keep things in the 64 bit registry now
//#define KEY_PACKS		_T("Software\\Wow6432Node\\Crestron Electronics Inc.\\Packs")

using namespace std;
typedef vector<tstring> PACK_VECTOR;
typedef std::list<PACK_VECTOR *> PACK_LIST;

class CmPCConfig_DeclSpec CSystemPatch
{
public:
	CSystemPatch(void);
	~CSystemPatch(void);

	void		   Cleanup();
	void			RetrievePackNames(_TCHAR * ptsKeyName, _TCHAR * ptsDesc);
	void			RetrievePacks(_TCHAR * ptsKeyName, _TCHAR * ptsDesc);
	void			RetrievePackDesc(PACK_VECTOR *, _TCHAR * ptsKeyName, _TCHAR * ptsSubKeyName, _TCHAR * ptsDesc);
	void			ParseKeyName(_TCHAR * ptsSubKeyName);
	void			InsertToVector(PACK_VECTOR * pVector, _TCHAR * ptsSubKeyName, _TCHAR * ptsDesc);
	tstring *		GetDescription(_TCHAR *);
	PACK_LIST *	GetPatches();
	PACK_LIST *	GetPatcheNames();
	// attribute
	PACK_LIST *	m_pPackList;
	BOOL		m_bInited;
};
