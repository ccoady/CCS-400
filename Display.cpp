#include "stdafx.h"
#include "Display.h"
#include <string>
#include <list>
#include <Wtsapi32.h>
#include <userenv.h>
#include <Psapi.h.>
#include "BaseLogger.h"

BOOL StartAppAsUser(TCHAR * tszCmdLine,bool show);

CmPCConfig_DeclSpec void CfgMapTouchToDisplay()
{
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    memset(&si, 0, sizeof(si));
    memset(&pi, 0, sizeof(pi));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;  
    si.wShowWindow = SW_SHOW;

    TCHAR szApp[_MAX_PATH];
    _stprintf_s(szApp, _MAX_PATH, _T("C:\\crestron\\bin\\LaunchDigitizerSetup.bat"));
    
    // test
    //VOID  OldValue;
    //Wow64DisableWow64FsRedirection(&OldValue);
    //if(OldValue == 0)
    //{
    //    BaseLogger::WriteLog(_T("Wow64DisableWow64FsRedirection\n"));
    //}

    StartAppAsUser(szApp,true);

    return;

    if ( !CreateProcess(szApp, NULL, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
		_TCHAR tszMsg[2048];  tszMsg[0] = 0;
		_TCHAR * pLastErrMsg =_T("");
		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
			0,
			::GetLastError(),
			MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
			(LPTSTR)&pLastErrMsg,
			0,
			NULL);

		//_stprintf(tszMsg, _T("CfgMapTouchToDisplay FAILED path= %s - error= (%u) \r\n"),pLastErrMsg,  ::GetLastError()  );
        // log to config server
		if ( pLastErrMsg )
			LocalFree(pLastErrMsg); /*free memory*/
    }else
    {
        // release handless 
        CloseHandle(pi.hProcess); 
        CloseHandle(pi.hThread);
    }

}



  BOOL StartAppAsUser(TCHAR * tszCmdLine,bool show)
  {
      tstring m_strAppPath;
      TCHAR *m_pTempPath;
      HANDLE            m_hNewProc;
      HANDLE            m_hNewThread;
      DWORD       m_dwNewProcessID;
      DWORD       m_dwNewThreadID;
      m_strAppPath = tszCmdLine;
      DWORD aProcesses[1024], cbNeeded, cProcesses;
      DWORD dwCurrSessionId = WTSGetActiveConsoleSessionId();
      DWORD dwSessionId;
      HANDLE hToken;
      HANDLE hTokenDup;
      STARTUPINFO si;
      PROCESS_INFORMATION pi;

      BOOL retVal = FALSE;

      if (m_strAppPath.length() == 0)
          return FALSE;

      ZeroMemory( &si, sizeof( STARTUPINFO ) );
      si.cb = sizeof( STARTUPINFO );
      ZeroMemory(&pi, sizeof(pi));

      LPVOID  pEnv = NULL;
      DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

      size_t iMaxPath = MAX_PATH + m_strAppPath.length();
      m_pTempPath = (TCHAR *)new TCHAR[iMaxPath];
      if (!m_pTempPath)
          return FALSE;

      memset(m_pTempPath,'\0',iMaxPath);

      _stprintf_s(m_pTempPath, iMaxPath, _T("%s"),m_strAppPath.c_str());

      if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
      {
          return retVal;
      }

      // Calculate how many process identifiers were returned.
      cProcesses = cbNeeded / sizeof(DWORD);

      for ( unsigned int i = 0; i < cProcesses; i++ )
      {
          if( aProcesses[i] != 0 )
          {
              TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

              // Get a handle to the process.

              dwSessionId = 0;
              HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                  PROCESS_VM_READ,
                  FALSE, aProcesses[i] );

              // Get the process name.
              if (NULL != hProcess )
              {
                  GetModuleBaseName( hProcess, 0, szProcessName, 
                      TSIZEOF(szProcessName) );

                  BaseLogger::WriteLog(szProcessName);
                  if (_tcsicmp(szProcessName, _T("utsEngine.exe")) == 0)
                  {
                      if (ProcessIdToSessionId(aProcesses[i],&dwSessionId))
                      {
                          if (dwSessionId == dwCurrSessionId)
                          {
                              if (OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_DUPLICATE, &hToken))
                              {
                                  SECURITY_ATTRIBUTES sa;
                                  sa.bInheritHandle = false;
                                  sa.nLength = sizeof(sa);
                                  sa.lpSecurityDescriptor = NULL;
                                  if (DuplicateTokenEx(hToken,GENERIC_ALL,&sa,SecurityIdentification,TokenPrimary,&hTokenDup))
                                  {
                                      if ( !CreateProcessAsUser(hTokenDup,NULL,tszCmdLine,&sa,&sa,FALSE,dwCreationFlag,pEnv,NULL,&si,&pi) )
                                      {

                                          TCHAR tszMsg[2048];  tszMsg[0] = 0;
                                          TCHAR * pLastErrMsg = _T("");
                                          FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
                                              0,
                                              ::GetLastError(),
                                              MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
                                              (LPTSTR)&pLastErrMsg,
                                              0,
                                              NULL);


                                          _stprintf_s(tszMsg, TSIZEOF(tszMsg), _T("CAppLauncher::StartApp  FAILED  error = %s (%u) \r\n"), pLastErrMsg, ::GetLastError() );
                                          //StatusMsg("tszMsg" ,MsgType::LOGFILE,false);
                                          if ( pLastErrMsg )
                                              LocalFree(pLastErrMsg); /*free memory*/
                                          CloseHandle( hProcess );
                                          CloseHandle(hToken);
                                          break;
                                      }
                                      m_hNewProc = pi.hProcess;
                                      m_hNewThread = pi.hThread;
                                      m_dwNewProcessID = pi.dwProcessId;
                                      m_dwNewThreadID = pi.dwThreadId;
                                      CloseHandle( hProcess );
                                      CloseHandle(hToken);
                                      CloseHandle(hTokenDup);
                                      retVal = TRUE;
                                      break;

                                  }
                                  CloseHandle(hToken);
                              }
                          }
                      }

                  }
              }
              CloseHandle( hProcess );
          }
      }
      return retVal;
  }