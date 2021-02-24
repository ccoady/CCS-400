#pragma once
#include "stdafx.h"

#include <iostream>
#include <sstream>
#include <vector>
#include <tchar.h>
#include <string>
#include <atlstr.h>

// Windows Vista/7 audio api
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>

#include "AudioErrorDefs.h"
#include <audioclient.h>
#include "CommonDefs.h"
#include "BaseLogger.h"

#define EXIT_ON_ERROR(hres)  \
              if (FAILED(hres)) { goto Exit; }
#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }
#define LogHostError(HRES, func) __LogHostError(HRES, func)
static HRESULT __LogHostError(HRESULT res, TCHAR *func)
{
    USES_CONVERSION;

    const TCHAR *text = NULL;
    switch (res)
    {
    case S_OK: return res;
    case E_POINTER                              :text =_T("E_POINTER"); break;
    case E_INVALIDARG                           :text =_T("E_INVALIDARG"); break;

    case AUDCLNT_E_NOT_INITIALIZED              :text =_T("AUDCLNT_E_NOT_INITIALIZED"); break;
    case AUDCLNT_E_ALREADY_INITIALIZED          :text =_T("AUDCLNT_E_ALREADY_INITIALIZED"); break;
    case AUDCLNT_E_WRONG_ENDPOINT_TYPE          :text =_T("AUDCLNT_E_WRONG_ENDPOINT_TYPE"); break;
    case AUDCLNT_E_DEVICE_INVALIDATED           :text =_T("AUDCLNT_E_DEVICE_INVALIDATED"); break;
    case AUDCLNT_E_NOT_STOPPED                  :text =_T("AUDCLNT_E_NOT_STOPPED"); break;
    case AUDCLNT_E_BUFFER_TOO_LARGE             :text =_T("AUDCLNT_E_BUFFER_TOO_LARGE"); break;
    case AUDCLNT_E_OUT_OF_ORDER                 :text =_T("AUDCLNT_E_OUT_OF_ORDER"); break;
    case AUDCLNT_E_UNSUPPORTED_FORMAT           :text =_T("AUDCLNT_E_UNSUPPORTED_FORMAT"); break;
    case AUDCLNT_E_INVALID_SIZE                 :text =_T("AUDCLNT_E_INVALID_SIZE"); break;
    case AUDCLNT_E_DEVICE_IN_USE                :text =_T("AUDCLNT_E_DEVICE_IN_USE"); break;
    case AUDCLNT_E_BUFFER_OPERATION_PENDING     :text =_T("AUDCLNT_E_BUFFER_OPERATION_PENDING"); break;
    case AUDCLNT_E_THREAD_NOT_REGISTERED        :text =_T("AUDCLNT_E_THREAD_NOT_REGISTERED"); break;
    case AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED   :text =_T("AUDCLNT_E_EXCLUSIVE_MODE_NOT_ALLOWED"); break;
    case AUDCLNT_E_ENDPOINT_CREATE_FAILED       :text =_T("AUDCLNT_E_ENDPOINT_CREATE_FAILED"); break;
    case AUDCLNT_E_SERVICE_NOT_RUNNING          :text =_T("AUDCLNT_E_SERVICE_NOT_RUNNING"); break;
    case AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED     :text =_T("AUDCLNT_E_EVENTHANDLE_NOT_EXPECTED"); break;
    case AUDCLNT_E_EXCLUSIVE_MODE_ONLY          :text =_T("AUDCLNT_E_EXCLUSIVE_MODE_ONLY"); break;
    case AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL :text =_T("AUDCLNT_E_BUFDURATION_PERIOD_NOT_EQUAL"); break;
    case AUDCLNT_E_EVENTHANDLE_NOT_SET          :text =_T("AUDCLNT_E_EVENTHANDLE_NOT_SET"); break;
    case AUDCLNT_E_INCORRECT_BUFFER_SIZE        :text =_T("AUDCLNT_E_INCORRECT_BUFFER_SIZE"); break;
    case AUDCLNT_E_BUFFER_SIZE_ERROR            :text =_T("AUDCLNT_E_BUFFER_SIZE_ERROR"); break;
    case AUDCLNT_E_CPUUSAGE_EXCEEDED            :text =_T("AUDCLNT_E_CPUUSAGE_EXCEEDED"); break;
    case AUDCLNT_E_BUFFER_ERROR					:text =_T("AUDCLNT_E_BUFFER_ERROR"); break;
    case AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED		:text =_T("AUDCLNT_E_BUFFER_SIZE_NOT_ALIGNED"); break;
    case AUDCLNT_E_INVALID_DEVICE_PERIOD		:text =_T("AUDCLNT_E_INVALID_DEVICE_PERIOD"); break;

    case AUDCLNT_S_BUFFER_EMPTY                 :text =_T("AUDCLNT_S_BUFFER_EMPTY"); break;
    case AUDCLNT_S_THREAD_ALREADY_REGISTERED    :text =_T("AUDCLNT_S_THREAD_ALREADY_REGISTERED"); break;
    case AUDCLNT_S_POSITION_STALLED				:text =_T("AUDCLNT_S_POSITION_STALLED"); break;

    // other windows common errors:
    case CO_E_NOTINITIALIZED                    :text =_T("CO_E_NOTINITIALIZED: you must call CoInitialize() before Pa_OpenStream()"); break;

    default:
        text = _T("UNKNOWN ERROR");
    }
    _TCHAR * pLastErrMsg =_T("");
    TCHAR dbgMsg[256] = {_T('\0')};
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,
        0,
        res,
        MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
        (LPTSTR)&pLastErrMsg,
        0,
        NULL);

    BaseLogger::WriteFormattedLog(_T("Audio ERROR  %d: %s : %s   [FUNCTION: %s ]\n"), res, pLastErrMsg, text, func);
    return res;
}
