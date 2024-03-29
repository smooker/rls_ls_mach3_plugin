#pragma once

// Linear scales setup dialog

class LSSetup : public CDialog
{
	DECLARE_DYNAMIC(LSSetup)

public:
	LSSetup(CWnd* pParent = NULL);   // standard constructor
	virtual ~LSSetup();

// Dialog Data
	enum { IDD = IDD_JOYSTICKCON };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	BOOL m_JoyOn;
	int m_Horiz;
	// Vertical Joystick Axis
	int m_Vert;
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg void OnTtnGetDispInfoCustom1(NMHDR *pNMHDR, LRESULT *pResult);
};
