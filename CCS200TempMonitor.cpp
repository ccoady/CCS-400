#include "StdAfx.h"
#include "CCS200TempMonitor.h"
#include "BaseLogger.h"
#include "ComponentLogger.h"

static HMODULE _hInst_ISIDll = NULL;
#include "ISIDll.h"

static int	MONITOR_LOG_INTERVAL_MINUTES = 15;


////////////////////////////////////////////////////////////////////////////////
//
// Parameters:	
//
// Returns:
//
// Description:	Temperature monitoring thread.  Run constantly and keep a record
//				of the CPU and mainboard temperature on set intervals.
//
// Author:      Jason Vourtsis
//
////////////////////////////////////////////////////////////////////////////////
DWORD WINAPI TempMonitorThreadProc(LPVOID pVoid)
{
	BaseLogger::WriteLog(_T("Starting CCS200 Temperature Monitoring Thread."));
	CComponentLogger::Init(_T("Temperature.log"));

	int nCPUTemp;
	int nBoardTemp;

	CCS200TempMonitor* pMonitor = (CCS200TempMonitor*)pVoid;

	nCPUTemp = pMonitor->GetCPUTemp();
	nBoardTemp = pMonitor->GetSysTemp();
	CComponentLogger::WriteFormattedLine(_T("CPU: %d C Board: %d C"), nCPUTemp, nBoardTemp);

	// Loop every x seconds until we get signalled to end
	while (WAIT_TIMEOUT == ::WaitForSingleObject(pMonitor->GetMonitorExitEvent(), (MONITOR_LOG_INTERVAL_MINUTES * 60 * 1000) ))
	{
		nCPUTemp = pMonitor->GetCPUTemp();
		nBoardTemp = pMonitor->GetSysTemp();
		CComponentLogger::WriteFormattedLine(_T("CPU: %d C Board: %d C"), nCPUTemp, nBoardTemp);		
	}

	BaseLogger::WriteLog(_T("Ending CCS200 Temperature Monitoring Thread."));
	
	
	return 0;
}








CCS200TempMonitor::CCS200TempMonitor(void)
	: m_hTempMonitorThread(NULL)
	, m_hMonitorExitEvent(NULL)
{
	m_hMonitorExitEvent = ::CreateEvent(NULL, TRUE, FALSE, NULL);
	
}


CCS200TempMonitor::~CCS200TempMonitor(void)
{
	KillMonitorThread();

	::CloseHandle(m_hMonitorExitEvent);
	m_hMonitorExitEvent = NULL;
}


void CCS200TempMonitor::Init()
{
	if (_hInst_ISIDll == NULL)
	{
		_hInst_ISIDll = LoadLibrary(_T("CR101-CST_ISIDLL.dll"));
		if(_hInst_ISIDll == NULL)
		{
			int last = GetLastError();
			BaseLogger::WriteFormattedLog(_T("CCS200TempMonitor::Init - LoadLibrary failed to load file CR101-CST_ISIDLL.dll (%d)"), last);			
		}
	}

}




bool CCS200TempMonitor::Start()
{	
	::ResetEvent(m_hMonitorExitEvent);
	m_hTempMonitorThread = ::CreateThread(NULL, 0, &TempMonitorThreadProc, this, 0, NULL);
	
	return true;
}





////////////////////////////////////////////////////////////////////////////////
//
// Parameters:	
// Returns:
//
// Description:	Clean up our Temperature Monitor thread
//
// Author:      Jason Vourtsis
//
////////////////////////////////////////////////////////////////////////////////
void CCS200TempMonitor::KillMonitorThread()
{
	if (m_hTempMonitorThread)
	{
		::SetEvent(m_hMonitorExitEvent);
		
		if (WaitForSingleObject(m_hTempMonitorThread, 500) != WAIT_OBJECT_0)
			TerminateThread(m_hTempMonitorThread, 42);
				
		CloseHandle(m_hTempMonitorThread);
		m_hTempMonitorThread = NULL;
	}
}






unsigned long CCS200TempMonitor::GetCPUTemp()
{
	Init();

	(FARPROC &) HM_GetCpuTemp = GetProcAddress(_hInst_ISIDll, "HM_GetCpuTemp");        
	return HM_GetCpuTemp();
}



unsigned long CCS200TempMonitor::GetSysTemp()
{
	Init();

	(FARPROC &) HM_GetSysTemp = GetProcAddress(_hInst_ISIDll, "HM_GetSysTemp");
	return HM_GetSysTemp();
}




///////////////////////////////////////////////////////////////////////////////////
///
/// <description/>	Convert Celsius (or Centigrade) Temperature to Fahrenheit
/// <param=			ulCelsiusTemperature/> unsigned long Celsius temperature
///	<return=		ulFTemperature/> double Fahrenheit Temperature value.
/// <author/>		Michael Rinaldi
/// <date/>			15-July-2008
///
///////////////////////////////////////////////////////////////////////////////////
double CCS200TempMonitor::Celsius_to_Fahrenheit(unsigned long ulCelsiusTemperature)
{
	double	dblFTemperature = 0.0;
	dblFTemperature = (((9*(double)ulCelsiusTemperature))/5) + 32.0;
	return	dblFTemperature;
}

////////////////////////////////////////////////////////////////////////////////
// Function:	 GetProcessorTemperature()
// Parameters:   
// Returns:      ptszTemperature - the temperature
// Description:  Retrieves the CPU temperature
// Author:       Jeff Spruiel 9/27/2013
////////////////////////////////////////////////////////////////////////////////
void CCS200TempMonitor::GetProcessorTemperature(_TCHAR* ptszTemperature)	// Get and format processor board temperature
{
    unsigned long	ulTemperature = GetCPUTemp();
	double	dblFTemperature = Celsius_to_Fahrenheit(ulTemperature);

    _stprintf(ptszTemperature, L"%6.2fF (%6.2fC)", dblFTemperature, (double)ulTemperature);
}

////////////////////////////////////////////////////////////////////////////////
// Function:	 GetBoardTemperature()
// Parameters:   
// Returns:      ptszTemperature - the temperature
// Description:  Retrieves the board temperature
// Author:       Jeff Spruiel 9/27/2013
////////////////////////////////////////////////////////////////////////////////
void CCS200TempMonitor::GetBoardTemperature(_TCHAR* ptszTemperature)	// Get and format main board temperature
{
    unsigned long	ulTemperature = GetSysTemp();
	double	dblFTemperature = Celsius_to_Fahrenheit(ulTemperature);
	_stprintf(ptszTemperature, L"%6.2fF (%6.2fC)", dblFTemperature, (double)ulTemperature);
}