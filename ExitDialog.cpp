#include "pch.h"
#include "ExitDialog.h"


CExitDialog::CExitDialog() noexcept : CDialogEx(IDD_EXITDIALOG)
{
}

void CExitDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CExitDialog, CDialogEx)
END_MESSAGE_MAP()
