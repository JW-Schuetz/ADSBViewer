#pragma once

#include "CommonData.h"
#include "ADSBThread.h"


class CADSBViewerDoc : public CDocument
{
protected: // create from serialization only
	CADSBViewerDoc() { thread = new CADSBThread(&rawQueue, &airPlane); }
	DECLARE_DYNCREATE(CADSBViewerDoc)

	// Attributes
private:
	ADSBData rawQueue;
	ICAOData airPlane;
	void eraseQueue();

	CADSBThread* thread;

	// Operations
public:

	// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

	// Viewinterface
	ADSBData* GetQueue() { return &rawQueue; }
	void SetViewHandle(HWND hWnd) { thread->SetViewHandle(hWnd); }
	void StartThread() { thread->StartThread(); }
	void StopThread() { thread->StopThread(); }

	// Implementation
public:
	virtual ~CADSBViewerDoc();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

	// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
};