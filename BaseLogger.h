#pragma once

#include "stdafx.h"
#include "SharedDefs.h"
//#include "Windows.h"

using namespace std;


#define DEFAULT_CONFIG_LOGGING_DIR  	_T("C:\\LOGS\\")
#define CONFIG_LOGGING_FILE  			_T("Service.log")
#define CONFIG_LOGGING_BACKUP  			_T("ServiceBak.log")

#define TSIZEOF(a) sizeof(a)/sizeof(TCHAR)
#ifdef tstring
#undef tstring
typedef std::basic_string <TCHAR> tstring;
#endif

class  CmPCConfig_DeclSpec  BaseLogger
	//: public CBaseLogger
{
public:
	BaseLogger();
	~BaseLogger();
	
	static inline void BaseLogger::EnterCS(){ EnterCriticalSection(&m_LoggingServiceCS); }
	static inline void BaseLogger::LeaveCS(){ LeaveCriticalSection(&m_LoggingServiceCS); }
	
	static void init();
	static void SetTraceFlags(int iTraceFlags);
    static int GetTraceFlags();
	
	static void WriteLog(TCHAR* pMsg);
	static void WriteLog(int iTraceFlag, TCHAR* pMsg);
	static void WriteFormattedLog(int iTraceFlag, TCHAR* pMsg, ...);
	static void WriteFormattedLog(TCHAR* pMsg, ...);

	static int filter(unsigned int code, struct _EXCEPTION_POINTERS *ep);
	static void FileSizeCheck(TCHAR* pMsg);
protected:
	static bool FileExists(LPCTSTR pFileName);
	static CRITICAL_SECTION m_LoggingServiceCS;
	static int m_iTraceFlag;
	static std::wstring m_LogFile;
	static bool m_bStaticInitialized;};

