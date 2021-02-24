#include "stdafx.h"
#include "CUser.h"
#include "CUser.hpp"


CmPCConfig_DeclSpec BOOL CfgShowAllUsers()
{
	CUser u;
	return u.EnumUsers();
}

CmPCConfig_DeclSpec BOOL CfgAddUser(LPWSTR name, LPWSTR password, DWORD privillage)
{
	CUser u;
	return u.CreateNewAccount(name, password, privillage); //USER_PRIV_GUEST=0, USER_PRIV_USER=1, USER_PRIV_ADMIN=2
}

CmPCConfig_DeclSpec BOOL CfgDeleteUser(LPWSTR name)
{
	CUser u;
	return u.DelAnAccount(name);
}
