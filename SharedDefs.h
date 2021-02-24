#pragma once
#include "stdafx.h"
#include <string>

// Define the tracing flags used by config server and cmppconfig.dll
// This will likely be used by most components within UtsConfigManager.
enum TraceFlag
{
	TRACENONE			        = 0,
	TRACEJOINS			        = 0x00000001,
	TRACEMESSAGES		        = 0x00000002,
	TRACEUPDATES		        = 0x00000004,
	TRACEIPLINK			        = 0x00000008,
    TRACESTACK                  = 0x00000010,
    TRACE_CMDEXEC               = 0x00000020,
    TRACE_ETHERNET              = 0x00000040,
    TRACE_AUDIO                 = 0x00000080,
    TRACE_AUDIO_L1              = 0x00000100,
    VERBOSE                     = 0x00000200,
    TRACEALL			        = TRACEJOINS | TRACEMESSAGES | TRACEUPDATES |TRACEIPLINK | TRACESTACK | TRACE_CMDEXEC | TRACE_ETHERNET | TRACE_AUDIO 
};
