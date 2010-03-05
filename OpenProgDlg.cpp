/*
 * Openprog - control program for the open programmer
 * Copyright (C) 2009 Alberto Maccioni
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA
 * or see <http://www.gnu.org/licenses/>
 */

// OpenProgDlg.cpp : implementation file
//

#include "stdafx.h"
#include <string.h>
#include <math.h>
#include "setupapi.h"
#include "hidsdi.h"
#include "DatiPage.h"
#include "DispositivoPage.h"
#include "OpzioniPage.h"
#include "I2CSPIPage.h"
#include "OpenProg.h"
#include "OpenProgDlg.h"
#include "strings.h"
#include "instructions.h"

#define DIMBUF 65
#define COL 16
#define VERSION "0.7.2"
#define G (12.0/34*1024/5)		//=72,2823529412

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	CString	m_license;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	m_license = _T("");
	//}}AFX_DATA_INIT
	m_license.Format("  OpenProg v.%s - control program for the open programmer\
\r\n	For detailed info see http://openprog.altervista.org/\
\r\n \
\r\n               Copyright (C) 2009 Alberto Maccioni\
\r\n \
\r\n  This program is free software; you can redistribute it and/or modify\
\r\n  it under the terms of the GNU General Public License as published by\
\r\n  the Free Software Foundation; either version 2 of the License, or \
\r\n  (at your option) any later version.\
\r\n  \
\r\n  This program is distributed in the hope that it will be useful,\
\r\n  but WITHOUT ANY WARRANTY; without even the implied warranty of\
\r\n  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\
\r\n  See the GNU General Public License for more details.\
\r\n  \
\r\n  You should have received a copy of the GNU General Public License\
\r\n  along with this program; if not, write to the Free Software\
\r\n  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA\
\r\n  or see <http://www.gnu.org/licenses/>",VERSION);
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	DDX_Text(pDX, IDC_LICENSE, m_license);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenProgDlg dialog

COpenProgDlg::COpenProgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpenProgDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpenProgDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void COpenProgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpenProgDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(COpenProgDlg, CDialog)
	//{{AFX_MSG_MAP(COpenProgDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_CLOSE()
	ON_COMMAND(ID_I2CSPI_R, OnI2cspiR)
	ON_COMMAND(ID_I2CSPI_S, OnI2cspiS)
	ON_COMMAND(ID_TEST_HW, OnTestHw)
	ON_COMMAND(ID_LEGGI, OnRead)
	ON_COMMAND(ID_OPZIONI_PROGRAMMATORE_CONNETTI, OnConnect)
	ON_COMMAND(ID_FILE_SALVA, OnFileSave)
	ON_COMMAND(ID_FILE_APRI, OnFileOpen)
	ON_COMMAND(ID_SCRIVI, OnWrite)
	ON_COMMAND(ID_CAMBIA_LINGUA, ChangeLanguage)
	ON_COMMAND(ID_WRITE_LANG_FILE, OnWriteLangFile)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpenProgDlg message handlers

BOOL COpenProgDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.
	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	CStdioFile f;
	CString lang="English";
	strings=0;
	CString s,dev="12F683",vid="0x04D8",pid="0x0100";
	wfile=rfile="";
	maxerr=200;
	MinRit=5;
	hvreg=0;
	s.Format("OpenProg v%s",VERSION);
	this->SetWindowText(s);
	s=GetCommandLine();
	s+=" ";
	CString t;
	int gui=0,delta_v=0;
	argc=0;
	int q=0;
	for(int i=0;i<s.GetLength()&&argc<32;i++){
		if(!q){
			if(s[i]==' '){
				if(t!="")argv[argc++]=t;
				t="";
			}
			else if(s[i]=='"') q=1;
			else t+=s[i];
		}
		else{ 
			if(q&&s[i]=='"'){
				argv[argc]=t;
				t="";
				argc++;
				q=0;
			}
			else t+=s[i];
		}

	}
	s=argv[0];
	s.Replace(".exe",".ini");
	if (f.Open(s,CFile::modeRead))	{
		CString line;
		char temp[256];
		for(;f.ReadString(line);){
			if(sscanf(line,"device %s",temp)>0)dev=temp;
			if(sscanf(line,"language %s",temp)>0)lang=temp;
			if(sscanf(line,"vid %s",temp)>0)vid=temp;
			if(sscanf(line,"pid %s",temp)>0)pid=temp;
			sscanf(line,"maxerr %d",&maxerr);
			sscanf(line,"usb_delay %d",&MinRit);
			//sscanf(line,"delta_v %d",&delta_v);
		}
		f.Close();
	}
	for(i=0;i<argc;i++){
		if(argv[i]=="-d"&&i+1<argc){
			dev=argv[++i];
			dev.MakeUpper();
		}
		else if(argv[i]=="-w"&&i+1<argc){
			wfile=argv[++i];
		}
		else if(argv[i]=="-r"&&i+1<argc){
			rfile=argv[++i];
		}
		else if(argv[i]=="-gui"){
			gui=1;
		}
	}
	dati_hex.SetSize(0x2400);
	for(i=0;i<0x2400;i++) dati_hex[i]=0x3fff;
	StatusBar.Create(WS_CHILD|WS_VISIBLE,CRect(0,0,0,0),this,0);
	ToolBar.Create(this);
	ToolBar.LoadToolBar(IDR_TOOLBAR1);
	ToolBar.SetBarStyle(ToolBar.GetBarStyle() |
		CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
	m_dlgPropSheet.AddPage(&m_DatiPage);
	m_dlgPropSheet.AddPage(&m_DispoPage);
	m_dlgPropSheet.AddPage(&m_OpzioniPage);
	m_dlgPropSheet.AddPage(&m_I2CSPIPage);
	m_dlgPropSheet.Create(this,WS_CHILD|WS_VISIBLE|WS_TABSTOP,WS_EX_CONTROLPARENT);
	m_dlgPropSheet.SetActivePage(3);
	m_dlgPropSheet.SetActivePage(2);
	m_dlgPropSheet.SetActivePage(1);
	m_dlgPropSheet.SetActivePage(0);
	m_OpzioniPage.m_language.AddString("Italiano");
	m_OpzioniPage.m_language.AddString("English");
	m_DispoPage.m_dispo.AddString("10F200");
	m_DispoPage.m_dispo.AddString("10F202");
	m_DispoPage.m_dispo.AddString("10F204");
	m_DispoPage.m_dispo.AddString("10F206");
	m_DispoPage.m_dispo.AddString("10F220");
	m_DispoPage.m_dispo.AddString("10F222");
	m_DispoPage.m_dispo.AddString("12C508/A");
	m_DispoPage.m_dispo.AddString("12C509/A");
	m_DispoPage.m_dispo.AddString("12F508");
	m_DispoPage.m_dispo.AddString("12F509");
	m_DispoPage.m_dispo.AddString("12F510");
	m_DispoPage.m_dispo.AddString("12F519");
	m_DispoPage.m_dispo.AddString("12F609");
	m_DispoPage.m_dispo.AddString("12F615");
	m_DispoPage.m_dispo.AddString("12F629");
	m_DispoPage.m_dispo.AddString("12F635");
	m_DispoPage.m_dispo.AddString("12C671");
	m_DispoPage.m_dispo.AddString("12C672");
	m_DispoPage.m_dispo.AddString("12CE673");
	m_DispoPage.m_dispo.AddString("12CE674");
	m_DispoPage.m_dispo.AddString("12F675");
	m_DispoPage.m_dispo.AddString("12F683");
	m_DispoPage.m_dispo.AddString("16F505");
	m_DispoPage.m_dispo.AddString("16F506");
	m_DispoPage.m_dispo.AddString("16F526");
	m_DispoPage.m_dispo.AddString("16F54");
	m_DispoPage.m_dispo.AddString("16F57");
	m_DispoPage.m_dispo.AddString("16F59");
	m_DispoPage.m_dispo.AddString("16F610");
	m_DispoPage.m_dispo.AddString("16F616");
	m_DispoPage.m_dispo.AddString("16F627");
	m_DispoPage.m_dispo.AddString("16F627A");
	m_DispoPage.m_dispo.AddString("16F628");
	m_DispoPage.m_dispo.AddString("16F628A");
	m_DispoPage.m_dispo.AddString("16F630");
	m_DispoPage.m_dispo.AddString("16F631");
	m_DispoPage.m_dispo.AddString("16F636");
	m_DispoPage.m_dispo.AddString("16F639");
	m_DispoPage.m_dispo.AddString("16F648A");
	m_DispoPage.m_dispo.AddString("16F676");
	m_DispoPage.m_dispo.AddString("16F677");
	m_DispoPage.m_dispo.AddString("16F684");
	m_DispoPage.m_dispo.AddString("16F685");
	m_DispoPage.m_dispo.AddString("16F687");
	m_DispoPage.m_dispo.AddString("16F688");
	m_DispoPage.m_dispo.AddString("16F689");
	m_DispoPage.m_dispo.AddString("16F690");
	m_DispoPage.m_dispo.AddString("16F716");
	m_DispoPage.m_dispo.AddString("16F73");
	m_DispoPage.m_dispo.AddString("16F737");
	m_DispoPage.m_dispo.AddString("16F74");
	m_DispoPage.m_dispo.AddString("16F747");
	m_DispoPage.m_dispo.AddString("16F76");
	m_DispoPage.m_dispo.AddString("16F767");
	m_DispoPage.m_dispo.AddString("16F77");
	m_DispoPage.m_dispo.AddString("16F777");
	m_DispoPage.m_dispo.AddString("16F785");
	m_DispoPage.m_dispo.AddString("16F818");
	m_DispoPage.m_dispo.AddString("16F819");
	m_DispoPage.m_dispo.AddString("16C83");
	m_DispoPage.m_dispo.AddString("16F83");
	m_DispoPage.m_dispo.AddString("16F83A");
	m_DispoPage.m_dispo.AddString("16C84");
	m_DispoPage.m_dispo.AddString("16F84");
	m_DispoPage.m_dispo.AddString("16F84A");
	m_DispoPage.m_dispo.AddString("16F87");
	m_DispoPage.m_dispo.AddString("16F870");
	m_DispoPage.m_dispo.AddString("16F871");
	m_DispoPage.m_dispo.AddString("16F872");
	m_DispoPage.m_dispo.AddString("16F873");
	m_DispoPage.m_dispo.AddString("16F873A");
	m_DispoPage.m_dispo.AddString("16F874");
	m_DispoPage.m_dispo.AddString("16F874A");
	m_DispoPage.m_dispo.AddString("16F876");
	m_DispoPage.m_dispo.AddString("16F876A");
	m_DispoPage.m_dispo.AddString("16F877");
	m_DispoPage.m_dispo.AddString("16F877A");
	m_DispoPage.m_dispo.AddString("16F88");
	m_DispoPage.m_dispo.AddString("16F882");
	m_DispoPage.m_dispo.AddString("16F883");
	m_DispoPage.m_dispo.AddString("16F884");
	m_DispoPage.m_dispo.AddString("16F886");
	m_DispoPage.m_dispo.AddString("16F887");
	m_DispoPage.m_dispo.AddString("16F913");
	m_DispoPage.m_dispo.AddString("16F914");
	m_DispoPage.m_dispo.AddString("16F916");
	m_DispoPage.m_dispo.AddString("16F917");
	m_DispoPage.m_dispo.AddString("16F946");
	m_DispoPage.m_dispo.AddString("18F242");
	m_DispoPage.m_dispo.AddString("18F248");
	m_DispoPage.m_dispo.AddString("18F252");
	m_DispoPage.m_dispo.AddString("18F258");
	m_DispoPage.m_dispo.AddString("18F442");
	m_DispoPage.m_dispo.AddString("18F448");
	m_DispoPage.m_dispo.AddString("18F452");
	m_DispoPage.m_dispo.AddString("18F458");
	m_DispoPage.m_dispo.AddString("18F1220");
	m_DispoPage.m_dispo.AddString("18F1230");
	m_DispoPage.m_dispo.AddString("18F1320");
	m_DispoPage.m_dispo.AddString("18F1330");
	m_DispoPage.m_dispo.AddString("18F2220");
	m_DispoPage.m_dispo.AddString("18F2221");
	m_DispoPage.m_dispo.AddString("18F2320");
	m_DispoPage.m_dispo.AddString("18F2321");
	m_DispoPage.m_dispo.AddString("18F2331");
	m_DispoPage.m_dispo.AddString("18F2410");
	m_DispoPage.m_dispo.AddString("18F2420");
	m_DispoPage.m_dispo.AddString("18F2423");
	m_DispoPage.m_dispo.AddString("18F2431");
	m_DispoPage.m_dispo.AddString("18F2439");
	m_DispoPage.m_dispo.AddString("18F2450");
	m_DispoPage.m_dispo.AddString("18F2455");
	m_DispoPage.m_dispo.AddString("18F2458");
	m_DispoPage.m_dispo.AddString("18F2480");
	m_DispoPage.m_dispo.AddString("18F2510");
	m_DispoPage.m_dispo.AddString("18F2515");
	m_DispoPage.m_dispo.AddString("18F2520");
	m_DispoPage.m_dispo.AddString("18F2523");
	m_DispoPage.m_dispo.AddString("18F2525");
	m_DispoPage.m_dispo.AddString("18F2539");
	m_DispoPage.m_dispo.AddString("18F2550");
	m_DispoPage.m_dispo.AddString("18F2553");
	m_DispoPage.m_dispo.AddString("18F2580");
	m_DispoPage.m_dispo.AddString("18F2585");
	m_DispoPage.m_dispo.AddString("18F2610");
	m_DispoPage.m_dispo.AddString("18F2620");
	m_DispoPage.m_dispo.AddString("18F2680");
	m_DispoPage.m_dispo.AddString("18F2682");
	m_DispoPage.m_dispo.AddString("18F2685");
	m_DispoPage.m_dispo.AddString("18F4220");
	m_DispoPage.m_dispo.AddString("18F4221");
	m_DispoPage.m_dispo.AddString("18F4320");
	m_DispoPage.m_dispo.AddString("18F4321");
	m_DispoPage.m_dispo.AddString("18F4331");
	m_DispoPage.m_dispo.AddString("18F4410");
	m_DispoPage.m_dispo.AddString("18F4420");
	m_DispoPage.m_dispo.AddString("18F4423");
	m_DispoPage.m_dispo.AddString("18F4431");
	m_DispoPage.m_dispo.AddString("18F4439");
	m_DispoPage.m_dispo.AddString("18F4450");
	m_DispoPage.m_dispo.AddString("18F4455");
	m_DispoPage.m_dispo.AddString("18F4458");
	m_DispoPage.m_dispo.AddString("18F4480");
	m_DispoPage.m_dispo.AddString("18F4510");
	m_DispoPage.m_dispo.AddString("18F4515");
	m_DispoPage.m_dispo.AddString("18F4520");
	m_DispoPage.m_dispo.AddString("18F4523");
	m_DispoPage.m_dispo.AddString("18F4525");
	m_DispoPage.m_dispo.AddString("18F4539");
	m_DispoPage.m_dispo.AddString("18F4550");
	m_DispoPage.m_dispo.AddString("18F4553");
	m_DispoPage.m_dispo.AddString("18F4580");
	m_DispoPage.m_dispo.AddString("18F4585");
	m_DispoPage.m_dispo.AddString("18F4610");
	m_DispoPage.m_dispo.AddString("18F4620");
	m_DispoPage.m_dispo.AddString("18F4680");
	m_DispoPage.m_dispo.AddString("18F4682");
	m_DispoPage.m_dispo.AddString("18F4685");
	m_DispoPage.m_dispo.AddString("18F8722");
	m_DispoPage.m_dispo.AddString("24F04KA200");
	m_DispoPage.m_dispo.AddString("24F04KA201");
	m_DispoPage.m_dispo.AddString("24F08KA101");
	m_DispoPage.m_dispo.AddString("24F08KA102");
	m_DispoPage.m_dispo.AddString("24F16KA101");
	m_DispoPage.m_dispo.AddString("24F16KA102");
	m_DispoPage.m_dispo.AddString("24FJ16GA002");
	m_DispoPage.m_dispo.AddString("24FJ16GA004");
	m_DispoPage.m_dispo.AddString("24FJ32GA002");
	m_DispoPage.m_dispo.AddString("24FJ32GA004");
	m_DispoPage.m_dispo.AddString("24FJ48GA002");
	m_DispoPage.m_dispo.AddString("24FJ48GA004");
	m_DispoPage.m_dispo.AddString("24FJ64GA002");
	m_DispoPage.m_dispo.AddString("24FJ64GA004");
	m_DispoPage.m_dispo.AddString("24FJ64GA006");
	m_DispoPage.m_dispo.AddString("24FJ64GA008");
	m_DispoPage.m_dispo.AddString("24FJ64GA010");
	m_DispoPage.m_dispo.AddString("24FJ96GA006");
	m_DispoPage.m_dispo.AddString("24FJ96GA008");
	m_DispoPage.m_dispo.AddString("24FJ96GA010");
	m_DispoPage.m_dispo.AddString("24FJ128GA006");
	m_DispoPage.m_dispo.AddString("24FJ128GA008");
	m_DispoPage.m_dispo.AddString("24FJ128GA010");
	m_DispoPage.m_dispo.AddString("AT90S1200");
	m_DispoPage.m_dispo.AddString("AT90S2313");
	m_DispoPage.m_dispo.AddString("AT90S8515");
	m_DispoPage.m_dispo.AddString("AT90S8535");
	m_DispoPage.m_dispo.AddString("ATmega8");
	m_DispoPage.m_dispo.AddString("ATmega8A");
	m_DispoPage.m_dispo.AddString("ATmega8515");
	m_DispoPage.m_dispo.AddString("ATmega8535");
	m_DispoPage.m_dispo.AddString("ATmega16");
	m_DispoPage.m_dispo.AddString("ATmega16A");
	m_DispoPage.m_dispo.AddString("ATmega32");
	m_DispoPage.m_dispo.AddString("ATmega32A");
	m_DispoPage.m_dispo.AddString("ATmega64");
	m_DispoPage.m_dispo.AddString("ATmega64A");
	m_DispoPage.m_dispo.AddString("2400");
	m_DispoPage.m_dispo.AddString("2401");
	m_DispoPage.m_dispo.AddString("2402");
	m_DispoPage.m_dispo.AddString("2404");
	m_DispoPage.m_dispo.AddString("2408");
	m_DispoPage.m_dispo.AddString("2416");
	m_DispoPage.m_dispo.AddString("2432");
	m_DispoPage.m_dispo.AddString("2464");
	m_DispoPage.m_dispo.AddString("24128");
	m_DispoPage.m_dispo.AddString("24256");
	m_DispoPage.m_dispo.AddString("24512");
	m_DispoPage.m_dispo.AddString("241025");
	m_DispoPage.m_dispo.AddString("25010");
	m_DispoPage.m_dispo.AddString("25020");
	m_DispoPage.m_dispo.AddString("25040");
	m_DispoPage.m_dispo.AddString("25080");
	m_DispoPage.m_dispo.AddString("25160");
	m_DispoPage.m_dispo.AddString("25320");
	m_DispoPage.m_dispo.AddString("25640");
	m_DispoPage.m_dispo.AddString("25128");
	m_DispoPage.m_dispo.AddString("25256");
	m_DispoPage.m_dispo.AddString("25512");
	m_DispoPage.m_dispo.AddString("251024");
	m_DispoPage.m_dispo.AddString("93S46");
	m_DispoPage.m_dispo.AddString("93x46");
	m_DispoPage.m_dispo.AddString("93x46A");
	m_DispoPage.m_dispo.AddString("93S56");
	m_DispoPage.m_dispo.AddString("93x56");
	m_DispoPage.m_dispo.AddString("93x56A");
	m_DispoPage.m_dispo.AddString("93S66");
	m_DispoPage.m_dispo.AddString("93x66");
	m_DispoPage.m_dispo.AddString("93x66A");
	m_DispoPage.m_dispo.AddString("93x76");
	m_DispoPage.m_dispo.AddString("93x76A");
	m_DispoPage.m_dispo.AddString("93x86");
	m_DispoPage.m_dispo.AddString("93x86A");

	s.Replace("OpenProg.ini","languages.rc");
	if (f.Open(s,CFile::modeRead))	{
		CString line;
		char temp[256];
		for(;f.ReadString(line);){
			if(sscanf(line,"[%s]",temp)>0){
				line=temp;
				line.Remove(']');
				for(int i=m_OpzioniPage.m_language.GetCount();i;i--){
					m_OpzioniPage.m_language.GetLBText(i-1,s);
					if(s==line)	break;
				}
				if(!i)	m_OpzioniPage.m_language.AddString(line);
			}
		}
		f.Close();
	}
	if(m_OpzioniPage.m_language.SelectString(-1,lang)==CB_ERR)
		m_OpzioniPage.m_language.SelectString(-1,"English");

	ChangeLanguage();
	m_OpzioniPage.SetDlgItemText(IDC_VID,vid);
	m_OpzioniPage.SetDlgItemText(IDC_PID,pid);
	m_OpzioniPage.SetDlgItemInt(IDC_USBDMIN,MinRit);
	if(m_DispoPage.m_dispo.SelectString(-1,dev)==CB_ERR)
		m_DispoPage.m_dispo.SelectString(-1,"12F683");
	RECT rect;
	m_dlgPropSheet.GetClientRect(&rect);
	rect.top+=25;
	rect.bottom+=25;
	m_dlgPropSheet.MoveWindow(&rect,FALSE);
	CSpinButtonCtrl*e=(CSpinButtonCtrl*)m_I2CSPIPage.GetDlgItem(IDC_NB);
	e->SetRange(0,60);
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	b->SetCheck(0);
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_LOAD);
	b->SetCheck(0);
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_OSCCAL);
	b->SetCheck(0);
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_BK);
	b->SetCheck(1);
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_EEPROM);
	b->SetCheck(1);
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_I2C8B);
	b->SetCheck(1);
	m_OpzioniPage.SetDlgItemInt(IDC_ERRMAX,maxerr);
	m_I2CSPIPage.SetDlgItemInt(IDC_NUMB,1);
	MyDeviceDetected = FALSE;
	hEventObject=0;
	OnConnect();
	ProgID();
	//PrintMessage(GetCommandLine());
	//for(i=0;i<argc;i++) PrintMessage(argv[i]);
	if(wfile!=""){
		OnFileOpen();
		OnWrite();
		wfile="";
		if(!gui) exit(0);
	}
	if(rfile!=""){
		OnRead();
		OnFileSave();
		rfile="";
		if(!gui) exit(0);
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}



void COpenProgDlg::ChangeLanguage()
{
	CString lang;
	m_OpzioniPage.m_language.GetLBText(m_OpzioniPage.m_language.GetCurSel(),lang);
	#include "strings.c"
	strings_ID=new char*[DIM];
	CString ss;
	for(int i=0;i<DIM;i++){
		ss.Format("%s#%s",STR_ID[i],strings_en[i]);
		ss.Replace("\r","\\r");
		ss.Replace("\n","\\n");
		ss.Replace("\t","\\t");
		ss.Replace("#","\t\t\t");
		ss+="\n";
		strings_ID[i]=new char[ss.GetLength()+1];
		strcpy(strings_ID[i],ss);
	}
	strings=new char*[DIM];
	for(i=0;i<DIM;i++) strings[i]=0;
	if(lang=="Italiano"){
		for(i=0;i<DIM;i++){
			strings[i]=new char[strlen(strings_it[i])+1];
			strcpy(strings[i],strings_it[i]);
		}
	}
	else if(lang!="English"){
		CStdioFile f;
		CString s;
		s="languages.rc";
		if (f.Open(s,CFile::modeRead))	{
			CString line,id;
			char temp[256];
			temp[0]=0;
			lang+="]";
			for(;f.ReadString(line);){
				if(sscanf(line,"[%s]",temp)&&lang==temp) break;
			}
			if(lang==temp){
				for(int i=0;i<DIM;i++) strings[i]=0;
				for(;f.ReadString(line);){
					if(line.GetLength()>0&&line[0]=='[') break;
					id=line;
					id.Replace('\t',' ');
					id=id.Left(id.Find(" "));
					line.Replace(id,"");
					line.TrimLeft();
					line.Replace("\\n","\n");
					line.Replace("\\r","\r");
					line.Replace("\\t","\t");
					for(int j=0;j<DIM;j++){
						if(id==STR_ID[j]){
							strings[j]=new char[line.GetLength()+1];
							strcpy(strings[j],line);
							j=DIM;
						}
					}
				}
				for(i=0;i<DIM;i++){
					if(strings[i]==0){
						strings[i]=new char[strlen(strings_en[i])+1];
						strcpy(strings[i],strings_en[i]);
					}
				}
			}
			else lang="English";
			f.Close();
		}
		else lang="English";
	}
	if(lang=="English"){				//english or any error
		for(int i=0;i<DIM;i++){
			strings[i]=new char[strlen(strings_en[i])+1];
			strcpy(strings[i],strings_en[i]);
		}
	}
    CTabCtrl* pTab =m_dlgPropSheet.GetTabControl();
    ASSERT (pTab);
    TC_ITEM ti;
    ti.mask = TCIF_TEXT;
    ti.pszText = strings[I_Data];
    VERIFY (pTab->SetItem (0, &ti));
    ti.pszText = strings[I_Dev];
    VERIFY (pTab->SetItem (1, &ti));
    ti.pszText = strings[I_Opt];
    VERIFY (pTab->SetItem (2, &ti));
	m_DispoPage.SetDlgItemText(IDC_STATICdev,strings[I_Dev]);
	m_DispoPage.SetDlgItemText(IDC_RISERVATA,strings[I_ReadRes]);
	m_DispoPage.SetDlgItemText(IDC_IDPROG,strings[I_ID_BKo_W]);
	m_DispoPage.SetDlgItemText(IDC_EEPROM,strings[I_EE]);
	m_DispoPage.SetDlgItemText(IDC_CALIB_LOAD,strings[I_CalW]);
	m_DispoPage.SetDlgItemText(IDC_STATIC2,strings[I_OSCW]);
	m_DispoPage.SetDlgItemText(IDC_OSC_OSCCAL,strings[I_OSC]);
	m_DispoPage.SetDlgItemText(IDC_OSC_BK,strings[I_BKOSC]);
	m_DispoPage.SetDlgItemText(IDC_OSC_LOAD,strings[I_OSCF]);
	m_DispoPage.SetDlgItemText(IDC_STATICdev,strings[I_Dev]);
	m_DispoPage.SetDlgItemText(IDC_ATCONF,strings[I_AT_CONFIG]);
	m_DispoPage.SetDlgItemText(IDC_FUSE_P,strings[I_AT_FUSE]);
	m_DispoPage.SetDlgItemText(IDC_FUSEH_P,strings[I_AT_FUSEH]);
	m_DispoPage.SetDlgItemText(IDC_FUSEX_P,strings[I_AT_FUSEX]);
	m_DispoPage.SetDlgItemText(IDC_LOCK_P,strings[I_AT_LOCK]);
	m_OpzioniPage.SetDlgItemText(IDC_CONNETTI,strings[I_CONN]);
	m_OpzioniPage.SetDlgItemText(IDC_REGISTRO,strings[I_LOG]);
	m_OpzioniPage.SetDlgItemText(IDC_STATICerr,strings[I_MAXERR]);
	m_OpzioniPage.SetDlgItemText(IDC_STATIC_L,strings[I_LANG]);
	m_OpzioniPage.SetDlgItemText(IDC_STATIC_USBD,strings[I_USBD]);
	m_OpzioniPage.SetDlgItemText(IDC_TESTHW,strings[I_TestHWB]);
	m_OpzioniPage.SetDlgItemText(IDC_WLANGFILE,strings[I_W_LANGFILE]);
	m_I2CSPIPage.SetDlgItemText(IDC_MODE,strings[I_I2CMode]);
	m_I2CSPIPage.SetDlgItemText(IDC_NBS,strings[I_I2C_NB]);
	m_I2CSPIPage.SetDlgItemText(IDC_REC,strings[I_I2CReceive]);
	m_I2CSPIPage.SetDlgItemText(IDC_SEND,strings[I_I2CSend]);
	m_I2CSPIPage.SetDlgItemText(IDC_msgSTRI,strings[I_I2CDATAOUT]);
	m_I2CSPIPage.SetDlgItemText(IDC_msgSTRU,strings[I_I2CDATATR]);
}


void COpenProgDlg::OnClose()
{
	CStdioFile f;
	CString s,t;
	MinRit=m_OpzioniPage.GetDlgItemInt(IDC_USBDMIN);
	s=argv[0];
	s.Replace(".exe",".ini");
	if (f.Open((LPCTSTR)s,CFile::modeCreate | CFile::modeWrite)){
		CString dev;
		m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
		s.Format("device %s\n",dev);
		f.WriteString(s);
		m_OpzioniPage.m_language.GetLBText(m_OpzioniPage.m_language.GetCurSel(),t);
    	s.Format("language %s\n",t);
		f.WriteString(s);
		int vid=0,pid=0,maxerr=200;
		CString a;
		m_OpzioniPage.GetDlgItemText(IDC_VID,a);
		sscanf(a,"0x%X",&vid);
		m_OpzioniPage.GetDlgItemText(IDC_PID,a);
		sscanf(a,"0x%X",&pid);
		s.Format("vid 0x%x\n",vid);
		f.WriteString(s);
		s.Format("pid 0x%x\n",pid);
		f.WriteString(s);
		m_OpzioniPage.GetDlgItemText(IDC_ERRMAX,a);
		sscanf(a,"%d",&maxerr);
		s.Format("maxerr %d\n",maxerr);
		f.WriteString(s);
		s.Format("usb_delay %d\n",MinRit);
		f.WriteString(s);
		//int Dvreg=m_OpzioniPage.GetDlgItemInt(IDC_HVDV);
		//s.Format("delta_v %d\n",Dvreg);
		//f.WriteString(s);
		f.Close();
	}
	CDialog::OnClose();
}

void COpenProgDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void COpenProgDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR COpenProgDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


void COpenProgDlg::OnConnect()
{
	typedef struct _HIDD_ATTRIBUTES {
	    ULONG   Size;
	    USHORT  VendorID;
	    USHORT  ProductID;
	    USHORT  VersionNumber;
	} HIDD_ATTRIBUTES, *PHIDD_ATTRIBUTES;

	typedef void (__stdcall*GETHIDGUID) (OUT LPGUID HidGuid);
	typedef BOOLEAN (__stdcall*GETATTRIBUTES)(IN HANDLE HidDeviceObject,OUT PHIDD_ATTRIBUTES Attributes);
	typedef BOOLEAN (__stdcall*SETNUMINPUTBUFFERS)(IN  HANDLE HidDeviceObject,OUT ULONG  NumberBuffers);
	typedef BOOLEAN (__stdcall*GETNUMINPUTBUFFERS)(IN  HANDLE HidDeviceObject,OUT PULONG  NumberBuffers);
	typedef BOOLEAN (__stdcall*GETFEATURE) (IN  HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*SETFEATURE) (IN  HANDLE HidDeviceObject, IN PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*GETREPORT) (IN  HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*SETREPORT) (IN  HANDLE HidDeviceObject, IN PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*GETMANUFACTURERSTRING) (IN  HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*GETPRODUCTSTRING) (IN  HANDLE HidDeviceObject, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
	typedef BOOLEAN (__stdcall*GETINDEXEDSTRING) (IN  HANDLE HidDeviceObject, IN ULONG  StringIndex, OUT PVOID ReportBuffer, IN ULONG ReportBufferLength);
	HIDD_ATTRIBUTES Attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	bool LastDevice = FALSE;
	int MemberIndex = 0;
	LONG Result;
//	char UsageDescription[256];

	Length=0;
	detailData=NULL;
	DeviceHandle=NULL;

	HMODULE hHID=0;
	GETHIDGUID HidD_GetHidGuid=0;
	GETATTRIBUTES HidD_GetAttributes=0;
	SETNUMINPUTBUFFERS HidD_SetNumInputBuffers=0;
	GETNUMINPUTBUFFERS HidD_GetNumInputBuffers=0;
	GETFEATURE HidD_GetFeature=0;
	SETFEATURE HidD_SetFeature=0;
	GETREPORT HidD_GetInputReport=0;
	SETREPORT HidD_SetOutputReport=0;
	GETMANUFACTURERSTRING HidD_GetManufacturerString=0;
	GETPRODUCTSTRING HidD_GetProductString=0;
	hHID = LoadLibrary("hid.dll");
	if(!hHID){ 
		MessageBox(strings[S_noDLL]);					//"no hid.dll"
		return;
	}
	HidD_GetHidGuid=(GETHIDGUID)GetProcAddress(hHID,"HidD_GetHidGuid");
	HidD_GetAttributes=(GETATTRIBUTES)GetProcAddress(hHID,"HidD_GetAttributes");
	HidD_SetNumInputBuffers=(SETNUMINPUTBUFFERS)GetProcAddress(hHID,"HidD_SetNumInputBuffers");
	HidD_GetNumInputBuffers=(GETNUMINPUTBUFFERS)GetProcAddress(hHID,"HidD_GetNumInputBuffers");
	HidD_GetFeature=(GETFEATURE)GetProcAddress(hHID,"HidD_GetFeature");
	HidD_SetFeature=(SETFEATURE)GetProcAddress(hHID,"HidD_SetFeature");
	HidD_GetInputReport=(GETREPORT)GetProcAddress(hHID,"HidD_GetInputReport");
	HidD_SetOutputReport=(SETREPORT)GetProcAddress(hHID,"HidD_SetOutputReport");
	HidD_GetManufacturerString=(GETMANUFACTURERSTRING)GetProcAddress(hHID,"HidD_GetManufacturerString");
	HidD_GetProductString=(GETPRODUCTSTRING)GetProcAddress(hHID,"HidD_GetProductString");
	if(HidD_GetHidGuid==NULL\
		||HidD_GetAttributes==NULL\
		||HidD_GetFeature==NULL\
		||HidD_SetFeature==NULL\
		||HidD_GetInputReport==NULL\
		||HidD_SetOutputReport==NULL\
		||HidD_GetManufacturerString==NULL\
		||HidD_GetProductString==NULL\
		||HidD_SetNumInputBuffers==NULL\
		||HidD_GetNumInputBuffers==NULL) return;


	HMODULE hSAPI=0;
	hSAPI = LoadLibrary("setupapi.dll");
	if(!hSAPI){ 
		MessageBox("Can't find setupapi.dll");
		return;
	}
	typedef HDEVINFO (WINAPI* SETUPDIGETCLASSDEVS) (CONST GUID*,PCSTR,HWND,DWORD);
	typedef BOOL (WINAPI* SETUPDIENUMDEVICEINTERFACES) (HDEVINFO,PSP_DEVINFO_DATA,CONST GUID*,DWORD,PSP_DEVICE_INTERFACE_DATA);
	typedef BOOL (WINAPI* SETUPDIGETDEVICEINTERFACEDETAIL) (HDEVINFO,PSP_DEVICE_INTERFACE_DATA,PSP_DEVICE_INTERFACE_DETAIL_DATA_A,DWORD,PDWORD,PSP_DEVINFO_DATA);
	typedef BOOL (WINAPI* SETUPDIDESTROYDEVICEINFOLIST) (HDEVINFO);	
	SETUPDIGETCLASSDEVS SetupDiGetClassDevsA=0;
	SETUPDIENUMDEVICEINTERFACES SetupDiEnumDeviceInterfaces=0;
	SETUPDIGETDEVICEINTERFACEDETAIL SetupDiGetDeviceInterfaceDetailA=0;
	SETUPDIDESTROYDEVICEINFOLIST SetupDiDestroyDeviceInfoList=0;
	SetupDiGetClassDevsA=(SETUPDIGETCLASSDEVS) GetProcAddress(hSAPI,"SetupDiGetClassDevsA");
	SetupDiEnumDeviceInterfaces=(SETUPDIENUMDEVICEINTERFACES) GetProcAddress(hSAPI,"SetupDiEnumDeviceInterfaces");
	SetupDiGetDeviceInterfaceDetailA=(SETUPDIGETDEVICEINTERFACEDETAIL) GetProcAddress(hSAPI,"SetupDiGetDeviceInterfaceDetailA");
	SetupDiDestroyDeviceInfoList=(SETUPDIDESTROYDEVICEINFOLIST) GetProcAddress(hSAPI,"SetupDiDestroyDeviceInfoList");
	if(SetupDiGetClassDevsA==NULL\
		||SetupDiEnumDeviceInterfaces==NULL\
		||SetupDiDestroyDeviceInfoList==NULL\
		||SetupDiGetDeviceInterfaceDetailA==NULL) return;
	/*
	The following code is adapted from Usbhidio_vc6 application example by Jan Axelson
	for more information see see http://www.lvr.com/hidpage.htm
	*/
/*
	typedef void (__stdcall*GETHIDGUID) (OUT LPGUID HidGuid);
	typedef BOOLEAN (__stdcall*GETATTRIBUTES)(IN HANDLE HidDeviceObject,OUT PHIDD_ATTRIBUTES Attributes);
	typedef BOOLEAN (__stdcall*SETNUMINPUTBUFFERS)(IN  HANDLE HidDeviceObject,OUT ULONG  NumberBuffers);
	typedef BOOLEAN (__stdcall*GETNUMINPUTBUFFERS)(IN  HANDLE HidDeviceObject,OUT PULONG  NumberBuffers);
	HIDD_ATTRIBUTES Attributes;
	SP_DEVICE_INTERFACE_DATA devInfoData;
	bool LastDevice = FALSE;
	int MemberIndex = 0;
	LONG Result;
	CString UsageDescription;

	Length=0;
	detailData=NULL;
	DeviceHandle=NULL;

	HMODULE hHID=0;
	GETHIDGUID HidD_GetHidGuid=0;
	GETATTRIBUTES HidD_GetAttributes=0;
	SETNUMINPUTBUFFERS HidD_SetNumInputBuffers=0;
	GETNUMINPUTBUFFERS HidD_GetNumInputBuffers=0;
	hHID = LoadLibrary("hid.dll");
	if(!hHID){
		MessageBox(strings[S_noDLL]);					//"no hid.dll"
		return;
	}
	HidD_GetHidGuid=(GETHIDGUID)GetProcAddress(hHID,"HidD_GetHidGuid");
	HidD_GetAttributes=(GETATTRIBUTES)GetProcAddress(hHID,"HidD_GetAttributes");
	HidD_SetNumInputBuffers=(SETNUMINPUTBUFFERS)GetProcAddress(hHID,"HidD_SetNumInputBuffers");
	HidD_GetNumInputBuffers=(GETNUMINPUTBUFFERS)GetProcAddress(hHID,"HidD_GetNumInputBuffers");
	if(HidD_GetHidGuid==NULL||HidD_GetAttributes==NULL||HidD_SetNumInputBuffers==NULL) return;
*/
	/*
	API function: HidD_GetHidGuid
	Get the GUID for all system HIDs.
	Returns: the GUID in HidGuid.
	*/
	HidD_GetHidGuid(&HidGuid);

	/*
	API function: SetupDiGetClassDevs
	Returns: a handle to a device information set for all installed devices.
	Requires: the GUID returned by GetHidGuid.
	*/
	hDevInfo=SetupDiGetClassDevs(&HidGuid,NULL,NULL,DIGCF_PRESENT|DIGCF_INTERFACEDEVICE);
	devInfoData.cbSize = sizeof(devInfoData);

	//Step through the available devices looking for the one we want.
	//Quit on detecting the desired device or checking all available devices without success.
	MemberIndex = 0;
	LastDevice = FALSE;
	do
	{
		/*
		API function: SetupDiEnumDeviceInterfaces
		On return, MyDeviceInterfaceData contains the handle to a
		SP_DEVICE_INTERFACE_DATA structure for a detected device.
		Requires:
		The DeviceInfoSet returned in SetupDiGetClassDevs.
		The HidGuid returned in GetHidGuid.
		An index to specify a device.
		*/
		Result=SetupDiEnumDeviceInterfaces (hDevInfo, 0, &HidGuid, MemberIndex, &devInfoData);
		if (Result != 0)
		{
			//A device has been detected, so get more information about it.
			/*
			API function: SetupDiGetDeviceInterfaceDetail
			Returns: an SP_DEVICE_INTERFACE_DETAIL_DATA structure
			containing information about a device.
			To retrieve the information, call this function twice.
			The first time returns the size of the structure in Length.
			The second time returns a pointer to the data in DeviceInfoSet.
			Requires:
			A DeviceInfoSet returned by SetupDiGetClassDevs
			The SP_DEVICE_INTERFACE_DATA structure returned by SetupDiEnumDeviceInterfaces.

			The final parameter is an optional pointer to an SP_DEV_INFO_DATA structure.
			This application doesn't retrieve or use the structure.
			If retrieving the structure, set
			MyDeviceInfoData.cbSize = length of MyDeviceInfoData.
			and pass the structure's address.
			*/
			//Get the Length value.
			//The call will return with a "buffer too small" error which can be ignored.
			Result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, NULL, 0, &Length, NULL);

			//Allocate memory for the hDevInfo structure, using the returned Length.
			detailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(Length);

			//Set cbSize in the detailData structure.
			detailData -> cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

			//Call the function again, this time passing it the returned buffer size.
			Result = SetupDiGetDeviceInterfaceDetail(hDevInfo, &devInfoData, detailData, Length,&Required, NULL);

			// Open a handle to the device.
			// To enable retrieving information about a system mouse or keyboard,
			// don't request Read or Write access for this handle.
			/*
			API function: CreateFile
			Returns: a handle that enables reading and writing to the device.
			Requires:
			The DevicePath in the detailData structure
			returned by SetupDiGetDeviceInterfaceDetail.
			*/
			DeviceHandle=CreateFile(detailData->DevicePath,
				0, FILE_SHARE_READ|FILE_SHARE_WRITE,
				(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING, 0, NULL);

			/*
			API function: HidD_GetAttributes
			Requests information from the device.
			Requires: the handle returned by CreateFile.
			Returns: a HIDD_ATTRIBUTES structure containing
			the Vendor ID, Product ID, and Product Version Number.
			Use this information to decide if the detected device is
			the one we're looking for.
			*/

			//Set the Size to the number of bytes in the structure.
			Attributes.Size = sizeof(Attributes);
			Result = HidD_GetAttributes(DeviceHandle,&Attributes);

			//Is it the desired device?
			MyDeviceDetected = FALSE;
			CString a;
			int vid=0,pid=0;
			m_OpzioniPage.GetDlgItemText(IDC_VID,a);
			sscanf(a,"0x%X",&vid);
			m_OpzioniPage.GetDlgItemText(IDC_PID,a);
			sscanf(a,"0x%X",&pid);
			if (Attributes.VendorID == vid)
			{
				if (Attributes.ProductID == pid)
				{
					//Both the Vendor ID and Product ID match.
					MyDeviceDetected = TRUE;
					MyDevicePathName = detailData->DevicePath;
					a.Format(strings[S_dev_det],vid,pid,MyDevicePathName);	//"Device detected: vid=0x%04X pid=0x%04X\r\nPath: %s\r\n"
					PrintMessage(a);

					// Get a handle for writing Output reports.
					WriteHandle=CreateFile(detailData->DevicePath,
						GENERIC_WRITE, FILE_SHARE_READ|FILE_SHARE_WRITE,
						(LPSECURITY_ATTRIBUTES)NULL,OPEN_EXISTING,0,NULL);

					//Get a handle to the device for the overlapped ReadFiles.
					ReadHandle=CreateFile(detailData->DevicePath,
						GENERIC_READ,FILE_SHARE_READ|FILE_SHARE_WRITE,(LPSECURITY_ATTRIBUTES)NULL,
						OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);

					if (hEventObject) CloseHandle(hEventObject);
					hEventObject = CreateEvent(NULL,TRUE,TRUE,"");

					//Set the members of the overlapped structure.
					HIDOverlapped.hEvent = hEventObject;
					HIDOverlapped.Offset = 0;
					HIDOverlapped.OffsetHigh = 0;
					Result=HidD_SetNumInputBuffers(DeviceHandle,64);
				}
				else
					//The Product ID doesn't match.
					CloseHandle(DeviceHandle);
			}
			else
				//The Vendor ID doesn't match.
				CloseHandle(DeviceHandle);
		//Free the memory used by the detailData structure (no longer needed).
		free(detailData);
		}
		else
			//SetupDiEnumDeviceInterfaces returned 0, so there are no more devices to check.
			LastDevice=TRUE;
		//If we haven't found the device yet, and haven't tried every available device,
		//try the next one.
		MemberIndex = MemberIndex + 1;
	} //do
	while ((LastDevice == FALSE) && (MyDeviceDetected == FALSE));

	if (MyDeviceDetected == FALSE){
		PrintMessage(strings[S_noprog]);	//"Programmatore non rilevato\r\n"
		StatusBar.SetWindowText(strings[S_noprog]);
	}
	else{
		PrintMessage(strings[S_prog]);	//"Programmatore rilevato\r\n");
		StatusBar.SetWindowText(strings[S_prog]);
	}
	//Free the memory reserved for hDevInfo by SetupDiClassDevs.
	SetupDiDestroyDeviceInfoList(hDevInfo);
	hvreg=0;		//assume vreg not working
}

#define  LOCK	1
#define  FUSE	2
#define  FUSE_H 4
#define  FUSE_X	8
#define  CAL	16

//this is needed to use the same code to both win and linux
#define EQ(s) c==s	

void COpenProgDlg::OnWrite()
{
	CString c;
	int ee;
	MinRit=m_OpzioniPage.GetDlgItemInt(IDC_USBDMIN);
	m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),c);
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_EEPROM);
	ee=b->GetCheck();
	if(ee) ee=0xffff;
	if(!hvreg&&(c.Left(2)=="10"||c.Left(2)=="12"||c.Left(2)=="16")) StartHVReg(13);
	else if(!hvreg&&(c.Left(2)=="18")) StartHVReg(12);
//-------------PIC10-16---------------------------------------------------------
		if(EQ("10F200")||EQ("10F204")||EQ("10F220")){
			Write12F5xx(0x100,0xFF);						//256
		}
		else if(EQ("12F508")||EQ("10F202")||EQ("10F206")||EQ("10F222")){
			Write12F5xx(0x200,0x1FF);						//512
		}
		else if(EQ("16F54")){
			Write12F5xx(0x200,-1);							//512, no osccal
		}
		else if(EQ("16C83")||EQ("16F83")||EQ("16F83A")){
			Write16F8x(0x200,ee?0x40:0);					//512, 64
		}
		else if(EQ("12F509")||EQ("12F510")||EQ("16F505")||EQ("16F506")){
			Write12F5xx(0x400,0x3FF);						//1K
		}
		else if(EQ("12F519")||EQ("16F526")){
			Write12F5xx(0x440,0x3FF);						//1K + 64
		}
		else if(EQ("12F609")||EQ("12F615")||EQ("16F610")){
			Write12F61x(0x400);								//1K
		}
		else if(EQ("16C84")||EQ("16F84")||EQ("16F84A")){
			Write16F8x(0x400,ee?0x40:0);					//1K, 64
		}
		else if(EQ("12F629")||EQ("12F675")||EQ("16F630")||EQ("16F676")){
			Write12F62x(0x400,ee?0x80:0);					//1K, 128
		}
		else if(EQ("16F627")){
			Write16F62x(0x400,ee?0x80:0);					//1K, 128
		}
		else if(EQ("12F635")||EQ("16F631")||EQ("16F627A")||EQ("16F785")){
			Write12F6xx(0x400,ee?0x80:0);					//1K, 128
		}
		else if(EQ("16F818")){
			Write16F81x(0x400,ee?0x80:0);					//1K, 128, vdd no delay
		}
		else if(EQ("16F57")||EQ("16F59")){
			Write12F5xx(0x800,-1);							//2K, no osccal
		}
		else if(EQ("16F616")){
			Write12F61x(0x800);								//2K
		}
		else if(EQ("16F716")){
			Write16F71x(0x800,1);							//2K, vdd
		}
		else if(EQ("16F870")||EQ("16F871")||EQ("16F872")){
			Write16F87x(0x800,ee?0x40:0);					//2K, 64
		}
		else if(EQ("16F628A")){
			Write12F6xx(0x800,ee?0x80:0);					//2K, 128
		}
		else if(EQ("16F628")){
			Write16F62x(0x800,ee?0x80:0);					//2K, 128
		}
		else if(EQ("16F882")){
			Write16F88x(0x800,ee?0x80:0);					//2K, 128
		}
		else if(EQ("12F683")||EQ("16F636")||EQ("16F639")||EQ("16F677")||EQ("16F684")||EQ("16F687")||EQ("16F785")){
			Write12F6xx(0x800,ee?0x100:0);					//2K, 256
		}
		else if(EQ("16F819")){
			Write16F81x(0x800,ee?0x100:0);					//2K, 256, vdd no delay
		}
		else if(EQ("16F73")||EQ("16F74")){
			Write16F7x(0x1000,0);							//4K
		}
		else if(EQ("16F737")||EQ("16F747")){
			Write16F7x(0x1000,1);							//4K, vdd no delay
		}
		else if(EQ("16F873")||EQ("16F874")){
			Write16F87x(0x1000,ee?-0x80:0);					//4K, 128, ee@0x2200
		}
		else if(EQ("16F648A")||EQ("16F685")||EQ("16F688")||EQ("16F689")||EQ("16F690")||EQ("16F913")||EQ("16F914")){
			Write12F6xx(0x1000,ee?0x100:0);					//4K, 256
		}
		else if(EQ("16F873A")||EQ("16F874A")){
			Write16F87xA(0x1000,ee?0x80:0,0);				//4K, 128
		}
		else if(EQ("16F883")||EQ("16F884")){
			Write16F88x(0x1000,ee?0x100:0);					//4K, 256
		}
		else if(EQ("16F87")||EQ("16F88")){
			Write16F81x(0x1000,ee?0x100:0);					//4K, 256, vdd no delay
		}
		else if(EQ("16F76")||EQ("16F77")){
			Write16F7x(0x2000,0);							//8K
		}
		else if(EQ("16F767")||EQ("16F777")){
			Write16F7x(0x2000,1);							//8K, vdd no delay
		}
		else if(EQ("16F916")||EQ("16F917")||EQ("16F946")){
			Write12F6xx(0x2000,ee?0x100:0);					//8K, 256
		}
		else if(EQ("16F876")||EQ("16F877")){
			Write16F87x(0x2000,ee?-0x100:0);				//8K, 256, ee@0x2200
		}
		else if(EQ("16F876A")||EQ("16F877A")){
			Write16F87xA(0x2000,ee?0x100:0,0);				//8K, 256,
		}
		else if(EQ("16F886")||EQ("16F887")){
			Write16F88x(0x2000,ee?0x100:0);					//8K, 256
		}
//-------------PIC18---------------------------------------------------------
		else if(EQ("18F1230")){
			Write18Fx(0x1000,ee?0x80:0,8,0x0F0F,0x8787,0);		//4K, 128, 8
		}
		else if(EQ("18F2221")||EQ("18F4221")){
			Write18Fx(0x1000,ee?0x100:0,8,0x3F3F,0x8F8F,0);		//4K, 256, 8
		}
		else if(EQ("18F1220")||EQ("18F2220")||EQ("18F4220")){
			Write18Fx(0x1000,ee?0x100:0,8,0x10000,0x80,1);		//4K, 256, 8, EE with unlock
		}
		else if(EQ("18F1330")){
			Write18Fx(0x2000,ee?0x80:0,8,0x0F0F,0x8787,0);		//8K, 128, 8
		}
		else if(EQ("18F2321")||EQ("18F4321")){
			Write18Fx(0x2000,ee?0x100:0,8,0x3F3F,0x8F8F,0);		//8K, 256, 8
		}
		else if(EQ("18F1320")||EQ("18F2320")||EQ("18F4320")||EQ("18F2331")||EQ("18F4331")){
			Write18Fx(0x2000,ee?0x100:0,8,0x10000,0x80,1);		//8K, 256, 8, EE with unlock
		}
		else if(EQ("18F2439")||EQ("18F4439")){
			Write18Fx(0x3000,ee?0x100:0,8,0x10000,0x80,1);		//12K, 256, 8, EE with unlock
		}
		else if(EQ("18F2410")||EQ("18F4410")){
			Write18Fx(0x4000,0,32,0x3F3F,0x8F8F,0);				//16K, 0, 32
		}
		else if(EQ("18F2450")||EQ("18F4450")){
			Write18Fx(0x4000,0,16,0x3F3F,0x8F8F,0);				//16K, 0, 16
		}
		else if(EQ("18F2431")||EQ("18F4431")||EQ("18F242")||EQ("18F248")||EQ("18F442")||EQ("18F448")){
			Write18Fx(0x4000,ee?0x100:0,8,0x10000,0x80,1);		//16K, 256, 8, EE with unlock
		}
		else if(EQ("18F2420")||EQ("18F2423")||EQ("18F4420")||EQ("18F4423")||EQ("18F2480")||EQ("18F4480")){
			Write18Fx(0x4000,ee?0x100:0,32,0x3F3F,0x8F8F,0);	//16K, 256, 32
		}
		else if(EQ("18F2455")||EQ("18F2458")||EQ("18F4455")||EQ("18F4458")){
			Write18Fx(0x6000,ee?0x100:0,32,0x3F3F,0x8F8F,0);	//24K, 256, 32
		}
		else if(EQ("18F2539")||EQ("18F4539")){
			Write18Fx(0x6000,ee?0x100:0,8,0x10000,0x80,1);		//24K, 256, 8, EE with unlock
		}
		else if(EQ("18F2510")||EQ("18F4510")){
			Write18Fx(0x8000,0,32,0x3F3F,0x8F8F,0);				//32K, 0, 32
		}
		else if(EQ("18F252")||EQ("18F258")||EQ("18F452")||EQ("18F458")){
			Write18Fx(0x8000,ee?0x100:0,8,0x10000,0x80,1);		//32K, 256, 8, EE with unlock
		}
		else if(EQ("18F2550")||EQ("18F2553")||EQ("18F4550")||EQ("18F4553")||EQ("18F2520")||EQ("18F2523")||EQ("18F4520")||EQ("18F4523")||EQ("18F2580")||EQ("18F4580")){
			Write18Fx(0x8000,ee?0x100:0,32,0x3F3F,0x8F8F,0);	//32K, 256, 32
		}
		else if(EQ("18F2515")||EQ("18F4515")){
			Write18Fx(0xC000,0,64,0x3F3F,0x8F8F,0);				//48K, 0, 64
		}
		else if(EQ("18F2525")||EQ("18F2585")||EQ("18F4525")||EQ("18F4585")){
			Write18Fx(0xC000,ee?0x400:0,64,0x3F3F,0x8F8F,0);	//48K, 1K, 64
		}
		else if(EQ("18F2610")||EQ("18F4610")){
			Write18Fx(0x10000,0,64,0x3F3F,0x8F8F,0);			//64K, 0, 64
		}
		else if(EQ("18F2620")||EQ("18F2680")||EQ("18F4620")||EQ("18F4680")){
			Write18Fx(0x10000,ee?0x400:0,64,0x3F3F,0x8F8F,0);	//64K, 1K, 64
		}
		else if(EQ("18F2682")||EQ("18F4682")){
			Write18Fx(0x14000,ee?0x400:0,64,0x3F3F,0x8F8F,0);	//80K, 1K, 64
		}
		else if(EQ("18F2685")||EQ("18F4685")){
			Write18Fx(0x18000,ee?0x400:0,64,0x3F3F,0x8F8F,0);	//96K, 1K, 64
		}
		else if(EQ("18F8722")){
			Write18Fx(0x20000,ee?0x400:0,64,0xFFFF,0x8787,0);	//128K, 1K, 64
		}
//-------------PIC24---------------------------------------------------------
		else if(EQ("24F04KA200")||EQ("24F04KA201")){
			Write24Fx(0xB00,0,3,0x05BE,32,2.0,0xFE00,0x4064,0x4004);			//1.375KW, HV
		}
		else if(EQ("24F08KA101")||EQ("24F08KA102")){
			Write24Fx(0x1600,ee?0x100:0,3,0x05BE,32,2.0,0xFE00,0x4064,0x4004);//2.75KW, HV, 256W
		}
		else if(EQ("24F16KA101")||EQ("24F16KA102")){
			Write24Fx(0x2C00,ee?0x100:0,3,0x05BE,32,2.0,0xFE00,0x4064,0x4004);//5.5KW, HV, 256W
		}
		else if(EQ("24FJ16GA002")||EQ("24FJ16GA004")){
			Write24Fx(0x2C00,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//5.5KW
		}
		else if(EQ("24FJ32GA002")||EQ("24FJ32GA004")){
			Write24Fx(0x5800,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//11KW
		}
		else if(EQ("24FJ48GA002")||EQ("24FJ48GA004")){
			Write24Fx(0x8400,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//16.5KW
		}
		else if(EQ("24FJ64GA002")||EQ("24FJ64GA004")||EQ("24FJ64GA006")||EQ("24FJ64GA008")||EQ("24FJ64GA010")){
			Write24Fx(0xAC00,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//22KW
		}
		else if(EQ("24FJ96GA006")||EQ("24FJ96GA008")||EQ("24FJ96GA010")){
			Write24Fx(0x10000,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//32KW
		}
		else if(EQ("24FJ128GA006")||EQ("24FJ128GA008")||EQ("24FJ128GA010")){
			Write24Fx(0x15800,0,0,0x05BE,64,2.0,0,0x404F,0x4001);				//44KW
		}
//-------------ATMEL---------------------------------------------------------
		else if(EQ("AT90S1200")){
			WriteAT(0x400,ee?0x40:0);						//1K, 64
		}
		else if(EQ("AT90S2313")){
			WriteAT(0x800,ee?0x80:0);						//2K, 128
		}
		else if(EQ("AT90S8515")||EQ("AT90S8535")){
			WriteAT(0x2000,ee?0x100:0);						//8K, 256
		}
		else if(EQ("ATmega8")||EQ("ATmega8A")||EQ("ATmega8515")||EQ("ATmega8535")){
			WriteATmega(0x2000,ee?0x200:0,32);				//8K, 512
		}
		else if(EQ("ATmega16")||EQ("ATmega16A")){
			WriteATmega(0x4000,ee?0x200:0,64);				//16K, 512
		}
		else if(EQ("ATmega32")||EQ("ATmega32A")){
			WriteATmega(0x8000,ee?0x400:0,64);				//32K, 1K
		}
		else if(EQ("ATmega64")||EQ("ATmega64A")){
			WriteATmega(0x10000,ee?0x800:0,128);			//64K, 2K
		}
//-------------I2C---------------------------------------------------------
		else if(EQ("2400")){
			WriteI2C(0x10,0,1,10);			//16, 1B addr.
		}
		else if(EQ("2401")){
			WriteI2C(0x80,0,8,10);			//128, 1B addr.
		}
		else if(EQ("2402")){
			WriteI2C(0x100,0,8,10);			//256, 1B addr.
		}
		else if(EQ("2404")){
			WriteI2C(0x200,0,16,10);		//512, 1B addr.
		}
		else if(EQ("2408")){
			WriteI2C(0x400,0,16,10);		//1K, 1B addr.
		}
		else if(EQ("2416")){
			WriteI2C(0x800,0,16,10);		//2K, 1B addr.
		}
		else if(EQ("2432")){
			WriteI2C(0x1000,1,32,5);		//4K, 2B addr.
		}
		else if(EQ("2464")){
			WriteI2C(0x2000,1,32,5);		//8K, 2B addr.
		}
		else if(EQ("24128")){
			WriteI2C(0x4000,1,64,5);		//16K, 2B addr.
		}
		else if(EQ("24256")){
			WriteI2C(0x8000,1,64,5);		//32K, 2B addr.
		}
		else if(EQ("24512")){
			WriteI2C(0x10000,1,128,5);		//64K, 2B addr.
		}
		else if(EQ("241025")){
			WriteI2C(0x20000,1,128,5);		//128K, 2B addr.
		}
//-------------Microwire EEPROM---------------------------------------------------------
		else if(EQ("93S46")){
			Write93Sx(0x80,6,8,10);							//128, 4W page, 10ms
		}
		else if(EQ("93x46")){
			Write93Cx(0x80,6,0);							//128,
		}
		else if(EQ("93x46A")){
			Write93Cx(0x80,7,1);							//128, x8
		}
		else if(EQ("93S56")){
			Write93Sx(0x100,8,8,10);						//256, 4W page, 10ms
		}
		else if(EQ("93x56")){
			Write93Cx(0x100,8,0);							//256,
		}
		else if(EQ("93x56A")){
			Write93Cx(0x100,9,1);							//256, x8
		}
		else if(EQ("93S66")){
			Write93Sx(0x200,8,8,10);						//512, 4W page, 10ms
		}
		else if(EQ("93x66")){
			Write93Cx(0x200,8,0);						//512,
		}
		else if(EQ("93x66A")){
			Write93Cx(0x200,9,1);						//512, x8
		}
		else if(EQ("93x76")){
			Write93Cx(0x400,10,0);						//1k
		}
		else if(EQ("93x76A")){
			Write93Cx(0x400,11,1);						//1k, x8
		}
		else if(EQ("93x86")){
			Write93Cx(0x800,10,0);						//2k,
		}
		else if(EQ("93x86A")){
			Write93Cx(0x800,11,1);						//2k, x8
		}
//-------------SPI---------------------------------------------------------
		else if(EQ("25010")){
			Write25xx(0x80,16,10);								//128
		}
		else if(EQ("25020")){
			Write25xx(0x100,16,10);								//256
		}
		else if(EQ("25040")){
			Write25xx(0x200,16,10);								//512
		}
		else if(EQ("25080")){
			Write25xx(0x400,16,5);								//1K
		}
		else if(EQ("25160")){
			Write25xx(0x800,16,5);								//2K
		}
		else if(EQ("25320")){
			Write25xx(0x1000,32,5);								//4K
		}
		else if(EQ("25640")){
			Write25xx(0x2000,32,5);								//8K
		}
		else if(EQ("25128")){
			Write25xx(0x4000,64,5);								//16K
		}
		else if(EQ("25256")){
			Write25xx(0x8000,64,5);								//32K
		}
		else if(EQ("25512")){
			Write25xx(0x10000,128,6);							//64K
		}
		else if(EQ("251024")){
			Write25xx(0x20000,256,5);							//128K
		}
//-------------Unsupported device---------------------------------------------------------
		else{
			PrintMessage(strings[S_nodev_w]); //"Dispositivo non supportato in scrittura\r\n");
		}
}

void COpenProgDlg::OnRead()
{
	CString c;
	int r,ee;
	MinRit=m_OpzioniPage.GetDlgItemInt(IDC_USBDMIN);
	m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),c);
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_RISERVATA);
	r=b->GetCheck();
	if(r) r=0xffff;
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_EEPROM);
	ee=b->GetCheck();
	if(ee) ee=0xffff;
	if(!hvreg&&(c.Left(2)=="10"||c.Left(2)=="12"||c.Left(2)=="16")) StartHVReg(13);
	else if(!hvreg&&(c.Left(2)=="18")) StartHVReg(12);
//-------------PIC10-16---------------------------------------------------------
		if(EQ("10F200")||EQ("10F204")||EQ("10F220")){
			Read12F5xx(0x100,r?0x40:5);						//256
		}
		else if(EQ("12C508/A")||EQ("16F54")){
			Read12F5xx(0x200,r?0x40:4);						//512
		}
		else if(EQ("12F508")||EQ("10F202")||EQ("10F206")||EQ("10F222")){
			Read12F5xx(0x200,r?0x40:5);						//512
		}
		else if(EQ("16C83")||EQ("16F83")||EQ("16F83A")){
			Read16Fxxx(0x200,ee?0x40:0,r?0x10:8,1);			//512, 64, vdd
		}
		else if(EQ("12C509/A")){
			Read12F5xx(0x400,r?0x40:4);						//1K
		}
		else if(EQ("12F509")||EQ("12F510")||EQ("16F505")||EQ("16F506")){
			Read12F5xx(0x400,r?0x40:5);						//1K
		}
		else if(EQ("12F519")||EQ("16F526")){
			Read12F5xx(0x440,r?0x60:8);						//1K + 64
		}
		else if(EQ("12C671")||EQ("12CE673")){
			Read16Fxxx(0x400,0,r?0x100:0,0);				//1K, vpp
		}
		else if(EQ("12F609")||EQ("12F615")||EQ("16F610")){
			Read16Fxxx(0x400,0,r?0x40:9,0);					//1K, vpp, cal1
		}
		else if(EQ("16C84")||EQ("16F84")||EQ("16F84A")){
			Read16Fxxx(0x400,ee?0x40:0,r?0x10:8,1);			//1K, 64, vdd
		}
		else if(EQ("12F635")){
			Read16Fxxx(0x400,ee?0x80:0,r?0x40:10,0);		//1K, 128, vpp, cal1 + cal2
		}
		else if(EQ("16F631")){
			Read16Fxxx(0x400,ee?0x80:0,r?0x80:9,0);			//1K, 128, vpp, cal1
		}
		else if(EQ("12F629")||EQ("12F675")||EQ("16F630")||EQ("16F676")){
			Read16Fxxx(0x400,ee?0x80:0,r?0x20:8,0);			//1K, 128, vpp
		}
		else if(EQ("16F627")){
			Read16Fxxx(0x400,ee?-0x80:0,r?0x10:8,0);		//1K, 128, vpp, ee@0x2200
		}
		else if(EQ("16F627A")){
			Read16Fxxx(0x400,ee?0x80:0,r?0x10:8,0);			//1K, 128, vpp
		}
		else if(EQ("16F818")){
			Read16Fxxx(0x400,ee?0x80:0,r?0x10:8,2);			//1K, 128, vdd short delay
		}
		else if(EQ("16F57")||EQ("16F59")){
			Read12F5xx(0x800,r?0x40:4);						//2K
		}
		else if(EQ("12C672")||EQ("12CE674")){
			Read16Fxxx(0x800,0,r?0x100:0,0);				//2K, vpp
		}
		else if(EQ("16F716")){
			Read16Fxxx(0x800,0,8,2);						//2K, vdd
		}
		else if(EQ("16F616")){
			Read16Fxxx(0x800,0,r?0x40:9,0);					//2K, vpp, cal1
		}
		else if(EQ("16F870")||EQ("16F871")||EQ("16F872")){
			Read16Fxxx(0x800,ee?0x40:0,r?0x100:8,1);		//2K, 64, vdd
		}
		else if(EQ("16F628")){
			Read16Fxxx(0x800,ee?-0x80:0,r?0x10:8,0);		//2K, 128, vpp, ee@0x2200
		}
		else if(EQ("16F628A")){
			Read16Fxxx(0x800,ee?0x80:0,r?0x10:8,0);			//2K, 128, vpp
		}
		else if(EQ("16F882")){
			Read16Fxxx(0x800,ee?0x80:0,r?0x80:10,0);		//2K, 128, vpp, config2 + cal1
		}
		else if(EQ("16F819")){
			Read16Fxxx(0x800,ee?0x100:0,r?0x10:8,2);		//2K, 256, vdd short delay
		}
		else if(EQ("12F683")||EQ("16F684")){
			Read16Fxxx(0x800,ee?0x100:0,r?0x40:9,0);		//2K, 256, vpp, cal1
		}
		else if(EQ("16F636")||EQ("16F639")||EQ("16F785")||EQ("16F785")){
			Read16Fxxx(0x800,ee?0x100:0,r?0x40:10,0);		//2K, 256, vpp, cal1 + cal2
		}
		else if(EQ("16F677")||EQ("16F687")){
			Read16Fxxx(0x800,ee?0x100:0,r?0x80:9,0);		//2K, 256, vpp, cal1
		}
		else if(EQ("16F73")||EQ("16F74")){
			Read16Fxxx(0x1000,0,r?0x20:8,1);				//4K, vdd
		}
		else if(EQ("16F737")||EQ("16F747")){
			Read16Fxxx(0x1000,0,r?0x40:9,2);				//4K, vdd short delay
		}
		else if(EQ("16F873A")||EQ("16F874A")){
			Read16Fxxx(0x1000,ee?0x80:0,r?0x100:8,1);		//4K, 128, vdd
		}
		else if(EQ("16F873")||EQ("16F874")){
			Read16Fxxx(0x1000,ee?-0x80:0,r?0x100:8,1);		//4K, 128, vdd, ee@0x2200
		}
		else if(EQ("16F685")||EQ("16F689")||EQ("16F690")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x80:9,0);		//4K, 256, vpp, cal1
		}
		else if(EQ("16F688")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x40:9,0);		//4K, 256, vpp, cal1
		}
		else if(EQ("16F883")||EQ("16F884")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x80:10,0);		//4K, 256, vpp, config2 + cal1
		}
		else if(EQ("16F648A")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x10:8,0);		//4K, 256, vpp
		}
		else if(EQ("16F87")||EQ("16F88")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x10:9,2);		//4K, 256, vdd short delay
		}
		else if(EQ("16F913")||EQ("16F914")){
			Read16Fxxx(0x1000,ee?0x100:0,r?0x40:10,0);		//4K, 256, vpp, cal1 + cal2
		}
		else if(EQ("16F76")||EQ("16F77")){
			Read16Fxxx(0x2000,0,r?0x20:8,1);				//8K, vdd
		}
		else if(EQ("16F767")||EQ("16F777")){
			Read16Fxxx(0x2000,0,r?0x40:9,2);				//8K, vdd short delay
		}
		else if(EQ("16F876A")||EQ("16F877A")){
			Read16Fxxx(0x2000,ee?0x100:0,r?0x100:8,1);		//8K, 256, vdd
		}
		else if(EQ("16F876")||EQ("16F877")){
			Read16Fxxx(0x2000,ee?-0x100:0,r?0x100:8,1);	//8K, 256, vdd, ee@0x2200
		}
		else if(EQ("16F886")||EQ("16F887")){
			Read16Fxxx(0x2000,ee?0x100:0,r?0x80:10,0);		//8K, 256, vpp, config2 + cal1
		}
		else if(EQ("16F916")||EQ("16F917")||EQ("16F946")){
			Read16Fxxx(0x2000,ee?0x100:0,r?0x40:10,0);		//8K, 256, vpp, cal1 + cal2
		}
//-------------PIC18---------------------------------------------------------
		else if(EQ("18F1230")){
			Read18Fx(0x1000,ee?0x80:0);					//4K, 128
		}
		else if(EQ("18F2221")||EQ("18F4221")||EQ("18F1220")||EQ("18F2220")||EQ("18F4220")){
			Read18Fx(0x1000,ee?0x100:0);					//4K, 256
		}
		else if(EQ("18F1330")){
			Read18Fx(0x2000,ee?0x80:0);					//8K, 128
		}
		else if(EQ("18F2321")||EQ("18F4321")||EQ("18F1320")||EQ("18F2320")||EQ("18F4320")||EQ("18F2331")||EQ("18F4331")){
			Read18Fx(0x2000,ee?0x100:0);					//8K, 256
		}
		else if(EQ("18F2439")||EQ("18F4439")){
			Read18Fx(0x3000,ee?0x100:0);					//12K, 256
		}
		else if(EQ("18F2410")||EQ("18F4410")||EQ("18F2450")||EQ("18F4450")){
			Read18Fx(0x4000,0);							//16K, 0
		}
		else if(EQ("18F2420")||EQ("18F2423")||EQ("18F4420")||EQ("18F4423")||EQ("18F2431")||EQ("18F4431")||EQ("18F2480")||EQ("18F4480")||EQ("18F242")||EQ("18F248")||EQ("18F442")||EQ("18F448")){
			Read18Fx(0x4000,ee?0x100:0);					//16K, 256
		}
		else if(EQ("18F2455")||EQ("18F2458")||EQ("18F4455")||EQ("18F4458")||EQ("18F2539")||EQ("18F4539")){
			Read18Fx(0x6000,ee?0x100:0);					//24K, 256
		}
		else if(EQ("18F2510")||EQ("18F4510")){
			Read18Fx(0x8000,0);							//32K, 0
		}
		else if(EQ("18F2550")||EQ("18F2553")||EQ("18F4550")||EQ("18F4553")||EQ("18F2520")||EQ("18F2523")||EQ("18F4520")||EQ("18F4523")||EQ("18F2580")||EQ("18F4580")||EQ("18F252")||EQ("18F258")||EQ("18F452")||EQ("18F458")){
			Read18Fx(0x8000,ee?0x100:0);					//32K, 256
		}
		else if(EQ("18F2515")||EQ("18F4515")){
			Read18Fx(0xC000,0);							//48K, 0
		}
		else if(EQ("18F2525")||EQ("18F2585")||EQ("18F4525")||EQ("18F4585")){
			Read18Fx(0xC000,ee?0x400:0);					//48K, 1K
		}
		else if(EQ("18F2610")||EQ("18F4610")){
			Read18Fx(0x10000,0);							//64K, 0
		}
		else if(EQ("18F2620")||EQ("18F2680")||EQ("18F4620")||EQ("18F4680")){
			Read18Fx(0x10000,ee?0x400:0);					//64K, 1K
		}
		else if(EQ("18F2682")||EQ("18F4682")){
			Read18Fx(0x14000,ee?0x400:0);					//80K, 1K
		}
		else if(EQ("18F2685")||EQ("18F4685")){
			Read18Fx(0x18000,ee?0x400:0);					//96K, 1K
		}
		else if(EQ("18F8722")){
			Read18Fx(0x20000,ee?0x400:0);					//128K, 1K
		}
//-------------PIC24---------------------------------------------------------
		else if(EQ("24F04KA200")||EQ("24F04KA201")){
			Read24Fx(0xB00,0,3,0x05BE,r?0x800:0,0xFE00);			//1.375KW, HV
		}
		else if(EQ("24F08KA101")||EQ("24F08KA102")){
			Read24Fx(0x1600,ee?0x100:0,3,0x05BE,r?0x800:0,0xFE00);	//2.75KW, HV, 256W
		}
		else if(EQ("24F16KA101")||EQ("24F16KA102")){
			Read24Fx(0x2C00,ee?0x100:0,3,0x05BE,r?0x800:0,0xFE00);	//5.5KW, HV, 256W
		}
		else if(EQ("24FJ16GA002")||EQ("24FJ16GA004")){
			Read24Fx(0x2C00,0,0,0x05BE,r?0x800:0,0);				//5.5KW
		}
		else if(EQ("24FJ32GA002")||EQ("24FJ32GA004")){
			Read24Fx(0x5800,0,0,0x05BE,r?0x800:0,0);				//11KW
		}
		else if(EQ("24FJ48GA002")||EQ("24FJ48GA004")){
			Read24Fx(0x8400,0,0,0x05BE,r?0x800:0,0);				//16.5KW
		}
		else if(EQ("24FJ64GA002")||EQ("24FJ64GA004")||EQ("24FJ64GA006")||EQ("24FJ64GA008")||EQ("24FJ64GA010")){
			Read24Fx(0xAC00,0,0,0x05BE,r?0x800:0,0);				//22KW
		}
		else if(EQ("24FJ96GA006")||EQ("24FJ96GA008")||EQ("24FJ96GA010")){
			Read24Fx(0x10000,0,0,0x05BE,r?0x800:0,0);				//32KW
		}
		else if(EQ("24FJ128GA006")||EQ("24FJ128GA008")||EQ("24FJ128GA010")){
			Read24Fx(0x15800,0,0,0x05BE,r?0x800:0,0);				//44KW
		}
//-------------ATMEL---------------------------------------------------------
		else if(EQ("AT90S1200")){
			ReadAT(0x400,ee?0x40:0,0);							//1K, 64
		}
		else if(EQ("AT90S2313")){
			ReadAT(0x800,ee?0x80:0,0);							//2K, 128
		}
		else if(EQ("AT90S8515")||EQ("AT90S8535")){
			ReadAT(0x2000,ee?0x100:0,0);						//8K, 256
		}
		else if(EQ("ATmega8")||EQ("ATmega8A")||EQ("ATmega8515")||EQ("ATmega8535")){
			ReadAT(0x2000,ee?0x200:0,LOCK+FUSE+FUSE_H+CAL);		//8K, 512
		}
		else if(EQ("ATmega16")||EQ("ATmega16A")){
			ReadAT(0x4000,ee?0x200:0,LOCK+FUSE+FUSE_H+CAL);		//16K, 512
		}
		else if(EQ("ATmega32")||EQ("ATmega32A")){
			ReadAT(0x8000,ee?0x400:0,LOCK+FUSE+FUSE_H+CAL);		//32K, 1K
		}
		else if(EQ("ATmega64")||EQ("ATmega64A")){
			ReadAT(0x10000,ee?0x800:0,LOCK+FUSE+FUSE_H+FUSE_X+CAL);	//64K, 2K
		}
//-------------I2C---------------------------------------------------------
		else if(EQ("2400")){
			ReadI2C(0x10,0);						//16, 1B addr.
		}
		else if(EQ("2401")){
			ReadI2C(0x80,0);						//128, 1B addr.
		}
		else if(EQ("2402")){
			ReadI2C(0x100,0);						//256, 1B addr.
		}
		else if(EQ("2404")){
			ReadI2C(0x200,0);						//512, 1B addr.
		}
		else if(EQ("2408")){
			ReadI2C(0x400,0);						//1K, 1B addr.
		}
		else if(EQ("2416")){
			ReadI2C(0x800,0);						//2K, 1B addr.
		}
		else if(EQ("2432")){
			ReadI2C(0x1000,1);						//4K, 2B addr.
		}
		else if(EQ("2464")){
			ReadI2C(0x2000,1);						//8K, 2B addr.
		}
		else if(EQ("24128")){
			ReadI2C(0x4000,1);						//16K, 2B addr.
		}
		else if(EQ("24256")){
			ReadI2C(0x8000,1);						//32K, 2B addr.
		}
		else if(EQ("24512")){
			ReadI2C(0x10000,1);					//64K, 2B addr.
		}
		else if(EQ("241025")){
			ReadI2C(0x20000,1);					//128K, 2B addr.
		}
//-------------Microwire EEPROM---------------------------------------------------------
		else if(EQ("93S46")||EQ("93x46")){
			Read93x(0x80,6,0);						//128, 6b addr
		}
		else if(EQ("93x46A")){
			Read93x(0x80,7,1);						//128, 6b addr x8
		}
		else if(EQ("93S56")||EQ("93x56")){
			Read93x(0x100,8,0);						//256, 8b addr
		}
		else if(EQ("93x56A")){
			Read93x(0x100,9,1);						//256, 8b addr x8
		}
		else if(EQ("93S66")||EQ("93x66")){
			Read93x(0x200,8,0);						//512, 8b addr
		}
		else if(EQ("93x66A")){
			Read93x(0x200,9,1);						//512, 8b addr x8
		}
		else if(EQ("93x76")){
			Read93x(0x400,10,0);						//1k, 10b addr
		}
		else if(EQ("93x76A")){
			Read93x(0x400,11,1);						//1k, 10b addr x8
		}
		else if(EQ("93x86")){
			Read93x(0x800,10,0);						//2k, 10b addr
		}
		else if(EQ("93x86A")){
			Read93x(0x800,11,1);						//2k, 10b addr x8
		}
//-------------SPI---------------------------------------------------------
		else if(EQ("25010")){
			Read25xx(0x80);							//128
		}
		else if(EQ("25020")){
			Read25xx(0x100);						//256
		}
		else if(EQ("25040")){
			Read25xx(0x200);						//512
		}
		else if(EQ("25080")){
			Read25xx(0x400);						//1K
		}
		else if(EQ("25160")){
			Read25xx(0x800);						//2K
		}
		else if(EQ("25320")){
			Read25xx(0x1000);						//4K
		}
		else if(EQ("25640")){
			Read25xx(0x2000);						//8K
		}
		else if(EQ("25128")){
			Read25xx(0x4000);						//16K
		}
		else if(EQ("25256")){
			Read25xx(0x8000);						//32K
		}
		else if(EQ("25512")){
			Read25xx(0x10000);						//64K
		}
		else if(EQ("251024")){
			Read25xx(0x20000);						//128K
		}
//-------------Unsupported device---------------------------------------------------------
	else{
		PrintMessage(strings[S_nodev_r]); //"Dispositivo non supportato in lettura\r\n");
	}
}

#define CS 8
#define HLD 16
#define write()	Result=WriteFile(WriteHandle,bufferU,DIMBUF,&BytesWritten,NULL);
#define read()	Result = ReadFile(ReadHandle,bufferI,DIMBUF,&NumberOfBytesRead,(LPOVERLAPPED) &HIDOverlapped);\
				Result = WaitForSingleObject(hEventObject,10);\
				ResetEvent(hEventObject);\
				if(Result!=WAIT_OBJECT_0){\
					PrintMessage(strings[S_comTimeout]);	/*"Timeout comunicazione\r\n"*/\
				}
//					return;

void COpenProgDlg::OnI2cspiR()		// I2C/SPI receive
{
	DWORD BytesWritten=0;
	ULONG Result;
	int j=1;
	int nbyte=m_I2CSPIPage.GetDlgItemInt(IDC_NUMB);
	if(nbyte<0) nbyte=0;
	if(nbyte>60) nbyte=60;
	int mode=0;
	CButton* b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_I2C16B);
	if(b->GetCheck()) mode=1;	//I2C mode
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI00);
	if(b->GetCheck()) mode=2;	//SPI mode 00
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI01);
	if(b->GetCheck()) mode=3;	//SPI mode 01
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI10);
	if(b->GetCheck()) mode=4;	//SPI mode 10
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI11);
	if(b->GetCheck()) mode=5;	//SPI mode 11
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	int registro=b->GetCheck();
	CString str,t;
	m_I2CSPIPage.GetDlgItemText(IDC_STRI,str);
	char* tok;
	BYTE tmpbuf[128];
	int i=0;
	for(tok=strtok(str.GetBuffer(512)," ");tok&&i<128;tok=strtok(NULL," ")){
		if(sscanf(tok,"%x",&tmpbuf[i])) i++;
	}
	for(;i<128;i++) tmpbuf[i]=0;
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(registro){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("I2C-SPI receive\tmode=%d\n",mode);
		WriteLog(str);
	}
	bufferU[0]=0;
	bufferU[j++]=VREG_DIS;		//Disable HV reg
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x1;
	if(mode<2){					//I2C mode
		bufferU[j++]=I2C_INIT;
		bufferU[j++]=0;
	}
	else{						//SPI mode
		bufferU[j++]=EXT_PORT;	//CS=1
		bufferU[j++]=CS;
		bufferU[j++]=0;
		bufferU[j++]=EXT_PORT;	//CS=0
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_INIT;
		bufferU[j++]=mode-2;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	if(registro)WriteLogIO();
	j=1;
	if(mode==0){					//I2C read
		bufferU[j++]=I2C_READ;
		bufferU[j++]=nbyte>(DIMBUF-4)?DIMBUF-4:nbyte;
		bufferU[j++]=tmpbuf[0];		//Control byte
		bufferU[j++]=tmpbuf[1];		//Address;
	}
	else if(mode==1){				//I2C read 16bit
		bufferU[j++]=I2C_READ2;
		bufferU[j++]=nbyte>(DIMBUF-4)?DIMBUF-4:nbyte;
		bufferU[j++]=tmpbuf[0];		//Control byte
		bufferU[j++]=tmpbuf[1];		//Address H;
		bufferU[j++]=tmpbuf[2];		//Address L;
	}
	else if(mode>=2){					//SPI read
		bufferU[j++]=SPI_READ;
		bufferU[j++]=nbyte>(DIMBUF-5)?DIMBUF-5:nbyte;
		bufferU[j++]=EXT_PORT;		//CS=1
		bufferU[j++]=CS;
		bufferU[j++]=0;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(10);
	read();
	if(registro){
		WriteLogIO();
		CloseLogFile();
	}
	if(bufferI[1]==I2C_READ||bufferI[1]==I2C_READ2||bufferI[1]==SPI_READ){
		if(bufferI[2]==0xFD){
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_I2CAckErr]); //"Errore di acknowledge I2C"
		}
		else if(bufferI[2]>0xFA){
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InsErr]); //"Istruzione sconosciuta"
		}
		else{
			str.Empty();
			if(mode==0)	str.Format("> %02X %02X\r\n",bufferU[3],bufferU[4]);
			if(mode==1)	str.Format("> %02X %02X %02X\r\n",bufferU[3],bufferU[4],bufferU[5]);
			str+="< ";
			for(UINT i=0;i<bufferI[2];i++){
				t.Format("%02X ",(BYTE)bufferI[i+3]);
				str+=t;
				if(i&&i%16==15){
					str+="\r\n";
				}
			}
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,str);
		}
	}

}

void COpenProgDlg::OnI2cspiS() // I2C/SPI send
{
	DWORD BytesWritten=0;
	ULONG Result;
	int j=1;
	int nbyte=m_I2CSPIPage.GetDlgItemInt(IDC_NUMB);
	if(nbyte<0) nbyte=0;
	if(nbyte>57) nbyte=57;
	int mode=0;
	CButton* b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_I2C16B);
	if(b->GetCheck()) mode=1;	//I2C mode
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI00);
	if(b->GetCheck()) mode=2;	//SPI mode 00
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI01);
	if(b->GetCheck()) mode=3;	//SPI mode 01
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI10);
	if(b->GetCheck()) mode=4;	//SPI mode 10
	b=(CButton*)m_I2CSPIPage.GetDlgItem(IDC_SPI11);
	if(b->GetCheck()) mode=5;	//SPI mode 11
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	int registro=b->GetCheck();
	CString str,t;
	m_I2CSPIPage.GetDlgItemText(IDC_STRI,str);
	char* tok;
	BYTE tmpbuf[128];
	int i=0;
	for(tok=strtok(str.GetBuffer(512)," ");tok&&i<128;tok=strtok(NULL," ")){
		if(sscanf(tok,"%x",&tmpbuf[i])) i++;
	}
	for(;i<128;i++) tmpbuf[i]=0;
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(registro){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("I2C-SPI send\tmode=%d\n",mode);
		WriteLog(str);
	}
	bufferU[0]=0;
	bufferU[j++]=VREG_DIS;		//Disable HV reg
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x1;
	if(mode<2){					//I2C mode
		bufferU[j++]=I2C_INIT;
		bufferU[j++]=0;
	}
	else{						//SPI mode
		bufferU[j++]=EXT_PORT;	//CS=1
		bufferU[j++]=CS;
		bufferU[j++]=0;
		bufferU[j++]=EXT_PORT;	//CS=0
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_INIT;
		bufferU[j++]=mode-2;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	if(registro)WriteLogIO();
	j=1;
	if(mode==0){					//I2C write
		bufferU[j++]=I2C_WRITE;
		bufferU[j++]=nbyte>(DIMBUF-5)?DIMBUF-5:nbyte;
		bufferU[j++]=tmpbuf[0];		//Control byte
		bufferU[j++]=tmpbuf[1];		//Address
		for(i=0;i<bufferU[2];i++) bufferU[j++]=tmpbuf[i+2];
	}
	else if(mode==1){				//I2C write 16bit
		bufferU[j++]=I2C_WRITE;
		bufferU[j++]=nbyte+1>(DIMBUF-5)?DIMBUF-5:nbyte+1;
		bufferU[j++]=tmpbuf[0];		//Control byte
		bufferU[j++]=tmpbuf[1];		//Address
		bufferU[j++]=tmpbuf[2];		//Address L
		for(i=0;i<bufferU[2]-1;i++) bufferU[j++]=tmpbuf[i+3];
	}
	if(mode==2){					//SPI write
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=nbyte>(DIMBUF-5)?DIMBUF-5:nbyte;
		for(i=0;i<bufferU[2];i++) bufferU[j++]=tmpbuf[i];
		bufferU[j++]=EXT_PORT;	//CS=1
		bufferU[j++]=CS;
		bufferU[j++]=0;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(20);
	read();
	if(registro){
		WriteLogIO();
		CloseLogFile();
	}
	if(bufferI[1]==I2C_WRITE||bufferI[1]==SPI_WRITE){
		if(bufferI[2]==0xFD){
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_I2CAckErr]); //"Errore di acknowledge I2C"
		}
		else if(bufferI[2]>0xFA){
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InsErr]); //"Istruzione sconosciuta"
		}
		else{
			str="> ";
			int n=3;
			if(mode<2) n=5;
			for(i=3;i<bufferU[2]+n;i++){
				t.Format("%02X ",(BYTE)bufferU[i]);
				str+=t;
				if(i&&i%16==15){
					str+="\r\n";
				}
			}
			m_I2CSPIPage.SetDlgItemText(IDC_STRU,str);
		}
	}
	else m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_ComErr]);	//"Errore di comunicazione\r\n"
}

void COpenProgDlg::PIC_ID(int id)
{
	CString str;
#define print(s,t) str.Format(s,t)
	if(id>0x100000){				//ID 24F
		switch(id&0xFFFF){
			case 0x0405:
				print("24FJ64GA006\r\n",0);
				break;
			case 0x0406:
				print("24FJ96GA006\r\n",0);
				break;
			case 0x0407:
				print("24FJ128GA006\r\n",0);
				break;
			case 0x0408:
				print("24FJ64GA008\r\n",0);
				break;
			case 0x0409:
				print("24FJ96GA008\r\n",0);
				break;
			case 0x040A:
				print("24FJ128GA008\r\n",0);
				break;
			case 0x040B:
				print("24FJ64GA010\r\n",0);
				break;
			case 0x040C:
				print("24FJ96GA010\r\n",0);
				break;
			case 0x040D:
				print("24FJ128GA010\r\n",0);
				break;
			case 0x0444:
				print("24FJ16GA002\r\n",0);
				break;
			case 0x0445:
				print("24FJ32GA002\r\n",0);
				break;
			case 0x0446:
				print("24FJ48GA002\r\n",0);
				break;
			case 0x0447:
				print("24FJ64GA002\r\n",0);
				break;
			case 0x044C:
				print("24FJ16GA004\r\n",0);
				break;
			case 0x044D:
				print("24FJ32GA004\r\n",0);
				break;
			case 0x044E:
				print("24FJ48GA004\r\n",0);
				break;
			case 0x044F:
				print("24FJ64GA004\r\n",0);
				break;
			case 0x0D00:
				print("24F04KA201\r\n",0);
				break;
			case 0x0D01:
				print("24F16KA101\r\n",0);
				break;
			case 0x0D02:
				print("24F04KA200\r\n",0);
				break;
			case 0x0D03:
				print("24F16KA102\r\n",0);
				break;
			case 0x0D08:
				print("24F08KA101\r\n",0);
				break;
			case 0x0D0A:
				print("24F08KA102\r\n",0);
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
	}
	else if(id>0x10000){				//ID 18F
		switch((id&0xFFFF)>>5){
			case 0x040>>1:
				print("18F252/2539 rev%d\r\n",id&0x1F);
				break;
			case 0x042>>1:
				print("18F452/4539 rev%d\r\n",id&0x1F);
				break;
			case 0x048>>1:
				print("18F242/2439 rev%d\r\n",id&0x1F);
				break;
			case 0x04A>>1:
				print("18F442/4439 rev%d\r\n",id&0x1F);
				break;
			case 0x050>>1:
				print("18F2320 rev%d\r\n",id&0x1F);
				break;
			case 0x052>>1:
				print("18F4320 rev%d\r\n",id&0x1F);
				break;
			case 0x058>>1:
				print("18F2220 rev%d\r\n",id&0x1F);
				break;
			case 0x05A>>1:
				print("18F4220 rev%d\r\n",id&0x1F);
				break;
			case 0x07C>>1:
				print("18F1320 rev%d\r\n",id&0x1F);
				break;
			case 0x07E>>1:
				print("18F1220 rev%d\r\n",id&0x1F);
				break;
			case 0x080>>1:
				print("18F248 rev%d\r\n",id&0x1F);
				break;
			case 0x082>>1:
				print("18F448 rev%d\r\n",id&0x1F);
				break;
			case 0x084>>1:
				print("18F258 rev%d\r\n",id&0x1F);
				break;
			case 0x086>>1:
				print("18F458 rev%d\r\n",id&0x1F);
				break;
			case 0x088>>1:
				print("18F4431 rev%d\r\n",id&0x1F);
				break;
			case 0x08A>>1:
				print("18F4331 rev%d\r\n",id&0x1F);
				break;
			case 0x08C>>1:
				print("18F2431 rev%d\r\n",id&0x1F);
				break;
			case 0x08E>>1:
				print("18F2331 rev%d\r\n",id&0x1F);
				break;
			case 0x0C0>>1:
				print("18F4620 rev%d\r\n",id&0x1F);
				break;
			case 0x0C2>>1:
				print("18F4610 rev%d\r\n",id&0x1F);
				break;
			case 0x0C4>>1:
				print("18F4525 rev%d\r\n",id&0x1F);
				break;
			case 0x0C6>>1:
				print("18F4515 rev%d\r\n",id&0x1F);
				break;
			case 0x0C8>>1:
				print("18F2620 rev%d\r\n",id&0x1F);
				break;
			case 0x0CA>>1:
				print("18F2610 rev%d\r\n",id&0x1F);
				break;
			case 0x0CC>>1:
				print("18F2525 rev%d\r\n",id&0x1F);
				break;
			case 0x0CE>>1:
				print("18F2515 rev%d\r\n",id&0x1F);
				break;
			case 0x0E8>>1:
				print("18F4680 rev%d\r\n",id&0x1F);
				break;
			case 0x0EA>>1:
				print("18F4585 rev%d\r\n",id&0x1F);
				break;
			case 0x0EC>>1:
				print("18F2680 rev%d\r\n",id&0x1F);
				break;
			case 0x0EE>>1:
				print("18F2585 rev%d\r\n",id&0x1F);
				break;
			case 0x108>>1:
				if(id&0x10) print("18F4523 rev%d\r\n",id&0x1F);
				else print("18F4520 rev%d\r\n",id&0x1F);
				break;
			case 0x10A>>1:
				print("18F4510 rev%d\r\n",id&0x1F);
				break;
			case 0x10C>>1:
				if(id&0x10) print("18F4423 rev%d\r\n",id&0x1F);
				else print("18F4420 rev%d\r\n",id&0x1F);
				break;
			case 0x10E>>1:
				print("18F4410 rev%d\r\n",id&0x1F);
				break;
			case 0x110>>1:
				if(id&0x10) print("18F2523 rev%d\r\n",id&0x1F);
				else print("18F2520 rev%d\r\n",id&0x1F);
				break;
			case 0x112>>1:
				print("18F2510 rev%d\r\n",id&0x1F);
				break;
			case 0x114>>1:
				if(id&0x10) print("18F2423 rev%d\r\n",id&0x1F);
				else print("18F2420 rev%d\r\n",id&0x1F);
				break;
			case 0x116>>1:
				print("18F2410 rev%d\r\n",id&0x1F);
				break;
			case 0x120>>1:
				print("18F4550 rev%d\r\n",id&0x1F);
				break;
			case 0x122>>1:
				print("18F4455 rev%d\r\n",id&0x1F);
				break;
			case 0x124>>1:
				print("18F2550 rev%d\r\n",id&0x1F);
				break;
			case 0x126>>1:
				print("18F2455 rev%d\r\n",id&0x1F);
				break;
			case 0x134>>1:
				print("18F6527 rev%d\r\n",id&0x1F);
				break;
			case 0x136>>1:
				print("18F8527 rev%d\r\n",id&0x1F);
				break;
			case 0x138>>1:
				print("18F6622 rev%d\r\n",id&0x1F);
				break;
			case 0x13A>>1:
				print("18F8622 rev%d\r\n",id&0x1F);
				break;
			case 0x13C>>1:
				print("18F6627 rev%d\r\n",id&0x1F);
				break;
			case 0x13E>>1:
				print("18F8627 rev%d\r\n",id&0x1F);
				break;
			case 0x140>>1:
				print("18F6722 rev%d\r\n",id&0x1F);
				break;
			case 0x142>>1:
				print("18F8722 rev%d\r\n",id&0x1F);
				break;
			case 0x1A8>>1:
				print("18F4580 rev%d\r\n",id&0x1F);
				break;
			case 0x1AA>>1:
				print("18F4480 rev%d\r\n",id&0x1F);
				break;
			case 0x1AC>>1:
				print("18F2580 rev%d\r\n",id&0x1F);
				break;
			case 0x1AE>>1:
				print("18F2480 rev%d\r\n",id&0x1F);
				break;
			case 0x1E0>>1:
				print("18F1230 rev%d\r\n",id&0x1F);
				break;
			case 0x1E2>>1:
				print("18F1330 rev%d\r\n",id&0x1F);
				break;
			case 0x1FE>>1:
				print("18F1330-ICD rev%d\r\n",id&0x1F);
				break;
			case 0x210>>1:
				print("18F4321 rev%d\r\n",id&0x1F);
				break;
			case 0x212>>1:
				print("18F2321 rev%d\r\n",id&0x1F);
				break;
			case 0x214>>1:
				print("18F4221 rev%d\r\n",id&0x1F);
				break;
			case 0x216>>1:
				print("18F2221 rev%d\r\n",id&0x1F);
				break;
			case 0x240>>1:
				print("18F4450 rev%d\r\n",id&0x1F);
				break;
			case 0x242>>1:
				print("18F2450 rev%d\r\n",id&0x1F);
				break;
			case 0x270>>1:
				print("18F2682 rev%d\r\n",id&0x1F);
				break;
			case 0x272>>1:
				print("18F2685 rev%d\r\n",id&0x1F);
				break;
			case 0x274>>1:
				print("18F4682 rev%d\r\n",id&0x1F);
				break;
			case 0x276>>1:
				print("18F4685 rev%d\r\n",id&0x1F);
				break;
			case 0x2A0>>1:
				print("18F4553 rev%d\r\n",id&0x1F);
				break;
			case 0x2A2>>1:
				print("18F4458 rev%d\r\n",id&0x1F);
				break;
			case 0x2A4>>1:
				print("18F2553 rev%d\r\n",id&0x1F);
				break;
			case 0x2A6>>1:
				print("18F2458 rev%d\r\n",id&0x1F);
				break;
			case 0x49C>>1:
				print("18F6628 rev%d\r\n",id&0x1F);
				break;
			case 0x49E>>1:
				print("18F8628 rev%d\r\n",id&0x1F);
				break;
			case 0x4A0>>1:
				print("18F6723 rev%d\r\n",id&0x1F);
				break;
			case 0x4A2>>1:
				print("18F8723 rev%d\r\n",id&0x1F);
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
	}
	else{						//ID16F
		switch(id>>5){
			case 0x046>>1:		//00 0100 011x xxxx
				print("12F683 rev%d\r\n",id&0x1F);
				break;
			case 0x04A>>1:		//00 0100 101x xxxx
				print("16F685 rev%d\r\n",id&0x1F);
				break;
			case 0x04C>>1:		//00 0100 110x xxxx
				print("16F818 rev%d\r\n",id&0x1F);
				break;
			case 0x04E>>1:		//00 0100 111x xxxx
				print("16F819 rev%d\r\n",id&0x1F);
				break;
			case 0x056>>1:		//00 0101 011x xxxx
				print("16F84A rev%d\r\n",id&0x1F);
				break;
			case 0x060>>1:		//00 0110 000x xxxx
				print("16F73 rev%d\r\n",id&0x1F);
				break;
			case 0x062>>1:		//00 0110 001x xxxx
				print("16F74 rev%d\r\n",id&0x1F);
				break;
			case 0x064>>1:		//00 0110 010x xxxx
				print("16F76 rev%d\r\n",id&0x1F);
				break;
			case 0x066>>1:		//00 0110 011x xxxx
				print("16F77 rev%d\r\n",id&0x1F);
				break;
			case 0x072>>1:		//00 0111 001x xxxx
				print("16F87 rev%d\r\n",id&0x1F);
				break;
			case 0x076>>1:		//00 0111 011x xxxx
				print("16F88 rev%d\r\n",id&0x1F);
				break;
			case 0x07A>>1:		//00 0111 101x xxxx
				print("16F627 rev%d\r\n",id&0x1F);
				break;
			case 0x07C>>1:		//00 0111 110x xxxx
				print("16F628 rev%d\r\n",id&0x1F);
				break;
			case 0x08E>>1:		//00 1000 111x xxxx
				print("16F872 rev%d\r\n",id&0x1F);
				break;
			case 0x092>>1:		//00 1001 001x xxxx
				print("16F874 rev%d\r\n",id&0x1F);
				break;
			case 0x096>>1:		//00 1001 011x xxxx
				print("16F873 rev%d\r\n",id&0x1F);
				break;
			case 0x09A>>1:		//00 1001 101x xxxx
				print("16F877 rev%d\r\n",id&0x1F);
				break;
			case 0x09E>>1:		//00 1001 111x xxxx
				print("16F876 rev%d\r\n",id&0x1F);
				break;
			case 0x0BA>>1:		//00 1011 101x xxxx
				print("16F737 rev%d\r\n",id&0x1F);
				break;
			case 0x0BE>>1:		//00 1011 111x xxxx
				print("16F747 rev%d\r\n",id&0x1F);
				break;
			case 0x0DE>>1:		//00 1101 111x xxxx
				print("16F777 rev%d\r\n",id&0x1F);
				break;
			case 0x0E0>>1:		//00 1110 0000 xxxx
				print("16F876A rev%d\r\n",id&0xF);
				break;
			case 0x0E2>>1:		//00 1110 0010 xxxx
				print("16F877A rev%d\r\n",id&0xF);
				break;
			case 0x0E4>>1:		//00 1110 0100 xxxx
				print("16F873A rev%d\r\n",id&0xF);
				break;
			case 0x0E6>>1:		//00 1110 0110 xxxx
				print("16F874A rev%d\r\n",id&0xF);
				break;
			case 0x0EA>>1:		//00 1110 101x xxxx
				print("16F767 rev%d\r\n",id&0x1F);
				break;
			case 0x0F8>>1:		//00 1111 100x xxxx
				print("12F629 rev%d\r\n",id&0x1F);
				break;
			case 0x0FA>>1:		//00 1111 101x xxxx
				print("12F635 rev%d\r\n",id&0x1F);
				break;
			case 0x0FC>>1:		//00 1111 110x xxxx
				print("12F675 rev%d\r\n",id&0x1F);
				break;
			case 0x104>>1:		//01 0000 010x xxxx
				print("16F627A rev%d\r\n",id&0x1F);
				break;
			case 0x106>>1:		//01 0000 011x xxxx
				print("16F628A rev%d\r\n",id&0x1F);
				break;
			case 0x108>>1:		//01 0000 100x xxxx
				print("16F684 rev%d\r\n",id&0x1F);
				break;
			case 0x110>>1:		//01 0001 000x xxxx
				print("16F648A rev%d\r\n",id&0x1F);
				break;
			case 0x10A>>1:		//01 0000 101x xxxx
				print("16F636-639 rev%d\r\n",id&0x1F);
				break;
			case 0x10C>>1:		//01 0000 110x xxxx
				print("16F630 rev%d\r\n",id&0x1F);
				break;
			case 0x10E>>1:		//01 0000 111x xxxx
				print("16F676 rev%d\r\n",id&0x1F);
				break;
			case 0x114>>1:		//01 0001 010x xxxx
				print("16F716 rev%d\r\n",id&0x1F);
				break;
			case 0x118>>1:		//01 0001 100x xxxx
				print("16F688 rev%d\r\n",id&0x1F);
				break;
			case 0x120>>1:		//01 0010 000x xxxx
				print("16F785 rev%d\r\n",id&0x1F);
				break;
			case 0x122>>1:		//01 0010 001x xxxx
				print("16HV785 rev%d\r\n",id&0x1F);
				break;
			case 0x124>>1:		//01 0010 010x xxxx
				print("16F616 rev%d\r\n",id&0x1F);
				break;
			case 0x126>>1:		//01 0010 011x xxxx
				print("16HV616 rev%d\r\n",id&0x1F);
				break;
			case 0x132>>1:		//01 0011 001x xxxx
				print("16F687 rev%d\r\n",id&0x1F);
				break;
			case 0x134>>1:		//01 0011 010x xxxx
				print("16F689 rev%d\r\n",id&0x1F);
				break;
			case 0x138>>1:		//01 0011 1000 xxxx
				print("16F917 rev%d\r\n",id&0xF);
				break;
			case 0x13A>>1:		//01 0011 1010 xxxx
				print("16F916 rev%d\r\n",id&0xF);
				break;
			case 0x13C>>1:		//01 0011 1100 xxxx
				print("16F914 rev%d\r\n",id&0xF);
				break;
			case 0x13E>>1:		//01 0011 1110 xxxx
				print("16F913 rev%d\r\n",id&0xF);
				break;
			case 0x140>>1:		//01 0100 000x xxxx
				print("16F690 rev%d\r\n",id&0x1F);
				break;
			case 0x142>>1:		//01 0100 001x xxxx
				print("16F631 rev%d\r\n",id&0x1F);
				break;
			case 0x144>>1:		//01 0100 010x xxxx
				print("16F677 rev%d\r\n",id&0x1F);
				break;
			case 0x146>>1:		//01 0100 0110 xxxx
				print("16F946 rev%d\r\n",id&0xF);
				break;
			case 0x1D0>>1:		//00 1101 000x xxxx
				print("16F870 rev%d\r\n",id&0x1F);
				break;
			case 0x1D2>>1:		//00 1101 001x xxxx
				print("16F871 rev%d\r\n",id&0x1F);
				break;
			case 0x200>>1:		//10 0000 000x xxxx
				print("16F882 rev%d\r\n",id&0x1F);
				break;
			case 0x202>>1:		//10 0000 001x xxxx
				print("16F883 rev%d\r\n",id&0x1F);
				break;
			case 0x204>>1:		//10 0000 010x xxxx
				print("16F884 rev%d\r\n",id&0x1F);
				break;
			case 0x206>>1:		//10 0000 011x xxxx
				print("16F886 rev%d\r\n",id&0x1F);
				break;
			case 0x208>>1:		//10 0000 100x xxxx
				print("16F887 rev%d\r\n",id&0x1F);
				break;
			case 0x218>>1:		//10 0001 100x xxxx
				print("12F615 rev%d\r\n",id&0x1F);
				break;
			case 0x21A>>1:		//10 0001 101x xxxx
				print("12HV615 rev%d\r\n",id&0x1F);
				break;
			case 0x224>>1:		//10 0010 010x xxxx
				print("12F609 rev%d\r\n",id&0x1F);
				break;
			case 0x226>>1:		//10 0010 011x xxxx
				print("16F610 rev%d\r\n",id&0x1F);
				break;
			case 0x228>>1:		//10 0010 100x xxxx
				print("12HV609 rev%d\r\n",id&0x1F);
				break;
			case 0x22A>>1:		//10 0010 101x xxxx
				print("16HV610 rev%d\r\n",id&0x1F);
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
	}
	PrintMessage(str);
}


void COpenProgDlg::PrintMessage(LPCTSTR s)
{
	dati+=s;
	if(dati.GetLength()>100000) dati=dati.Right(100000);
	CEdit* e=(CEdit*)m_DatiPage.GetDlgItem(IDC_DATI);
	e->SetWindowText(dati);
	e->LineScroll(5000);
}

void COpenProgDlg::DisplayEE(){
	CString str,aux,s,t;
	PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
	int valid;
	for(int i=0;i<memEE.GetSize();i+=COL){
		valid=0;
		for(int j=i;j<i+COL&&j<memEE.GetSize();j++){
			t.Format("%02X ",memEE[j]);
			s+=t;
			t.Format("%c",isprint(memEE[j])?memEE[j]:'.');
			str+=t;
			if(memEE[j]<0xff) valid=1;
		}
		if(valid){
			t.Format("%04X: %s %s\r\n",i,s,str);
			aux+=t;
		}
		s.Empty();
		str.Empty();
	}
	if(aux.GetLength()) PrintMessage(aux);
	else PrintMessage(strings[S_Empty]);	//empty
}

int COpenProgDlg::StartHVReg(double V=13)
{
	DWORD BytesWritten=0;
	ULONG Result;
	int j=1,z;
	int vreg=(int)(V*10.0);
	bufferU[0]=0;
	if(MyDeviceDetected==FALSE) return 0;
	DWORD t0,t;
	CString str;
	CancelIo(ReadHandle);
	if(V==-1){
		bufferU[j++]=VREG_DIS;			//disable HV regulator
		bufferU[j++]=FLUSH;
		write();
		msDelay(20);
		read();
		return -1;
	}
	t=t0=GetTickCount();
	bufferU[j++]=VREG_EN;			//enable HV regulator
	bufferU[j++]=SET_VPP;
	bufferU[j++]=vreg<80?vreg-8:vreg;		//set VPP, compensate for offset at low voltage
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=READ_ADC;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(20);
	read();
	for(z=1;z<DIMBUF-2&&bufferI[z]!=READ_ADC;z++);
	int v=(bufferI[z+1]<<8)+bufferI[z+2];
	if(v==0){
		PrintMessage(strings[S_lowUsbV]);	//"Tensione USB troppo bassa (VUSB<4.5V)\r\n"
		return 0;
	}
	for(;(v<(vreg/10.0-0.5)*G||v>(vreg/10.0+0.5)*G)&&t<t0+500;t=GetTickCount()){
		j=1;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=READ_ADC;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=READ_ADC;z++);
		v=(bufferI[z+1]<<8)+bufferI[z+2];
	}
	if(v>(vreg/10.0+0.7)*G){
		PrintMessage(strings[S_HiVPP]);	//"Attenzione: tensione regolatore troppo alta\r\n\r\n"
		StatusBar.SetWindowText(strings[S_HiVPP]);
		return 0;
	}
	else if(v<(vreg/10.0-0.7)*G){
		PrintMessage(strings[S_LowVPP]);	//"Attenzione: tensione regolatore troppo bassa\r\n\r\n"
		StatusBar.SetWindowText(strings[S_LowVPP]);
		return 0;
	}
	else if(v==0){
		PrintMessage(strings[S_lowUsbV]);	//"Tensione USB troppo bassa (VUSB<4.5V)\r\n"
		StatusBar.SetWindowText(strings[S_lowUsbV]);
		return 0;
	}
	else{
		str.Format(strings[S_reg],t-t0,v/G);	//"Regolatore avviato e funzionante dopo T=%d ms VPP=%.1f\r\n\r\n"
		PrintMessage(str);
		hvreg=1;
		CancelIo(ReadHandle);
		return vreg;
	}
}

void COpenProgDlg::ProgID()
{
	DWORD BytesWritten=0;
	ULONG Result;
	int j=1;
	bufferU[0]=0;
	if(MyDeviceDetected==FALSE) return;
	CString str;
	CancelIo(ReadHandle);
	bufferU[j++]=PROG_RST;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	str.Format(strings[S_progver],bufferI[2],bufferI[3],bufferI[4]); //"Programmatore versione %d.%d.%d\r\n"
	PrintMessage(str);
	FWVersion=(bufferI[2]<<16)+(bufferI[3]<<8)+bufferI[4];
	str.Format(strings[S_progid],bufferI[5],bufferI[6],bufferI[7]);	//"ID: %d.%d.%d\r\n"
	PrintMessage(str);
	if(bufferI[7]==1) str.Format(" (18F2550)\r\n\r\n");
	else if(bufferI[7]==2) str.Format(" (18F2450)\r\n\r\n");
	else str.Format(" (?)\r\n\r\n");
	PrintMessage(str);
	CancelIo(ReadHandle);
}

void COpenProgDlg::OnFileSave()
{
	CFileDialog dlg(FALSE,"hex",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,strings[S_file2]);	//"File Hex8 (*.hex)|*.hex|File binari (*.bin)|*.bin|Tutti i file (*.*)|*.*||"
	if (rfile!=""||dlg.DoModal()==IDOK){
		CFile f;
		if (f.Open(rfile!=""?rfile:dlg.GetPathName(),CFile::modeWrite|CFile::modeCreate))	{
			CString c;
			m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),c);
//**************** 10-16F *******************************************
			if(c.Left(2)=="10"||c.Left(2)=="12"||c.Left(2)=="16"){
				CString line,str,str1;
				int i,x=0xfff;
				if(c.Left(2)=="16") x=0x3fff;
				line.Format(":020000040000FA\n");			//extended address = 0
				f.Write(LPCTSTR(line),line.GetLength());
				int sum=0,count=0,s;
				for(i=0;i<dati_hex.GetSize()&&dati_hex[i]>=x;i++); //remove leading 0xFFF
				for(;i<dati_hex.GetSize();i++){
					sum+=(dati_hex[i]>>8)+dati_hex[i]&0xff;
					str.Format("%02X%02X",dati_hex[i]&0xff,dati_hex[i]>>8);
					str1+=str;
					count++;
					if(count==8||i==dati_hex.GetSize()-1){
						for(s=i;s>i-count&&dati_hex[s]>=x;s--){	//remove trailing 0xFFF
							sum-=(dati_hex[s]>>8)+dati_hex[s]&0xff;
							str1=str1.Left(str1.GetLength()-4);
						}
						count-=i-s;
						sum+=count*2+(((s-count+1)*2)&0xff)+(((s-count+1)*2)>>8);
						line.Format(":%02X%04X00%s%02X\n",count*2,(s-count+1)*2,str1,(-sum)&0xff);
						f.Write(LPCTSTR(line),line.GetLength());
						str1.Empty();
						count=sum=0;
					}
				}
				line.Format(":00000001FF\n");
				f.Write(LPCTSTR(line),line.GetLength());
			}
//**************** 18F *******************************************
			else if(c.Left(3)=="18F"){
				CString line,str,str1;
				int i,ext=0,base;
				line.Format(":020000040000FA\n");			//extended address = 0
				f.Write(LPCTSTR(line),line.GetLength());
				int sum=0,count=0,s;
				for(i=0;i<memCODE.GetSize()&&memCODE[i]==0xff;i++); //remove leading 0xFF
				for(;i<memCODE.GetSize();i++){
					sum+=memCODE[i];
					str.Format("%02X",memCODE[i]&0xff);
					str1+=str;
					count++;
					if(count==16||i==memCODE.GetSize()-1){
						base=i-count+1;
						for(s=i;s>=base&&memCODE[s]==0xff;s--){	//remove trailing 0xFF
							sum-=memCODE[s];
							str1=str1.Left(str1.GetLength()-2);
						}
						count-=i-s;
						sum+=count+(base&0xff)+(base>>8)&0xff;
						if(base>>16>ext){
							ext=base>>16;
							line.Format(":02000004%04X%02X\n",ext,(-6-ext)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						if(count){
							line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						str1.Empty();
						count=sum=0;
					}
				}
				for(i=0,count=sum=0;i<memID.GetSize()&&i<8;i++){
					sum+=memID[i];
					str.Format("%02X",memID[i]&0xff);
					str1+=str;
					count++;
					if(count==8||i==memID.GetSize()-1){
						line.Format(":020000040020DA\n");
						f.Write(LPCTSTR(line),line.GetLength());
						for(s=i;s>i-count&&memID[s]>=0xff;s--){	//remove trailing 0xFF
							sum-=memID[s]&0xff;
							str1=str1.Left(str1.GetLength()-2);
						}
						count-=i-s;
						sum+=count+(s-count+1)&0xff+((s-count+1)>>8);
						if(count){
							line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						str1.Empty();
						count=sum=0;
					}
				}
				if(memCONFIG.GetSize()){
					line.Format(":020000040030CA\n");
					f.Write(LPCTSTR(line),line.GetLength());
					for(i=0,count=sum=0;i<memCONFIG.GetSize()&&i<14;i++){
						sum+=memCONFIG[i];
						str.Format("%02X",memCONFIG[i]&0xff);
						str1+=str;
						count++;
						if(count==13||i==memCONFIG.GetSize()-1){
							for(s=i;s>i-count&&memCONFIG[s]>=0xff;s--){	//remove trailing 0xFF
								sum-=memCONFIG[s]&0xff;
								str1=str1.Left(str1.GetLength()-2);
							}
							count-=i-s;
							sum+=count+(s-count+1)&0xff+((s-count+1)>>8);
							if(count){
								line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
								f.Write(LPCTSTR(line),line.GetLength());
							}
							str1.Empty();
							count=sum=0;
						}
					}
				}
				if(memEE.GetSize()){
					line.Format(":0200000400F00A\n");
					f.Write(LPCTSTR(line),line.GetLength());
					for(i=0,count=sum=0;i<memEE.GetSize();i++){
						sum+=memEE[i];
						str.Format("%02X",memEE[i]&0xff);
						str1+=str;
						count++;
						if(count==16||i==memEE.GetSize()-1){
							for(s=i;s>i-count&&memEE[s]>=0xff;s--){	//remove trailing 0xFF
								sum-=memEE[s]&0xff;
								str1=str1.Left(str1.GetLength()-2);
							}
							count-=i-s;
							sum+=count+(s-count+1)&0xff+((s-count+1)>>8);
							if(count){
								line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
								f.Write(LPCTSTR(line),line.GetLength());
							}
							str1.Empty();
							count=sum=0;
						}
					}
				}
				line.Format(":00000001FF\n");
				f.Write(LPCTSTR(line),line.GetLength());
			}
//**************** 24F *******************************************
			else if(c.Left(3)=="24F"){
				CString line,str,str1;
				int i,ext=0,base;
				int valid;
				line.Format(":020000040000FA\n");			//extended address = 0
				f.Write(LPCTSTR(line),line.GetLength());
				int sum=0,count=0,s,word;
				word=memCODE[0]+(memCODE[1]<<8)+(memCODE[2]<<16)+(memCODE[3]<<24);
				for(i=0;i<memCODE.GetSize()&&word==0xffffffff;i+=4) //remove leading 0xFFFFFFFF
					word=memCODE[i]+(memCODE[i+1]<<8)+(memCODE[i+2]<<16)+(memCODE[i+3]<<24); 
				for(;i<memCODE.GetSize();i++){
					sum+=memCODE[i];
					str.Format("%02X",memCODE[i]&0xff);
					str1+=str;
					count++;
					if(count==16||i==memCODE.GetSize()-1){
						base=i-count+1;
						for(s=base,valid=0;s<=i&&!valid;s+=4){	//remove empty lines
							if(memCODE[s]<0xFF||memCODE[s+1]<0xFF||+memCODE[s+2]<0xFF) valid=1;
						}
/*						word=memCODE[i]+memCODE[i+1]<<8+memCODE[i+2]<<16+memCODE[i+3]<<24; 
						for(s=i;s>=base&&memCODE[s]==0xff;s--){	//remove trailing 0xFF
							sum-=memCODE[s];
							str1=str1.Left(str1.GetLength()-2);
						}
						count-=i-s;*/
						s=i;
						sum+=count+(base&0xff)+(base>>8)&0xff;
						if(base>>16>ext){
							ext=base>>16;
							line.Format(":02000004%04X%02X\n",ext,(-6-ext)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						if(count&&valid){
							line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						str1.Empty();
						count=sum=0;
					}
				}
				if(memCONFIG.GetSize()){
					line.Format(":0200000401F009\n");
					f.Write(LPCTSTR(line),line.GetLength());
					for(i=0,count=sum=0;i<memCONFIG.GetSize()&&i<34;i++){
						sum+=memCONFIG[i];
						str.Format("%02X",memCONFIG[i]&0xff);
						str1+=str;
						count++;
						if(count==4||i==memCONFIG.GetSize()-1){
/*							for(s=i;s>i-count&&memCONFIG[s]>=0xff;s--){	//remove trailing 0xFF
								sum-=memCONFIG[s]&0xff;
								str1=str1.Left(str1.GetLength()-2);
							}
							count-=i-s;*/
							s=i;
							sum+=count+(s-count+1)&0xff+((s-count+1)>>8);
							if(count){
								line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
								f.Write(LPCTSTR(line),line.GetLength());
							}
							str1.Empty();
							count=sum=0;
						}
					}
				}
				if(memEE.GetSize()){
					line.Format(":0200000400FFFB\n");
					f.Write(LPCTSTR(line),line.GetLength());
					for(i=0,count=sum=0;i<memEE.GetSize();i++){
						sum+=memEE[i];
						str.Format("%02X",memEE[i]&0xff);
						str1+=str;
						count++;
						if(count==16||i==memEE.GetSize()-1){
							for(s=i-count+1,valid=0;s<=i&&!valid;s+=4){	//remove empty lines
								if(memEE[s]<0xFF||memEE[s+1]<0xFF) valid=1;
							}
				/*			for(s=i;s>i-count&&memEE[s]>=0xff;s--){	//remove trailing 0xFF
								sum-=memEE[s]&0xff;
								str1=str1.Left(str1.GetLength()-2);
							}
							count-=i-s;*/
							s=i;
							sum+=0xfc+count+((s-count+1)&0xff)+((s-count+1)>>8);
							if(count&&valid){
								line.Format(":%02X%04X00%s%02X\n",count,(s-count+1)+0xFC00,str1,(-sum)&0xff);
								f.Write(LPCTSTR(line),line.GetLength());
							}
							str1.Empty();
							count=sum=0;
						}
					}
				}
				line.Format(":00000001FF\n");
				f.Write(LPCTSTR(line),line.GetLength());
			}
//**************** ATxxxx *******************************************
			else if(c.Left(2)=="AT"){
				CString line,str,str1;
				int i,ext=0,base;
				line.Format(":020000040000FA\n");			//extended address = 0
				f.Write(LPCTSTR(line),line.GetLength());
				int sum=0,count=0,s;
				for(i=0;i<memCODE.GetSize()&&memCODE[i]==0xff;i++); //remove leading 0xFF
				for(;i<memCODE.GetSize();i++){
					sum+=memCODE[i];
					str.Format("%02X",memCODE[i]&0xff);
					str1+=str;
					count++;
					if(count==16||i==memCODE.GetSize()-1){
						base=i-count+1;
						for(s=i;s>=base&&memCODE[s]==0xff;s--){	//remove trailing 0xFF
							sum-=memCODE[s];
							str1=str1.Left(str1.GetLength()-2);
						}
						count-=i-s;
						sum+=count+(base&0xff)+(base>>8)&0xff;
						if(base>>16>ext){
							ext=base>>16;
							line.Format(":02000004%04X%02X\n",ext,(-6-ext)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						if(count){
							line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
						}
						str1.Empty();
						count=sum=0;
					}
				}
				line.Format(":00000001FF\n");
				f.Write(LPCTSTR(line),line.GetLength());
				if(memEE.GetSize()){
					CFileDialog dlg2(FALSE,"hex;eep",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,strings[S_fileEEP]);	//"File Hex8 (*.hex;.eep ..."S_file]);
					dlg2.m_ofn.lpstrTitle =strings[S_saveEEfile];		//"Salva eeprom";
					if (dlg2.DoModal()==IDOK){
						CFile fe;
						if (fe.Open(dlg2.GetPathName(),CFile::modeWrite|CFile::modeCreate)){
							line.Format(":020000040000FA\n");			//extended address = 0
							fe.Write(LPCTSTR(line),line.GetLength());
							for(i=0,count=sum=0;i<memEE.GetSize();i++){
								sum+=memEE[i];
								str.Format("%02X",memEE[i]&0xff);
								str1+=str;
								count++;
								if(count==16||i==memEE.GetSize()-1){
									for(s=i;s>i-count&&memEE[s]>=0xff;s--){	//remove trailing 0xFF
										sum-=memEE[s]&0xff;
										str1=str1.Left(str1.GetLength()-2);
									}
									count-=i-s;
									sum+=count+(s-count+1)&0xff+((s-count+1)>>8);
									if(count){
										line.Format(":%02X%04X00%s%02X\n",count,(s-count+1),str1,(-sum)&0xff);
										fe.Write(LPCTSTR(line),line.GetLength());
									}
									str1.Empty();
									count=sum=0;
								}
							}
							line.Format(":00000001FF\n");
							fe.Write(LPCTSTR(line),line.GetLength());
							fe.Close();
						}
					}
				}
			}
//**************** 24xxx / 93xxx / 25xxx *******************************************
			else if(c.Left(2)=="24"||c.Left(2)=="93"||c.Left(2)=="25"){
				if(dlg.GetFileExt()=="bin"||dlg.GetFileExt()=="BIN"){
					f.Write(memEE.GetData(),memEE.GetSize());
				}
				else if(dlg.GetFileExt()=="hex"||dlg.GetFileExt()=="HEX"){
					CString line,str,str1;
					int i;
					line.Format(":020000040000FA\n");			//extended address = 0
					f.Write(LPCTSTR(line),line.GetLength());
					int sum=0,count=0;
					for(i=0;i<memEE.GetSize();i++){
						sum+=memEE[i];
						str.Format("%02X",memEE[i]);
						str1+=str;
						count++;
						if(count==16||i==memEE.GetSize()-1){
							sum+=count+((i-count+1)&0xff)+((i-count+1)>>8);
							line.Format(":%02X%04X00%s%02X\n",count,(i-count+1),str1,(-sum)&0xff);
							f.Write(LPCTSTR(line),line.GetLength());
							str1.Empty();
							count=sum=0;
						}
					}
					line.Format(":00000001FF\n");
					f.Write(LPCTSTR(line),line.GetLength());
				}
			}
		f.Close();
		}
	}
}

void COpenProgDlg::OnFileOpen()
{
	CFileDialog dlg(TRUE,"hex",NULL,OFN_HIDEREADONLY,strings[S_file2]);	//"File Hex8 (*.hex)|*.hex|File binari (*.bin)|*.bin|Tutti i file (*.*)|*.*||"
	if (wfile!=""||dlg.DoModal()==IDOK){
		CStdioFile f;
		CByteArray buffer;
		if (f.Open(wfile!=""?wfile:dlg.GetPathName(),CFile::modeRead))	{
			CString c;
			m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),c);
//**************** 10-16F *******************************************
			if(c.Left(2)=="10"||c.Left(2)=="12"||c.Left(2)=="16"){
				if(wfile.Find(".bin")>-1||dlg.GetFileExt()=="bin"||dlg.GetFileExt()=="BIN"){
					buffer.SetSize(f.GetLength()<0x100000?f.GetLength():0x100000);
					int read=f.Read(buffer.GetData(),buffer.GetSize());
					if(read!=buffer.GetSize())
						for(int i=read;i<buffer.GetSize();i++) buffer[i]=0xff;
				}
				else if(wfile.Find(".hex")>-1||dlg.GetFileExt()=="hex"||dlg.GetFileExt()=="HEX"){
					CString line;
					int input_address=0;
					int end_address=0;
					for(;f.ReadString(line);){
						if(line.GetLength()>9){
							int hex_count = htoi((LPCTSTR)line+1, 2);
							if (line.GetLength() - 11 < hex_count * 2) {
								CString err;
								err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
								PrintMessage(err);
							}
							else{
								input_address=htoi((LPCTSTR)line+3,4);
								int sum = 0;
								for (int i=1;i<=hex_count*2+9;i+=2)
									sum += htoi((LPCTSTR)line+i,2);
								if ((sum & 0xff)!=0) {
									CString err;
									err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
									PrintMessage(err);
								}
								else{
									if (htoi((LPCTSTR)line+7,2)==0){
										buffer.SetSize(input_address+hex_count);
										for(i=end_address;i<input_address;i++) buffer[i]=0xff;	//fill with 0xFF
										end_address=input_address+hex_count;
										for (i=0;i<hex_count;i++){
											buffer[input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
										}
									}
								}
							}
						}
					}
				}
				dati_hex.SetSize(buffer.GetSize()/2);
				for(int i=0;i<dati_hex.GetSize();i++){		//Swap bytes
					dati_hex[i]=(buffer[i*2+1]<<8)+buffer[i*2];
				}
				CString s,t,aux,str;
				t.Format("%s :\r\n",f.GetFileName());
				PrintMessage(t);
				PrintMessage(strings[S_CodeMem2]);	//"\r\nMemoria programma:\r\n"
				for(i=0;i<0x2100&&i<dati_hex.GetSize();i+=COL){
					int valid=0;
					for(int j=i;j<i+COL&&i<0x2100&&j<dati_hex.GetSize();j++){
						t.Format("%04X ",dati_hex[j]);
						s+=t;
						if(dati_hex[j]<0x3fff) valid=1;
					}
					if(valid){
						t.Format("%04X: %s\r\n",i,s);
						aux+=t;
					}
					s.Empty();
				}
				PrintMessage(aux);
				PrintMessage("\r\n");
				aux.Empty();
				if(dati_hex.GetSize()>=0x2100) PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
				for(i=0x2100;i<0x2800&&i<dati_hex.GetSize();i+=COL){
					int valid=0;
					for(int j=i;j<i+COL&&i<0x2800&&j<dati_hex.GetSize();j++){
						t.Format("%02X ",dati_hex[j]&0xff);
						s+=t;
						t.Format("%c",isprint(dati_hex[j]&0xff)?dati_hex[j]&0xff:'.');
						str+=t;
						if(dati_hex[j]<0xff) valid=1;
					}
					if(valid){
						t.Format("%04X: %s %s\r\n",i,s,str);
						aux+=t;
					}
					s.Empty();
					str.Empty();
				}
				PrintMessage(aux);
				PrintMessage("\r\n");
			}
//**************** 18F *******************************************
			else if(c.Left(2)=="18"&&(wfile.Find(".hex")>-1||dlg.GetFileExt()=="hex"|dlg.GetFileExt()=="HEX")){
				CString line;
				int input_address=0,ext_addr=0;
				int end_address=0,aa;
				memCODE.SetSize(0);
				memEE.SetSize(0);
				memID.SetSize(0);
				memCONFIG.SetSize(0);
				for(;f.ReadString(line);){
					if(line.GetLength()>9){
						int hex_count = htoi((LPCTSTR)line+1, 2);
						if (line.GetLength() - 11 < hex_count * 2) {
							CString err;
							err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
							PrintMessage(err);
						}
						else{
							input_address=htoi((LPCTSTR)line+3,4);
							int sum = 0;
							for (int i=1;i<=hex_count*2+9;i+=2)
								sum += htoi((LPCTSTR)line+i,2);
							if ((sum & 0xff)!=0) {
								CString err;
								err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
								PrintMessage(err);
							}
							else{
								switch(htoi((LPCTSTR)line+7,2)){
									case 0:		//Data record
										if(ext_addr<0x20){		//Code
											buffer.SetSize((ext_addr<<16)+input_address+hex_count);
											for(i=end_address;i<(ext_addr<<16)+input_address;i++) buffer[i]=0xff;	//fill with 0xFF
											end_address=(ext_addr<<16)+input_address+hex_count;
											for (i=0;i<hex_count;i++){
												buffer[(ext_addr<<16)+input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										else if(ext_addr==0x20&&input_address<8){	//ID
											aa=memID.GetSize();
											memID.SetSize(input_address+hex_count);
											for(i=aa;i<input_address+hex_count;i++) memID[i]=0xff;	//fill with 0xFF
											for (i=0;i<hex_count;i++){
												memID[input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										else if(ext_addr==0x30&&input_address<14){	//CONFIG
											aa=memCONFIG.GetSize();
											memCONFIG.SetSize(input_address+hex_count);
											for(i=aa;i<input_address+hex_count;i++) memCONFIG[i]=0xff;	//fill with 0xFF
											for (i=0;i<hex_count;i++){
												memCONFIG[input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										else if(ext_addr==0xF0&&input_address<0x1000){	//EEPROM
											aa=memEE.GetSize();
											memEE.SetSize(input_address+hex_count);
											for(i=aa;i<input_address+hex_count;i++) memEE[i]=0xff;	//fill with 0xFF
											for (i=0;i<hex_count;i++){
												memEE[input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										break;
									case 4:		//extended linear address record
										if(line.GetLength()>14)	ext_addr=htoi((LPCTSTR)line+9,4);
										break;
									default:
										break;
								}
							}
						}
					}
				}
				memCODE.Copy(buffer);
				CString s,t,aux,str;
				t.Format("%s :\r\n",f.GetFileName());
				PrintMessage(t);
				if(memID.GetSize()){
					PrintMessage(strings[S_IDMem]);	//"memoria ID:\r\n"
					for (int i=memID.GetSize();i<COL;i++) memID.SetAtGrow(i,0xFF);
					for(i=0;i<COL;i+=2){
						str.Format("ID%d: 0x%02X   ID%d: 0x%02X\r\n",i,memID[i],i+1,memID[i+1]);
						PrintMessage(str);
					}
				}
				if(memCONFIG.GetSize()){
					PrintMessage(strings[S_ConfigMem]);	//"memoria CONFIG:\r\n"
					for (int i=memCONFIG.GetSize();i<14;i++) memCONFIG.SetAtGrow(i,0xFF);
					for( i=0;i<7;i++){
						str.Format("CONFIG%dH: 0x%02X\t",i+1,memCONFIG[i*2+1]);
						PrintMessage(str);
						str.Format("CONFIG%dL: 0x%02X\r\n",i+1,memCONFIG[i*2]);
						PrintMessage(str);
					}
				}
				if(memCODE.GetSize()) PrintMessage(strings[S_CodeMem]);	//"\r\nmemoria CODICE:\r\n"
				for(int i=0;i<memCODE.GetSize();i+=COL*2){
					int valid=0;
					for(int j=i;j<i+COL*2&&j<memCODE.GetSize();j++){
						t.Format("%02X ",memCODE[j]);
						s+=t;
						if(memCODE[j]<0xff) valid=1;
					}
					if(valid){
						t.Format("%04X: %s\r\n",i,s);
						aux+=t;
					}
					s.Empty();
				}
				PrintMessage(aux);
				if(memEE.GetSize()){
					PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
					str.Empty();
					aux.Empty();
					for(i=0;i<memEE.GetSize();i+=COL){
						int valid=0;
						for(int j=i;j<i+COL&&j<memEE.GetSize();j++){
							t.Format("%02X ",memEE[j]);
							s+=t;
							t.Format("%c",isprint(memEE[j])?memEE[j]:'.');
							str+=t;
							if(memEE[j]<0xff) valid=1;
						}
						if(valid){
							t.Format("%04X: %s %s\r\n",i,s,str);
							aux+=t;
						}
						s.Empty();
						str.Empty();
					}
					PrintMessage(aux);
				}
				PrintMessage("\r\n");
			}
//**************** 24F *******************************************
			else if(c.Left(3)=="24F"&&(wfile.Find(".hex")>-1||dlg.GetFileExt()=="hex"|dlg.GetFileExt()=="HEX")){
				CString line;
				int input_address=0,ext_addr=0;
				int end_address=0,aa;
				memCODE.SetSize(0);
				memEE.SetSize(0);
				memID.SetSize(0);
				memCONFIG.SetSize(0);
				for(;f.ReadString(line);){
					if(line.GetLength()>9){
						int hex_count = htoi((LPCTSTR)line+1, 2);
						if (line.GetLength() - 11 < hex_count * 2) {
							CString err;
							err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
							PrintMessage(err);
						}
						else{
							input_address=htoi((LPCTSTR)line+3,4);
							int sum = 0;
							for (int i=1;i<=hex_count*2+9;i+=2)
								sum += htoi((LPCTSTR)line+i,2);
							if ((sum & 0xff)!=0) {
								CString err;
								err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
								PrintMessage(err);
							}
							else{
								switch(htoi((LPCTSTR)line+7,2)){
									case 0:		//Data record
										if(ext_addr<0x20){		//Code
											int end1=(ext_addr<<16)+input_address+hex_count;
											int end0=buffer.GetSize();
											if(end0<end1){			//grow array and fill with 0xFF
												buffer.SetSize(end1);
												for(i=end0;i<(ext_addr<<16)+input_address;i++) buffer[i]=0xff;
											}
											end_address=(ext_addr<<16)+input_address+hex_count;
											for (i=0;i<hex_count;i++){
												buffer[(ext_addr<<16)+input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										else if(ext_addr==0x1f0&&input_address<0x22){	//CONFIG
											aa=memCONFIG.GetSize();
											memCONFIG.SetSize(input_address+hex_count);
											for(i=aa;i<input_address+hex_count;i++) memCONFIG[i]=0xff;	//fill with 0xFF
											for (i=0;i<hex_count;i++){
												memCONFIG[input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										else if(ext_addr==0xFF&&input_address>=0xFC00){	//EEPROM
											aa=memEE.GetSize();
											memEE.SetSize(input_address-0xFC00+hex_count);
											for(i=aa;i<input_address-0xFC00+hex_count;i++) memEE[i]=0xff;	//fill with 0xFF
											for (i=0;i<hex_count;i++){
												memEE[input_address-0xFC00+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										break;
									case 4:		//extended linear address record
										if(line.GetLength()>14)	ext_addr=htoi((LPCTSTR)line+9,4);
										break;
									default:
										break;
								}
							}
						}
					}
				}
				memCODE.Copy(buffer);
				CString s,t,aux,str;
				t.Format("%s :\r\n",f.GetFileName());
				PrintMessage(t);
				if(memCONFIG.GetSize()){
					aa=memCONFIG.GetSize();
					memCONFIG.SetSize(0x22);
					for(int i=aa;i<0x22;i++) memCONFIG[i]=0xff;
					PrintMessage(strings[S_ConfigMem]);				//"\r\nMemoria CONFIG:\r\n"
					str.Format("0xF80000: FBS = 0x%02X\r\n",memCONFIG[0]);
					PrintMessage(str);
					str.Format("0xF80004: FGS = 0x%02X\r\n",memCONFIG[8]);
					PrintMessage(str);
					str.Format("0xF80006: FOSCSEL = 0x%02X\r\n",memCONFIG[12]);
					PrintMessage(str);
					str.Format("0xF80008: FOSC = 0x%02X\r\n",memCONFIG[16]);
					PrintMessage(str);
					str.Format("0xF8000A: FWDT = 0x%02X\r\n",memCONFIG[20]);
					PrintMessage(str);
					str.Format("0xF8000C: FPOR = 0x%02X\r\n",memCONFIG[24]);
					PrintMessage(str);
					str.Format("0xF8000E: FICD = 0x%02X\r\n",memCONFIG[28]);
					PrintMessage(str);
					str.Format("0xF80010: FDS = 0x%02X\r\n",memCONFIG[32]);
					PrintMessage(str);
				}
				if(memCODE.GetSize()) PrintMessage(strings[S_CodeMem]);	//"\r\nmemoria CODICE:\r\n"
				for(int i=0;i<memCODE.GetSize();i+=COL*2){
					int valid=0,d;
					for(int j=i;j<i+COL*2&&j<memCODE.GetSize();j+=4){
						d=(memCODE[j+3]<<24)+(memCODE[j+2]<<16)+(memCODE[j+1]<<8)+memCODE[j];
						t.Format("%08X ",d);
						s+=t;
						if(d!=0xffffffff) valid=1;
					}
					if(valid){
						t.Format("%06X: %s\r\n",i/2,s);
						aux+=t;
					}
					s.Empty();
				}
				PrintMessage(aux);
				if(memEE.GetSize()){
					PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
					str.Empty();
					aux.Empty();
					for(i=0;i<memEE.GetSize();i+=COL*2){
						int valid=0;
						for(int j=i;j<i+COL*2&&j<memEE.GetSize();j+=4){	//skip high word
							t.Format("%02X %02X ",memEE[j],memEE[j+1]);
							s+=t;
							t.Format("%c",isprint(memEE[j])?memEE[j]:'.');
							str+=t;
							if(memEE[j]<0xff) valid=1;
							t.Format("%c",isprint(memEE[j+1])?memEE[j+1]:'.');
							str+=t;
							if(memEE[j+1]<0xff) valid=1;
						}
						if(valid){
							t.Format("%04X: %s %s\r\n",i/2,s,str);
							aux+=t;
						}
						s.Empty();
						str.Empty();
					}
					PrintMessage(aux);
				}
				PrintMessage("\r\n");
			}
//**************** ATxxxx *******************************************
			else if(c.Left(2)=="AT"&&(wfile.Find(".hex")>-1||dlg.GetFileExt()=="hex"|dlg.GetFileExt()=="HEX")){
				CString line;
				int input_address=0,ext_addr=0;
				int end_address=0;
				memCODE.SetSize(0);
				memEE.SetSize(0);
				for(;f.ReadString(line);){
					if(line.GetLength()>9){
						int hex_count = htoi((LPCTSTR)line+1, 2);
						if (line.GetLength() - 11 < hex_count * 2) {
							CString err;
							err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
							PrintMessage(err);
						}
						else{
							input_address=htoi((LPCTSTR)line+3,4);
							int sum = 0;
							for (int i=1;i<=hex_count*2+9;i+=2)
								sum += htoi((LPCTSTR)line+i,2);
							if ((sum & 0xff)!=0) {
								CString err;
								err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
								PrintMessage(err);
							}
							else{
								switch(htoi((LPCTSTR)line+7,2)){
									case 0:		//Data record
										if(ext_addr<0x20){		//Code
											buffer.SetSize((ext_addr<<16)+input_address+hex_count);
											for(i=end_address;i<(ext_addr<<16)+input_address;i++) buffer[i]=0xff;	//fill with 0xFF
											end_address=(ext_addr<<16)+input_address+hex_count;
											for (i=0;i<hex_count;i++){
												buffer[(ext_addr<<16)+input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
										}
										break;
									case 4:		//extended linear address record
										if(line.GetLength()>14)	ext_addr=htoi((LPCTSTR)line+9,4);
										break;
									default:
										break;
								}
							}
						}
					}
				}
				memCODE.Copy(buffer);
				CString s,t,aux,str;
				t.Format("%s :\r\n",f.GetFileName());
				PrintMessage(t);
				if(memCODE.GetSize()) PrintMessage(strings[S_CodeMem]);	//"\r\nmemoria CODICE:\r\n"
				for(int i=0;i<memCODE.GetSize();i+=COL*2){
					int valid=0;
					for(int j=i;j<i+COL*2&&j<memCODE.GetSize();j++){
						t.Format("%02X ",memCODE[j]);
						s+=t;
						if(memCODE[j]<0xff) valid=1;
					}
					if(valid){
						t.Format("%04X: %s\r\n",i,s);
						aux+=t;
					}
					s.Empty();
				}
				PrintMessage(aux);
				CFileDialog dlg2(TRUE,"hex",NULL,OFN_HIDEREADONLY,strings[S_fileEEP]);	//"File Hex8 (*.hex;.eep ..."
				dlg2.m_ofn.lpstrTitle =strings[S_openEEfile];							//"Apri file eeprom";
				if (dlg2.DoModal()==IDOK){
					CStdioFile fe;
					buffer.RemoveAll();
					input_address=0,ext_addr=0;
					end_address=0;
					if (fe.Open(dlg2.GetPathName(),CFile::modeRead))	{
						for(;fe.ReadString(line);){
							if(line.GetLength()>9){
								int hex_count = htoi((LPCTSTR)line+1, 2);
								if (line.GetLength() - 11 < hex_count * 2) {
									CString err;
									err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
									PrintMessage(err);
								}
								else{
									input_address=htoi((LPCTSTR)line+3,4);
									int sum = 0;
									for (int i=1;i<=hex_count*2+9;i+=2)
										sum += htoi((LPCTSTR)line+i,2);
									if ((sum & 0xff)!=0) {
										CString err;
										err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
										PrintMessage(err);
									}
									else{
										switch(htoi((LPCTSTR)line+7,2)){
											case 0:		//Data record
												if(ext_addr<0x20){		//Code
													buffer.SetSize((ext_addr<<16)+input_address+hex_count);
													for(i=end_address;i<(ext_addr<<16)+input_address;i++) buffer[i]=0xff;	//fill with 0xFF
													end_address=(ext_addr<<16)+input_address+hex_count;
													for (i=0;i<hex_count;i++){
														buffer[(ext_addr<<16)+input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
													}
												}
												break;
											case 4:		//extended linear address record
												if(line.GetLength()>14)	ext_addr=htoi((LPCTSTR)line+9,4);
												break;
											default:
												break;
										}
									}
								}
							}
						}
						memEE.Copy(buffer);
						if(memEE.GetSize()){
							PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
							str.Empty();
							aux.Empty();
							for(i=0;i<memEE.GetSize();i+=COL){
								int valid=0;
								for(int j=i;j<i+COL&&j<memEE.GetSize();j++){
									t.Format("%02X ",memEE[j]);
									s+=t;
									t.Format("%c",isprint(memEE[j])?memEE[j]:'.');
									str+=t;
									if(memEE[j]<0xff) valid=1;
								}
								if(valid){
									t.Format("%04X: %s %s\r\n",i,s,str);
									aux+=t;
								}
								s.Empty();
								str.Empty();
							}
							PrintMessage(aux);
						}
						PrintMessage("\r\n");
						fe.Close();
					}
				}
			}
//**************** 24xxx / 93xxx / 25xxx **************************************
			else if(c.Left(2)=="24"||c.Left(2)=="93"||c.Left(2)=="25"){
				if(wfile.Find(".bin")>-1||dlg.GetFileExt()=="bin"||dlg.GetFileExt()=="BIN"){
					memEE.SetSize(f.GetLength()<0x100000?f.GetLength():0x100000);
					for(int i=0;i<memEE.GetSize();i++) memEE[i]=0xff;
					int read=f.Read(memEE.GetData(),memEE.GetSize());
				}
				else if(wfile.Find(".hex")>-1||dlg.GetFileExt()=="hex"||dlg.GetFileExt()=="HEX"){
					CString line;
					int input_address=0,ext_addr=0;
					int end_address=0;
					memEE.SetSize(0);
					for(;f.ReadString(line);){
						if(line.GetLength()>9){
							int hex_count = htoi((LPCTSTR)line+1, 2);
							if (line.GetLength() - 11 < hex_count * 2) {
								CString err;
								err.Format(strings[S_IhexShort],(LPCTSTR)line);	//"Intel hex8 line too short:\r\n%s\r\n"
								PrintMessage(err);
							}
							else{
								input_address=htoi((LPCTSTR)line+3,4);
								int sum = 0;
								int end0,end1;
								for (int i=1;i<=hex_count*2+9;i+=2)
									sum += htoi((LPCTSTR)line+i,2);
								if ((sum & 0xff)!=0) {
									CString err;
									err.Format(strings[S_IhexChecksum],(LPCTSTR)line);	//"Intel hex8 checksum error in line:\r\n%s\r\n"
									PrintMessage(err);
								}
								else{
									switch(htoi((LPCTSTR)line+7,2)){
										case 0:		//Data record
											end1=(ext_addr<<16)+input_address+hex_count;
											end0=memEE.GetSize();
											if(end0<end1){			//grow array and fill with 0xFF
												memEE.SetSize(end1);
												for(i=end0;i<(ext_addr<<16)+input_address;i++) memEE[i]=0xff;
											}									
											end_address=(ext_addr<<16)+input_address+hex_count;
											for (i=0;i<hex_count;i++){
												memEE[(ext_addr<<16)+input_address+i]=htoi((LPCTSTR)line+9+i*2,2);
											}
											break;
										case 4:		//extended linear address record
											if(line.GetLength()>14)	ext_addr=htoi((LPCTSTR)line+9,4);
											break;
										default:
											break;
									}
								}
							}
						}
					}
				}
				DisplayEE();	//visualize
				PrintMessage("\r\n");
			}
		f.Close();
		}
	}
}


unsigned int COpenProgDlg::htoi(const char *hex, int length)
{
	int i;
	unsigned int v = 0;
	for (i = 0; i < length; i++) {
		v <<= 4;
		if (hex[i] >= '0' && hex[i] <= '9') v += hex[i] - '0';
		else if (hex[i] >= 'a' && hex[i] <= 'f') v += hex[i] - 'a' + 10;
		else if (hex[i] >= 'A' && hex[i] <= 'F') v += hex[i] - 'A' + 10;
		else {
			CString err;
			err.Format(strings[S_Inohex],hex);	//"Error: '%.4s' doesn't look very hexadecimal, right?\n"
			PrintMessage(err);
		}
	}
	return v;
}

void COpenProgDlg::OpenLogFile(CString nome){
	if(!RegFile.Open(nome,CFile::modeWrite|CFile::modeCreate))return;
	CString str;
	struct tm *newtime;
    time_t long_time;
    time( &long_time );                /* Get time as long integer. */
    newtime = localtime( &long_time ); /* Convert to local time. */
	str.Format("***** %d:%02d:%02d *****\n",newtime->tm_hour,newtime->tm_min,newtime->tm_sec);
	RegFile.Write(str,str.GetLength());
}

void COpenProgDlg::CloseLogFile(){
	RegFile.Close();
}

void COpenProgDlg::WriteLog(CString str){
	RegFile.Write(str,str.GetLength());
}

void COpenProgDlg::WriteLogIO(){
	CString str;
	str.Format("bufferU=[%02X\n",bufferU[0]);
	RegFile.Write(str,str.GetLength());
	for(int i=1;i<DIMBUF;i++){
		str.Format("%02X ",bufferU[i]);
		RegFile.Write(str,str.GetLength());
		if(i%32==0) RegFile.Write("\n",1);
	}
	RegFile.Write("]\n",2);
	str.Format("bufferI=[%02X\n",bufferU[0]);
	RegFile.Write(str,str.GetLength());
	for(i=1;i<DIMBUF;i++){
		str.Format("%02X ",bufferI[i]);
		RegFile.Write(str,str.GetLength());
		if(i%32==0) RegFile.Write("\n",1);
	}
	RegFile.Write("]\n",2);
}



void COpenProgDlg::OnTestHw()
{
	DWORD BytesWritten=0;
	ULONG Result;
	int j=1;
	bufferU[0]=0;
	//CString str;
	if(MyDeviceDetected==FALSE) return;
	if(AfxMessageBox(strings[I_TestHW],MB_OKCANCEL)==IDOK){			//"Test hardware ..."
		StartHVReg(13);
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
		bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
		bufferU[j++]=0x5;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
		AfxMessageBox("VDD=5V, VPP=13V, D=0V, CK=0V, PGM=0V");
		j=1;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x15;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x1;			//VDD
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
		AfxMessageBox("VDD=5V, VPP=0V, D=5V, CK=5V, PGM=5V");
		j=1;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x4;			//VPP
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
		AfxMessageBox("VDD=0V, VPP=13V, D=5V, CK=0V, PGM=0V");
		j=1;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x4;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x0;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
		AfxMessageBox("VDD=0V, VPP=0V, D=0V, CK=5V, PGM=0V");
		j=1;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x0;			//VPP
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(20);
		read();
	}
}

void COpenProgDlg::msDelay(double delay)
{
	Sleep((long)ceil(delay)>MinRit?(long)ceil(delay):MinRit);
}

void COpenProgDlg::OnWriteLangFile() 
{
	CFileDialog dlg(FALSE,NULL,"languages.rc",OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT);	//"File Hex8 (*.hex)|*.hex|File binari (*.bin)|*.bin|Tutti i file (*.*)|*.*||"
	if (dlg.DoModal()==IDOK){
		CFile f;
		if (f.Open(dlg.GetPathName(),CFile::modeWrite|CFile::modeCreate)){
			f.Write("[sample]\n",9);
			CString c;
			for(int i=0;i<DIM;i++){
				f.Write(strings_ID[i],strlen(strings_ID[i]));
//				f.Write("\n",1);
			}
			f.Close();
		}
	}
}

// including code for various devices
// if anyone knows a better way to do it please tell me

#include "progP12.cpp"
#include "progP16.cpp"
#include "progP18.cpp"
#include "progP24.cpp"
#include "progEEPROM.cpp"
#include "progAVR.cpp"
