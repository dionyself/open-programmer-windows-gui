//MSVC common declarations

#include "DatiPage.h"
#include "DispositivoPage.h"
#include "OpzioniPage.h"
#include "I2CSPIPage.h"
#include "OpenProg.h"
#include "OpenProgDlg.h"
#include <string.h>
#include "strings.h"
#include "instructions.h"

#define VERSION "0.7.9"
#define G (12.0/34*1024/5)		//=72,2823529412
#define DIMBUF 65
#define COL 16
#define LOCK	1
#define FUSE	2
#define FUSE_H 4
#define FUSE_X	8
#define CAL	16
#define SLOW	256
#define PrintMessage1(s,p) {str.Format(s,p); PrintMessage(str);}
#define PrintMessage2(s,p1,p2) {str.Format(s,p1,p2); PrintMessage(str);}
#define PrintMessage3(s,p1,p2,p3) {str.Format(s,p1,p2,p3); PrintMessage(str);}
#define PrintMessage4(s,p1,p2,p3,p4) {str.Format(s,p1,p2,p3,p4); PrintMessage(str);}
#define PrintStatus(s,p1,p2) {str.Format(s,p1,p2); StatusBar.SetWindowText(str);}
#define	PrintStatusSetup() //only needed for console version
#define	PrintStatusEnd() //only needed for console version
#define	PrintStatusClear() StatusBar.SetWindowText("");
#define write()	Result=WriteFile(WriteHandle,bufferU,DIMBUF,&BytesWritten,NULL);
#define read()	Result = ReadFile(ReadHandle,bufferI,DIMBUF,&NumberOfBytesRead,(LPOVERLAPPED) &HIDOverlapped);\
				Result = WaitForSingleObject(hEventObject,10);\
				ResetEvent(hEventObject);\
				if(Result!=WAIT_OBJECT_0){\
					PrintMessage(strings[S_comTimeout]);	/*"communication timeout\r\n"*/\
				}
