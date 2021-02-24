#pragma once
#include "audiodefs.h"
#include "audiodevice.h"


class CAudioOutputDevice;


class CAudioInputDevice : public CAudioDevice
{
	friend DWORD WINAPI OutputThreadProc(LPVOID);

public:
	CAudioInputDevice(IMMDevice* pDevice);
	virtual ~CAudioInputDevice(void);

	HRESULT Init();
	void Cleanup();
	

protected:

	HRESULT InitInputAudioClient();
	HRESULT BeginCapture();
	HRESULT EndCapture();
	HRESULT StreamFrame();


	bool IsOutputConnected(CAudioOutputDevice* pOutput);





	IAudioCaptureClient*	m_pCaptureClient;
	HANDLE					m_hOutputThread;
	bool					m_fRunThread;

	vector<CAudioOutputDevice*>	m_OutputList;

};
