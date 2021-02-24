#pragma once


#include <string>
#include <iostream>
#include <fstream> 

////////////////////////////////////////////////////////////////////////////////////////////////
//
// Copied this class from CmUpxIF since we can't add the dependancy to this project
//
// The whole point of the update screen is to be self contained while other files are replaced
// so we can't rely on an external .dll.
//
// jmv - 10/24/13
//
////////////////////////////////////////////////////////////////////////////////////////////////




namespace std
{
	typedef wofstream       tofstream;
}


class CComponentLogger
{
public:
	CComponentLogger(void);
	//CComponentLogger(const TCHAR* tszFilename);	
	
	virtual ~CComponentLogger(void);

	static void Init(const TCHAR* ptsFilename);	
	static void WriteLine(const TCHAR* ptsLog, bool bTimestamp=true);
	static void WriteFormattedLine(TCHAR* pMsg, ...);

	
private:

	static tstring GetTimeStamp();
	static int GetLoggingLevel(int checkNow = 0);
	static void CheckFileSize(const TCHAR* ptsFile);

	static CRITICAL_SECTION*	m_pCS;
	static TCHAR*				m_ptsLogFilename;
	static DWORD m_dwLoggingLevel;
	static DWORD				m_nProcess;
	static DWORD				m_nLastCommitTick;



};
