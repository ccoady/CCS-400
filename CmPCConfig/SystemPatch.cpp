#include "stdafx.h"
#include "SystemPatch.h"

CSystemPatch::CSystemPatch(void)
	: m_pPackList(NULL)
	, m_bInited(FALSE)
{
	m_pPackList = new std::list<PACK_VECTOR *>();
}

CSystemPatch::~CSystemPatch(void)
{
	Cleanup();
}

void CSystemPatch::Cleanup()
{
	if (m_pPackList && m_pPackList->size())
	{
		PACK_LIST::iterator iterList;
		iterList = m_pPackList->begin();
		while (iterList != m_pPackList->end())
		{
			PACK_VECTOR * pVector = (PACK_VECTOR *)(*iterList);
			if (pVector->size())
				pVector->erase(pVector->begin(), pVector->end()); // remove all elements from this map
			
			delete pVector;
			pVector = NULL;

			iterList++;
		}

		m_pPackList->erase(m_pPackList->begin(), m_pPackList->end());
		delete m_pPackList;
		m_pPackList = NULL;
	}
}

PACK_LIST * CSystemPatch::GetPatches()
{
	if (m_bInited)
		return m_pPackList;

	m_bInited = TRUE;

	_TCHAR tsSubKey[MAX_PATH] = { 0 };
	_TCHAR tsDesc[PACK_DESCRIPTION_LEN] = { 0 };
	DWORD  dwSize = MAX_PATH;
	
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, KEY_PACKS)) // top-level packs 
	{
		DWORD dwIndex = 0;
		LONG iResult = ERROR_SUCCESS;

		// find out the sub key count
		while (iResult != ERROR_NO_MORE_ITEMS)
		{
			dwSize = MAX_PATH;
			iResult = reg.EnumKey(dwIndex++, tsSubKey, &dwSize);
			if (iResult == ERROR_SUCCESS)
			{
				RetrievePacks(tsSubKey, tsDesc);
			}
		}

		reg.Close();
	}

	return m_pPackList;
}

PACK_LIST * CSystemPatch::GetPatcheNames()
{
	if (m_bInited)
		return m_pPackList;

	m_bInited = TRUE;

	_TCHAR tsSubKey[MAX_PATH] = { 0 };
	_TCHAR tsDesc[PACK_DESCRIPTION_LEN] = { 0 };
	DWORD  dwSize = MAX_PATH;
	
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, KEY_PACKS)) // top-level packs 
	{
		DWORD dwIndex = 0;
		LONG iResult = ERROR_SUCCESS;

		// find out the sub key count
		while (iResult != ERROR_NO_MORE_ITEMS)
		{
			dwSize = MAX_PATH;
			iResult = reg.EnumKey(dwIndex++, tsSubKey, &dwSize);
			if (iResult == ERROR_SUCCESS)
			{
				RetrievePackNames(tsSubKey, tsDesc);
			}
		}

		reg.Close();
	}

	return m_pPackList;
}
// Loop through all subkeys under this key to retrieve all fixed in 
// contained in this pack
void CSystemPatch::RetrievePacks(_TCHAR * ptsKeyName, _TCHAR * ptsDesc)
{
	_TCHAR tsKey[MAX_PATH] = { 0 };
	_stprintf_s(tsKey, MAX_PATH, _T("%s\\%s"), KEY_PACKS, ptsKeyName);
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, tsKey)) // subkey under Packs, e.g. OSPack
	{
		DWORD dwIndex = 0;
		LONG iResult = ERROR_SUCCESS;

		PACK_VECTOR * pVector = new std::vector<tstring>(); // vector for holding the pack info

		// Sub-key (e.g., KB10000) info - need to use separate map to ensure the order of the output
		_TCHAR tsSubKey[MAX_PATH] = { 0 };
		DWORD dwSize = MAX_PATH;
		while (iResult != ERROR_NO_MORE_ITEMS)
		{
			dwSize = MAX_PATH;
			iResult = reg.EnumKey(dwIndex++, tsSubKey, &dwSize);
			if (iResult == ERROR_SUCCESS)
			{
				RetrievePackDesc(pVector, ptsKeyName, tsSubKey, ptsDesc);
			}
		}	

		// retrieve pack description
		m_pPackList->push_back(pVector);
		DWORD dwDesc = PACK_DESCRIPTION_LEN;
		if (reg.QueryStringValue(_T("Description"), ptsDesc, &dwDesc) == ERROR_SUCCESS)
		{
			ptsDesc[dwDesc] = 0;
			_TCHAR tsVer[32] = { 0 };
			DWORD dwVer = 32;
			if (reg.QueryStringValue(_T("Version"), tsVer, &dwVer) == ERROR_SUCCESS)
			{
				tsVer[dwVer] = 0;
				_tcscat(ptsKeyName, _T("_"));
				_tcscat(ptsKeyName, tsVer);
			}
		}
		else
			ptsDesc[0] = 0;
		PACK_VECTOR::iterator iterVector = pVector->begin();
		iterVector = pVector->insert(iterVector, ptsDesc);
		pVector->insert(iterVector, ptsKeyName);
		
		reg.Close();
	}
}

// Loop through all subkeys under this key to retrieve all fixed in 
// contained in this pack
void CSystemPatch::RetrievePackNames(_TCHAR * ptsKeyName, _TCHAR * ptsDesc)
{
	_TCHAR tsKey[MAX_PATH] = { 0 };
	_stprintf_s(tsKey, MAX_PATH, _T("%s\\%s"), KEY_PACKS, ptsKeyName);
	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, tsKey)) // subkey under Packs, e.g. OSPack
	{
		DWORD dwIndex = 0;
		LONG iResult = ERROR_SUCCESS;

		PACK_VECTOR * pVector = new std::vector<tstring>(); // vector for holding the pack info

		// Sub-key (e.g., KB10000) info - need to use separate map to ensure the order of the output
//		_TCHAR tsSubKey[MAX_PATH] = { 0 };
//		DWORD dwSize = MAX_PATH;
//		while (iResult != ERROR_NO_MORE_ITEMS)
//		{
//			dwSize = MAX_PATH;
//			iResult = reg.EnumKey(dwIndex++, tsSubKey, &dwSize);
//			if (iResult == ERROR_SUCCESS)
//			{
//				RetrievePackDesc(pVector, ptsKeyName, tsSubKey, ptsDesc);
//			}
//		}	

		// retrieve pack description
		m_pPackList->push_back(pVector);
		DWORD dwDesc = PACK_DESCRIPTION_LEN;
		if (reg.QueryStringValue(_T("Description"), ptsDesc, &dwDesc) == ERROR_SUCCESS)
		{
			ptsDesc[dwDesc] = 0;
			_TCHAR tsVer[32] = { 0 };
			DWORD dwVer = 32;
			if (reg.QueryStringValue(_T("Version"), tsVer, &dwVer) == ERROR_SUCCESS)
			{
				tsVer[dwVer] = 0;
				_tcscat(ptsKeyName, _T(":"));
				_tcscat(ptsKeyName, tsVer);
			}
		}
		else
			ptsDesc[0] = 0;
		PACK_VECTOR::iterator iterVector = pVector->begin();
		iterVector = pVector->insert(iterVector, ptsDesc);
		pVector->insert(iterVector, ptsKeyName);
		
		reg.Close();
	}
}
void CSystemPatch::RetrievePackDesc(PACK_VECTOR * pVector, _TCHAR * ptsKeyName, _TCHAR * ptsSubKeyName, _TCHAR * ptsDesc)
{
	_TCHAR tsSubKey[MAX_PATH] = { 0 };
	_stprintf_s(tsSubKey, MAX_PATH, _T("%s\\%s\\%s"), KEY_PACKS, ptsKeyName, ptsSubKeyName);

	CRegKey reg;
	if (ERROR_SUCCESS == reg.Open(HKEY_LOCAL_MACHINE, tsSubKey)) // subpack key, e.g. KB123445
	{
		DWORD dwDesc = PACK_DESCRIPTION_LEN;
		if (reg.QueryStringValue(_T("Description"), ptsDesc, &dwDesc) == ERROR_SUCCESS)
			ptsDesc[dwDesc] = 0;
		else
			ptsDesc[0] = 0;

		InsertToVector(pVector, ptsSubKeyName, ptsDesc);
		
		reg.Close();
	}
}

void CSystemPatch::InsertToVector(PACK_VECTOR * pVector, _TCHAR * ptsSubKeyName, _TCHAR * ptsDesc)
{
	PACK_VECTOR::iterator iterVector;
	iterVector = pVector->begin();
	if (pVector->size())
	{			
		while (iterVector != pVector->end())
		{													
			if (_tcsicmp((*iterVector).c_str(), ptsSubKeyName) > 0)
				break;
			
			iterVector++; // skip key desc
			iterVector++; // point to next key name
		}
	}

	if (iterVector == pVector->end()) // add to the end
	{
		pVector->push_back(ptsSubKeyName);
		pVector->push_back(ptsDesc);
	}
	else
	{
		iterVector = pVector->insert(iterVector, ptsDesc);
		pVector->insert(iterVector, ptsSubKeyName);
	}
}

// Parse sub pack key name to get rid of leading 0
void CSystemPatch::ParseKeyName(_TCHAR * ptsSubKeyName)
{
	_TCHAR * p = ptsSubKeyName;
	int iCount = 0;
	while (p && (*p == 0x4B || *p == 0x42) && iCount < 2)
	{
		iCount++;
		p++;
	}

	while (p && *p == 0x30)
		p++;

	memmove((ptsSubKeyName+iCount), p, _tcslen(p));
	ptsSubKeyName[_tcslen(ptsSubKeyName) - iCount] = 0;
}

tstring * CSystemPatch::GetDescription(_TCHAR * ptsKeyName)
{
	if (!m_pPackList)
		return NULL;

	if (m_pPackList->size())
	{		
		PACK_LIST::iterator iterList;
		iterList = m_pPackList->begin();
		while (iterList != m_pPackList->end())
		{
			PACK_VECTOR * pVector = (PACK_VECTOR *)(*iterList);
			if (pVector->size())
			{
				int iCount = 0;
				PACK_VECTOR::iterator iterVector;
				iterVector = pVector->begin();				
				while (iterVector != pVector->end())
				{		
					if (_tcsicmp((*iterVector).c_str(), ptsKeyName) == 0)
					{
						iterVector++; // point to desc
						return &(*iterVector);
					}

					iterVector++; // skip desc
					iterVector++; // pointer to next key name
				}					
			}
			iterList++;
		}
	}

	return NULL;
}
