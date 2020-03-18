#pragma once

#define NUM_COLUMNS 3	// z.Z. Zeitstempel, Rawmessage und dekodierte Message


class CADSBViewerView : public CListView
{
protected: // create from serialization only
	CADSBViewerView();
	DECLARE_DYNCREATE(CADSBViewerView)

private:
	ADSBData* rawQueue;
	CADSBViewerDoc* document;
	int threadFinishedCounter;

	void addItem(ADSBMessage);

// Attributes
public:
	CADSBViewerDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnInitialUpdate();
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

protected:

// Implementation
public:
	virtual ~CADSBViewerView();

#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	afx_msg LRESULT OnData(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnThreadFinishedUnexpected(WPARAM wParam, LPARAM lParam);
};

#ifndef _DEBUG  // debug version in ADSBViewerView.cpp
inline CADSBViewerDoc* CADSBViewerView::GetDocument() const
   { return reinterpret_cast<CADSBViewerDoc*>(m_pDocument); }
#endif