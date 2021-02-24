#include "stdafx.h"
#include "CEthernet.h"
#include "CDomain.h"


CmPCConfig_DeclSpec BOOL CfgGetNbrOfAdapters(int &nbrOfAdapter, tstring& tResponse)
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    CEthernet cfg(_T(""));
    BOOL bRetv = FALSE;

    bRetv = cfg.GetNbrOfAdapter(nbrOfAdapter);

    cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(tstring tDescripton, CAdapterInfo *pAdapterInfo, tstring& tResponse)
{
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCSIG__);	
	tResponse.clear();
	TCHAR szAdapterName[MAX_PATH] = {0};
	_stprintf_s(szAdapterName, MAX_PATH, _T("%s"), tDescripton.c_str() );

	BOOL bValue = CfgGetAdapterInfo(szAdapterName, pAdapterInfo, tResponse);

	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCSIG__);

	return bValue;
		
}

CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(TCHAR byDescripton[], CAdapterInfo *pAdapterInfo, tstring& tResponse)
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCSIG__);	
	tResponse.clear();
    CAdapterInfo* pInfo = 0;

    CEthernet cfg(byDescripton);

    if(pAdapterInfo)
    {
        try
        {
            pInfo = cfg.GetAdapterInfo(byDescripton);
            if(pInfo)
                //*pAdapterInfo = *pInfo;
                cfg.CopyFieldByField(*pAdapterInfo, *pInfo);
        }
        catch(...)
        {
            TCHAR buf[MAX_PATH];
            _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s OUT\r\n", __LINE__, __WFUNCTION__);
            cfg.SetResponseString(buf);
            BaseLogger::WriteLog(buf);
        }
    }

    BaseLogger::WriteFormattedLog(TRACE_ETHERNET|VERBOSE, _T("BEFORE cfg.GetResponseString "));	

    cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCSIG__);
    return pInfo ? TRUE : FALSE;
}

CmPCConfig_DeclSpec BOOL CfgGetAdapterInfo(tstring tAdapterName, int nbrOfAdapter,  CAdapterInfo *pAdapterInfo, tstring& tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCSIG__);
	tResponse.clear();
    CAdapterInfo* pInfo = 0;
    CEthernet cfg(tAdapterName);

    if(pAdapterInfo)
    {
        try
        {
            pInfo = cfg.GetAdapterInfo(nbrOfAdapter);
            if(pInfo)
                //*pAdapterInfo = *pInfo;
                cfg.CopyFieldByField(*pAdapterInfo, *pInfo);
        }
        catch(...)
        {
            TCHAR buf[MAX_PATH];
            _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
            cfg.SetResponseString(buf);
            BaseLogger::WriteLog(buf);
        }
    }

    cfg.GetResponseString(tResponse);

    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCSIG__);
    return pInfo ? TRUE : FALSE;
}

CmPCConfig_DeclSpec BOOL CfgPrintfAllFields(tstring tAdapterName, CAdapterInfo& AdapterInfo, tstring& tResponse)
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);		
	tResponse.clear();
    CEthernet cfg(tAdapterName);

    cfg.PrintfAllFields(AdapterInfo, tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return TRUE;
}

CmPCConfig_DeclSpec BOOL CfgEnableAllDhcp(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    bRetv = cfg.EnableAllDhcp(InterfaceName);

    cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;

}

CmPCConfig_DeclSpec BOOL CfgEnableDhcpIPAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    cfg.EnableDhcpIPAddress(InterfaceName);

    bRetv = cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgEnableDhcpDnsAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse )
{
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
	CEthernet cfg(tAdapterName);
	BOOL bRetv = FALSE;

    bRetv = cfg.EnableDhcpDnsAddress(InterfaceName);

    bRetv = cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgEnableDhcpWinsAddress(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse )
{
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
	CEthernet cfg(tAdapterName);
	BOOL bRetv = FALSE;

    bRetv = cfg.EnableDhcpWinsAddress(InterfaceName);

    bRetv = cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetStaticAddresses(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], TCHAR gateway[],tstring& resultBuf,bool &status )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	resultBuf.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    try
    {
        bRetv = cfg.SetStaticAddresses(InterfaceName, IpAddr, mask, gateway,resultBuf,status) ;
    }
    catch(...)
    {
        TCHAR buf[MAX_PATH];
        _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
        cfg.SetResponseString(buf);
        BaseLogger::WriteLog(buf);
        BaseLogger::WriteFormattedLog(_T("%s exception OUT"), __WFUNCTION__);	
    }

    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetStaticIPAddressAndMask(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[], TCHAR mask[], tstring& resultBuf, bool &status)
{
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);
	resultBuf.clear();
	CEthernet cfg(tAdapterName);
	BOOL bRetv = FALSE;

	try
	{
		bRetv = cfg.SetStaticIPAddressAndMask(InterfaceName, IpAddr, mask, resultBuf, status);
	}
	catch (...)
	{
		TCHAR buf[MAX_PATH];
		_stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
		cfg.SetResponseString(buf);
		BaseLogger::WriteLog(buf);
		BaseLogger::WriteFormattedLog(_T("%s exception OUT"), __WFUNCTION__);
	}

	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);
	return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetStaticIPAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR IpAddr[],tstring& resultBuf ,bool &status)
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	resultBuf.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    try
    {
        bRetv = cfg.SetStaticIPAddress(InterfaceName, IpAddr,resultBuf,status);
    }
    catch(...)
    {
        TCHAR buf[MAX_PATH];
        _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
        cfg.SetResponseString(buf);
        BaseLogger::WriteLog(buf);
    }
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

CmPCConfig_DeclSpec BOOL CfgSetStaticDnsAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR DnsAddres[],   tstring& resultBuf,bool &status, BOOL bClearCurAddresses )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	resultBuf.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    try
    {
        if(bClearCurAddresses)
            cfg.ClearDnsAddresses(InterfaceName);// will clear Dns table

        bRetv = cfg.SetStaticDnsAddress(InterfaceName, DnsAddres, resultBuf,status, bClearCurAddresses);
    }
    catch(...)
    {
        TCHAR buf[MAX_PATH];
        _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
        cfg.SetResponseString(buf);
        BaseLogger::WriteLog(buf);
    }

    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;

    //return
    //	cfg.CheckForSuccess( _T("Ok") );
}

CmPCConfig_DeclSpec BOOL CfgSetStaticWinsAddress(tstring tAdapterName, TCHAR InterfaceName[], TCHAR WinsAddres[], tstring& resultBuf,bool &status, BOOL bClearCurAddresses)
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	resultBuf.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    try
    {
        if(bClearCurAddresses) // Clear all current addresses
            cfg.ClearWinsAddresses(InterfaceName); // will clear Wins table

        bRetv = cfg.SetStaticWinsAddress(InterfaceName, WinsAddres, resultBuf,status, bClearCurAddresses);
    }
    catch(...)
    {
        TCHAR buf[MAX_PATH];
        _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
        cfg.SetResponseString(buf);
        BaseLogger::WriteLog(buf);
    }
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;

    //return 
    //	cfg.CheckForSuccess( _T("Ok") );
}

CmPCConfig_DeclSpec BOOL CfgCaptureIP(tstring tAdapterName, TCHAR cmd[], TCHAR param[] )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    bRetv = cfg.CaptureIP(cmd, param);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

BOOL CfgCaptureEthernet(tstring tAdapterName, TCHAR cmd[], TCHAR param[] )
{
	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	CEthernet cfg(tAdapterName);
	BOOL bRetv = FALSE;

	bRetv = cfg.StartAppAsUser(cmd, true);

	BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);

	return bRetv;
}

//
// returns true if the specifiged interface is connected
//
CmPCConfig_DeclSpec BOOL CfgGetInterfaceStatus(tstring tAdapterName, TCHAR InterfaceName[], tstring& tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    //"Netsh interface show interface name="Local Area Connect 5"  ", will display line by line each connection and its tatus
    //"Netstate -n

    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    try
    {
        cfg.GetInterfaceStatus(InterfaceName );
    }
    catch(...)
    {
        TCHAR buf[MAX_PATH];
        _stprintf_s(buf, TSIZEOF(buf), L"catch(...) exception %d  %s \r\n", __LINE__, __WFUNCTION__);
        cfg.SetResponseString(buf);
        BaseLogger::WriteLog(buf);
    }

    bRetv = cfg.CheckForSuccess( _T("Connected") );

    cfg.GetResponseString(tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

//
// returns true if the specifiged interface is connected
//
CmPCConfig_DeclSpec BOOL CfgShowProcesses(tstring tAdapterName, tstring cmd, tstring& tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    bRetv = cfg.ShowProcesses(cmd, tResponse );
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return TRUE;
}

BOOL CfgFirewall(tstring tAdapterName, tstring tCmd, tstring &tResponse )
{
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s IN"), __WFUNCTION__);	
	tResponse.clear();
    CEthernet cfg(tAdapterName);
    BOOL bRetv = FALSE;

    bRetv = cfg.Firewall(tCmd, tResponse);
    BaseLogger::WriteFormattedLog(TRACE_ETHERNET, _T("%s OUT"), __WFUNCTION__);	
    return bRetv;
}

BOOL CfgIpConfigAllLog()
{
   CEthernet cfg(_T("NoEnumeration"));
  return  cfg.IpConfigAll();
}


CmPCConfig_DeclSpec BOOL CfgHandleDirectConnect(TCHAR mode[], tstring& tResponse)
{
	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s IN"), __WFUNCTION__);
	BOOL bReturnValue = FALSE;
	tResponse.clear();

	CEthernetExt et;
	et.fGetAdapterInfo();
	et.fGetAdpaterAddresses();

	if (et.IsConnected())
	{
		// Change the update manifest to use our link local address, so direct connecteed devices can update from us without hostname lookup
		CfgUpdateManifest(_T("169.254.0.1"));
	}
	else
	{
		// Change the update manifest back to using our hostname so devices on the real network can update
		CfgUpdateManifest();
	}


	if (!et.NeedsChange())
		return 0;

	TCHAR cmd[MAX_PATH] = {};
	if (_tcsicmp(mode, L"static") == 0)
	{
		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set address name=\"%s\" source=static addr=169.254.0.1 mask=255.255.0.0 gateway=0.0.0.0 1", et.ConnectioName());
		bReturnValue = et.StartAppAsUserAndWait(cmd, false, 5000);
		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set dns name=\"%s\" source=static  0.0.0.0", et.ConnectioName());
		bReturnValue = et.StartAppAsUserAndWait(cmd, false, 5000);
		BaseLogger::WriteFormattedLog(L"DirectConnect: static settings applied");
	}
	else if (_tcsicmp(mode, L"dhcp") == 0)
	{
		_stprintf_s(cmd, MAX_PATH, L"/C netsh interface ip set ADDRESS name = \"%s\" source=dhcp", et.ConnectioName());
		bReturnValue = et.StartAppAsUserAndWait(cmd, false, 5000);
		_stprintf_s(cmd, MAX_PATH, L"C:\\Windows\\System32\\cmd.exe /C netsh interface ip set dns name=\"%s\" source=dhcp", et.ConnectioName());
		bReturnValue = et.StartAppAsUserAndWait(cmd, false, 5000);
		BaseLogger::WriteFormattedLog(L"DirectConnect: dhcp settings applied");
	}
	else
	{
		BaseLogger::WriteFormattedLog(L"%s: unhandled parameters", __WFUNCTION__);
		return 0;
	}

	BaseLogger::WriteFormattedLog(TRACESTACK, _T("%s OUT"), __WFUNCTION__);

	return bReturnValue;
}

CmPCConfig_DeclSpec BOOL CfgCaptureUSB(TCHAR cmd[], bool bShow)
{
	BaseLogger::WriteFormattedLog(L"%s IN", __WFUNCTION__ );

	CExecuteCommand ex;

	BOOL bRetv = ex.StartAppAsUser(cmd, bShow);

	BaseLogger::WriteFormattedLog(L"%s OUT", __WFUNCTION__);

	return bRetv;
}

