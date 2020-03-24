#include "pch.h"

#include "ADSBViewerDoc.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CADSBViewerDoc
IMPLEMENT_DYNCREATE(CADSBViewerDoc, CDocument)

BEGIN_MESSAGE_MAP(CADSBViewerDoc, CDocument)
END_MESSAGE_MAP()

CADSBViewerDoc::~CADSBViewerDoc()
{
	// Thread stoppen (libCURL gibt ihre Ressourcen frei)
	thread->StopThread();

	delete thread;
	thread = NULL;

	// aufräumen
	eraseQueue();
}

void CADSBViewerDoc::eraseQueue()
{
	// alle Elemente freigeben
	for (ADSBData::iterator it = rawQueue.begin(); it != rawQueue.end(); ++it)
		(*it).DeleteContent();
}

BOOL CADSBViewerDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument()) return FALSE;
	return TRUE;
}

// CADSBViewerDoc serialization
void CADSBViewerDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		// TODO: add storing code here
	}
	else
	{
		// TODO: add loading code here
	}
}

// CADSBViewerDoc diagnostics
#ifdef _DEBUG
void CADSBViewerDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CADSBViewerDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
