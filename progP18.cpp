/*
 * progP18F.cpp - algorithms to program the PIC18 family of microcontrollers
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

void COpenProgDlg::Read18Fx(int dim,int dim2){
// read 16 bit PIC 18Fxxxx
// dim=program size 	dim2=eeprom size
// vdd before vpp
	int k=0,k2=0,z=0,i,j;
	int saveLog;
	CString str;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim>0x1FFFFF||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Dimensione programma oltre i limiti\r\n"
		return;
	}
	if(dim2>0x800||dim2<0){
		PrintMessage(strings[S_EELim]);	//"Dimensione eeprom oltre i limiti\r\n"
		return;
	}
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Read18F(%d,%d)    (0x%X,0x%X)\n",dim,dim2,dim,dim2);
		WriteLog(str);
	}
	memCODE.RemoveAll();
	memCODE.SetSize(dim);		//CODE
	memEE.RemoveAll();
	memEE.SetSize(dim2);		//EEPROM
	memID.RemoveAll();
	memID.SetSize(8);			//ID+CONFIG
	memCONFIG.RemoveAll();
	memCONFIG.SetSize(14);		//CONFIG
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//3F
	bufferU[j++]=0x3F;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRU
	bufferU[j++]=0xF8;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//FF
	bufferU[j++]=0xFF;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRH
	bufferU[j++]=0xF7;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//FE
	bufferU[j++]=0xFE;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRL
	bufferU[j++]=0xF6;
	bufferU[j++]=TBLR_INC_N;		//DevID1-2	0x3FFFFE-F
	bufferU[j++]=2;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRU
	bufferU[j++]=0xF8;			//TBLPTRU
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH
	bufferU[j++]=0xF7;			//TBLPTRH
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRL
	bufferU[j++]=0xF6;			//TBLPTRL
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;bufferI[z]!=TBLR_INC_N&&z<DIMBUF;z++);
	if(z<DIMBUF-3){
		str.Format(strings[S_DevID2],bufferI[z+3],bufferI[z+2]);	//"DevID: 0x%02X%02X\r\n"
		PrintMessage(str);
		PIC_ID(0x10000+bufferI[z+2]+(bufferI[z+3]<<8));
	}
//****************** read code ********************
	PrintMessage(strings[S_CodeReading1]);		//lettura codice ...
	for(i=0,j=1;i<dim;i+=DIMBUF-4){
		bufferU[j++]=TBLR_INC_N;
		bufferU[j++]=i<dim-(DIMBUF-4)?DIMBUF-4:dim-i;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(2);
		read();
		if(bufferI[1]==TBLR_INC_N){
			for(z=3;z<bufferI[2]+3&&z<DIMBUF;z++) memCODE[k++]=bufferI[z];
		}
		str.Format(strings[S_CodeReading2],i*100/(dim+dim2),i);	//"Lettura: %d%%, ind. %05X"
		StatusBar.SetWindowText(str);
		j=1;
		if(saveLog){
			str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
			WriteLog(str);
			WriteLogIO();
		}
	}
	if(k!=dim){
		str.Format(strings[S_ReadCodeErr2],dim,k);	//"Errore in lettura area programma, richiesti %d byte, letti %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
//****************** read config area ********************
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//TBLPTRU	ID 0x200000
	bufferU[j++]=0x20;			//TBLPTRU	ID 0x200000
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//TBLPTRU
	bufferU[j++]=0xF8;			//TBLPTRU
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH
	bufferU[j++]=0xF7;			//TBLPTRH
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRL
	bufferU[j++]=0xF6;			//TBLPTRL
	bufferU[j++]=TBLR_INC_N;
	bufferU[j++]=8;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//TBLPTRU	CONFIG 0x300000
	bufferU[j++]=0x30;			//TBLPTRU	CONFIG 0x300000
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//TBLPTRU
	bufferU[j++]=0xF8;			//TBLPTRU
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH
	bufferU[j++]=0xF7;			//TBLPTRH
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRL
	bufferU[j++]=0xF6;			//TBLPTRL
	bufferU[j++]=TBLR_INC_N;
	bufferU[j++]=14;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	StatusBar.SetWindowText(strings[S_ReadID_CONFIG]);	//"Lettura ID e CONFIG"
	write();
	msDelay(8);
	read();
	for(z=1;bufferI[z]!=TBLR_INC_N&&z<DIMBUF-28;z++);
	if(z<DIMBUF-28){
		for(i=0;i<8;i++) memID[k2++]=bufferI[z+i+2];
		for(;i<14+8;i++) memCONFIG[-8+k2++]=bufferI[z+i+8];
	}
	j=1;
	if(saveLog){
		str.Format(strings[S_Log7],i,i,k2,k2);	//"i=%d(0x%X), k=%d(0x%X)\n"
		WriteLog(str);
		WriteLogIO();
	}
	if(k2!=22){
		str.Format(strings[S_ReadConfigErr],22,k2);	//"Errore in lettura area configurazione, richiesti %d byte, letti %d\r\n"
		PrintMessage(str);
	}
//****************** read eeprom ********************
	if(dim2){
		PrintMessage(strings[S_ReadEE]);		//lettura eeprom ...
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x9E;				//EEPGD=0
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x9C;				//CFGS=0
		bufferU[j++]=0xA6;
		for(k2=0,i=0;i<dim2;i++){
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x0E;
			bufferU[j++]=i&0xFF;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x6E;			//ADDR
			bufferU[j++]=0xA9;			//ADDR
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x0E;
			bufferU[j++]=(i>>8)&0xFF;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x6E;			//ADDRH
			bufferU[j++]=0xAA;			//ADDRH
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x80;			//RD=1 :Read
			bufferU[j++]=0xA6;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x50;			//MOVF EEDATA,W
			bufferU[j++]=0xA8;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x6E;			//MOVWF TABLAT
			bufferU[j++]=0xF5;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x00;			//NOP
			bufferU[j++]=0x00;
			bufferU[j++]=SHIFT_TABLAT;
			if(j>DIMBUF-26||i==dim2-1){
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(10);
				read();
				for(z=1;z<DIMBUF-1;z++){
					if(bufferI[z]==SHIFT_TABLAT){
						memEE[k2++]=bufferI[z+1];
						z+=8;
					}
				}
				str.Format(strings[S_CodeReading],(i+dim)*100/(dim+dim2),i);	//"Lettura: %d%%, ind. %03X"
				StatusBar.SetWindowText(str);
				j=1;
				if(saveLog){
					str.Format(strings[S_Log7],i,i,k2,k2);	//"i=%d(0x%X), k=%d(0x%X)\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		if(k2!=dim2){
			str.Format(strings[S_ReadEEErr],dim2,k2);	//"Errore in lettura area EEPROM, richiesti %d byte, letti %d\r\n"
			PrintMessage(str);
		}
		else PrintMessage(strings[S_Compl]);
	}
//****************** exit ********************
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=1;
	bufferU[j++]=EN_VPP_VCC;		//0
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	StatusBar.SetWindowText("");
//****************** visualize ********************
	for(i=0;i<8;i+=2){
		str.Format(strings[S_ChipID2],i,memID[i],i+1,memID[i+1]);	//"ID%d: 0x%02X   ID%d: 0x%02X\r\n"
		PrintMessage(str);
	}
	for(i=0;i<7;i++){
		str.Format(strings[S_ConfigWordH],i+1,memCONFIG[i*2+1]);	//"CONFIG%dH: 0x%02X\t"
		PrintMessage(str);
		str.Format(strings[S_ConfigWordL],i+1,memCONFIG[i*2]);	//"CONFIG%dL: 0x%02X\r\n"
		PrintMessage(str);
	}
	PrintMessage(strings[S_CodeMem]);	//"\r\nMemoria programma:\r\n"
	CString aux;
	char s[256],t[256];
	s[0]=0;
	int valid=0,empty=1;
	for(i=0;i<dim;i+=COL*2){
		valid=0;
		for(j=i;j<i+COL*2&&j<dim;j++){
			sprintf(t,"%02X ",memCODE[j]);
			strcat(s,t);
			if(memCODE[j]<0xff) valid=1;
		}
		if(valid){
			sprintf(t,"%04X: %s\r\n",i,s);
			aux+=t;
			empty=0;
		}
		s[0]=0;
	}
	PrintMessage(aux);
	if(empty) PrintMessage(strings[S_Empty]);	//empty
	if(dim2){
		DisplayEE();
	}
	str.Format(strings[S_End],(stop-start)/1000.0);	//"\r\nFine (%.2f s)\r\n"
	PrintMessage(str);
	if(saveLog) CloseLogFile();
}

void COpenProgDlg::Write18Fx(int dim,int dim2,int wbuf=8,int eraseW1=0xFFFF,int eraseW2=0xFFFF,int EEalgo=0)
// write 16 bit PIC 18Fxxxx
// dim=program size 	dim2=eeprom size	wbuf=write buffer size {<=64}
// eraseW1=erase word @3C0005	(not used if > 0x10000)
// eraseW2=erase word @3C0004	(not used if > 0x10000)
// EEalgo=eeprom write algoritm: 0->new, 1->old (with sequence 55 AA)
// vdd before vpp
{
	int k=0,k2=0,z=0,i,j;
	int programID=0,max_err,err=0,devID=0;
	int saveLog;
	CString str;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim>0x1FFFFF||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Dimensione programma oltre i limiti\r\n"
		return;
	}
	if(dim2>0x800||dim2<0){
		PrintMessage(strings[S_EELim]);	//"Dimensione eeprom oltre i limiti\r\n"
		return;
	}
	if(wbuf>64){
		PrintMessage(strings[S_WbufLim]);	//"Dimensione buffer scrittura oltre i limiti\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write18F(%d,%d,%d)    (0x%X,0x%X,0x%X)\n",dim,dim2,wbuf,dim,dim2,wbuf);
		WriteLog(str);
	}
	if(dim>memCODE.GetSize()) dim=memCODE.GetSize();
	if(dim2>memEE.GetSize()) dim2=memEE.GetSize();
	if(dim<1){
		PrintMessage(strings[S_NoCode]);	//"Area dati vuota\r\n"
		return;
	}
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=5100>>8;
	bufferU[j++]=5100&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//3F
	bufferU[j++]=0x3F;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRU
	bufferU[j++]=0xF8;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//FF
	bufferU[j++]=0xFF;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRH
	bufferU[j++]=0xF7;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//FE
	bufferU[j++]=0xFE;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRL
	bufferU[j++]=0xF6;
	bufferU[j++]=TBLR_INC_N;		//DevID1-2	0x3FFFFE-F
	bufferU[j++]=2;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;bufferI[z]!=TBLR_INC_N&&z<DIMBUF;z++);
	if(z<DIMBUF-3){
		str.Format(strings[S_DevID2],bufferI[z+3],bufferI[z+2]);	//"DevID: 0x%02X%02X\r\n"
		PrintMessage(str);
		PIC_ID(0x10000+bufferI[z+2]+(bufferI[z+3]<<8));
	}
	j=1;
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x0E;			//3C
	bufferU[j++]=0x3C;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6E;			//-> TBLPTRU
	bufferU[j++]=0xF8;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH=0
	bufferU[j++]=0xF7;
	if(eraseW1<0x10000){
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//05
		bufferU[j++]=0x05;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//-> TBLPTRL
		bufferU[j++]=0xF6;
		bufferU[j++]=TABLE_WRITE;		// eraseW1@3C0005
		bufferU[j++]=(eraseW1>>8)&0xFF; 	//0x3F;
		bufferU[j++]=eraseW1&0xFF; 		//0x3F;
	}
	if(eraseW2<0x10000){
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//04
		bufferU[j++]=0x04;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//-> TBLPTRL
		bufferU[j++]=0xF6;
		bufferU[j++]=TABLE_WRITE;		// eraseW2@3C0004
		bufferU[j++]=(eraseW2>>8)&0xFF; 	//0x8F;
		bufferU[j++]=eraseW2&0xFF; 		//0x8F;
	}
	bufferU[j++]=CORE_INS;		//NOP
	bufferU[j++]=0x00;
	bufferU[j++]=0x00;
	bufferU[j++]=CORE_INS;		//NOP
	bufferU[j++]=0x00;
	bufferU[j++]=0x00;
	bufferU[j++]=WAIT_T3;		//ritardo cancellazione
//****************** prepare write ********************
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x8E;			//EEPGD=1
	bufferU[j++]=0xA6;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x9C;			//CFCGS=0
	bufferU[j++]=0xA6;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRU
	bufferU[j++]=0xF8;			//TBLPTRU
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH
	bufferU[j++]=0xF7;			//TBLPTRH
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRL
	bufferU[j++]=0xF6;			//TBLPTRL
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(6);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	int ww,dimx,st;
	ww=memCODE.GetSize();
	for(dimx=dim-1;dimx&&memCODE[dimx]==0xFF;dimx--);
	if(!dimx) PrintMessage(strings[S_NoCode2]);	//"Niente da scrivere"
	dimx++;
	if(dimx%wbuf) dimx+=wbuf-dimx%wbuf;		//arrotonda a wbuf
	if(memCODE.GetSize()<dimx){
		i=memCODE.GetSize();
		memCODE.SetSize(dimx);
		for(;i<memCODE.GetSize();i++) memCODE[i]=0xFF;
	}
	if(saveLog){
		str.Format(strings[S_Log5],dim,dim,dimx,dimx,dimx/wbuf);	//"dim=%d(0x%X), dimx=%d(0x%X), dimx/wbuf=%d \n\n"
		WriteLog(str);
	}
	for(i=0,j=1,st=0;i<dimx;){
		bufferU[j++]=TBLW_INC_N;
		if(st){
			ww=wbuf/2-1-ww;
			st=0;
		}
		else if(j+wbuf+5>DIMBUF){
			ww=(DIMBUF-1-5-j)/2;
			st=1;
		}
		else{
			ww=wbuf/2-1;
			st=0;
		}
		bufferU[j++]=ww;
		for(k=0;k<ww;k++){
			bufferU[j++]=memCODE[i+1];
			bufferU[j++]=memCODE[i];
			i+=2;
		}
		if(!st){
			bufferU[j++]=TBLW_PROG_INC;
			bufferU[j++]=memCODE[i+1];
			bufferU[j++]=memCODE[i];
			bufferU[j++]=1000>>8;
			bufferU[j++]=1000&0xFF;
			i+=2;
		}
		if(j>=DIMBUF-1-6||i>=dimx-1){
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(DIMBUF/wbuf+1);
			read();
			str.Format(strings[S_CodeWriting2],i*100/(dim+dim2+22),i);	//"Scrittura: %d%%, ind. %04X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,0,0);	//"i=%d, k=%d 0=%d\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	if(i!=dimx){
		str.Format(strings[S_CodeWError4],i,dimx);	//"Errore in scrittura area programma, richiesti %d byte, scritti %d\r\n"
		PrintMessage(str);
	}
	PrintMessage(strings[S_Compl]);	//"terminata\r\n"
//****************** write ID ********************
	if(memID.GetSize()){
		PrintMessage(strings[S_IDW]);	//"Scrittura ID ... "
		for(i=memID.GetSize();i<8;i++) memID[i]=0xFF;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x8E;
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x9C;
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//TBLPTRU	ID 0x200000
		bufferU[j++]=0x20;			//TBLPTRU	ID 0x200000
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//TBLPTRU
		bufferU[j++]=0xF8;			//TBLPTRU
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRH
		bufferU[j++]=0xF7;			//TBLPTRH
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRL
		bufferU[j++]=0xF6;			//TBLPTRL
		bufferU[j++]=TBLW_INC_N;
		bufferU[j++]=3;
		for(i=0;i<3;i++){
			bufferU[j++]=memID[i*2+1];
			bufferU[j++]=memID[i*2];
		}
		bufferU[j++]=TBLW_PROG;
		bufferU[j++]=memID[i*2+1];
		bufferU[j++]=memID[i*2];
		bufferU[j++]=1000>>8;
		bufferU[j++]=1000&0xFF;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(5);
		read();
		j=1;
		if(saveLog){
			str.Format(strings[S_Log7],i,i,0,0);	//"i=%d, k=%d 0=%d\n"
			WriteLog(str);
			WriteLogIO();
		}
		PrintMessage(strings[S_Compl]);	//"terminata\r\n"
	}
//****************** write and verify EEPROM ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Scrittura EEPROM ... "
		int errEE=0;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x9E;			//EEPGD=0
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x9C;			//CFGS=0
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x84;			//WREN=1
		bufferU[j++]=0xA6;
		for(i=0;i<dim2&&err<=max_err;i++){
			if(memEE[i]!=0xFF){
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x0E;
				bufferU[j++]=i&0xFF;
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x6E;
				bufferU[j++]=0xA9;			//ADDR
				if(EEalgo==0){
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x0E;
					bufferU[j++]=(i>>8)&0xFF;
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x6E;
					bufferU[j++]=0xAA;		//ADDRH
				}
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x0E;
				bufferU[j++]=memEE[i];
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x6E;
				bufferU[j++]=0xA8;			//EEDATA
				if(EEalgo==1){				//memory unlock
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x0E;
					bufferU[j++]=0x55;
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x6E;
					bufferU[j++]=0xA7;			//EECON2
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x0E;
					bufferU[j++]=0xAA;
					bufferU[j++]=CORE_INS;
					bufferU[j++]=0x6E;
					bufferU[j++]=0xA7;			//EECON2
				}
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x82;
				bufferU[j++]=0xA6;			//WR=1
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x00;
				bufferU[j++]=0x00;			//NOP
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x00;
				bufferU[j++]=0x00;			//NOP
				bufferU[j++]=WAIT_T3;		//ritardo scrittura
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x80;			//RD=1
				bufferU[j++]=0xA6;
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x50;			//MOVF EEDATA,w
				bufferU[j++]=0xA8;
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x6E;			//MOVWF TABLAT
				bufferU[j++]=0xF5;
				bufferU[j++]=CORE_INS;
				bufferU[j++]=0x00;			//NOP
				bufferU[j++]=0x00;
				bufferU[j++]=SHIFT_TABLAT;
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(8);
				read();
				str.Format(strings[S_CodeWriting],(i+dim)*100/(dim+dim2),i);	//"Scrittura: %d%%, ind. %03X"
				StatusBar.SetWindowText(str);
				j=1;
				for(z=DIMBUF-1;z&&bufferI[z]!=SHIFT_TABLAT;z--);
				if(z&&memEE[i]!=bufferI[z+1]) errEE++;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,errEE);	//"i=%d, k=%d, errori=%d\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		str.Format(strings[S_ComplErr],errEE);	//"terminata: %d errori \r\n"
		PrintMessage(str);
		err+=errEE;
	}
//****************** verify code ********************
	PrintMessage(strings[S_CodeV]);	//"Verifica codice ... "
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x8E;			//EEPGD=1
	bufferU[j++]=0xA6;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x9C;			//CFCGS=0
	bufferU[j++]=0xA6;
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRU
	bufferU[j++]=0xF8;			//TBLPTRU
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRH
	bufferU[j++]=0xF7;			//TBLPTRH
	bufferU[j++]=CORE_INS;
	bufferU[j++]=0x6A;			//TBLPTRL
	bufferU[j++]=0xF6;			//TBLPTRL
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog){
		WriteLogIO();
		WriteLog("\n\n");
	}
	for(i=0,j=1,k=0;i<dimx;i+=DIMBUF-4){
		bufferU[j++]=TBLR_INC_N;
		bufferU[j++]=i<dimx-(DIMBUF-4)?DIMBUF-4:dimx-i;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(2);
		read();
		if(bufferI[1]==TBLR_INC_N){
			for(z=0;z<bufferI[2]&&z<DIMBUF;z++){
				if(memCODE[i+z]!=bufferI[z+3]){
					str.Format(strings[S_CodeVError],i+z,i+z,memCODE[i+z],bufferI[z+3]);	//"Errore in verifica, indirizzo %04X (%d), scritto %02X, letto %02X\r\n"
					PrintMessage(str);
					err++;
				}
				k++;
			}
		}
		str.Format(strings[S_CodeV2],i*100/(dim+dim2),i);	//"Verifica: %d%%, ind. %04X"
		StatusBar.SetWindowText(str);
		j=1;
		if(saveLog){
			str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d\n"
			WriteLog(str);
			WriteLogIO();
		}
		if(err>=max_err) break;
	}
	if(k<dimx){
		str.Format(strings[S_CodeVError2],dimx,k);	//"Errore in verifica area programma, richiesti %d byte, letti %d\r\n"
		PrintMessage(str);
	}
	str.Format(strings[S_ComplErr],err);	//"terminata: %d errori\r\n"
	PrintMessage(str);
	if(err>=max_err){
		str.Format(strings[S_MaxErr],err);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
		PrintMessage(str);
	}
//****************** verify ID ********************
	if(memID.GetSize()>=8&&err<max_err){
		PrintMessage(strings[S_IDV]);	//"Verifica ID ... "
		int errID=0;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//TBLPTRU	ID 0x200000
		bufferU[j++]=0x20;			//TBLPTRU	ID 0x200000
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//TBLPTRU
		bufferU[j++]=0xF8;			//TBLPTRU
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRH
		bufferU[j++]=0xF7;			//TBLPTRH
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRL
		bufferU[j++]=0xF6;			//TBLPTRL
		bufferU[j++]=TBLR_INC_N;
		bufferU[j++]=8;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(2);
		read();
		for(z=0;bufferI[z]!=TBLR_INC_N&&z<DIMBUF;z++);
		for(i=0;i<8;i++) if(memID[i]!=0xFF&&memID[i]!=bufferI[z+i+2]) errID++;
		str.Format(strings[S_ComplErr],errID);	//"terminata: %d errori\r\n"
		PrintMessage(str);
		err+=errID;
		if(err>=max_err){
			str.Format(strings[S_MaxErr],err);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
			PrintMessage(str);
		}
		j=1;
		if(saveLog){
			str.Format(strings[S_Log8],i,i,0,0,err);	//"i=%d, k=%d, errori=%d\n"
			WriteLog(str);
			WriteLogIO();
		}
	}
//****************** write CONFIG ********************
	if(memCONFIG.GetSize()&&err<max_err){
		PrintMessage(strings[S_ConfigW]);	//"Programmazione CONFIG ..."
		for(i=memCONFIG.GetSize();i<14;i++) memCONFIG[i]=0xFF;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x8E;
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x8C;
		bufferU[j++]=0xA6;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//CONFIG 0x300000
		bufferU[j++]=0x30;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//TBLPTRU
		bufferU[j++]=0xF8;			//TBLPTRU
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRH
		bufferU[j++]=0xF7;			//TBLPTRH
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRL
		bufferU[j++]=0xF6;			//TBLPTRL
		for(i=0;i<14;){
			bufferU[j++]=TBLW_PROG;
			bufferU[j++]=0;
			bufferU[j++]=memCONFIG[i++];
			bufferU[j++]=1000>>8;
			bufferU[j++]=1000&0xFF;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x2A;			//INCF
			bufferU[j++]=0xF6;			//TBLPTRL
			bufferU[j++]=TBLW_PROG;
			bufferU[j++]=memCONFIG[i++];
			bufferU[j++]=0;
			bufferU[j++]=1000>>8;
			bufferU[j++]=1000&0xFF;
			bufferU[j++]=CORE_INS;
			bufferU[j++]=0x2A;			//INCF
			bufferU[j++]=0xF6;			//TBLPTRL
			if(j>DIMBUF-17||i==14){
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(5);
				read();
				j=1;
				if(saveLog){
					str.Format(strings[S_Log7],i,i,0,0);	//"i=%d, k=%d\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		PrintMessage(strings[S_Compl]);	//"terminata\r\n"
//****************** verify CONFIG ********************
		PrintMessage(strings[S_ConfigV]);	//"Verifica CONFIG ... "
		int errC=0;
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x0E;			//TBLPTRU	CONFIG 0x300000
		bufferU[j++]=0x30;			//TBLPTRU	CONFIG 0x300000
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6E;			//TBLPTRU
		bufferU[j++]=0xF8;			//TBLPTRU
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRH
		bufferU[j++]=0xF7;			//TBLPTRH
		bufferU[j++]=CORE_INS;
		bufferU[j++]=0x6A;			//TBLPTRL
		bufferU[j++]=0xF6;			//TBLPTRL
		bufferU[j++]=TBLR_INC_N;
		bufferU[j++]=14;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(2);
		read();
		for(z=1;bufferI[z]!=TBLR_INC_N&&z<DIMBUF-16;z++);
		if(z<DIMBUF-16){
			for(i=0;i<14;i++) if(!memCONFIG[i]&bufferI[z+i+2]) errC++;	//error if written 0 and read 1 (!W&R)
		}
		str.Format(strings[S_ComplErr],errC);	//"terminata: %d errori\r\n"
		PrintMessage(str);
		err+=errC;
		if(err>=max_err){
			str.Format(strings[S_MaxErr],err);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
			PrintMessage(str);
		}
		j=1;
		if(saveLog){
			str.Format(strings[S_Log8],i,i,0,0,err);	//"i=%d, k=%d, errori=%d\n"
			WriteLog(str);
			WriteLogIO();
		}
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=1;
	bufferU[j++]=EN_VPP_VCC;		//0
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	StatusBar.SetWindowText("");
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nFine (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog) CloseLogFile();
}
