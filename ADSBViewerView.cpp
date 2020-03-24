#include "pch.h"
#include "framework.h"

#include "ADSBViewerDoc.h"
#include "ADSBViewerView.h"
#include "ExitDialog.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static _TCHAR* headerText[NUM_COLUMNS] = { _T("Zeitmarke"), _T("Roh-Message"), _T("Dekodierte-Message") };
static int columnWidth[NUM_COLUMNS] = {	300, 500, 1500 };

// CADSBViewerView
IMPLEMENT_DYNCREATE(CADSBViewerView, CListView)

BEGIN_MESSAGE_MAP(CADSBViewerView, CListView)
	ON_MESSAGE(ADSB_DATA, &CADSBViewerView::OnData)
	ON_MESSAGE(ADSB_THREAD_FINISHED_UNEXPECTED, &CADSBViewerView::OnThreadFinishedUnexpected)
END_MESSAGE_MAP()


// CADSBViewerView construction/destruction
CADSBViewerView::CADSBViewerView()
{
	rawQueue = NULL;
	document = NULL;
	threadFinishedCounter = 0;
}

CADSBViewerView::~CADSBViewerView()
{
}

BOOL CADSBViewerView::PreCreateWindow(CREATESTRUCT& cs)
{	
	// Header ohne Actions, nur eine Selection
	cs.style |= LVS_SHOWSELALWAYS | LVS_REPORT | LVS_NOSORTHEADER | LVS_SINGLESEL;

	return CListView::PreCreateWindow(cs);
}

void CADSBViewerView::OnInitialUpdate()
{
	// Dokument holen
	document = GetDocument();

	// Message-Queue holen, View Handle senden (wird für Message ADSB_MESSAGE_ID benötigt), Thread starten
	rawQueue = document->GetQueue();
	document->SetViewHandle(m_hWnd);
	document->StartThread();

	// ListCtrl initialisieren
	CListCtrl& listCtrl = GetListCtrl();

	//CFont font;
	//font.CreateFont(
	//	0,							// Height
	//	0,							// Width
	//	0,							// Escapement
	//	0,							// Orientation
	//	FW_NORMAL,					// Weight
	//	FALSE,						// Italic
	//	FALSE,						// Underline
	//	0,							// StrikeOut
	//	DEFAULT_CHARSET,			// CharSet
	//	OUT_DEFAULT_PRECIS,			// OutPrecision
	//	CLIP_CHARACTER_PRECIS,		// ClipPrecision
	//	ANTIALIASED_QUALITY,		// Quality
	//	DEFAULT_PITCH,				// PitchAndFamily
	//	L"Consolas"					// Facename
	//);
	//listCtrl.SetFont(&font);

	listCtrl.SetItemCount(ITEM_COUNT);			// es werden evtl. viele Items
	listCtrl.SetTextColor(RGB(255, 255, 255));	// weisser Text auf schwarzem Hintergrund
	listCtrl.SetTextBkColor(RGB(0, 0, 0));

	CRect rect;
	listCtrl.GetWindowRect(&rect);
	long int halfWidth = (rect.right - rect.left) / 2;

	// insert columns
	LV_COLUMN lvc;
	lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;

	for (int i = 0; i < NUM_COLUMNS; i++)
	{
		lvc.iSubItem = i;
		lvc.pszText = headerText[i];
		lvc.cx = columnWidth[i];
		lvc.fmt = LVCFMT_LEFT;

		listCtrl.InsertColumn(i, &lvc);
	}
}

void CADSBViewerView::addItem(ADSBMessage item)
{
	// Konvertierung string -> wstring
	string f = *item.timeMark;
	string s = *item.rawMessage;
	string d = *item.decodedMessage;
	wstring wf(f.begin(), f.end());
	wstring ws(s.begin(), s.end());
	wstring wd(d.begin(), d.end());

	CListCtrl& listCtrl = GetListCtrl();
	int insertPos = listCtrl.GetItemCount();

	LV_ITEM lvi;
	lvi.mask = LVIF_TEXT;
	lvi.iItem = insertPos;	// Index
	lvi.iSubItem = 0;
	lvi.pszText = (LPWSTR)wf.c_str();
	listCtrl.InsertItem(&lvi);

	listCtrl.SetItemData(insertPos, insertPos);		// Itemdata=Index
	listCtrl.SetItemText(insertPos, 1, ws.c_str());	// Raw-Message
	listCtrl.SetItemText(insertPos, 2, wd.c_str());	// decoded Message
}

afx_msg LRESULT CADSBViewerView::OnData(WPARAM wParam, LPARAM lParam)
{
	for (int i = (int)wParam; i < (int)lParam; ++i)
		addItem((*rawQueue)[i]);

	return 0;
}

afx_msg LRESULT CADSBViewerView::OnThreadFinishedUnexpected(WPARAM wParam, LPARAM lParam)
{
	// Zähler erhöhen
	++threadFinishedCounter;

	// pThread zu NULL normieren, wg. evtl. Restart ein paar Zeilen weiter unten
	document->StopThread();

	#define MAX_THREAD_FINISH_COUNTS 3
	if (threadFinishedCounter < MAX_THREAD_FINISH_COUNTS)
		document->StartThread();	// Thread restarten
	else
	{
		// Meldungsfenster "Es können keine Daten empfangen werden!" und die Applikation beenden
		CString str = NULL;
		switch (wParam)
		{
		case CURLE_COULDNT_CONNECT:
			str = "E_COULDNT_CONNECT";
			break;

		case CURLE_OPERATION_TIMEDOUT:
			str = "E_OPERATION_TIMEDOUT";
			break;
		}

		CExitDialog exitDlg;
		exitDlg.DoModal();

		// WM_CLOSE an das Hauptfenster senden
		CWnd* main = AfxGetMainWnd();
		::SendMessage(main->m_hWnd, WM_CLOSE, 0, 0);
	}

	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////

// CADSBViewerView diagnostics
#ifdef _DEBUG
void CADSBViewerView::AssertValid() const
{
	CView::AssertValid();
}

void CADSBViewerView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CADSBViewerDoc* CADSBViewerView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CADSBViewerDoc)));
	return (CADSBViewerDoc*)m_pDocument;
}
#endif //_DEBUG
