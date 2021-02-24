#pragma once











class CmPCConfig_DeclSpec CCS200TempMonitor
{
public:
	CCS200TempMonitor(void);
	virtual ~CCS200TempMonitor(void);

	void Init();
	bool Start();

	unsigned long GetCPUTemp();
	unsigned long GetSysTemp();



protected:

	void KillMonitorThread();

	double Celsius_to_Fahrenheit(unsigned long ulCelsiusTemperature);
	
	
public:
	void GetProcessorTemperature(_TCHAR* ptszTemperature);
	void GetBoardTemperature(_TCHAR* ptszTemperature);

	HANDLE GetMonitorExitEvent()	{ return m_hMonitorExitEvent; };

protected:

	HANDLE			m_hTempMonitorThread;
	HANDLE			m_hMonitorExitEvent;







};

