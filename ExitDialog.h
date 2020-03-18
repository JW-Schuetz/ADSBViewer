#pragma once

#include "pch.h"
#include "Resource.h"


// CExitDialog
class CExitDialog : public CDialogEx
{
public:
	CExitDialog() noexcept;

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EXITDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};
