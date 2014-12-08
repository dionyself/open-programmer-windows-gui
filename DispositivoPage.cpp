// DispositivoPage.cpp : implementation file
//

#include "stdafx.h"
#include "OpenProg.h"
#include "DispositivoPage.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CDispositivoPage property page

IMPLEMENT_DYNCREATE(CDispositivoPage, CPropertyPage)

CDispositivoPage::CDispositivoPage() : CPropertyPage(CDispositivoPage::IDD)
{
	//{{AFX_DATA_INIT(CDispositivoPage)
	//}}AFX_DATA_INIT
}

CDispositivoPage::~CDispositivoPage()
{
}

void CDispositivoPage::DoDataExchange(CDataExchange* pDX)
{
	CPropertyPage::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CDispositivoPage)
	DDX_Control(pDX, IDC_DISPO, m_dispo);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CDispositivoPage, CPropertyPage)
	//{{AFX_MSG_MAP(CDispositivoPage)
	ON_BN_CLICKED(IDC_FUSE3K_B, OnFuse3kB)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CDispositivoPage message handlers


void CDispositivoPage::OnFuse3kB() 
{
	SendMessage(WM_COMMAND, ID_FUSE3k, 0L);		
}
