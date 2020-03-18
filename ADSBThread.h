#pragma once

#define CURL_STATICLIB
#include <curl/curl.h>

#include "Common.h"
#include "CommonData.h"
#include "CombineChunks.h"


class CADSBThread : public CombineChunks
{
	static HWND viewHnd;
	static BOOL loopBreak;
	CWinThread* pThread;
	static CEvent threadFinished;

	static UINT Thread(LPVOID);	// Worker-Thread
	static size_t write_callback(void*, size_t, size_t, void*);

public:
	CADSBThread(ADSBData*, ICAOData*);
	~CADSBThread() {};

	void SetViewHandle(HWND hnd) { viewHnd = hnd; }

	void StartThread();
	void StopThread();
};