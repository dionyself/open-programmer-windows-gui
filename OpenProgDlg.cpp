/*
 * Openprog - control program for the open programmer
 * Copyright (C) 2009-2010 Alberto Maccioni
 * for detailed info see:
 * http://openprog.altervista.org/
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
#include <math.h>
#include "setupapi.h"
#include "hidsdi.h"
#include "msvc_common.h"
#include "DatiPage.h"
#include "DispositivoPage.h"
#include "OpzioniPage.h"
#include "I2CSPIPage.h"
#include "OpenProg.h"
#include "OpenProgDlg.h"
#include <string.h>

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
\r\n               Copyright (C) 2009-2011 Alberto Maccioni\
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
	ON_COMMAND(ID_WRITE_LANG_FILE, OnWriteLangFile)
	ON_COMMAND(ID_LEGGI, OnRead)
	ON_COMMAND(ID_OPZIONI_PROGRAMMATORE_CONNETTI, OnConnect)
	ON_COMMAND(ID_FILE_SALVA, OnFileSave)
	ON_COMMAND(ID_FILE_APRI, OnFileOpen)
	ON_COMMAND(ID_SCRIVI, OnWrite)
	ON_COMMAND(ID_CAMBIA_LINGUA, ChangeLanguage)
	ON_COMMAND(ID_FUSE3k, OnFUSE3k)
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
	MinDly=1;
	hvreg=0;
	logfile=0;
	size=sizeW=sizeEE=sizeCONFIG=sizeUSERID=0;
	memCODE_W=0;
	memCODE=memEE=0;
	progress=0;
	skipV33check=0;
	RWstop=0;
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
		}
		f.Close();
	}
	dev_ini=dev;
	vid_ini=vid;
	pid_ini=pid;
	lang_ini=lang;
	max_err_ini=max_err;
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
	sizeW=0x2400;
	memCODE_W=(WORD*)malloc(sizeW*sizeof(WORD));
	for(i=0;i<0x2400;i++) memCODE_W[i]=0x3fff;
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
	AddDevices();	//populate device list
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
	m_OpzioniPage.SetDlgItemInt(IDC_USBDMIN,MinDly);
	m_DispoPage.SetDlgItemText(IDC_ICDADDR,"1F00");
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
	FindDevice();	//connect to USB programmer
	ProgID();		//get firmware version and reset
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
	extern char* strings_it[];
	extern char* strings_en[];
	extern char* STR_ID[];
	strinit();
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
						if(STR_ID[j] && id==STR_ID[j]){
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
	m_DispoPage.SetDlgItemText(IDC_PICCONF,strings[I_PIC_CONFIG]);
	m_DispoPage.SetDlgItemText(IDC_FUSE_P,strings[I_AT_FUSE]);
	m_DispoPage.SetDlgItemText(IDC_FUSEH_P,strings[I_AT_FUSEH]);
	m_DispoPage.SetDlgItemText(IDC_FUSEX_P,strings[I_AT_FUSEX]);
	m_DispoPage.SetDlgItemText(IDC_FUSE3K_B,strings[I_AT_FUSELF]);
	m_DispoPage.SetDlgItemText(IDC_LOCK_P,strings[I_AT_LOCK]);
	m_DispoPage.SetDlgItemText(IDC_ICD_EN,strings[I_ICD_ENABLE]);
	m_DispoPage.SetDlgItemText(IDC_ICD_ADDR,strings[I_ICD_ADDRESS]);
	m_OpzioniPage.SetDlgItemText(IDC_CONNETTI,strings[I_CONN]);
	m_OpzioniPage.SetDlgItemText(IDC_REGISTRO,strings[I_LOG]);
	m_OpzioniPage.SetDlgItemText(IDC_NOLV,strings[I_CK_V33]);
	m_OpzioniPage.SetDlgItemText(IDC_STATICerr,strings[I_MAXERR]);
	m_OpzioniPage.SetDlgItemText(IDC_STATIC_L,strings[I_LANG]);
	m_OpzioniPage.SetDlgItemText(IDC_TESTHW,strings[I_TestHWB]);
	m_OpzioniPage.SetDlgItemText(IDC_WLANGFILE,strings[I_W_LANGFILE]);
	m_I2CSPIPage.SetDlgItemText(IDC_MODE,strings[I_I2CMode]);
	m_I2CSPIPage.SetDlgItemText(IDC_NBS,strings[I_I2C_NB]);
	m_I2CSPIPage.SetDlgItemText(IDC_REC,strings[I_I2CReceive]);
	m_I2CSPIPage.SetDlgItemText(IDC_SEND,strings[I_I2CSend]);
	m_I2CSPIPage.SetDlgItemText(IDC_msgSTRI,strings[I_I2CDATAOUT]);
	m_I2CSPIPage.SetDlgItemText(IDC_msgSTRU,strings[I_I2CDATATR]);
	strncpy(LogFileName,strings[S_LogFile],sizeof(LogFileName));
}


void COpenProgDlg::OnClose()
{
	FILE *f;
	CString lang,a,s;
	CString dev,vid,pid;
	int maxerr=200,x;
	m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
	m_OpzioniPage.m_language.GetLBText(m_OpzioniPage.m_language.GetCurSel(),lang);
	m_OpzioniPage.GetDlgItemText(IDC_VID,a);
	sscanf(a,"0x%X",&x);
	vid.Format("0x%X",x);
	m_OpzioniPage.GetDlgItemText(IDC_PID,a);
	sscanf(a,"0x%X",&x);
	pid.Format("0x%X",x);
	m_OpzioniPage.GetDlgItemText(IDC_ERRMAX,a);
	sscanf(a,"%d",&maxerr);
// Save ini file only if parameters are changed
	if(dev_ini!=dev||vid_ini!=vid||pid_ini!=pid||max_err_ini!=max_err||lang_ini!=lang){
		s=argv[0];
		s.Replace(".exe",".ini");
		if(f=fopen(s,"w")){
			fprintf(f,"device %s\n",dev);
    		fprintf(f,"language %s\n",lang);
			fprintf(f,"vid %s\n",vid);
			fprintf(f,"pid %s\n",pid);
			fprintf(f,"maxerr %d\n",maxerr);
		}
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

#define EQ(s) !strncmp(s,dev,64)

void COpenProgDlg::OnFileOpen()
{
	if(progress) return;
	progress=1;
	CFileDialog dlg(TRUE,"hex",NULL,OFN_HIDEREADONLY,strings[S_file2]);	//"File Hex8 (*.hex)|*.hex|File binari (*.bin)|*.bin|Tutti i file (*.*)|*.*||"
	if (wfile!=""||dlg.DoModal()==IDOK){
		char dev[32];
		CString aux,err,str;
		m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
		char loadfile[512];
		strncpy(loadfile,wfile!=""?wfile:dlg.GetFileName(),sizeof(loadfile));
		Load(dev,loadfile);
		if(!strncmp(dev,"AT",2)){	//load EEPROM from separate file for ATMEL chips
			CFileDialog dlgA(TRUE,"hex",NULL,OFN_HIDEREADONLY,strings[S_fileEEP]);	//"File Hex8 (*.hex;.eep ..."
			dlgA.m_ofn.lpstrTitle =strings[S_openEEfile];							//"Apri file eeprom";
			if (dlgA.DoModal()==IDOK){
				char loadfileEE[256];
				strncpy(loadfileEE,dlgA.GetPathName(),sizeof(loadfileEE));
				LoadEE(dev,loadfileEE);
			}
		}
	}
	progress=0;
}

void COpenProgDlg::OnFileSave()
{
	if(progress) return;
	progress=1;
	CFileDialog dlg(FALSE,"hex",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,strings[S_file2]);	//"File Hex8 (*.hex)|*.hex|File binari (*.bin)|*.bin|Tutti i file (*.*)|*.*||"
	if (rfile!=""||dlg.DoModal()==IDOK){
		char dev[32];
		m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
		char savefile[512];
		strncpy(savefile,rfile!=""?rfile:dlg.GetPathName(),sizeof(savefile));
		Save(dev,savefile);
		PrintMessage1(strings[S_FileSaved],savefile);
		if(!strncmp(dev,"AT",2)&&sizeEE){	//save EEPROM on separate file for ATMEL chips
			CFileDialog dlgA(FALSE,"hex;eep",NULL,OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,strings[S_fileEEP]);	//"File Hex8 (*.hex;.eep ..."S_file]);
			dlgA.m_ofn.lpstrTitle =strings[S_saveEEfile];		//"Save eeprom";
			if (dlgA.DoModal()==IDOK){
				char savefileEE[256];
				strncpy(savefileEE,dlgA.GetPathName(),sizeof(savefileEE));
				SaveEE(dev,savefileEE);
				PrintMessage1(strings[S_FileSaved],savefileEE);
			}
		}
	}
	progress=0;
}

void COpenProgDlg::OnWrite()
{
	char dev[64];
	int ee;
	if(progress) return;
	progress=1;
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"invalid handle \r\n"
		return;
	}
	m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_NOLV);
	skipV33check=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_EEPROM);
	ee=b->GetCheck();
	if(ee) ee=0xffff;
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	load_calibword=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_LOAD);
	load_osccal=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_ICD_EN);
	ICDenable=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_OSCCAL);
	use_osccal=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_BK);
	use_BKosccal=b->GetCheck();
	CString str;
	m_DispoPage.GetDlgItemText(IDC_ICDADDR,str);
	int i=sscanf(str,"%x",&ICDaddr);
	if(i!=1||ICDaddr<0||ICDaddr>0xFFFF) ICDaddr=0x1FF0;
	AVRlock=0x100;
	AVRfuse=0x100;
	AVRfuse_h=0x100;
	AVRfuse_x=0x100;
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSE_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSE,str);
		i=sscanf(str,"%x",&AVRfuse);
		if(i!=1||AVRfuse<0||AVRfuse>0xFF) AVRfuse=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSEH_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSEH,str);
		i=sscanf(str,"%x",&AVRfuse_h);
		if(i!=1||AVRfuse_h<0||AVRfuse_h>0xFF) AVRfuse_h=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSEX_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSEX,str);
		i=sscanf(str,"%x",&AVRfuse_x);
		if(i!=1||AVRfuse_x<0||AVRfuse_x>0xFF) AVRfuse_x=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_LOCK_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_LOCK,str);
		i=sscanf(str,"%x",&AVRlock);
		if(i!=1||AVRlock<0||AVRlock>0xFF) AVRlock=0x100;
	}
	Write(dev,ee);	//choose the right function
	progress=0;
}

void COpenProgDlg::OnRead()
{
	char dev[64];
	int r,ee;
	if(progress) return;
	progress=1;
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"invalid handle \r\n"
		return;
	}
	m_DispoPage.m_dispo.GetLBText(m_DispoPage.m_dispo.GetCurSel(),dev);
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_RISERVATA);
	r=b->GetCheck();
	if(r) r=0xffff;
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_NOLV);
	skipV33check=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_EEPROM);
	ee=b->GetCheck();
	if(ee) ee=0xffff;
	Read(dev,ee,r);	//choose the right function
	progress=0;
}


void COpenProgDlg::OnI2cspiR()		// I2C/SPI receive
{
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	CancelIo(ReadHandle);
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
	I2CReceive(mode,0,nbyte,tmpbuf);
}

void COpenProgDlg::OnI2cspiS() // I2C/SPI send
{
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		m_I2CSPIPage.SetDlgItemText(IDC_STRU,strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	CancelIo(ReadHandle);
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
	I2CSend(mode,0,nbyte,tmpbuf);
}

void COpenProgDlg::PrintMessage(LPCTSTR s)
{
	dati+=s;
	if(dati.GetLength()>100000) dati=dati.Right(100000);
	CEdit* e=(CEdit*)m_DatiPage.GetDlgItem(IDC_DATI);
	e->SetWindowText(dati);
	e->LineScroll(5000);
	DoEvents();
}

void COpenProgDlg::DisplayEE(){
	CString str,aux;
	char s[256],t[256],v[256];
	int valid=0,empty=1;
	int i,j,max;
	s[0]=0;
	v[0]=0;
	PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
	i=0;
	max=sizeEE>7000?7000:sizeEE;
	for(i=0;i<max;i+=COL){
		valid=0;
		for(j=i;j<i+COL&&j<sizeEE;j++){
			sprintf(t,"%02X ",memEE[j]);
			strcat(s,t);
			sprintf(t,"%c",isprint(memEE[j])?memEE[j]:'.');
			strcat(v,t);
			if(memEE[j]<0xff) valid=1;
		}
		if(valid){
			sprintf(t,"%04X: %s %s\r\n",i,s,v);
			if(aux.GetLength()<90000) aux+=t;
			empty=0;
		}
		s[0]=0;
		v[0]=0;
	}
	if(empty) PrintMessage(strings[S_Empty]);	//empty
	else{
		PrintMessage(aux);
		if(sizeEE>max) PrintMessage("(...)\r\n");
	}
}

int COpenProgDlg::StartHVReg(double V)
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
		msDelay(40);
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
	for(;(v<(vreg/10.0-0.5)*G||v>(vreg/10.0+0.5)*G)&&t<t0+1500;t=GetTickCount()){
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
		if(HwID==3) v>>=2;		//if 12 bit ADC
//		str.Format("v=%d=%fV\r\n",v,v/G);
//		PrintMessage(str);
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
		PrintMessage2(strings[S_reg],t-t0,v/G);	//"Regolatore avviato e funzionante dopo T=%d ms VPP=%.1f\r\n\r\n"
		hvreg=1;
		CancelIo(ReadHandle);
		return vreg;
	}
}

void COpenProgDlg::ProgID()
{
	DWORD BytesWritten=0;
	ULONG Result;
	CString str;
	int j=1;
	bufferU[0]=0;
	if(MyDeviceDetected==FALSE) return;
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
	HwID=bufferI[7];
	if(HwID==1) str.Format(" (18F2550)\r\n\r\n");
	else if(HwID==2) str.Format(" (18F2450)\r\n\r\n");
	else if(HwID==3) str.Format(" (18F2458/2553)\r\n\r\n");
	else str.Format(" (?)\r\n\r\n");
	PrintMessage(str);
	CancelIo(ReadHandle);
}

int COpenProgDlg::CheckV33Regulator()
{
	DWORD BytesWritten=0;
	ULONG Result;
	int i,j=1;
	if(skipV33check) return 1;
	bufferU[j++]=WRITE_RAM;
	bufferU[j++]=0x0F;
	bufferU[j++]=0x93;
	bufferU[j++]=0xFE;	//B0 = output
	bufferU[j++]=EXT_PORT;
	bufferU[j++]=0x01;	//B0=1
	bufferU[j++]=0;
	bufferU[j++]=READ_RAM;
	bufferU[j++]=0x0F;
	bufferU[j++]=0x81;	//Check if B1=1
	bufferU[j++]=EXT_PORT;
	bufferU[j++]=0x00;	//B0=0
	bufferU[j++]=0;
	bufferU[j++]=READ_RAM;
	bufferU[j++]=0x0F;
	bufferU[j++]=0x81;	//Check if B1=0
	bufferU[j++]=WRITE_RAM;
	bufferU[j++]=0x0F;
	bufferU[j++]=0x93;
	bufferU[j++]=0xFF;	//BX = input
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	for(j=1;j<DIMBUF-3&&bufferI[j]!=READ_RAM;j++);
	i=bufferI[j+3]&0x2;		//B1 should be high
	for(j+=3;j<DIMBUF-3&&bufferI[j]!=READ_RAM;j++);
	return (i+bufferI[j+3]&0x2)==2?1:0;
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
	Sleep((long)ceil(delay)>MinDly?(long)ceil(delay):MinDly);
}

void COpenProgDlg::OnConnect()
{
	FindDevice();
	ProgID();
}

int COpenProgDlg::FindDevice(){
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
		return 0;
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
		||HidD_GetNumInputBuffers==NULL) return 0;


	HMODULE hSAPI=0;
	hSAPI = LoadLibrary("setupapi.dll");
	if(!hSAPI){
		MessageBox("Can't find setupapi.dll");
		return 0;
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
		||SetupDiGetDeviceInterfaceDetailA==NULL) return 0;
	/*
	The following code is adapted from Usbhidio_vc6 application example by Jan Axelson
	for more information see see http://www.lvr.com/hidpage.htm
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
	return MyDeviceDetected?1:0;
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
			}
			f.Close();
		}
	}
}


void COpenProgDlg::DoEvents()
{
	MSG msg;  
    while ( ::PeekMessage(&msg, NULL, 0, 0, PM_REMOVE ) )
    {  
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
    }
}

void COpenProgDlg::OnFUSE3k() 
{
	int i;
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSE_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSE,str);
		i=sscanf(str,"%x",&AVRfuse);
		if(i!=1||AVRfuse<0||AVRfuse>0xFF) AVRfuse=0x100;
		else WriteATfuseSlow(AVRfuse);

	}
}

///
///Write data packet, wait for X milliseconds, read response
void COpenProgDlg::PacketIO(double delay){
	#define TIMEOUT 50
	if(saveLog) fprintf(logfile,"PacketIO(%.2f)\n",delay);
	__int64 start,stop,freq;
	QueryPerformanceCounter((LARGE_INTEGER *)&start);
	QueryPerformanceFrequency((LARGE_INTEGER *)&freq);
	delay-=TIMEOUT-10;	//shorter delays are covered by 50ms timeout
	if(delay<0) delay=0;
	//write
	Result = WriteFile(WriteHandle,bufferU,DIMBUF,&BytesWritten,NULL);
	//delay(delay)
	Sleep((long)ceil(delay)>MinDly?(long)ceil(delay):MinDly);
	//read
	Result = ReadFile(ReadHandle,bufferI,DIMBUF,&NumberOfBytesRead,(LPOVERLAPPED) &HIDOverlapped);
	Result = WaitForSingleObject(hEventObject,50);
	if(saveLog) WriteLogIO();
	ResetEvent(hEventObject);
	if(Result!=WAIT_OBJECT_0){
		PrintMessage(strings[S_comTimeout]);	/*"comm timeout\r\n"*/
		if(saveLog) fprintf(logfile,strings[S_comTimeout]);
	}
	QueryPerformanceCounter((LARGE_INTEGER *)&stop);
	if(saveLog) fprintf(logfile,"T=%.2f ms\n",(stop-start)*1000.0/freq);
}


