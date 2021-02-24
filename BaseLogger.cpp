#include "stdafx.h"
#include "BaseLogger.h"

#define MB(x)((x)<< 20)

/*static */
CRITICAL_SECTION BaseLogger::m_LoggingServiceCS;
/*static */
int BaseLogger::m_iTraceFlag=0;
/*static */
std::wstring BaseLogger::m_LogFile=L"";
/*static*/
bool BaseLogger::m_bStaticInitialized = false;

BaseLogger::BaseLogger()
{

}

BaseLogger::~BaseLogger()
{
	::DeleteCriticalSection(&m_LoggingServiceCS);
}

/*static */void BaseLogger::init()
{
	try
	{
		::InitializeCriticalSection(&m_LoggingServiceCS);

		EnterCS();

		tstring LogDir = _T("");
		tstring LogDirBack = _T("");
		_TCHAR logDirStr[200];
		if(GetEnvironmentVariable(_T("CRESTRON_LOG"),logDirStr, TSIZEOF(logDirStr)) !=  0)
		{
			m_LogFile = logDirStr;
			m_LogFile += _T("\\");
			LogDirBack = logDirStr;
			LogDirBack += _T("\\");
		}
		else
		{
			m_LogFile = DEFAULT_CONFIG_LOGGING_DIR;
		}
		m_LogFile  +=  CONFIG_LOGGING_FILE;
		LogDirBack +=  CONFIG_LOGGING_BACKUP;

		// makes a backup of Service.log if its size is more than 3 MB
		struct _stat filestatus;
		_tstat(m_LogFile.c_str(), &filestatus );
		if( filestatus.st_size > MB(3) )
		{
			_tremove( LogDirBack.c_str() );
			_trename(m_LogFile.c_str(), LogDirBack.c_str() );
		}

		// Read trace flag from ini file for logging before engine start
		tstring location;
		TCHAR defaultDir[_MAX_PATH] = { 0 };
		if (GetEnvironmentVariable(_T("CRESTRON_INTERNAL_SETTINGS"), defaultDir, sizeof(defaultDir)) != 0)
			location = defaultDir;
		else
			location = _T("D:\\SETTINGS");

		location += _T("\\Console.ini");

		TCHAR tszDefault[] = _T("0");
		TCHAR tTraceFlag[MAX_PATH]= _T("");
		::GetPrivateProfileString(_T("UTS_CONFIG_LOGGING"), _T("TraceFlag"), tszDefault, tTraceFlag, MAX_PATH, location.c_str());
		m_iTraceFlag = _tstoi(tTraceFlag); // if 0 then trace flag will be TRACENONE

		m_bStaticInitialized = true;

		WriteLog(_T("  "));
		WriteLog(_T("***************************************************************"));
		WriteFormattedLog( _T("Service Logger created with TraceFlag %d (0x%X)"), m_iTraceFlag, m_iTraceFlag );
		WriteLog(_T("***************************************************************"));

	}
	catch(...)
	{
		OutputDebugString(_T("BaseLogger::init(TCHAR* pMsg)") );
		LeaveCS();
		return;
	}

	LeaveCS();
}

/*static */void BaseLogger::SetTraceFlags(int iTraceFlags)
{
	if(m_bStaticInitialized==false)
		return;

	// Write current trace flag to ini file
	tstring location;
	TCHAR defaultDir[_MAX_PATH] = { 0 };
	if (GetEnvironmentVariable(_T("CRESTRON_INTERNAL_SETTINGS"), defaultDir, sizeof(defaultDir)) != 0)
		location = defaultDir;
	else
		location = _T("D:\\SETTINGS");

	location += _T("\\Console.ini");

	TCHAR tszValue[16] = {0};
	_itot(iTraceFlags, tszValue, 10);
	::WritePrivateProfileString(_T("UTS_CONFIG_LOGGING"), _T("TraceFlag"), tszValue, location.c_str());

    m_iTraceFlag = iTraceFlags;
	WriteFormattedLog( _T("Service Logger TraceFlag set to %d (0x%X)"), m_iTraceFlag, m_iTraceFlag );

}

/*static */int BaseLogger::GetTraceFlags()
{
    return m_iTraceFlag;
}

/*static */void BaseLogger::WriteLog(TCHAR* pMsg)
{
	WriteLog(0, pMsg);
}

/*static */void BaseLogger::WriteLog(int iTraceFlag, TCHAR* pMsg)
{
    int threadId = GetCurrentThreadId();
	if(m_bStaticInitialized==false)
		return;

	__try
	{	
		EnterCS();

		//if((m_iTraceFlag & iTraceFlag) == 0){
		//	LeaveCS();
		//	return;
		//}
		if( !((m_iTraceFlag & iTraceFlag)==iTraceFlag) )
		{
			LeaveCS();
			return;
		}

		SYSTEMTIME oT;
		::GetLocalTime(&oT);
		FILE* pLog = _tfopen(m_LogFile.c_str(), _T("a") );
		if(pLog != NULL && pMsg != NULL)
		{
			if (_tcslen(pMsg) > 0)
				_ftprintf(pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\t%s\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, iTraceFlag, threadId,pMsg);
			else
				_ftprintf(pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\tEmpty String\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, iTraceFlag, threadId);
			fclose(pLog);
		}
	} 
	__except(filter(GetExceptionCode(), GetExceptionInformation()))
	{
		OutputDebugString(_T("CBaseLogger::WriteLog(TCHAR* pMsg)"));
		LeaveCS();
		return;

	}

	LeaveCS();
}

/*static */void BaseLogger::WriteFormattedLog(int iTraceFlag, TCHAR* pMsg, ...)
{
    int threadId = GetCurrentThreadId();
	if(m_bStaticInitialized==false)
		return;

	TCHAR *pLogMsg = NULL;

	__try
	{	
		EnterCS();

		/*if((m_iTraceFlag & iTraceFlag) == 0){
			::LeaveCriticalSection(&m_LoggingServiceCS);
			return;
		}*/

		if( !((m_iTraceFlag & iTraceFlag)==iTraceFlag) )
		{
			LeaveCS();
			return;
		}

		va_list args;
		va_start (args, pMsg);
		unsigned int iLen = _vsctprintf( pMsg, args )+ TSIZEOF(TCHAR);

		pLogMsg = new TCHAR[iLen];
		if (pLogMsg)
		{
			int iSize = _vsntprintf( pLogMsg,iLen, pMsg, args );
			SYSTEMTIME oT;
			::GetLocalTime(&oT);
			FILE* pLog = _tfopen(m_LogFile.c_str(),_T("a"));
			if(pLog != NULL && pMsg != NULL && iSize > 0)
			{
				if (_tcslen(pMsg) > 0)
					_ftprintf (pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\t%s\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, iTraceFlag, threadId,pLogMsg);
				else
					_ftprintf (pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\tEmpty String\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, iTraceFlag, threadId);
				fclose(pLog);
			}
			delete[] pLogMsg;
		}
	} 
	__except(filter(GetExceptionCode(), GetExceptionInformation())){

		OutputDebugString(_T("CBaseLogger::WriteFormattedLog(TCHAR* pMsg, ...) Exception... "));
		if (pLogMsg)
			delete[] pLogMsg;
		LeaveCS();
		return;


	}
	
	LeaveCS();
}

/*static */void BaseLogger::WriteFormattedLog(TCHAR* pMsg, ...)
{
    int threadId = GetCurrentThreadId();
	if(m_bStaticInitialized==false)
		return;

	TCHAR *pLogMsg = NULL;

	__try
	{	
		EnterCS();


		va_list args;
		va_start (args, pMsg);
		unsigned int iLen = _vsctprintf( pMsg, args )+ TSIZEOF(TCHAR);

		pLogMsg = new TCHAR[iLen];
		if (pLogMsg)
		{
			int iSize = _vsntprintf( pLogMsg,iLen, pMsg, args );
			SYSTEMTIME oT;
			::GetLocalTime(&oT);
			FILE* pLog = _tfopen(m_LogFile.c_str(),_T("a"));
			if(pLog != NULL && pMsg != NULL && iSize > 0)
			{
				if (_tcslen(pMsg) > 0)
					_ftprintf (pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\t%s\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, 255, threadId,pLogMsg);
				else
					_ftprintf (pLog,_T("%02d/%02d/%04d, %02d:%02d:%02d:%03d, %04X, t(%04d)\tEmpty String\n"),oT.wMonth,oT.wDay,oT.wYear,oT.wHour,oT.wMinute,oT.wSecond,oT.wMilliseconds, 255, threadId);
				fclose(pLog);
			}
			delete[] pLogMsg;
		}
	} 
	__except(filter(GetExceptionCode(), GetExceptionInformation())){

		OutputDebugString(_T("CBaseLogger::WriteFormattedLog(TCHAR* pMsg, ...)"));
		if (pLogMsg)
			delete[] pLogMsg;
		LeaveCS();
		return;

	}
	LeaveCS();
}


/*static */int BaseLogger::filter(unsigned int code, struct _EXCEPTION_POINTERS *ep)
{
	if(m_bStaticInitialized==false)
		return STATUS_USER_APC;

	OutputDebugString(_T("BaseLogger in filter."));
	if (code == EXCEPTION_ACCESS_VIOLATION)
	{
		OutputDebugString(_T("EXCEPTION_ACCESS_VIOLATION."));
		return EXCEPTION_EXECUTE_HANDLER;
	}
	else
	{
		OutputDebugString(_T("didn't catch AV, unexpected."));
		return EXCEPTION_CONTINUE_SEARCH;
	};
}

/*static */void BaseLogger::FileSizeCheck(TCHAR* pMsg)
{
	if(m_bStaticInitialized==false)
		return;

	__try
	{	
		EnterCS();

		FILE* pLog = _tfopen(m_LogFile.c_str(), _T("a") );
		if(pLog != NULL)
		{
			fseek(pLog, 0, SEEK_END);
			long  size = ftell(pLog);
			if(size >= 1000000)
				fseek(pLog, 0, SEEK_SET);
			fclose(pLog);

		}
	}
	__except(filter(GetExceptionCode(), GetExceptionInformation()))
	{
		OutputDebugString(_T("CBaseLogger::FileSizeCheck(TCHAR* pMsg)"));
		LeaveCS();
		return;
	}
	LeaveCS();
}


/*static */bool BaseLogger::FileExists(LPCTSTR pFileName)
{
	if(m_bStaticInitialized==false)
		return false;

	__try
	{	
		WIN32_FIND_DATA findData;

		HANDLE hSearch = FindFirstFile(pFileName, &findData); 
		if (hSearch != INVALID_HANDLE_VALUE) 
		{ 
			::FindClose(hSearch);
			return true;
		} 
	}
	__except(filter(GetExceptionCode(), GetExceptionInformation()))
	{
		OutputDebugString(_T("CBaseLogger::FileExists(LPCTSTR pFileName)"));
		LeaveCS();
		return false;

	}

	return false;
}




