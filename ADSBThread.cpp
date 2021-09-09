#include "pch.h"

#include "ADSBThread.h"


// Deklaration der static class members
HWND CADSBThread::viewHnd;
BOOL CADSBThread::loopBreak;
CEvent CADSBThread::threadFinished;


CADSBThread::CADSBThread(ADSBData* rawQueue, ICAOData* icaoData) :
	CombineChunks(rawQueue,icaoData)
{
	viewHnd = NULL;
	pThread = NULL;
	loopBreak = false;
	threadFinished.ResetEvent();
}

size_t CADSBThread::write_callback(void* buffer, size_t size, size_t nmemb, void* userp)
{
	if (loopBreak)	// evtl. Thread beenden
		return 0;

	size_t realsize = size * nmemb;
	if (realsize != 0)
	{
		// Inhalt von buffer ist NICHT 0-terminiert: siehe https://curl.haxx.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
		// Für 0-Terminierung 1 Byte mehr allozieren
		char* localBuffer = new char[realsize + 1];
		if (localBuffer != NULL)
		{
			// umkopieren und 0-Terminierung
			memcpy_s(localBuffer, realsize + 1, buffer, realsize);
			*(localBuffer + realsize) = 0;

			int vorher = rawQueue->size();

			// Chunks parsen und Ergebnisse in Queue eintragen
			nextChunk(localBuffer);

			int nachher = rawQueue->size();

			// Indizes der neuen Messages an View senden
			::SendMessage(viewHnd, ADSB_DATA, (WPARAM)vorher, (LPARAM)nachher);
		}

		// aufräumen
		delete[] localBuffer;
	}

	return realsize;
}

// ADS-B-Server: Arbeitszimmer-Rasberry-Pi
constexpr auto ADSBSERVER = "http://46.243.87.163:4321";
//constexpr auto ADSBSERVER = "192.168.178.91:4321";

UINT CADSBThread::Thread(LPVOID param)
{
	// LibCurl initialisieren
	curl_global_init(CURL_GLOBAL_DEFAULT);

	// easy handle konstruieren
	CURL* curlHnd = curl_easy_init();

	// set verbose mode on/off
	CURLcode res = curl_easy_setopt(curlHnd, CURLOPT_VERBOSE, true);

	// provide the URL to use in the request
	res = curl_easy_setopt(curlHnd, CURLOPT_URL, ADSBSERVER);

	res = curl_easy_setopt(curlHnd, CURLOPT_TCP_NODELAY, 0);

	// set callback for writing received data
	res = curl_easy_setopt(curlHnd, CURLOPT_WRITEDATA);

	// set callback for writing received data
	res = curl_easy_setopt(curlHnd, CURLOPT_WRITEFUNCTION, write_callback);

	res = curl_easy_perform(curlHnd);

	// aufräumen: LibCurl easy handle
	curl_easy_cleanup(curlHnd);

	// aufräumen: LibCurl
	curl_global_cleanup();

	// "Thread ist beendet" signalisieren
	threadFinished.SetEvent();

	if (!loopBreak)
		// nur falls Thread nicht vom User initiiert beendet wurde: App beenden
		::SendMessage(viewHnd, ADSB_THREAD_FINISHED_UNEXPECTED, res, 0);

	return res;
}

void CADSBThread::StartThread()
{
	if (pThread == NULL)
	{
		loopBreak = false;

		// Worker-Thread erzeugen und starten
		// siehe: https://stackoverflow.com/questions/38725768/threading-safe-stdlist-c
		pThread = AfxBeginThread(&Thread, NULL, THREAD_PRIORITY_ABOVE_NORMAL);
	}
}

void CADSBThread::StopThread()
{
	if (pThread != NULL)
	{
		// Thread beenden
		loopBreak = true;

		// Auf Threadende warten
		WaitForSingleObject(threadFinished.m_hObject, INFINITE);

		// Event wieder normieren
		threadFinished.ResetEvent();

		pThread = NULL;
	}
}
