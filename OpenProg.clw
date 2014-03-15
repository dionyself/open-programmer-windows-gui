; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=COpenProgDlg
LastTemplate=CPropertyPage
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "OpenProg.h"

ClassCount=7
Class1=COpenProgApp
Class2=COpenProgDlg
Class3=CAboutDlg

ResourceCount=9
Resource1=IDD_DATI
Resource2=IDR_MAINFRAME
Resource3=IDD_DISP
Resource4=IDD_OPENPROG_DIALOG
Class4=CDatiPage
Resource5=IDD_OPZIONI
Class5=CDispositivoPage
Resource6=IDR_MENU1
Resource7=IDD_I2C_SPI
Class6=COpzioniPage
Class7=CI2CSPIPage
Resource8=IDD_ABOUTBOX
Resource9=IDR_TOOLBAR1

[CLS:COpenProgApp]
Type=0
HeaderFile=OpenProg.h
ImplementationFile=OpenProg.cpp
Filter=N
LastObject=COpenProgApp

[CLS:COpenProgDlg]
Type=0
HeaderFile=OpenProgDlg.h
ImplementationFile=OpenProgDlg.cpp
Filter=D
BaseClass=CDialog
VirtualFilter=dWC
LastObject=ID_CAMBIA_LINGUA

[CLS:CAboutDlg]
Type=0
HeaderFile=OpenProgDlg.h
ImplementationFile=OpenProgDlg.cpp
Filter=D
LastObject=CAboutDlg
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=3
Control1=IDC_STATIC,static,1342177283
Control2=IDOK,button,1342373889
Control3=IDC_LICENSE,edit,1344342148

[DLG:IDD_OPENPROG_DIALOG]
Type=1
Class=COpenProgDlg
ControlCount=0

[MNU:IDR_MENU1]
Type=1
Class=?
Command1=ID_FILE_APRI
Command2=ID_FILE_SALVA
Command3=ID_FILE_ESCI
Command4=ID_OPZIONI_PROGRAMMATORE_CONNETTI
Command5=ID_CAMBIA_LINGUA
Command6=ID_TEST_HW
Command7=ID_WRITE_LANG_FILE
Command8=ID__INFORMAZIONI
CommandCount=8

[DLG:IDD_DATI]
Type=1
Class=CDatiPage
ControlCount=1
Control1=IDC_DATI,edit,1352730756

[CLS:CDatiPage]
Type=0
HeaderFile=DatiPage.h
ImplementationFile=DatiPage.cpp
BaseClass=CPropertyPage
Filter=D
VirtualFilter=idWC
LastObject=CDatiPage

[DLG:IDD_DISP]
Type=1
Class=CDispositivoPage
ControlCount=24
Control1=IDC_DISPO,combobox,1344339971
Control2=IDC_STATICdev,static,1342308352
Control3=IDC_RISERVATA,button,1342242819
Control4=IDC_OSC_LOAD,button,1342308361
Control5=IDC_OSC_OSCCAL,button,1342177289
Control6=IDC_OSC_BK,button,1342177289
Control7=IDC_STATIC2,button,1342177287
Control8=IDC_IDPROG,button,1342242819
Control9=IDC_EEPROM,button,1342242819
Control10=IDC_CALIB_LOAD,button,1342242819
Control11=IDC_LOCK,edit,1350631552
Control12=IDC_LOCK_P,button,1342242819
Control13=IDC_ATCONF,button,1342177287
Control14=IDC_FUSE,edit,1350631552
Control15=IDC_FUSE_P,button,1342242819
Control16=IDC_FUSEH,edit,1350631552
Control17=IDC_FUSEH_P,button,1342242819
Control18=IDC_FUSEX,edit,1350631552
Control19=IDC_FUSEX_P,button,1342242819
Control20=IDC_ICD_EN,button,1342242819
Control21=IDC_ICDADDR,edit,1350631552
Control22=IDC_ICD_ADDR,static,1342308352
Control23=IDC_PICCONF,button,1342177287
Control24=IDC_STATIC3,button,1342177287

[CLS:CDispositivoPage]
Type=0
HeaderFile=DispositivoPage.h
ImplementationFile=DispositivoPage.cpp
BaseClass=CPropertyPage
Filter=T
LastObject=CDispositivoPage
VirtualFilter=idWC

[TB:IDR_TOOLBAR1]
Type=1
Class=?
Command1=ID_FILE_APRI
Command2=ID_FILE_SALVA
Command3=ID_LEGGI
Command4=ID_SCRIVI
CommandCount=4

[DLG:IDD_OPZIONI]
Type=1
Class=COpzioniPage
ControlCount=13
Control1=IDC_VID,edit,1350631552
Control2=IDC_STATICvid,static,1342308352
Control3=IDC_PID,edit,1350631552
Control4=IDC_STATICpid,static,1342308352
Control5=IDC_CONNETTI,button,1342242816
Control6=IDC_REGISTRO,button,1342242819
Control7=IDC_STATICerr,static,1342308354
Control8=IDC_ERRMAX,edit,1350631552
Control9=IDC_STATIC_L,static,1342308354
Control10=IDC_LANGUAGE,combobox,1344340226
Control11=IDC_TESTHW,button,1342242816
Control12=IDC_WLANGFILE,button,1342242816
Control13=IDC_NOLV,button,1342242819

[CLS:COpzioniPage]
Type=0
HeaderFile=OpzioniPage.h
ImplementationFile=OpzioniPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=IDC_WLANGFILE
VirtualFilter=idWC

[DLG:IDD_I2C_SPI]
Type=1
Class=CI2CSPIPage
ControlCount=16
Control1=IDC_SEND,button,1342242816
Control2=IDC_REC,button,1342242816
Control3=IDC_STRI,edit,1350631620
Control4=IDC_MODE,button,1342177287
Control5=IDC_I2C8B,button,1342177289
Control6=IDC_I2C16B,button,1342177289
Control7=IDC_NUMB,edit,1350631552
Control8=IDC_NB,msctls_updown32,1342177335
Control9=IDC_STRU,edit,1350633668
Control10=IDC_msgSTRI,static,1342308352
Control11=IDC_msgSTRU,static,1342308352
Control12=IDC_NBS,static,1342308352
Control13=IDC_SPI00,button,1342177289
Control14=IDC_SPI10,button,1342177289
Control15=IDC_SPI01,button,1342177289
Control16=IDC_SPI11,button,1342177289

[CLS:CI2CSPIPage]
Type=0
HeaderFile=I2CSPIPage.h
ImplementationFile=I2CSPIPage.cpp
BaseClass=CPropertyPage
Filter=D
LastObject=CI2CSPIPage
VirtualFilter=idWC

