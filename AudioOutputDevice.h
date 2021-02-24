#pragma once
#include "audiodevice.h"


class CAudioOutputDevice : public CAudioDevice
{
public:
	CAudioOutputDevice(IMMDevice* pDevice);
	CAudioOutputDevice(CAudioOutputDevice* pOther);
	virtual ~CAudioOutputDevice(void);

	HRESULT Init();
	void Cleanup();
};
