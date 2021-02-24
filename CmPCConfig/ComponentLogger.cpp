#include "stdafx.h"


#include "ComponentLogger.h"
#include <assert.h>

#include <sys/stat.h>
#include <sys/timeb.h>
#include <time.h>
#include <sstream>
#include <atlbase.h> 

#define KB(x)((x) << 10)
#define FileBufferSize KB(1)

using namespace std;


TCHAR* CComponentLogger::m_ptsLogFilename = NULL;
CRITICAL_SECTION* CComponentLogger::m_pCS = NULL;
DWORD CComponentLogger::m_dwLoggingLevel = 0;
DWORD CComponentLogger::m_nProcess = 0;
DWORD CComponentLogger::m_nLastCommitTick = 0;

////////////////////////////////////////////////////////////////////////////////
// Parameters:	
// Returns:
// Description:	Default Constructor.  Called when logger is created on the stack.
//				This will result in log files written to the C: drive if Init is
//				not called.
//
// Author:      Jason Vourtsis - 03/19/2013
////////////////////////////////////////////////////////////////////////////////
CComponentLogger::CComponentLogger(void)
{		
}



////////////////////////////////////////////////////////////////////////////////
// Parameters:	strFilename		- the filename of the log file to create
// Returns:
// Description:	Called when logger is created on the heap and passed a logfile name.
//				The client doesn't need to call Init() this way, but needs to delete
//				the logger object.
//
// Author:      Jason Vourtsis - 03/19/2013
////////////////////////////////////////////////////////////////////////////////
//CComponentLogger::CComponentLogger(const TCHAR* tszFilename)
//{
//	Init(tszFilename);
//}



////////////////////////////////////////////////////////////////////////////////
// Parameters:	
// Returns:
// Description:	Destructor
//
// Author:      Jason Vourtsis - 03/19/2013
////////////////////////////////////////////////////////////////////////////////
CComponentLogger::~CComponentLogger(void)
{
	if (m_ptsLogFilename)
	{
		delete m_ptsLogFilename;
		m_ptsLogFilename = NULL;
	}

	if (m_pCS)
	{
		::DeleteCriticalSection(m_pCS);
		m_pCS = NULL;
	}
}



////////////////////////////////////////////////////////////////////////////////
// Parameters:	strFilename		- the filename of the log file to create
// Returns:
// Description:	Find our full filename in the firmware logs directory and cap the
//				size of our log files at 1 Megabyte.  If the current file is too large
//				copy it to a backup version and delete the current one.  Only keep
//				one current file and one backup.
//
// Author:      Jason Vourtsis - 03/19/2013
////////////////////////////////////////////////////////////////////////////////
void CComponentLogger::Init(const TCHAR* tszFilename)
{
	if (m_pCS)
		return;

	m_pCS = new CRITICAL_SECTION();
	::InitializeCriticalSection(m_pCS);

	::EnterCriticalSection(m_pCS);

	m_nProcess = ::GetCurrentProcessId();
	m_nLastCommitTick = ::GetTickCount();

	// Figure out our full filename
	TCHAR tszLogDir[MAX_PATH];
	if(GetEnvironmentVariable(_T("CRESTRON_LOG"), tszLogDir, MAX_PATH) ==  0)
		_stprintf(tszLogDir, _T("C:"));
	
	_tcscat_s(tszLogDir, _MAX_PATH, _T("\\"));
	_tcscat_s(tszLogDir, _MAX_PATH, tszFilename);

	// Save it to the static member
	m_ptsLogFilename = new TCHAR[_tcslen(tszLogDir)+1];
	_stprintf(m_ptsLogFilename, tszLogDir);
	
	// Back up the file if it gets too big
	CheckFileSize(m_ptsLogFilename);
	
	// Put a banner on the top of each run of the log
	WriteLine(_T(""), false);
	WriteLine(_T("****************************************************************"), false);
	
	tstring strInit = m_ptsLogFilename;
	strInit += _T(" Init");
	WriteLine(strInit.c_str());

	WriteLine(_T("****************************************************************"), false);

	// read from registry current logging level
	m_dwLoggingLevel = GetLoggingLevel(true);
	WriteFormattedLine(L"Enhanced Logging level is set to %s", m_dwLoggingLevel ? L"ON" : L"OFF");

	::LeaveCriticalSection(m_pCS);
}



////////////////////////////////////////////////////////////////////////////////
// Parameters:	
// Returns:
// Description:	Get and format the current timestamp from the system
//
// Author:      Jason Vourtsis - 03/15/2013
////////////////////////////////////////////////////////////////////////////////
tstring CComponentLogger::GetTimeStamp()
{
	SYSTEMTIME lt;
	TCHAR buf[MAX_PATH];
	::GetLocalTime(&lt);
	_stprintf(buf, _T("%02d/%02d/%04d, %02d:%02d:%02d:%03d %d\t"),lt.wMonth,lt.wDay,lt.wYear,lt.wHour,lt.wMinute,lt.wSecond,lt.wMilliseconds, m_nProcess);
	
	return buf;
	
}

int CComponentLogger::GetLoggingLevel(int checkNow)
{ 
	static DWORD skipCount = 0;

	if ((skipCount % 100 == 0) || checkNow > 0)
	{
#ifdef WritingToRegisterIsProblem
		CRegKey reg; DWORD value = 0;
		LPCTSTR hKey = L"SOFTWARE\\Crestron Electronics Inc.\\RigelSettings";
		LSTATUS status = reg.Open(HKEY_LOCAL_MACHINE, hKey, KEY_READ | KEY_WRITE | KEY_WOW64_64KEY);
		if (status == ERROR_SUCCESS)
		{
			status = reg.QueryDWORDValue(L"LoggingLevel", value);
			reg.Close();
			if (status == ERROR_SUCCESS)
			{
				if (m_dwLoggingLevel != value || checkNow > 0)
					//WriteFormattedLine(L"Enhanced Logging level is set to %s", value ? L"ON" : L"OFF");
					ATLTRACE(L"Enhanced Logging level is set to %s", value ? L"ON" : L"OFF");

				m_dwLoggingLevel = value;
			}
			else
				//WriteFormattedLine(L"ERROR: failed to query registry for logging level");
				ATLTRACE(L"ERROR: failed to query registry for logging level");
		}
		else
			//WriteFormattedLine(L"ERROR: failed to open registry for logging level");
			ATLTRACE(L"ERROR: failed to open registry for logging level");

#endif //WritingToRegisterIsProblem

		TCHAR inifile[MAX_PATH] = {};
		if (GetEnvironmentVariable(L"CRESTRON_USER", inifile, MAX_PATH) != 0)
		{
			_tcscat_s(inifile, MAX_PATH, L"\\hwSettings\\hw.dat");
			m_dwLoggingLevel = GetPrivateProfileInt(L"LoggingLevel", _T("Value"), 0x0, inifile);
		}
	}
	skipCount++;
 
	return m_dwLoggingLevel;
}


////////////////////////////////////////////////////////////////////////////////
// Parameters:	tszLog			- The message to be logged
//				bTimestamp		- Optional flag to turn logging timestamp on and off
// Returns:
// Description:	Log the string to a file in the log directory
//
// Author:      Jason Vourtsis - 03/15/2013
////////////////////////////////////////////////////////////////////////////////
void CComponentLogger::WriteLine(const TCHAR* tszLog, bool bTimestamp)
{
	// Make sure the client has called Init()
	assert(m_ptsLogFilename != NULL);
	
	::EnterCriticalSection(m_pCS);
	
	wstring wt(L"");
	if (bTimestamp)
		wt = GetTimeStamp();

	if (GetLoggingLevel() > 0)
	{
		wofstream(m_ptsLogFilename, ios::out | ios::app)
			<< wt << tszLog << endl;		
	}
	else // buffered logging
	{
		static wstringstream ss;

		DWORD totalBytes = wt.length() + wcslen(tszLog) + 1/*endln*/ + ss.tellp();
		DWORD nNow = ::GetTickCount();

		if ((totalBytes >= FileBufferSize) || ((nNow - m_nLastCommitTick) > 10000))
		{
			// flush ss messages to file
			wofstream(m_ptsLogFilename, ios::out | ios::app) << ss.rdbuf();
			//wofstream(m_ptsLogFilename, ios::out | ios::app) << nNow << " - " << m_nLastCommitTick << " = " << (nNow - m_nLastCommitTick) << " / " << totalBytes << ":" << FileBufferSize << endl;
			ss.clear();
			ss.str(L"");

			m_nLastCommitTick = nNow;
		}

		// buffer current message in ss
		ss << wt << tszLog << endl;
	}

	::LeaveCriticalSection(m_pCS);
}




////////////////////////////////////////////////////////////////////////////////
// Parameters:	tszLog			- The message to be logged
//				...				- Variable parameter list
// Returns:
// Description:	Log the string to a file in the log directory
//
// Author:      Jason Vourtsis - 03/15/2013
////////////////////////////////////////////////////////////////////////////////
void CComponentLogger::WriteFormattedLine(TCHAR* tszLog, ...)
{
	::EnterCriticalSection(m_pCS);

	TCHAR *pLogMsg = NULL;
	va_list args;
	va_start (args, tszLog);
	unsigned int iLen = _vsctprintf( tszLog, args )+ TSIZEOF(TCHAR);

	pLogMsg = new TCHAR[iLen];
	if (pLogMsg)
	{
		_vsntprintf( pLogMsg, iLen, tszLog, args );		
		WriteLine(pLogMsg);
		delete pLogMsg;
	}
	
	::LeaveCriticalSection(m_pCS);
}





////////////////////////////////////////////////////////////////////////////////
// Parameters:	ptsFile			- The full path to the log file to check
// Returns:
// Description:	If we have a file that's over 1 Meg, copy it to a backup and start over
//
// Author:      Jason Vourtsis - 03/15/2013
////////////////////////////////////////////////////////////////////////////////
void CComponentLogger::CheckFileSize(const TCHAR* tszFile)
{
	WIN32_FIND_DATA fd;
	HANDLE hFile = ::FindFirstFile(tszFile, &fd);
	
	if (INVALID_HANDLE_VALUE != hFile)
	{
		if (fd.nFileSizeLow > 1000000)
		{
			tstring strBackupFile = tszFile;
			strBackupFile += _T(".bak");
			CopyFile(tszFile, strBackupFile.c_str(), FALSE);			
			DeleteFile(tszFile);			
		}

		::FindClose(hFile);
	}
}
