#pragma once;

#include "windows.h"

#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
//#include "CStringUtil.hpp"
#include "tchar.h"
#include "shellapi.h"
#include "BaseLogger.h"
#include "comdef.h"

#include <Wtsapi32.h>
#include <userenv.h>
#include <Psapi.h.>

#include "time.h"

using namespace std;

#ifdef tstring
#undef tstring
typedef std::basic_string <TCHAR> tstring;
#endif

#define KB(x)((x) << 10)
#define MB(x)((x)<< 20)

#define MAX_RESPONSE_STR_LEN  KB(5)

#ifndef TSIZEOF
#define TSIZEOF(a) sizeof(a)/sizeof(TCHAR)
#endif


 class CExecuteCommand/*: public CStringUtil*/
{
	TCHAR m_tResponseBuf[MAX_RESPONSE_STR_LEN];
	size_t m_ResponseBufLen;
	DWORD	m_dwLastExitCode;
	//BaseLogger* m_pLogger;

public:


	CExecuteCommand()
	{
		//m_pLogger = NULL;
		Init();
		WriteLog(__WFUNCTION__, TRACESTACK);
	}

	~CExecuteCommand(){}

	BOOL Init()
	{
		WriteLog(__WFUNCTION__, TRACESTACK);
		m_ResponseBufLen = MAX_RESPONSE_STR_LEN;
		memset(m_tResponseBuf, NULL, m_ResponseBufLen * sizeof(TCHAR) );
		return TRUE;
	}

	BOOL SetResponseString(TCHAR *str)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);
		BOOL bRetv = TRUE;
	
		size_t idx = _tcslen(m_tResponseBuf);
		m_ResponseBufLen = m_ResponseBufLen - idx;
		idx = _sntprintf_s(&m_tResponseBuf[idx], m_ResponseBufLen, _TRUNCATE, _T("%s\r\n"), str  ); 

		return bRetv;
	}	
	
	BOOL CheckForSuccess(TCHAR *str)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);
		BOOL bRetv;
	
		_tcsstr(m_tResponseBuf, str) ? bRetv=TRUE : bRetv=FALSE;

		if(bRetv==FALSE)// log error message
			BaseLogger::WriteFormattedLog(L"*** FALSE: %s, %s", __WFUNCTION__, m_tResponseBuf);

		return bRetv;
	}

	DWORD CheckExitCode()
	{
		return m_dwLastExitCode;
	}

	BOOL CheckNetshResponse()
	{
        WriteLog(__WFUNCTION__, TRACESTACK);
		BOOL bRetv = FALSE;

		if(_tcslen(m_tResponseBuf)==2 && m_tResponseBuf[0]=='\r' && m_tResponseBuf[1]=='\n' )
			bRetv = TRUE;
		else
			bRetv = FALSE;

		if(bRetv==FALSE)// log error message
			BaseLogger::WriteFormattedLog(L"*** FALSE: %s, %s", __WFUNCTION__, m_tResponseBuf);

		return bRetv;
	}

	BOOL GetResponseString(tstring& tResponseString)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);
		tResponseString = m_tResponseBuf;

		return TRUE;
	}	
	
	BOOL GetResponseString(TCHAR *tResponseBuf, DWORD &ResponseBufLen)
	{
		WriteLog(__WFUNCTION__, TRACESTACK);
		int retv = _stprintf_s(tResponseBuf, ResponseBufLen, _T("%wS"), m_tResponseBuf );
		if(retv>0)
			ResponseBufLen = retv;
		else
			ResponseBufLen = 0;

		return TRUE;
	}

	string trim(const string& str) // removes only left and right white spaces
	{
		size_t first = str.find_first_not_of(' ');
		if (string::npos == first)
		{
			return str;
		}
		size_t last = str.find_last_not_of(' ');
		return str.substr(first, (last - first + 1));
	}

	void RemoveWhiteSpaces(tstring& str) // removes all white spaces
	{
 		// " \r Hello \t World! \r\n" 
		//to "HelloWorld!"
		try
		{
			str.erase(std::remove_if(str.begin(), str.end(), isspace ), str.end());
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"RemoveWhiteSpaces");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s exception OUT"), __WFUNCTION__);	            
		}
	}

vector<string> split(const char *str, char c = _T(' '))
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s IN"), __WFUNCTION__);	
		vector<string> result;
		try
		{
			do
			{
				const char *begin = str;

				while(*str != c && *str)
					str++;

				result.push_back( string(begin, str) );
			} while (0 != *str++);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"split");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s exception OUT"), __WFUNCTION__);	
		}

        BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s OUT"), __WFUNCTION__);	
		return result;
	}

	vector<tstring> split(const TCHAR *str, TCHAR c = _T(' '))
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s IN"), __WFUNCTION__);	
		vector<tstring> result;
		try
		{
			do
			{
				const TCHAR *begin = str;

				while(*str != c && *str)
					str++;

				result.push_back(tstring(begin, str));
			} while (0 != *str++);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"split");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s exception OUT"), __WFUNCTION__);	
		}

        BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s OUT"), __WFUNCTION__);	
		return result;
	}



	int Execut(LPCTSTR commandLine,tstring &resultBuf,bool &status)
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s(%s)) IN"),__WFUNCTION__, commandLine);	
		try
		{
            BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("*Executing command = %s"), commandLine );

			resultBuf += _T("\n>>>> Executing Netork command[");
			resultBuf += commandLine;
			resultBuf += _T("] ");

			memset(m_tResponseBuf, NULL, m_ResponseBufLen * sizeof(TCHAR) );

			std::string responseStr = "";
			char buffer[MAX_RESPONSE_STR_LEN*2];
			DWORD read;
			HANDLE hOutput, hProcess;		

			hProcess = SpawnAndRedirect(commandLine, &hOutput, NULL);
			if (!hProcess) 
			{
				_stprintf_s(m_tResponseBuf, m_ResponseBufLen, 
					_T("Error: failed to execute this command, GetLastError %d"), GetLastError());
				resultBuf += m_tResponseBuf;
				status = false;
                BaseLogger::WriteFormattedLog(_T("%s(p1, p2, p3)  err1 OUT"), __WFUNCTION__);	
				return FALSE;
			}

			USES_CONVERSION;
			//collect the response
			while (ReadFile(hOutput, buffer, MAX_RESPONSE_STR_LEN-1, &read, NULL))
			{
				buffer[read] = '\0';
				responseStr += buffer;
			}
			if(read > 0)
				status = false;
			else
				status =  true;
			CloseHandle(hOutput);
			
			GetExitCodeProcess(hProcess, &m_dwLastExitCode);
			CloseHandle(hProcess);		

			_sntprintf_s(m_tResponseBuf, m_ResponseBufLen, _TRUNCATE, A2T( responseStr.c_str() )  ); 
			WriteLog( m_tResponseBuf );
			resultBuf +=  m_tResponseBuf  ;
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s ", __LINE__, L"Execut");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s(p1, p2, p3)  exception OUT"), __WFUNCTION__);	
		}

        BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s(p1, p2, p3)  OUT"), __WFUNCTION__);	
		return TRUE;
	}

    //
    //
    //
	int Execut(LPCTSTR commandLine, DWORD dwMsTimeout = INFINITE)
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s(%s) IN"), __WFUNCTION__, commandLine);	
		try
		{
            BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("*Executing command = %s"), commandLine );

			memset(m_tResponseBuf, NULL, m_ResponseBufLen * sizeof(TCHAR) );

			std::string responseStr = "";
			char buffer[MAX_RESPONSE_STR_LEN];
			DWORD read;
			HANDLE hOutput, hProcess;		

			hProcess = SpawnAndRedirect(commandLine, &hOutput, NULL, dwMsTimeout);
			if (!hProcess) 
			{
				_stprintf_s(m_tResponseBuf, m_ResponseBufLen, 
					_T("Error: failed to execute this command, GetLastError %d"), GetLastError());
                BaseLogger::WriteFormattedLog(_T("%s(%s) OUT"), __WFUNCTION__, commandLine);	
				return FALSE;
			}

			//collect the response
			while (ReadFile(hOutput, buffer, MAX_RESPONSE_STR_LEN-1, &read, NULL))
			{
				buffer[read] = '\0';
				responseStr += buffer;
			}
			CloseHandle(hOutput);
			
			GetExitCodeProcess(hProcess, &m_dwLastExitCode);
			CloseHandle(hProcess);

			USES_CONVERSION;
			_sntprintf_s(m_tResponseBuf, m_ResponseBufLen, _TRUNCATE, _T("%wS"), A2T( responseStr.c_str() )  ); 
			BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("*Response line %d: %s"), __LINE__, m_tResponseBuf );
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s ", __LINE__, L"Execut");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s(%s) exception OUT"), __WFUNCTION__, commandLine);	
		}

        BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s(%s) OUT"), __WFUNCTION__, commandLine);	
		return TRUE;
	}


	BOOL ExecElevatedShell(HWND hwnd, LPCTSTR pszPath, LPCTSTR pszParameters = NULL, LPCTSTR pszDirectory = NULL )
	{
		WriteLog(__WFUNCTION__, TRACESTACK);

		//Function Parameters:
		// hwnd: can be null. if non null then response messages will be send to the handle
		//pszPath: its command. make sure there are no white spaces any where in the command
		//pszParamaeters example: shex.lpParameters = "An example: \"\"\"quoted text\"\"\"";

		SHELLEXECUTEINFO shex;
		BOOL bRetv = FALSE;
		try
		{
			memset( &shex, 0, sizeof( shex) );

			shex.cbSize = sizeof( SHELLEXECUTEINFO );
			shex.fMask = 0;
			shex.hwnd = hwnd;
			shex.lpVerb = _T("runas");
			shex.lpFile = pszPath;
			shex.lpParameters = pszParameters;
			shex.lpDirectory = pszDirectory;
			//shex.nShow = SW_NORMAL;
			shex.nShow = SW_HIDE;
			bRetv = ::ShellExecuteEx( &shex );
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"ExecElevatedShell");
			WriteLog(buf);
		}

		return bRetv;
	}

	BOOL StartAppAsUser(TCHAR * tszCmdLine,bool show)
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	

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
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = show ? SW_SHOW : SW_HIDE; // also requires si.dwFlags = STARTF_USESHOWWINDOW
		ZeroMemory(&pi, sizeof(pi));

		LPVOID  pEnv = NULL;
		DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

		size_t iMaxPath = MAX_PATH + m_strAppPath.length();
		m_pTempPath = (TCHAR *)new TCHAR[iMaxPath];
		if (!m_pTempPath)
		{
			BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CaptureIP new failed, GetLastError %d"), GetLastError() );
			return FALSE;
		}

		memset(m_pTempPath,'\0',iMaxPath);

		_stprintf_s(m_pTempPath, iMaxPath, _T("%s"),m_strAppPath.c_str());

		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s, cmd: %s "), __WFUNCTION__, m_pTempPath);	

		if ( !EnumProcesses( aProcesses, sizeof(aProcesses), &cbNeeded ) )
		{
			BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CaptureIP EnumProcess failed, GetLastError %d"), GetLastError() );
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

					// No need to log name of each process on system
					//BaseLogger::WriteFormattedLog(TRACE_ETHERNET, szProcessName);

					//if (_tcsicmp(szProcessName, _T("UtsEngine.exe")) == 0)
					if (_tcsicmp(szProcessName, _T("UtsConfigManager.exe")) == 0)
					{
						if (ProcessIdToSessionId(aProcesses[i],&dwSessionId))
						{
							if (true)//dwSessionId == dwCurrSessionId)
							{
								if (OpenProcessToken(hProcess, TOKEN_QUERY|TOKEN_IMPERSONATE|TOKEN_DUPLICATE, &hToken))
								{
									SECURITY_ATTRIBUTES sa;
									sa.bInheritHandle = false;
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									if (DuplicateTokenEx(hToken,GENERIC_ALL,&sa,SecurityIdentification,TokenPrimary,&hTokenDup))
									{
										if ( !CreateProcessAsUser(hTokenDup,NULL,m_pTempPath,&sa,&sa,FALSE,dwCreationFlag,pEnv,NULL,&si,&pi) )
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


											_stprintf_s(tszMsg, TSIZEOF(tszMsg), _T("ERROR CaputeIP, %s (%u) \r\n"), pLastErrMsg, ::GetLastError() );
											BaseLogger::WriteFormattedLog(TRACE_ETHERNET, tszMsg );
											if ( pLastErrMsg )
												LocalFree(pLastErrMsg); /*free memory*/
											CloseHandle( hProcess );
											CloseHandle(hToken);
											break;
										}
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CaptureIP CreateProcessAsUser OK") );
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

		delete[] m_pTempPath;

		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	

		return retVal;
	}

	BOOL StartAppAsUserAndWait(TCHAR * tszCmdLine, bool show, int iWaitTimeout) // waits for specified time or till application exits
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);

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

		ZeroMemory(&si, sizeof(STARTUPINFO));
		si.cb = sizeof(STARTUPINFO);
		si.dwFlags = STARTF_USESHOWWINDOW;
		si.wShowWindow = show ? SW_SHOW : SW_HIDE; // also requires si.dwFlags = STARTF_USESHOWWINDOW
		ZeroMemory(&pi, sizeof(pi));

		LPVOID  pEnv = NULL;
		DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;

		size_t iMaxPath = MAX_PATH + m_strAppPath.length();
		m_pTempPath = (TCHAR *)new TCHAR[iMaxPath];
		if (!m_pTempPath)
		{
			BaseLogger::WriteFormattedLog(_T("CaptureIP new failed, GetLastError %d"), GetLastError());
			return FALSE;
		}

		memset(m_pTempPath, '\0', iMaxPath);

		_stprintf_s(m_pTempPath, iMaxPath, _T("%s"), m_strAppPath.c_str());

		BaseLogger::WriteFormattedLog(_T("%s, cmd: %s "), __WFUNCTION__, m_pTempPath);

		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CaptureIP EnumProcess failed, GetLastError %d"), GetLastError());
			return retVal;
		}

		// Calculate how many process identifiers were returned.
		cProcesses = cbNeeded / sizeof(DWORD); 

		for (unsigned int i = 0; i < cProcesses; i++)
		{
			if (aProcesses[i] != 0)
			{
				TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

				// Get a handle to the process.

				dwSessionId = 0;
				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
					PROCESS_VM_READ,
					FALSE, aProcesses[i]);

				// Get the process name.
				if (NULL != hProcess)
				{
					GetModuleBaseName(hProcess, 0, szProcessName,
						TSIZEOF(szProcessName));

					// No need to log name of each process on system
					//BaseLogger::WriteFormattedLog(TRACE_ETHERNET, szProcessName);

					//if (_tcsicmp(szProcessName, _T("UtsEngine.exe")) == 0)
					if (_tcsicmp(szProcessName, _T("UtsConfigManager.exe")) == 0)
					{
						if (ProcessIdToSessionId(aProcesses[i], &dwSessionId))
						{
							if (true)//dwSessionId == dwCurrSessionId)
							{
								if (OpenProcessToken(hProcess, TOKEN_QUERY | TOKEN_IMPERSONATE | TOKEN_DUPLICATE, &hToken))
								{
									SECURITY_ATTRIBUTES sa;
									sa.bInheritHandle = false;
									sa.nLength = sizeof(sa);
									sa.lpSecurityDescriptor = NULL;
									if (DuplicateTokenEx(hToken, GENERIC_ALL, &sa, SecurityIdentification, TokenPrimary, &hTokenDup))
									{
										if (!CreateProcessAsUser(hTokenDup, NULL, m_pTempPath, &sa, &sa, FALSE, dwCreationFlag, pEnv, NULL, &si, &pi))
										{
											TCHAR tszMsg[2048];  tszMsg[0] = 0;
											TCHAR * pLastErrMsg = _T("");
											FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
												0,
												::GetLastError(),
												MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
												(LPTSTR)&pLastErrMsg,
												0,
												NULL);

											_stprintf_s(tszMsg, TSIZEOF(tszMsg), _T("ERROR CaputeIP, %s (%u) \r\n"), pLastErrMsg, ::GetLastError());
											BaseLogger::WriteFormattedLog(TRACE_ETHERNET, tszMsg);
											if (pLastErrMsg)
												LocalFree(pLastErrMsg); /*free memory*/				   

											CloseHandle(hProcess);
											CloseHandle(hToken);
											break;
										}
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("CaptureIP CreateProcessAsUser OK"));
										
										// waits for specified time or till application exits
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, L"Start waiting for cmd: '%s', time: %d", tszCmdLine, clock());

										DWORD dwExitWaitRetValue = WaitForSingleObject(pi.hProcess, iWaitTimeout);
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, L"dwExitWaitRetValue %d", dwExitWaitRetValue);

										DWORD dwLastExitCode = 0;
										BOOL bRetv = GetExitCodeProcess(pi.hProcess, &dwLastExitCode);
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, L"GetExitCodeProcess bRetv %d, ExitCode %d", bRetv, dwLastExitCode);
										
										BaseLogger::WriteFormattedLog(TRACE_ETHERNET, L"Stopped waiting for cmd: '%s', time: %d", tszCmdLine, clock());

										m_hNewProc = pi.hProcess;
										m_hNewThread = pi.hThread;
										m_dwNewProcessID = pi.dwProcessId;
										m_dwNewThreadID = pi.dwThreadId;
										CloseHandle(hProcess);
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

				CloseHandle(hProcess);
			}
		}

		delete[] m_pTempPath;

		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);

		return retVal;
	}

	BOOL ShouldRunHuddlyIQ()
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);

		DWORD aProcesses[1024], cbNeeded, cProcesses;

		BOOL retVal = FALSE;

		LPVOID  pEnv = NULL;
		DWORD dwCreationFlag = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;


		if (!EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded))
		{
			BaseLogger::WriteFormattedLog(L"ERROR: ShouldRunHuddlyIQ failed to Enumerate Processes." );
			return retVal;
		}

		// Calculate how many process identifiers were returned.
		cProcesses = cbNeeded / sizeof(DWORD);

		for (unsigned int i = 0; i < cProcesses; i++)
		{
			if (aProcesses[i] != 0)
			{
				TCHAR szProcessName[MAX_PATH] = L"<unknown>";

				// Get a handle to the process.

				HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, aProcesses[i]);

				// Get the process name.
				if (NULL != hProcess)
				{
					GetModuleBaseName(hProcess, 0, szProcessName, sizeof(szProcessName) / sizeof(TCHAR));

					if (_tcsicmp(szProcessName, L"<unknown>") != 0)
						_tprintf(L"%s  \n", szProcessName);

					if (_tcsicmp(szProcessName, L"HuddlyIQ.exe") == 0)
					{
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("HuddlyIQ.exe is already running, %s"), __WFUNCTION__);
						return false;
					}
					if (_tcsicmp(szProcessName, L"LaunchHuddlyUpdater.exe") == 0)
					{
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("LaunchHuddlyUpdater.exe is already running, %s"), __WFUNCTION__);
						return false;
					}
					if (_tcsicmp(szProcessName, L"hdc.exe") == 0)
					{
						BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("hdc.exe is already running, %s"), __WFUNCTION__);
						return false;
					}
				}
			}
		}

		BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("HuddlyIQ.exe, LaunchHuddlyUpdater.exe or hdc.exe is not running, %s"), __WFUNCTION__);

		return true;
	}

protected:

	HANDLE SpawnAndRedirect(LPCTSTR commandLine, HANDLE *hStdOutputReadPipe, LPCTSTR lpCurrentDirectory, DWORD dwMsTimeout = INFINITE)
	{
		BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s IN"), __WFUNCTION__);	

		HANDLE hStdOutputWritePipe, hStdOutput, hStdError;
		PROCESS_INFORMATION pi;
		STARTUPINFO si;

		try
		{
			CreatePipe(
				hStdOutputReadPipe, 
				&hStdOutputWritePipe, 
				NULL, 
				0);  // create a non-inheritable pipe

			DuplicateHandle(
				GetCurrentProcess(), 
				hStdOutputWritePipe,                                  
				GetCurrentProcess(), 
				&hStdOutput, // duplicate the "write" end as inheritable stdout                                  
				0, 
				TRUE, 
				DUPLICATE_SAME_ACCESS);

			DuplicateHandle(
				GetCurrentProcess(), 
				hStdOutput,
				GetCurrentProcess(), 
				&hStdError, // duplicate stdout as inheritable stderr                                  
				0, 
				TRUE, 
				DUPLICATE_SAME_ACCESS);

			CloseHandle(hStdOutputWritePipe); // no longer need the non-inheritable "write" end of the pipe

			ZeroMemory(&si, sizeof(STARTUPINFO));
			si.cb = sizeof(STARTUPINFO);
			si.dwFlags = STARTF_USESTDHANDLES|STARTF_USESHOWWINDOW;
			si.hStdInput  = GetStdHandle(STD_INPUT_HANDLE); // (this is bad on a GUI app)
			si.hStdOutput = hStdOutput;
			si.hStdError  = hStdError;
			si.wShowWindow = SW_HIDE;  // IMPORTANT: hide subprocess console window

			TCHAR commandLineCopy[1024];  // CreateProcess requires a modifiable buffer

			//_tcscpy(commandLineCopy, commandLine);
			_tcscpy_s(commandLineCopy, TSIZEOF(commandLineCopy), commandLine);

			if (!CreateProcess( NULL, 
				commandLineCopy, 
				NULL, 
				NULL, 
				TRUE,                
				CREATE_NEW_CONSOLE, NULL, 
				lpCurrentDirectory, 
				&si, 
				&pi))
			{
				GetLastError();
				CloseHandle(hStdOutput);
				CloseHandle(hStdError);
				CloseHandle(*hStdOutputReadPipe);
				*hStdOutputReadPipe = INVALID_HANDLE_VALUE;
                BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s(%s) err1 OUT"), __WFUNCTION__, commandLine);	
				return NULL;
			}

			// Wait until child process exits.
			if(WAIT_TIMEOUT == WaitForSingleObject( pi.hProcess, dwMsTimeout) )
			{
				//??  timeout message 
				TerminateRunningProcess(pi.dwProcessId);
			}

			CloseHandle(pi.hThread);
			CloseHandle(hStdOutput);
			CloseHandle(hStdError);
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"SpawnAndRedirect");
			WriteLog(buf);
            BaseLogger::WriteFormattedLog(_T("%s OUT"), __WFUNCTION__);	
		}

        BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("%s OUT"), __WFUNCTION__);	
		return pi.hProcess;
	}

	DWORD TerminateRunningProcess(DWORD dwPID)
	{
		WriteLog(__WFUNCTION__, TRACE_ETHERNET|VERBOSE);
		//??Status.clear();

		HANDLE hProc;
		DWORD dwRet; enum {FAILED = 0, SUCCESS_CLEAN, SUCCESS_KILL};
		DWORD dwTimeout = 5*1000;

		try
		{
			// get process handle with termination rights
			hProc = OpenProcess(SYNCHRONIZE|PROCESS_TERMINATE, FALSE, dwPID);
			if(hProc == NULL)
			{
				//?? Status.push_back("ERROR: Failed to get handle to the process.\r\n");
				dwRet = FAILED ;
			}
			else // got process handle with termination rights
			{

				// TerminateAppEnum() posts WM_CLOSE to all windows whose PID matches with process's.
				EnumWindows( (WNDENUMPROC)TerminateAppEnum, (LPARAM) dwPID) ;

				// Wait on the handle. If it signals, great. If it times out, then kill it.
				if(WaitForSingleObject(hProc, dwTimeout)!=WAIT_OBJECT_0)
				{
					dwRet = TerminateProcess(hProc,0);
					if(dwRet == TRUE)
					{
						dwRet = SUCCESS_KILL;
						//?? Status.push_back("Terminated CaptureIP process. Check if data is saved\r\n");
					}
					else
					{
						dwRet = FAILED;
						//?? Status.push_back("ERROR: Failed to terminated the process CaptureIP. \r\n");
					}
				}
				else
				{
					dwRet = SUCCESS_CLEAN ;
					//?? Status.push_back("Successfully stopped the process CaptureIP. Data is saved\r\n");
				}

				CloseHandle(hProc) ;
			}
		}
		catch(...)
		{
			TCHAR buf[MAX_PATH];
			_stprintf_s(buf, TSIZEOF(buf), L"catch(...) %d  %s \r\n", __LINE__, L"TerminateRunningProcess");
			WriteLog(buf);
		}


		return dwRet ;
	}

	// Declare Callback Enum Functions.
	static BOOL CALLBACK TerminateAppEnum( HWND hwnd, LPARAM lParam )
	{

		DWORD dwID ;
		GetWindowThreadProcessId(hwnd, &dwID) ;
		if(dwID == (DWORD)lParam)
		{
			PostMessage(hwnd, WM_CLOSE, 0, 0) ;
		}
		return TRUE ;
	}

	int WriteLog( TCHAR msg[], int level=TRACEALL )
	{

		BaseLogger::WriteLog(level, msg);

		return 0;
	}

};

