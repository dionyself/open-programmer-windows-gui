/*
 * progAVR.cpp - algorithms to program the Atmel AVR family of microcontrollers
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

void COpenProgDlg::AtmelID(BYTE id[])
{
	CString str,str2;
	if(id[0]==0x1e) print("%s","Atmel ");
	str2+=str;
	if(id[1]==0x90){
		switch(id[2]){
			case 0x01:
				print("%s","AT90S1200");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 1KB Flash");
	}
	else if(id[1]==0x91){
		switch(id[2]){
			case 0x01:
				print("%s","AT90S2313");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 2KB Flash");
	}
	else if(id[1]==0x93){
		switch(id[2]){
			case 0x01:
				print("%s","AT90S8515");
				break;
			case 0x03:
				print("%s","AT90S8535");
				break;
			case 0x06:
				print("%s","ATmega8515");
				break;
			case 0x07:
				print("%s","ATmega8");
				break;
			case 0x08:
				print("%s","ATmega8535");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 8KB Flash");
	}
	else if(id[1]==0x94){
		switch(id[2]){
			case 0x03:
				print("%s","ATmega16");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 16KB Flash");
	}
	else if(id[1]==0x95){
		switch(id[2]){
			case 0x02:
				print("%s","ATmega32");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 32KB Flash");
	}
	else if(id[1]==0x96){
		switch(id[2]){
			case 0x02:
				print("%s","ATmega64");
				break;
			default:
				print("%s",strings[S_nodev]); //"Dispositivo sconosciuto\r\n");
		}
		str2+=str;
		print("%s"," 64KB Flash");
	}
	if(id[0]==0&&id[1]==1&&id[2]==2) print("%s",strings[S_Protected]);		//"Dispositivo protetto"
	str2+=str;
	str2+="\r\n";
	PrintMessage(str2);
}

#define RST 0x40
void COpenProgDlg::ReadAT(int dim, int dim2, int options)
// read ATMEL AVR
// dim=FLASH size in bytes, dim2=EEPROM size
// options: LOCK,FUSE,FUSE_H,FUSE_X,CAL
{
	int k=0,k2=0,z=0,i,j;
	int saveLog;
	BYTE signature[]={0,0,0};
	CString str,s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim>0x20000||dim<0){
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
		str.Format("ReadAT(0x%X,0x%X,0x%X)\n",dim,dim2,options);
		WriteLog(str);
	}
	hvreg=0;
	memCODE.RemoveAll();
	memCODE.SetSize(dim);		//CODE
	for(j=0;j<memCODE.GetSize();j++) memCODE[j]=0x55;
	memEE.RemoveAll();
	memEE.SetSize(dim2);		//EEPROM
	for(j=0;j<memEE.GetSize();j++) memEE[j]=0x55;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=20000>>8;
	bufferU[j++]=20000&0xff;
	bufferU[j++]=VREG_DIS;		//Disable HV reg
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x0;
	bufferU[j++]=SPI_INIT;
	bufferU[j++]=1;				//0=100k, 1=200k
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=3;				//0=100k,200k,500k,1M,2M
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=EXT_PORT;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(i=0;i<32;i++){
		j=1;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=RST;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=4;
		bufferU[j++]=WAIT_T3;		//20ms
		bufferU[j++]=SPI_WRITE;		//Programming enable
		bufferU[j++]=2;
		bufferU[j++]=0xAC;
		bufferU[j++]=0x53;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=2;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(25);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		//str.Format("i=%d z=%d   rx:%02X%02X\r\n",i,z,bufferI[z+2],bufferI[z+3]);
		//PrintMessage(str);
		if(bufferI[z+2]==0x53) i=32;
	}
	if(i<33){
		j=1;
		bufferU[j++]=EN_VPP_VCC;	//VDD
		bufferU[j++]=0x0;
		bufferU[j++]=SPI_INIT;
		bufferU[j++]=0xFF;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(3);
		read();
		if(saveLog)WriteLogIO();
		PrintMessage(strings[S_SyncErr]);	//"Errore di sincronizzazione\r\n"
		if(saveLog) CloseLogFile();
		return;
	}
	j=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=1;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=2;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	if(options&LOCK){			//LOCK byte
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x58;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
	}
	if(options&FUSE){			//FUSE byte
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x50;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
	}
	if(options&FUSE_H){			//FUSE high byte
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x58;
		bufferU[j++]=8;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
	}
	if(options&FUSE_X){			//extended FUSE byte
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x50;
		bufferU[j++]=8;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(8);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[0]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[1]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[2]=bufferI[z+2];
	str.Format("CHIP ID:%02X%02X%02X\r\n",signature[0],signature[1],signature[2]);
	PrintMessage(str);
	AtmelID(signature);
	if(options&LOCK){			//LOCK byte
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format("LOCK bits:\t  0x%02X\r\n",bufferI[z+2]);
		PrintMessage(str);
	}
	if(options&FUSE){			//FUSE byte
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format("FUSE bits:\t  0x%02X\r\n",bufferI[z+2]);
		PrintMessage(str);
	}
	if(options&FUSE_H){			//FUSE high byte
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format("FUSE HIGH bits:\t  0x%02X\r\n",bufferI[z+2]);
		PrintMessage(str);
	}
	if(options&FUSE_X){			//extended FUSE byte
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format("Extended FUSE bits: 0x%02X\r\n",bufferI[z+2]);
		PrintMessage(str);
	}
	if(options&CAL){			//calibration byte
		j=1;
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x38;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x38;
		bufferU[j++]=0;
		bufferU[j++]=1;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x38;
		bufferU[j++]=0;
		bufferU[j++]=2;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x38;
		bufferU[j++]=0;
		bufferU[j++]=3;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(4);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format("Calibration bits:\t  0x%02X",bufferI[z+2]);
		PrintMessage(str);
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format(",0x%02X",bufferI[z+2]);
		PrintMessage(str);
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format(",0x%02X",bufferI[z+2]);
		PrintMessage(str);
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		str.Format(",0x%02X\r\n",bufferI[z+2]);
		PrintMessage(str);
	}/**/
//****************** read code ********************
	PrintMessage(strings[S_CodeReading1]);		//lettura codice ...
	int c=(DIMBUF-5)/2;
	for(i=0,j=1;i<dim;i+=c*2){
		bufferU[j++]=AT_READ_DATA;
		bufferU[j++]=i<(dim-2*c)?c:(dim-i)/2;
		bufferU[j++]=i>>9;
		bufferU[j++]=i>>1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(15);
		read();
		if(bufferI[1]==AT_READ_DATA){
			for(z=3;z<bufferI[2]*2+3&&z<DIMBUF;z++) memCODE[k++]=bufferI[z];
		}
		str.Format(strings[S_CodeReading],i*100/(dim+dim2),i);	//"Lettura: %d%%, ind. %03X"
		StatusBar.SetWindowText(str);
		j=1;
		if(saveLog){
			str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
			WriteLog(str);
			WriteLogIO();
		}
	}/**/
/*	for(k=0,i=0,j=1;i<dim;i+=2){	//read byte by byte
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x20;
		bufferU[j++]=i>>9;
		bufferU[j++]=i>>1;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x28;
		bufferU[j++]=i>>9;
		bufferU[j++]=i>>1;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		if(j>DIMBUF-16||i>=dim-2){
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(12);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==SPI_READ&&bufferI[z+1]==1){
					memCODE[k++]=bufferI[z+2];
					z+=2;
				}
			}
			str.Format(strings[S_CodeReading],i*100/(dim+dim2),i);	//"Lettura: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
*/
	if(k!=dim){
		str.Format(strings[S_ReadCodeErr2],dim,k);	//"Errore in lettura area programma, richiesti %d byte, letti %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** read eeprom ********************
	if(dim2){
		PrintMessage(strings[S_ReadEE]);		//lettura EE ...
		for(k2=0,i=0,j=1;i<dim2;i++){
			bufferU[j++]=SPI_WRITE;		//Read eeprom memory
			bufferU[j++]=3;
			bufferU[j++]=0xA0;
			bufferU[j++]=i>>8;
			bufferU[j++]=i;
			bufferU[j++]=SPI_READ;
			bufferU[j++]=1;
			if(j>DIMBUF-9||i==dim-1){
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(4);
				read();
				for(z=1;z<DIMBUF-2;z++){
					if(bufferI[z]==SPI_READ&&bufferI[z+1]==1){
						memEE[k2++]=bufferI[z+2];
						z+=3;
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
		else PrintMessage(strings[S_Compl]);	//"completata\r\n"
	}
//****************** exit program mode ********************
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=SPI_WRITE;
	bufferU[j++]=1;
	bufferU[j++]=0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	StatusBar.SetWindowText("");
//****************** visualize ********************
	PrintMessage(strings[S_CodeMem]);	//"\r\nMemoria programma:\r\n"
	CString aux;
	for(i=0;i<dim;i+=COL*2){
		int valid=0;
		for(j=i;j<i+COL*2&&j<dim;j++){
			t.Format("%02X ",memCODE[j]&0xff);
			s+=t;
			if(memCODE[j]<0xff) valid=1;
		}
		if(valid){
			t.Format("%04X: %s\r\n",i,s);
			aux+=t;
		}
		s.Empty();
	}
	if(aux.GetLength()) PrintMessage(aux);
	else PrintMessage(strings[S_Empty]);	//empty
	if(dim2){
		DisplayEE();
	}
	str.Format(strings[S_End],(stop-start)/1000.0);	//"\r\nFine (%.2f s)\r\n"
	PrintMessage(str);
	if(saveLog) CloseLogFile();
}

void COpenProgDlg::WriteAT(int dim, int dim2)
// write ATMEL micro
// dim=FLASH size in bytes, dim2=EEPROM size
{
	int k=0,k2=0,z=0,i,j;
	int saveLog, errori=0,erroriEE=0,ritenta=0,maxtent=0;
	int lock=0x100;
	BYTE signature[]={0,0,0};
	CString str,s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim>0x8000||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Dimensione programma oltre i limiti\r\n"
		return;
	}
	if(dim2>0x800||dim2<0){
		PrintMessage(strings[S_EELim]);	//"Dimensione eeprom oltre i limiti\r\n"
		return;
	}
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_LOCK_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_LOCK,str);
		i=sscanf(str,"%x",&lock);
		if(i!=1||lock<0||lock>0xFF) lock=0x100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("WriteAT(0x%X,0x%X)\n",dim,dim2);
		WriteLog(str);
	}
	int max_errori=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim>memCODE.GetSize()) dim=memCODE.GetSize();
	if(dim2>memEE.GetSize()) dim2=memEE.GetSize();
	if(dim<1){
		PrintMessage(strings[S_NoCode]);	//"Area dati vuota\r\n"
		return;
	}
	hvreg=0;
	PrintMessage(strings[S_Writing]);	//"Inizio scrittura...\r\n"
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=20000>>8;
	bufferU[j++]=20000&0xff;
	bufferU[j++]=VREG_DIS;		//Disable HV reg
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x0;
	bufferU[j++]=SPI_INIT;
	bufferU[j++]=1;
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=3;
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=EXT_PORT;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(i=0;i<32;i++){
		j=1;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=RST;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=4;
		bufferU[j++]=WAIT_T3;		//20ms
		bufferU[j++]=SPI_WRITE;		//Programming enable
		bufferU[j++]=2;
		bufferU[j++]=0xAC;
		bufferU[j++]=0x53;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=2;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(25);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		//str.Format("i=%d z=%d   rx:%02X%02X\r\n",i,z,bufferI[z+2],bufferI[z+3]);
		//PrintMessage(str);
		if(bufferI[z+2]==0x53) i=32;
	}
	if(i<33){
		j=1;
		bufferU[j++]=EN_VPP_VCC;	//VDD
		bufferU[j++]=0x0;
		bufferU[j++]=SPI_INIT;
		bufferU[j++]=0xFF;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(1);
		read();
		if(saveLog)WriteLogIO();
		PrintMessage(strings[S_SyncErr]);	//"Errore di sincronizzazione\r\n"
		if(saveLog) CloseLogFile();
		return;
	}
	j=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=1;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=2;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[0]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[1]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[2]=bufferI[z+2];
	str.Format("CHIP ID:%02X%02X%02X\r\n",signature[0],signature[1],signature[2]);
	PrintMessage(str);
	AtmelID(signature);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SPI_WRITE;		//Chip erase
	bufferU[j++]=4;
	bufferU[j++]=0xAC;
	bufferU[j++]=0x80;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(i=0,j=1;i<dim;i++){
		if(memCODE[i]!=0xFF){
			bufferU[j++]=SPI_WRITE;		//Write program memory
			bufferU[j++]=4;
			bufferU[j++]=0x40+(i&1?8:0);
			bufferU[j++]=i>>9;
			bufferU[j++]=i>>1;
			bufferU[j++]=memCODE[i];
			bufferU[j++]=WAIT_T3;		//6ms
			bufferU[j++]=WAIT_T3;
			bufferU[j++]=WAIT_T3;
			bufferU[j++]=SPI_WRITE;		//Read program memory
			bufferU[j++]=3;
			bufferU[j++]=0x20+(i&1?8:0);
			bufferU[j++]=i>>9;
			bufferU[j++]=i>>1;
			bufferU[j++]=SPI_READ;
			bufferU[j++]=1;
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			j=1;
			write();
			msDelay(9);
			read();
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Scrittura: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
			if(z==DIMBUF-2||memCODE[i]!=bufferI[z+2]){
				if(ritenta<5){
					ritenta++;
					if (ritenta>maxtent) maxtent=ritenta;
					i--;
				}
				else{
					errori++;
					ritenta=0;
				}
			}
			if(max_errori&&errori>max_errori){
				str.Format(strings[S_MaxErr],errori);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
				PrintMessage(str);
				PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
				i=dim;
			}
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,errori);	//"i=%d, k=%d, errori=%d\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	str.Format(strings[S_ComplErr],errori);	//"completata, %d errori\r\n"
	PrintMessage(str);
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Scrittura area EEPROM ... "
		for(i=0,j=1;i<dim2;i++){
			if(memEE[i]!=0xFF){
				bufferU[j++]=SPI_WRITE;		//Write EEPROM memory
				bufferU[j++]=4;
				bufferU[j++]=0xC0;
				bufferU[j++]=i>>8;
				bufferU[j++]=i;
				bufferU[j++]=memEE[i];
				bufferU[j++]=WAIT_T3;		//6ms
				bufferU[j++]=WAIT_T3;
				bufferU[j++]=WAIT_T3;
				bufferU[j++]=SPI_WRITE;		//Read EEPROM memory
				bufferU[j++]=3;
				bufferU[j++]=0xA0;
				bufferU[j++]=i>>8;
				bufferU[j++]=i;
				bufferU[j++]=SPI_READ;
				bufferU[j++]=1;
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				j=1;
				write();
				msDelay(9);
				read();
				str.Format(strings[S_CodeWriting],i*100/dim2,i);	//"Scrittura: %d%%, ind. %03X"
				StatusBar.SetWindowText(str);
				for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
				if(z==DIMBUF-2||memEE[i]!=bufferI[z+2]){
					if(ritenta<10){
						ritenta++;
						if (ritenta>maxtent) maxtent=ritenta;
						i--;
					}
					else{
						erroriEE++;
						ritenta=0;
					}
				}
				if(max_errori&&errori+erroriEE>max_errori){
					str.Format(strings[S_MaxErr],errori+erroriEE);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
					PrintMessage(str);
					PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
					i=dim2;
				}
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,erroriEE);	//"i=%d, k=%d, errori=%d\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		str.Format(strings[S_ComplErr],erroriEE);	//"completata, %d errori\r\n"
		PrintMessage(str);
		errori+=erroriEE;
	}
	if (maxtent){
		str.Format(strings[S_MaxRetry],maxtent); 	//"Max tentativi di scrittura: %d\r\n"
		PrintMessage(str);
	}
//****************** write FUSE ********************
	if(lock<0x100){
		PrintMessage(strings[S_FuseAreaW]);	//"Scrittura area Fuse ... "
		bufferU[j++]=SPI_WRITE;		//Write lock
		bufferU[j++]=4;
		bufferU[j++]=0xAC;
		bufferU[j++]=0xF9+(lock&0x06);
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=WAIT_T3;		//9ms
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		j=1;
		write();
		msDelay(9);
		read();
		if(saveLog)WriteLogIO();
		PrintMessage(strings[S_Compl]);	//"completata\r\n"
	}
//****************** exit program mode ********************
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,errori,errori!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nFine (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::WriteATmega(int dim, int dim2, int page)
// write ATMEL micro
// dim=FLASH size in bytes, dim2=EEPROM, page=FLASH page size in bytes
{
	int k=0,k2=0,z=0,i,j;
	int saveLog, errori=0,erroriEE=0,ritenta=0,maxtent=0;
	BYTE signature[]={0,0,0};
	int lock=0x100,fuse=0x100,fuse_h=0x100,fuse_x=0x100;
	CString str,s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim>0x10000||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Dimensione programma oltre i limiti\r\n"
		return;
	}
	if(dim2>0x1000||dim2<0){
		PrintMessage(strings[S_EELim]);	//"Dimensione eeprom oltre i limiti\r\n"
		return;
	}
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSE_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSE,str);
		i=sscanf(str,"%x",&fuse);
		if(i!=1||fuse<0||fuse>0xFF) fuse=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSEH_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSEH,str);
		i=sscanf(str,"%x",&fuse_h);
		if(i!=1||fuse_h<0||fuse_h>0xFF) fuse_h=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_FUSEX_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_FUSEX,str);
		i=sscanf(str,"%x",&fuse_x);
		if(i!=1||fuse_x<0||fuse_x>0xFF) fuse_x=0x100;
	}
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_LOCK_P);
	if(b->GetCheck()){
		m_DispoPage.GetDlgItemText(IDC_LOCK,str);
		i=sscanf(str,"%x",&lock);
		if(i!=1||lock<0||lock>0xFF) lock=0x100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("WriteATmega(0x%X,0x%X,0x%X)\n",dim,dim2,page);
		WriteLog(str);
	}
	int max_errori=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim>memCODE.GetSize()) dim=memCODE.GetSize();
	else memCODE.SetSize(dim);
	if(memCODE.GetSize()%(page*2)){
		j=memCODE.GetSize();
		memCODE.SetSize((j/(page*2)+1)*page*2);
		for(;j<memCODE.GetSize();j++) memCODE[j]=0xFF;
		dim=memCODE.GetSize();
	}
	if(dim2>memEE.GetSize()) dim2=memEE.GetSize();
	if(dim<1){
		PrintMessage(strings[S_NoCode]);	//"Area dati vuota\r\n"
		return;
	}
	PrintMessage(strings[S_Writing]);	//"Inizio scrittura...\r\n"
	hvreg=0;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=20000>>8;
	bufferU[j++]=20000&0xff;
	bufferU[j++]=VREG_DIS;		//Disable HV reg
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x0;
	bufferU[j++]=SPI_INIT;
	bufferU[j++]=1;
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=3;
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=EXT_PORT;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=EN_VPP_VCC;	//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=WAIT_T3;		//20ms
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(i=0;i<32;i++){
		j=1;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=RST;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=4;
		bufferU[j++]=WAIT_T3;		//20ms
		bufferU[j++]=SPI_WRITE;		//Programming enable
		bufferU[j++]=2;
		bufferU[j++]=0xAC;
		bufferU[j++]=0x53;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=2;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(25);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		//str.Format("i=%d z=%d   rx:%02X%02X\r\n",i,z,bufferI[z+2],bufferI[z+3]);
		//PrintMessage(str);
		if(bufferI[z+2]==0x53) i=32;
	}
	if(i<33){
		j=1;
		bufferU[j++]=EN_VPP_VCC;	//VDD
		bufferU[j++]=0x0;
		bufferU[j++]=SPI_INIT;
		bufferU[j++]=0xFF;
		bufferU[j++]=CLOCK_GEN;
		bufferU[j++]=0xFF;
		bufferU[j++]=EXT_PORT;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(2);
		read();
		if(saveLog)WriteLogIO();
		PrintMessage(strings[S_SyncErr]);	//"Errore di sincronizzazione\r\n"
		if(saveLog) CloseLogFile();
		return;
	}
	j=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=1;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=SPI_WRITE;		//Read signature bytes
	bufferU[j++]=3;
	bufferU[j++]=0x30;
	bufferU[j++]=0;
	bufferU[j++]=2;
	bufferU[j++]=SPI_READ;
	bufferU[j++]=1;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[0]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[1]=bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
	signature[2]=bufferI[z+2];
	str.Format("CHIP ID:%02X%02X%02X\r\n",signature[0],signature[1],signature[2]);
	PrintMessage(str);
	AtmelID(signature);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SPI_WRITE;		//Chip erase
	bufferU[j++]=4;
	bufferU[j++]=0xAC;
	bufferU[j++]=0x80;
	bufferU[j++]=0;
	bufferU[j++]=0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=5000>>8;
	bufferU[j++]=5000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	int w,v,c,Rtry;
	for(i=0;i<dim;i+=page*2){
		for(z=i,v=0;z<i+page*2;z++) if(memCODE[z]<0xFF)v=1;
		if(v){
			for(k=0,j=1,v=0;k<page;k+=w){
				w=(page-k)<(DIMBUF-6)/2?(page-k):(DIMBUF-6)/2;
				bufferU[j++]=AT_LOAD_DATA;
				bufferU[j++]=w;
				bufferU[j++]=k>>8;
				bufferU[j++]=k;
				for(z=0;z<w*2;z++)	bufferU[j++]=memCODE[i+k*2+z];
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				j=1;
				write();
				msDelay(15);
				read();
				if(saveLog)WriteLogIO();
			}
			bufferU[j++]=SPI_WRITE;		//Write program memory page
			bufferU[j++]=4;
			bufferU[j++]=0x4C;
			bufferU[j++]=i>>9;
			bufferU[j++]=i>>1;
			bufferU[j++]=0;
			bufferU[j++]=WAIT_T3;		//5ms
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			j=1;
			write();
			msDelay(10);
			read();
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Scrittura: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			if(saveLog)WriteLogIO();
			c=(DIMBUF-5)/2;
			for(k=0,j=1;k<page;k+=c){
				for(Rtry=0;Rtry<5;Rtry++){		//Try to read a few times
					bufferU[j++]=AT_READ_DATA;
					bufferU[j++]=k<(page-c)?c:page-k;
					bufferU[j++]=(i+k*2)>>9;
					bufferU[j++]=(i+k*2)>>1;
					bufferU[j++]=FLUSH;
					for(;j<DIMBUF;j++) bufferU[j]=0x0;
					write();
					msDelay(25);
					read();
					if(saveLog)WriteLogIO();
					if(bufferI[1]==AT_READ_DATA){
						for(w=0,z=3;z<bufferI[2]*2+3&&z<DIMBUF;z++){
							if(memCODE[i+k*2+w]!=bufferI[z]){
								if(Rtry<4)	z=DIMBUF;
								else errori++;
							}
							w++;
						}
						if(z<DIMBUF) Rtry=100;
					}
					j=1;
				}
			}
			if(saveLog){
				str.Format(strings[S_Log8],i,i,w,w,errori);	//"i=%d, k=%d, errori=%d\n"
				WriteLog(str);
				WriteLog("\n");
			}
			if(max_errori&&errori>max_errori){
				str.Format(strings[S_MaxErr],errori);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
				PrintMessage(str);
				PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
				i=dim;
			}
		}
	}
	str.Format(strings[S_ComplErr],errori);	//"completata, %d errori\r\n"
	PrintMessage(str);
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Scrittura area EEPROM ... "
		for(i=0,j=1;i<dim2;i++){
			if(memEE[i]!=0xFF){
				bufferU[j++]=SPI_WRITE;		//Write EEPROM memory
				bufferU[j++]=4;
				bufferU[j++]=0xC0;
				bufferU[j++]=i>>8;
				bufferU[j++]=i;
				bufferU[j++]=memEE[i];
				bufferU[j++]=WAIT_T3;		//5ms
				bufferU[j++]=WAIT_T3;		//5ms
				bufferU[j++]=SPI_WRITE;		//Read EEPROM memory
				bufferU[j++]=3;
				bufferU[j++]=0xA0;
				bufferU[j++]=i>>8;
				bufferU[j++]=i;
				bufferU[j++]=SPI_READ;
				bufferU[j++]=1;
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				j=1;
				write();
				msDelay(11);
				read();
				str.Format(strings[S_CodeWriting],i*100/dim2,i);	//"Scrittura: %d%%, ind. %03X"
				StatusBar.SetWindowText(str);
				for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
				if(z==DIMBUF-2||memEE[i]!=bufferI[z+2]){
					if(ritenta<4){
						ritenta++;
						if (ritenta>maxtent) maxtent=ritenta;
						i--;
					}
					else{
						erroriEE++;
						ritenta=0;
					}
				}
				if(max_errori&&errori+erroriEE>max_errori){
					str.Format(strings[S_MaxErr],errori+erroriEE);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
					PrintMessage(str);
					PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
					i=dim2;
				}
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,erroriEE);	//"i=%d, k=%d, errori=%d\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		str.Format(strings[S_ComplErr],erroriEE);	//"completata, %d errori\r\n"
		PrintMessage(str);
		errori+=erroriEE;
	}
//****************** write FUSE ********************
	int err_f=0;
	if(lock<0x100||fuse<0x100||fuse_h<0x100||fuse_x<0x100)PrintMessage(strings[S_FuseAreaW]);	//"Scrittura area Fuse ... "
	if(lock<0x100){
		bufferU[j++]=SPI_WRITE;		//Write lock
		bufferU[j++]=4;
		bufferU[j++]=0xAC;
		bufferU[j++]=0xE0;
		bufferU[j++]=0;
		bufferU[j++]=lock;
		bufferU[j++]=WAIT_T3;		//9ms
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x58;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		j=1;
		write();
		msDelay(9);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		if(z==DIMBUF-2||lock!=bufferI[z+2]) err_f++;
	}
	if(fuse<0x100){
		bufferU[j++]=SPI_WRITE;		//Write fuse
		bufferU[j++]=4;
		bufferU[j++]=0xAC;
		bufferU[j++]=0xA0;
		bufferU[j++]=0;
		bufferU[j++]=fuse;
		bufferU[j++]=WAIT_T3;		//9ms
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x50;
		bufferU[j++]=0;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		j=1;
		write();
		msDelay(9);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		if(z==DIMBUF-2||fuse!=bufferI[z+2]) err_f++;
	}
	if(fuse_h<0x100){
		bufferU[j++]=SPI_WRITE;		//Write fuse_h
		bufferU[j++]=4;
		bufferU[j++]=0xAC;
		bufferU[j++]=0xA8;
		bufferU[j++]=0;
		bufferU[j++]=fuse_h;
		bufferU[j++]=WAIT_T3;		//9ms
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x58;
		bufferU[j++]=8;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		j=1;
		write();
		msDelay(9);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		if(z==DIMBUF-2||fuse_h!=bufferI[z+2]) err_f++;
	}
	if(fuse_x<0x100){
		bufferU[j++]=SPI_WRITE;		//Write ext fuse
		bufferU[j++]=4;
		bufferU[j++]=0xAC;
		bufferU[j++]=0xA4;
		bufferU[j++]=0;
		bufferU[j++]=fuse;
		bufferU[j++]=WAIT_T3;		//9ms
		bufferU[j++]=SPI_WRITE;
		bufferU[j++]=3;
		bufferU[j++]=0x50;
		bufferU[j++]=8;
		bufferU[j++]=0;
		bufferU[j++]=SPI_READ;
		bufferU[j++]=1;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		j=1;
		write();
		msDelay(9);
		read();
		if(saveLog)WriteLogIO();
		for(z=1;z<DIMBUF-2&&bufferI[z]!=SPI_READ;z++);
		if(z==DIMBUF-2||fuse_x!=bufferI[z+2]) err_f++;
	}
	errori+=err_f;
	if(lock<0x100||fuse<0x100||fuse_h<0x100||fuse_x<0x100){
		str.Format(strings[S_ComplErr],err_f);	//"completata, %d errori\r\n"
		PrintMessage(str);
	}
/*	if (maxtent){
		str.Format(strings[S_MaxRetry],maxtent); 	//"Max tentativi di scrittura: %d\r\n"
		PrintMessage(str);
	}*/
//****************** exit program mode ********************
	bufferU[j++]=CLOCK_GEN;
	bufferU[j++]=0xFF;
	bufferU[j++]=SPI_WRITE;
	bufferU[j++]=1;
	bufferU[j++]=0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,errori,errori!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nFine (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

