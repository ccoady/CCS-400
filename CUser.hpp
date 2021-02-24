//FILE: CUser.h

#pragma once;

#include <stdio.h>
#include <windows.h> 
#include <lm.h>
#include <assert.h>
#include "CExecuteCommand.hpp"


#pragma comment(lib, "netapi32.lib")

class CUser : public CExecuteCommand
{
public: 
	CUser(){}
	~CUser(){}

	int EnumUsers()
	{
		LPUSER_INFO_0 pBuf = NULL;
		LPUSER_INFO_0 pTmpBuf;
		DWORD dwLevel = 0;
		DWORD dwPrefMaxLen = MAX_PREFERRED_LENGTH;
		DWORD dwEntriesRead = 0;
		DWORD dwTotalEntries = 0;
		DWORD dwResumeHandle = 0;
		DWORD i;
		DWORD dwTotalCount = 0;
		NET_API_STATUS nStatus;
		LPCWSTR pszServerName = NULL;
		BOOL bRetv = TRUE;

		pszServerName = L"\\\\127.0.0.1"; //localhost
		wprintf(L"\nUser account on %s: \n", pszServerName);

		//
		// Call the NetUserEnum function, specifying level 0; 
		//   enumerate global user account types only.
		//
		do // begin do
		{
			nStatus = NetUserEnum(pszServerName,
				dwLevel,
				FILTER_NORMAL_ACCOUNT, // global users
				(LPBYTE*)&pBuf,
				dwPrefMaxLen,
				&dwEntriesRead,
				&dwTotalEntries,
				&dwResumeHandle);
			//
			// If the call succeeds,
			//
			if ((nStatus == NERR_Success) || (nStatus == ERROR_MORE_DATA))
			{
				if ((pTmpBuf = pBuf) != NULL)
				{
					//
					// Loop through the entries.
					//
					for (i = 0; (i < dwEntriesRead); i++)
					{
						assert(pTmpBuf != NULL);

						if (pTmpBuf == NULL)
						{
							fprintf(stderr, "An access violation has occurred\n");
							break;
						}
						//
						//  Print the name of the user account.
						//
						wprintf(L"\t-- %s\n", pTmpBuf->usri0_name);

						pTmpBuf++;
						dwTotalCount++;
					}
				}
			}
			//
			// Otherwise, print the system error.
			//
			else
			{
				fprintf(stderr, "A system error has occurred: %d\n", nStatus);
				bRetv = FALSE;
			}

			//
			// Free the allocated buffer.
			//
			if (pBuf != NULL)
			{
				NetApiBufferFree(pBuf);
				pBuf = NULL;
			}
		}
		// Continue to call NetUserEnum while 
		//  there are more entries. 
		// 
		while (nStatus == ERROR_MORE_DATA); // end do
		//
		// Check again for allocated memory.
		//
		if (pBuf != NULL)
			NetApiBufferFree(pBuf);
		//
		// Print the final count of users enumerated.
		//
		fprintf(stderr, "\nTotal of %d entries enumerated\n", dwTotalCount);

		return bRetv;
	}

	int CreateNewAccount(LPWSTR name, LPWSTR password, DWORD privillage)
	{
		USER_INFO_1 ui;
		DWORD dwLevel = 1;
		DWORD dwError = 0;
		NET_API_STATUS nStatus;

		ui.usri1_name = name;
		ui.usri1_password = password;
		ui.usri1_priv = privillage; //USER_PRIV_GUEST=0, USER_PRIV_USER=1, USER_PRIV_ADMIN=2
		ui.usri1_home_dir = NULL;
		ui.usri1_comment = NULL;
		ui.usri1_flags = UF_SCRIPT;
		ui.usri1_script_path = NULL;

		//
		// Call the NetUserAdd function, specifying level 1.
		//
		nStatus = NetUserAdd(
			NULL, //NULL means localhost
			dwLevel,
			(LPBYTE)&ui,
			&dwError);
		//
		// If the call succeeds, inform the user.
		//
		if (nStatus == NERR_Success)
		{
			fwprintf(stderr, L"User %s has been successfully added on %s\n",
				name, L"This Computer" );

			// to see this user in cpl
			LOCALGROUP_MEMBERS_INFO_3 lmi3;
			ZeroMemory(&lmi3, sizeof lmi3);
			lmi3.lgrmi3_domainandname = ui.usri1_name;
			dwError = NetLocalGroupAddMembers(NULL, L"Users", 3, (LPBYTE) &lmi3, 1);
		}
		//
		// Otherwise, print the system error.
		//
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);



		return TRUE;
	}

	int DelAnAccount(wchar_t *userName)
	{
		DWORD dwError = 0;
		NET_API_STATUS nStatus;
		BOOL bRetv = TRUE;

		//
		// Call the NetUserDel function to delete the share.
		//
		nStatus = NetUserDel(NULL, userName);
		//
		// Display the result of the call.
		//
		if (nStatus == NERR_Success)
		{
			fwprintf(stderr, L"User %s has been successfully deleted from This Computer\n",
				userName);
		}
		else
		{
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);
			bRetv = FALSE;
		}

		return bRetv;
	}

	//int ChangeAnAccount()
	int ChangeAccountInfo(int argc, wchar_t *argv[])
	{
		DWORD dwLevel = 1008;
		USER_INFO_1008 ui;
		NET_API_STATUS nStatus;

		if (argc != 2)
		{
			fwprintf(stderr, L"Usage: %s UserName\n", argv[0]);
			exit(1);
		}
		// Fill in the USER_INFO_1008 structure member.
		// UF_SCRIPT: required for LAN Manager 2.0 and
		//  Windows NT and later.
		//
		ui.usri1008_flags = UF_SCRIPT | UF_ACCOUNTDISABLE;
		//
		// Call the NetUserSetInfo function 
		//  to disable the account, specifying level 1008.
		//
		nStatus = NetUserSetInfo(
			NULL, // local system
			argv[1], // user name
			dwLevel,
			(LPBYTE)&ui,
			NULL);
		//
		// Display the result of the call.
		//
		if (nStatus == NERR_Success)
			fwprintf(stderr, L"User account %s has been disabled\n", argv[1]);
		else
			fprintf(stderr, "A system error has occurred: %d\n", nStatus);


		return TRUE;
	}


	int ChangeName()
	{


		return TRUE;
	}

	int CreatePassword(WCHAR strUserName[], WCHAR strPassword[])
	{
		LPWSTR lpszUserName = strUserName; //(LPWSTR)strUserName.AllocSysString();
		LPWSTR lpszPrimaryDC = strPassword; //(LPWSTR)m_pDomain.m_strPDC.AllocSysString();
		USER_INFO_1003 usriSetPassword;
		DWORD dwLevel = 1003;
		NET_API_STATUS nStatus = 0;
		DWORD dwParmError = 0;

		usriSetPassword.usri1003_password  = strPassword; //(LPWSTR)strPassword.AllocSysString();

		nStatus = NetUserSetInfo(lpszPrimaryDC, 
			lpszUserName,  
			dwLevel, 
			(LPBYTE)&usriSetPassword,
			&dwParmError);

		//SetLastErrorString(nStatus, dwParmError);

		return nStatus;

		return TRUE;
	}

	int ChangePassword(WCHAR strUserName[], WCHAR strOldPassword[], WCHAR strNewPassword[])
	{
		NET_API_STATUS nStatus = 0;

		LPWSTR lpszDomainName = L"DomainName"; //(LPWSTR)m_pDomain.GetDomainName().AllocSysString();
		LPWSTR lpszUserName = strUserName; //(LPWSTR)strUserName.AllocSysString();
		LPWSTR lpszOldPassword = strOldPassword; //(LPWSTR)strOldPassword.AllocSysString();
		LPWSTR lpszNewPassword = strNewPassword; //(LPWSTR)strNewPassword.AllocSysString();

		nStatus = NetUserChangePassword(lpszDomainName, 
			lpszUserName, 
			lpszOldPassword, 
			lpszNewPassword);

		//SetLastErrorString(nStatus);

		return nStatus;	

	}

	int ChangePicture()
	{


		return TRUE;
	}


	int ChangeAccountType()
	{


		return TRUE;
	}




};
