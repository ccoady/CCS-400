////////////////////////////////////////////////////////////////////////////////////////////////////
/// @file        SystemApps.h
///
/// Header file for Time related configuration functions.
///
/// @author      William Levine
/// @date        07/12/04
////////////////////////////////////////////////////////////////////////////////////////////////////
#if !defined(_SYSTEMAPPS_H_)
#define _SYSTEMAPPS_H_

CmPCConfig_DeclSpec HRESULT CfgSystemAppSetIEEnablePopups(BOOL bEnable);
CmPCConfig_DeclSpec HRESULT CfgSystemAppGetIEEnablePopups(BOOL &bEnable);


#endif