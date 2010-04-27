/*
 * progP16.cpp - algorithms to program the PIC16 (14 bit word) family of microcontrollers
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


void COpenProgDlg::Read16Fxxx(int dim,int dim2,int dim3,int vdd){
// read 14 bit PIC
// dim=program size 	dim2=eeprom size   dim3=config size
// dim2<0 -> eeprom @ 0x2200
// vdd=0 -> vpp before vdd
// vdd=1 -> vdd (+50ms) before vpp
// vdd=2 -> vdd before vpp
// DevID@0x2006
// Config@0x2007
// Calib1/Config2@0x2008
// Calib2/Calib1@0x2009
// eeprom@0x2100
	int k=0,k2=0,z=0,i,j,ee2200=0;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim2<0){
		dim2=-dim2;
		ee2200=1;
	}
	if(dim>0x2000||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Code size exceeds limits\r\n"
		return;
	}
	if(dim2>0x400||dim2<0){		//Max 1K
		PrintMessage(strings[S_EELim]);	//"EEPROM size exceeds limits\r\n"
		return;
	}
	if(dim3>0x100||dim3<0){
		PrintMessage(strings[S_ConfigLim]);	//"Config area size exceeds limits\r\n"
		return;
	}
	if(dim3<8)dim3=8;
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Read16Fxxx(%d,%d,%d,%d)\n",dim,dim2,dim3,vdd);
		WriteLog(str);
	}
	dati_hex.RemoveAll();
	dati_hex.SetSize(0x2100+dim2);
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	if(vdd==0){						//VPP before VDD
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=4;				//VPP
		bufferU[j++]=NOP;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x5;			//VDD+VPP
	}
	else if(vdd==1){				//VDD before VPP with delay 50ms
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=1;				//VDD
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=25000>>8;
		bufferU[j++]=25000&0xff;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=2000>>8;
		bufferU[j++]=2000&0xff;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x5;			//VDD+VPP
	}
	else if(vdd==2){				//VDD before VPP without delay
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=1;				//VDD
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x5;			//VDD+VPP
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	if(vdd) msDelay(50);
	read();
	if(saveLog)WriteLogIO();
//****************** read code ********************
	PrintMessage(strings[S_CodeReading1]);		//read code ...
	for(i=0,j=1;i<dim;i++){
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==dim-1){		//2B cmd -> 4B data
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(5);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==READ_DATA_PROG){
					dati_hex[k++]=(bufferI[z+1]<<8)+bufferI[z+2];
					z+=2;
				}
			}
			str.Format(strings[S_CodeReading],i*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	if(k!=dim){
		str.Format(strings[S_ReadCodeErr],dim,k);	//"Errore in lettura area programma, richieste %d word, lette %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
	for(i=k;i<0x2000;i++) dati_hex[i]=0x3fff;
//****************** read config area ********************
	PrintMessage(strings[S_Read_CONFIG_A]);		//lettura config ...
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	for(i=0x2000;i<0x2000+dim3;i++){		//Config
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==0x2000+dim3-1){		//2B cmd -> 4B data
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(5);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==READ_DATA_PROG){
					dati_hex[0x2000+k2++]=(bufferI[z+1]<<8)+bufferI[z+2];
					z+=2;
				}
			}
			str.Format(strings[S_CodeReading],(i-0x2000+dim)*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,k2,k2);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	if(k2!=dim3){
		str.Format(strings[S_ConfigErr],dim3,k2);	//"Errore in lettura area configurazione, richieste %d word, lette %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
	for(i=0x2000+k2;i<0x2000+dim3;i++) dati_hex[i]=0x3fff;
//****************** read eeprom ********************
	if(dim2){
		PrintMessage(strings[S_ReadEE]);		//lettura EE ...
		if(ee2200){		//eeprom a 0x2200
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=0xFF;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x100-dim3;
		for(k2=0,i=0x2100;i<0x2100+dim2;i++){
			bufferU[j++]=READ_DATA_DATA;
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF*2/4-2||i==0x2100+dim2-1){		//2B cmd -> 4B data
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(10);
				read();
				for(z=1;z<DIMBUF-1;z++){
					if(bufferI[z]==READ_DATA_DATA){
						dati_hex[0x2100+k2++]=bufferI[z+1];
						z++;
					}
				}
				str.Format(strings[S_CodeReading],(i-0x2100+dim)*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
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
			str.Format(strings[S_ReadEEErr],dim2,k2);	//"Errore in lettura area EE, ..."
			PrintMessage(str);
			for(i=0x2100+k2;i<0x2100+dim2;i++) dati_hex[i]=0x3fff;
		}
		else PrintMessage(strings[S_Compl]);
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=1;					//VDD
	bufferU[j++]=EN_VPP_VCC;
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
	for(i=0;i<4;i+=2){
		str.Format("ID%d: 0x%04X\tID%d: 0x%04X\r\n",i, dati_hex[0x2000+i],i+1, dati_hex[0x2000+i+1]);
		PrintMessage(str);
	}
	str.Format(strings[S_DevID],dati_hex[0x2006]);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(dati_hex[0x2006]);
	str.Format(strings[S_ConfigWord],dati_hex[0x2007]);	//"Configuration word: 0x%04X\r\n"
	PrintMessage(str);
	if(dim3>8){
		str.Format(strings[S_Config2Cal1],dati_hex[0x2008]);	//"Config 2 or Cal1: 0x%04X\r\n"
		PrintMessage(str);
	}
	if(dim3>9){
		str.Format(strings[S_Calib1_2],dati_hex[0x2009]);	//"Calibration word 1 or 2: 0x%04X\r\n"
		PrintMessage(str);
	}
	PrintMessage(strings[S_CodeMem2]);	//"\r\nMemoria programma:\r\n"
	CString aux;
	for(i=0;i<dim;i+=COL){
		int valid=0;
		for(j=i;j<i+COL&&j<dim;j++){
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
	if(!aux.IsEmpty()) PrintMessage(aux);
	else PrintMessage(strings[S_Empty]);	//empty
	aux.Empty();
	if(dim3>8){
		PrintMessage(strings[S_ConfigResMem]);	//"\r\nMemoria configurazione e riservata:\r\n"
		for(i=0x2000;i<0x2000+dim3;i+=COL){
			int valid=0;
			for(j=i;j<i+COL&&j<0x2000+dim3;j++){
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
		if(aux.GetLength()) PrintMessage(aux);
		else PrintMessage(strings[S_Empty]);	//empty
	}
	aux.Empty();
	if(dim2){
		str.Empty();
		PrintMessage(strings[S_EEMem]);	//"\r\nmemoria EEPROM:\r\n"
		for(i=0x2100;i<0x2100+dim2;i+=COL){
			int valid=0;
			for(j=i;j<i+COL&&j<0x2100+dim2;j++){
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
		if(aux.GetLength()) PrintMessage(aux);
		else PrintMessage(strings[S_Empty]);	//empty
	}
	str.Format(strings[S_End],(stop-start)/1000.0);	//"\r\nFine (%.2f s)\r\n"
	PrintMessage(str);
	StatusBar.SetWindowText("");
	if(saveLog) CloseLogFile();
}

void COpenProgDlg::Read16F1xxx(int dim,int dim2,int dim3,int options){
// read 14 bit enhanced PIC
// dim=program size 	dim2=eeprom size   dim3=config size
// options:
//		bit0=0 -> vpp before vdd
//		bit0=1 -> vdd before vpp
//		bit1=1 -> LVP programming
// DevID@0x8006
// Config1@0x8007
// Config2@0x8008
// Calib1@0x8009
// Calib2@0x800A
// eeprom@0x0
	int k=0,k2=0,z=0,i,j;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(!CheckV33Regulator()){ 
		PrintMessage(strings[S_noV33reg]);	//Can't find 3.3V expnsion board
		return;
	}
	if(dim>0x8000||dim<0){
		PrintMessage(strings[S_CodeLim]);	//"Code size exceeds limits\r\n"
		return;
	}
	if(dim2>0x400||dim2<0){		//Max 1K
		PrintMessage(strings[S_EELim]);	//"EEPROM size exceeds limits\r\n"
		return;
	}
	if(dim3>0x200||dim3<0){
		PrintMessage(strings[S_ConfigLim]);	//"Config area size exceeds limits\r\n"
		return;
	}
	if(dim3<11)dim3=11;		//at least config1-2 + calib1-2
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Log.txt"
		str.Format("Read16F1xxx(%d,%d,%d,%d)\n",dim,dim2,dim3,options);
		WriteLog(str);
	}
	if((options&2)==0){				//HV entry
		if(!StartHVReg(8.5)){
			PrintMessage(strings[S_HVregErr]); //"HV regulator error\r\n"
			return;
		}
	}
	else StartHVReg(-1);			//LVP mode, turn off HV
	dati_hex.RemoveAll();
	dati_hex.SetSize(0x8000+dim3);
	memEE.SetSize(dim2);
	for(i=0;i<memEE.GetSize();i++) memEE[i]=0xFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	if((options&2)==0){				//HV entry
		if((options&1)==0){				//VPP before VDD
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=4;				//VPP
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=0x5;			//VDD+VPP
		}
		else{							//VDD before VPP without delay
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=1;				//VDD
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=0x5;			//VDD+VPP
		}
	}
	else{			//Low voltage programming
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=4;				//VPP
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=TX16;			//0000 1010 0001 0010 1100 0010 1011 0010 = 0A12C2B2
		bufferU[j++]=2;
		bufferU[j++]=0x0A;
		bufferU[j++]=0x12;
		bufferU[j++]=0xC2;
		bufferU[j++]=0xB2;
		bufferU[j++]=SET_CK_D;		//Clock pulse
		bufferU[j++]=0x4;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)WriteLogIO();
//****************** read code ********************
	PrintMessage(strings[S_CodeReading1]);		//read code ...
	for(i=0,j=1;i<dim;i++){
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==dim-1){		//2B cmd -> 4B data
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(5);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==READ_DATA_PROG){
					dati_hex[k++]=(bufferI[z+1]<<8)+bufferI[z+2];
					z+=2;
				}
			}
			str.Format(strings[S_CodeReading],i*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	if(k!=dim){
		str.Format(strings[S_ReadCodeErr],dim,k);	//"Errore in lettura area programma, richieste %d word, lette %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
	for(i=k;i<0x8000;i++) dati_hex[i]=0x3fff;
//****************** read config area ********************
	PrintMessage(strings[S_Read_CONFIG_A]);		//lettura config ...
	bufferU[j++]=LOAD_CONF;			//counter at 0x8000
	bufferU[j++]=0xFF;
	bufferU[j++]=0xFF;
	for(i=0x8000;i<0x8000+dim3;i++){		//Config
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==0x8000+dim3-1){		//2B cmd -> 4B data
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(5);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==READ_DATA_PROG){
					dati_hex[0x8000+k2++]=(bufferI[z+1]<<8)+bufferI[z+2];
					z+=2;
				}
			}
			str.Format(strings[S_CodeReading],(i-0x8000+dim)*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,k2,k2);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	if(k2!=dim3){
		str.Format(strings[S_ConfigErr],dim3,k2);	//"Errore in lettura area configurazione, richieste %d word, lette %d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
	for(i=0x8000+k2;i<0x8000+dim3;i++) dati_hex[i]=0x3fff;
//****************** read eeprom ********************
	if(dim2){
		PrintMessage(strings[S_ReadEE]);		//lettura EE ...
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x16;		//Reset address
		for(i=k=0;i<dim2;i++){
			bufferU[j++]=READ_DATA_DATA;
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF*2/4-2||i==dim2-1){		//2B cmd -> 4B data
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(5);
				read();
				for(z=1;z<DIMBUF-1;z++){
					if(bufferI[z]==READ_DATA_DATA){
						memEE[k++]=bufferI[z+1];
						z++;
					}
				}
				str.Format(strings[S_CodeReading],i*100/(dim+dim2+dim3),i);	//"Read: %d%%, ind. %03X"
				StatusBar.SetWindowText(str);
				j=1;
				if(saveLog){
					str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		if(i!=dim2){
			str.Format(strings[S_ReadEEErr],dim2,i);	//"Errore in lettura area EE, ..."
			PrintMessage(str);
			for(;i<dim2;i++) memEE[i]=0xff;
		}
		else PrintMessage(strings[S_Compl]);
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=1;					//VDD
	bufferU[j++]=EN_VPP_VCC;
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
	for(i=0;i<4;i+=2){
		str.Format("ID%d: 0x%04X\tID%d: 0x%04X\r\n",i, dati_hex[0x8000+i],i+1, dati_hex[0x8000+i+1]);
		PrintMessage(str);
	}
	str.Format(strings[S_DevID],dati_hex[0x8006]);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(dati_hex[0x8006]);
	str.Format(strings[S_ConfigWordX],1,dati_hex[0x8007]);	//"Configuration word %d: 0x%04X\r\n"
	PrintMessage(str);
	str.Format(strings[S_ConfigWordX],2,dati_hex[0x8008]);	//"Configuration word %d: 0x%04X\r\n"
	PrintMessage(str);
	str.Format(strings[S_CalibWordX],1,dati_hex[0x8009]);	//"Calibration word %d: 0x%04X\r\n"
	PrintMessage(str);
	str.Format(strings[S_CalibWordX],2,dati_hex[0x800A]);	//"Calibration word %d: 0x%04X\r\n"
	PrintMessage(str);
	PrintMessage(strings[S_CodeMem2]);	//"\r\nMemoria programma:\r\n"
	CString aux;
	for(i=0;i<dim;i+=COL){
		int valid=0;
		for(j=i;j<i+COL&&j<dim;j++){
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
	if(aux.GetLength()) PrintMessage(aux);
	else PrintMessage(strings[S_Empty]);	//empty
	aux.Empty();
	if(dim3>11){
		PrintMessage(strings[S_ConfigResMem]);	//"\r\nMemoria configurazione e riservata:\r\n"
		for(i=0x8000;i<0x8000+dim3;i+=COL){
			int valid=0;
			for(j=i;j<i+COL&&j<0x8000+dim3;j++){
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
		if(aux.GetLength()) PrintMessage(aux);
		else PrintMessage(strings[S_Empty]);	//empty
		aux.Empty();
	}
	if(dim2) DisplayEE();	//visualize
	str.Format(strings[S_End],(stop-start)/1000.0);	//"\r\nFine (%.2f s)\r\n"
	PrintMessage(str);
	StatusBar.SetWindowText("");
	if(saveLog) CloseLogFile();
}

void COpenProgDlg::Write12F6xx(int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// vpp before vdd
// DevID@0x2006
// Config@0x2007
// Calib1@0x2008 (save)
// Calib2@0x2009 (save)
// eeprom@0x2100
// erase: BULK_ERASE_PROG (1001) +10ms
// write:LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 4ms
// eeprom:	BULK_ERASE_DATA (1011) + 16ms
//			LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 8ms
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID=0x3fff,calib1=0x3fff,calib2=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	if(b->GetCheck()){
		if(dati_hex.GetSize()>0x2009) load_calibword=2;
		else if(dati_hex.GetSize()>0x2008) load_calibword=1;
		else PrintMessage(strings[S_NoCalibW]);	//"Can't find calibration data\r\n"
	}
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write12F6xx(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
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
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib1
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib2
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib1=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib1<0x3fff){
		str.Format(strings[S_CalibWord1],calib1);	//"Calib1: 0x%04X\r\n"
		PrintMessage(str);
	}
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib2=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib2<0x3fff){
		str.Format(strings[S_CalibWord2],calib2);	//"Calib2: 0x%04X\r\n"
		PrintMessage(str);
	}
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(programID||load_calibword){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		if(load_calibword){
			bufferU[j++]=INC_ADDR_N;
			if(load_calibword==2) bufferU[j++]=0x09;
			else bufferU[j++]=0x08;
		}
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	if(dim2){
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	}
	bufferU[j++]=EN_VPP_VCC;		//exit program mode
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms tra exit e rientro program mode
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=4000>>8;
	bufferU[j++]=4000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 4ms
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-12||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=8000>>8;
		bufferU[j++]=8000&0xff;
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=WAIT_T3;			// ritardo T3=8ms
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=INC_ADDR;
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2100-0x2001;		//azzera il contatore EEPROM
		for(w=2,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG;			//internally timed, T=6ms min
				bufferU[j++]=WAIT_T3;				//Tprogram 8ms
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-12||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*9+2);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err+=i-k;
		str.Format(strings[S_ComplErr],i-k);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=4000>>8;
	bufferU[j++]=4000&0xff;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 4ms
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
	bufferU[j++]=WAIT_T3;				//Tprogram 4ms
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(load_calibword){
		bufferU[j++]=LOAD_DATA_PROG;			//Calib word 1
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 4ms
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		bufferU[j++]=LOAD_DATA_PROG;			//Calib word 2
		bufferU[j++]=dati_hex[0x2009]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2009]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 4ms
		bufferU[j++]=READ_DATA_PROG;
	}
	else bufferU[j++]=INC_ADDR;			//0x2009
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(load_calibword){
		for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2008]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_Calib1Err],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Errore in scrittura Calib1: scritto %04X, letto %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		if(load_calibword==2){
			for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
			if (dati_hex[0x2009]!=(bufferI[z+1]<<8)+bufferI[z+2]){
				str.Format(strings[S_Calib2Err],dati_hex[0x2009],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Errore in scrittura Calib2: scritto %04X, letto %04X\r\n"
				PrintMessage(str);
				err_c++;
			}
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F8x (int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// vdd + 50ms + vdd&vpp
// DevID@0x2006
// Config@0x2007
// eeprom@0x2100
// erase if protected:
// LOAD_CONF (0)(0x3FFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 10ms
// + CUST_CMD (0001) + CUST_CMD (0111)
// erase if not protected and DevID=16F84A:
// LOAD_DATA_PROG (0010)(0x3FFF) + BULK_ERASE_PROG (1001) +10ms
// LOAD_DATA_DATA (0011)(0xFF) + BULK_ERASE_DATA (1011) + BEGIN_PROG (1000) + 10ms
// erase erase if not protected and DevID!=16F84A:
// LOAD_DATA_PROG (0010)(0x3FFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 10ms + CUST_CMD (0001) + CUST_CMD (0111)
// LOAD_DATA_DATA (0011)(0xFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 10ms + CUST_CMD (0001) + CUST_CMD (0111)
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 20ms o 8ms(16F84A)
// write eeprom: LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 20ms o 8ms(16F84A)
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID,config;
	int k=0,k2=0,z=0,i,j,w,r;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F8x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=25000>>8;
	bufferU[j++]=25000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=WAIT_T3;			//50ms
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(140);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_ConfigWord],config);	//"Config word: 0x%04X\r\n"
	PrintMessage(str);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	if(config<0x3FF0){
		PrintMessage(strings[S_ProtErase]);	//"Il dispositivo ï¿½ protetto, sovrascrivo la protezione.\r\n"
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0x3F;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x07;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
	}
	else if(devID>>5==0x2B){			//16F84A
		bufferU[j++]=LOAD_DATA_PROG;
		bufferU[j++]=0x3f;				//MSB
		bufferU[j++]=0xff;				//LSB
		bufferU[j++]=BULK_ERASE_PROG;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		if(dim2){
			bufferU[j++]=LOAD_DATA_DATA;
			bufferU[j++]=0xff;				//LSB
			bufferU[j++]=BULK_ERASE_DATA;
			bufferU[j++]=BEGIN_PROG;
			bufferU[j++]=WAIT_T3;
		}
	}
	else{								//altri
		bufferU[j++]=LOAD_DATA_PROG;
		bufferU[j++]=0x3f;				//MSB
		bufferU[j++]=0xff;				//LSB
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		if(dim2){
			bufferU[j++]=LOAD_DATA_DATA;	//EEPROM:  errore nelle spec?
			bufferU[j++]=0xff;				//LSB
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x01;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x07;
			bufferU[j++]=BEGIN_PROG;
			bufferU[j++]=WAIT_T3;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x01;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x07;
		}
	}
	if(!programID){					//torna in memoria programma
		bufferU[j++]=NOP;				//exit program mode
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x0;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
		bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
		bufferU[j++]=EN_VPP_VCC;		//VDD
		bufferU[j++]=0x1;
		bufferU[j++]=WAIT_T3;			//50ms
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
		bufferU[j++]=0x5;
	}
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	r=(devID>>5==0x2B)?8000:20000;
	bufferU[j++]=r>>8;
	bufferU[j++]=r&0xff;
	bufferU[j++]=FLUSH;
	r/=1000;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(60);
	if(!programID) msDelay(80);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*r+4);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;			//internally timed
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	if(programID) msDelay(90);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2100-0x2008;		//azzera il contatore EEPROM
		for(w=0,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG;			//internally timed
				bufferU[j++]=WAIT_T3;				//Tprogram
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*r+5);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err+=i-k;
		str.Format(strings[S_ComplErr],i-k);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	PrintMessage(str);
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F62x (int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// vpp before vdd
// DevID@0x2006
// Config@0x2007
// eeprom@0x2200
// erase if protected:
// LOAD_CONF (0000)(0) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 15ms + CUST_CMD (0001) + CUST_CMD (0111)
// erase if not protected:
// LOAD_DATA_PROG (0010)(0x3FFF) + BULK_ERASE_PROG (1001) +5ms
// LOAD_DATA_DATA (0011)(0xFF) + BULK_ERASE_DATA (1011) + BEGIN_PROG (1000) + 5ms
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 8ms
// write ID: LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 16ms
// write CONFIG: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 8ms
// eeprom: LOAD_DATA_DATA (0011) + BEGIN_PROG2 (11000) + 8ms
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID,config;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F62x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=EN_VPP_VCC;		//0
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo exit-enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(12);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_ConfigWord],config);	//"Config word: 0x%04X\r\n"
	PrintMessage(str);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=5000>>8;
	bufferU[j++]=5000&0xff;
	if(config<0x3C00){
		PrintMessage(strings[S_ProtErase]);	//"Il dispositivo è protetto, sovrascrivo la protezione.\r\n"
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0x3F;				//config fasulla	ERRORE spec!!! c'era scritto dati=0!!
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x07;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=BEGIN_PROG;		//Tera+Tprog=5+8 ms
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
	}
	else{
		bufferU[j++]=LOAD_DATA_PROG;
		bufferU[j++]=0x3f;				//MSB
		bufferU[j++]=0xff;				//LSB
		bufferU[j++]=BULK_ERASE_PROG;
		bufferU[j++]=BEGIN_PROG;		//Tera=5ms
		bufferU[j++]=WAIT_T3;
		if(dim2){
			bufferU[j++]=LOAD_DATA_DATA;
			bufferU[j++]=0xff;				//LSB
			bufferU[j++]=BULK_ERASE_DATA;
			bufferU[j++]=BEGIN_PROG;		//Tera=5ms
			bufferU[j++]=WAIT_T3;
		}
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x4;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=8000>>8;
	bufferU[j++]=8000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(60);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//program only, internally timed
			bufferU[j++]=WAIT_T3;				//Tprogram=8ms
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*9+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//erase + prog internally timed, T=8+5 ms
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG2;				//prog only, internally timed, T=8 ms
	bufferU[j++]=WAIT_T3;					//Tprogram 8ms
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	if(programID) msDelay(90);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d\n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2102-0x2008;		//azzera il contatore EEPROM
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2200-0x2102;		//azzera il contatore EEPROM
		for(w=0,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG2;			//internally timed
				bufferU[j++]=WAIT_T3;				//Tprogram=8ms
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*14+1);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err+=i-k;
		str.Format(strings[S_ComplErr],i-k);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write12F62x(int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// vpp before vdd
// salva OSCCAL a dim-1
// CONFIG@0x2007 includes 2  calibration bits
// DevID@0x2006
// eeprom@0x2100
// erase: BULK_ERASE_PROG (1001) +10ms
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 3ms
// eeprom: BULK_ERASE_DATA (1011) + 9ms
// LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 6ms
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0,load_osccal;
	WORD devID,config,osccal;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_LOAD);
	load_osccal=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	load_calibword=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write12F62x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	PrintMessage(strings[S_Writing]);	//"Inizio scrittura...\r\n"
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
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	for(i=0;i<dim-0xff;i+=0xff){
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0xff;
	}
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=dim-1-i;
	bufferU[j++]=READ_DATA_PROG;	// OSCCAL
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	for(z=1;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	osccal=(bufferI[z+1]<<8)+bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+1];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(programID){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=EN_VPP_VCC;		//exit program mode
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms tra exit e rientro program mode
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=3000>>8;
	bufferU[j++]=3000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	if(!load_osccal) dati_hex[dim-1]=osccal;	//backup osccal
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms
			bufferU[j++]=WAIT_T3;				//Tprogram 3ms
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*6.5);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms
			bufferU[j++]=WAIT_T3;				//Tprogram 3ms
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	if(!load_calibword)	dati_hex[0x2007]=(dati_hex[0x2007]&0xfff)+(config&0x3000);
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms
	bufferU[j++]=WAIT_T3;				//Tprogram 3ms
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=6000>>8;
		bufferU[j++]=6000&0xff;
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2100-0x2007;		//azzera il contatore EEPROM
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=WAIT_T3;			// ritardo=12ms
		bufferU[j++]=WAIT_T3;
		for(w=3,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG;			//internally timed, T=6ms
				bufferU[j++]=WAIT_T3;				//Tprogram 6ms
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*7+2);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err+=i-k;
		str.Format(strings[S_ComplErr],i-k);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//exit program mode
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F87x (int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// dim2<0 -> eeprom @ 0x2200
// vdd + (50ms?) + vdd&vpp
// DevID@0x2006
// Config@0x2007
// eeprom@0x2100
// erase if protected:
// LOAD_CONF (0000)(0x3FFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 8ms
// + CUST_CMD (0001) + CUST_CMD (0111)
// erase if not protected:
// LOAD_DATA_PROG (0010)(0x3FFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 8ms + CUST_CMD (0001) + CUST_CMD (0111)
// LOAD_DATA_DATA (0011)(0xFF) + CUST_CMD (0001) + CUST_CMD (0111)
// + BEGIN_PROG (1000) + 8ms + CUST_CMD (0001) + CUST_CMD (0111)
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 4ms
// write eeprom: LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 8ms
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID,config;
	int k=0,k2=0,z=0,i,j,w,ee2200=0;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dim2<0){
		dim2=-dim2;
		ee2200=1;
	}
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F87x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
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
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(60);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_ConfigWord],config);	//"Config word: 0x%04X\r\n"
	PrintMessage(str);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=8000>>8;
	bufferU[j++]=8000&0xff;
	if((config&0x3130)!=0x3130){
		PrintMessage(strings[S_ProtErase]);	//"Il dispositivo è protetto, sovrascrivo la protezione.\r\n"
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0x3F;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x07;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=NOP;				//exit program mode
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x0;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
		bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
		bufferU[j++]=EN_VPP_VCC;		//VDD
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
		bufferU[j++]=0x5;
	}
	else{
		bufferU[j++]=LOAD_DATA_PROG;
		bufferU[j++]=0x3f;				//MSB
		bufferU[j++]=0xff;				//LSB
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x01;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x07;
		if(dim2){
			bufferU[j++]=LOAD_DATA_DATA;	//EEPROM:  errore nelle spec?
			bufferU[j++]=0xff;				//LSB
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x01;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x07;
			bufferU[j++]=BEGIN_PROG;
			bufferU[j++]=WAIT_T3;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x01;
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x07;
		}
	}
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=4000>>8;
	bufferU[j++]=4000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(60);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//internally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=8000>>8;
	bufferU[j++]=8000&0xff;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//internally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG2;			//internally timed
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	if(programID) msDelay(90);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		int err_e=0;
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		if(ee2200){		//eeprom a 0x2200
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=0xFF;
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=1;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2100-0x2007;		//azzera il contatore EEPROM
		for(w=0,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG2;			//internally timed ?????
				bufferU[j++]=WAIT_T3;				//Tprogram         ?????
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*8+5);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err_e++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err_e+=i-k;
		err+=err_e;
		str.Format(strings[S_ComplErr],err_e);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	PrintMessage(str);
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F87xA (int dim,int dim2,int seq)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// seq=0: vdd + (50ms) + vdd&vpp
// seq=1: vdd + (50us) + vdd&vpp
// DevID@0x2006
// Config@0x2007
// write CONFIG2@0x2008 if different from 3FFF
// eeprom@0x2100
// erase:
// CHIP ERASE (11111) + 8ms
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 1.5ms + END_PROGX (10111)
// write eeprom: LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 8ms
// verify during write
{
	int programID=0,max_err,err=0;
	WORD devID,config;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F87xA(%d,%d,%d)\n",dim,dim2,seq);
		WriteLog(str);
	}
	for(i=0;i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=25000>>8;
	bufferU[j++]=25000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(seq==0){
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	if(seq==0) msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_ConfigWord],config);	//"Config word: 0x%04X\r\n"
	PrintMessage(str);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=8000>>8;
	bufferU[j++]=8000&0xff;
	if(programID){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0x3F;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
	}
	bufferU[j++]=CUST_CMD;
	bufferU[j++]=0x1F;					// CHIP_ERASE (11111)
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=1200>>8;
	bufferU[j++]=1200&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(60);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x17;					//END_PROGX (10111)
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-11||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*1.5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-6;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+4]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+5]<<8)+bufferI[z+6]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+5]<<8)+bufferI[z+6]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=7;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x17;					//END_PROGX (10111)
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=8000>>8;
	bufferU[j++]=8000&0xff;
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;				//internally timed
	bufferU[j++]=WAIT_T3;					//Tprogram
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(dati_hex[0x2008]!=0x3fff){
		bufferU[j++]=LOAD_DATA_PROG;			//Config word2 0x2008
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;				//internally timed
		bufferU[j++]=WAIT_T3;					//Tprogram
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=7;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(dati_hex[0x2008]!=0x3fff){
		for(z+=7;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x2008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		int err_e=0;
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0xFF;				//azzera il contatore EEPROM
		bufferU[j++]=INC_ADDR;
		for(w=0,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG;			//internally timed
				bufferU[j++]=WAIT_T3;				//Tprogram
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*9+5);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err_e++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
					}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err_e+=i-k;
		err+=err_e;
		str.Format(strings[S_ComplErr],err_e);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	PrintMessage(str);
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F81x (int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// seq=0: vdd + (50ms) + vdd&vpp
// seq=1: vdd + (50us) + vdd&vpp
// DevID@0x2006
// Config@0x2007
// write CONFIG2@0x2008 if different from 3FFF
// erase if protected: CHIP ERASE (11111) + 8ms
// erase if not protected: 
// BULK_ERASE_PROG (1001) + BEGIN_PROG (1001) + 2ms + END_PROGX (10111)
// BULK_ERASE_DATA (1011) + BEGIN_PROG (1001) + 2ms + END_PROGX (10111)
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 1.5ms + END_PROGX (10111)
// write eeprom: LOAD_DATA_DATA (0011) + BEGIN_PROG2 (11000) + 1.5ms + END_PROGX (10111)
// verify during write
{
	int programID=0,max_err,err=0;
	WORD devID,config;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F81x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=25000>>8;
	bufferU[j++]=25000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Config
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	config=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_ConfigWord],config);	//"Config word: 0x%04X\r\n"
	PrintMessage(str);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	if(programID){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0x3F;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
	}
	if((config&0x2100)!=0x2100){
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x1F;					// CHIP_ERASE (11111)
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=WAIT_T3;
	}
	else{
		bufferU[j++]=BULK_ERASE_PROG;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x17;					//END_PROGX (10111)
	}
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x17;					//END_PROGX (10111)
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-11||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*2.5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-6;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+4]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+5]<<8)+bufferI[z+6]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+5]<<8)+bufferI[z+6]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=7;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write eeprom ********************
	if(dim2){
		int err_e=0;
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=0x0;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
		bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
		bufferU[j++]=EN_VPP_VCC;		//VDD
		bufferU[j++]=0x1;
		bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
		bufferU[j++]=0x5;
		bufferU[j++]=LOAD_DATA_DATA;
		bufferU[j++]=0x01;
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=BEGIN_PROG;
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x17;					//END_PROGX (10111)
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(5);
		read();
		if(saveLog)WriteLogIO();
		j=1;
		for(w=0,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG2;			//externally timed
				bufferU[j++]=WAIT_T3;				//Tprogram
				bufferU[j++]=CUST_CMD;
				bufferU[j++]=0x17;					//END_PROGX (10111)
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-10||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*2.5+5);
				w=0;
				read();
				for(z=1;z<DIMBUF-5;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+4]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+5]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+5]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err_e++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
					}
						}
						k++;
						z+=6;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err_e+=i-k;
		err+=err_e;
		str.Format(strings[S_ComplErr],err_e);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed
			bufferU[j++]=WAIT_T3;				//Tprogram
			bufferU[j++]=CUST_CMD;
			bufferU[j++]=0x17;					//END_PROGX (10111)
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG2;				//externally timed
	bufferU[j++]=WAIT_T3;					//Tprogram
	bufferU[j++]=CUST_CMD;
	bufferU[j++]=0x17;						//END_PROGX (10111)
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(dati_hex[0x2008]!=0x3fff){
		bufferU[j++]=LOAD_DATA_PROG;			//Config word2 0x2008
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG2;				//externally timed
		bufferU[j++]=WAIT_T3;					//Tprogram
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x17;						//END_PROGX (10111)
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=7;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(dati_hex[0x2008]!=0x3fff){
		for(z+=7;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x2008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	PrintMessage(str);
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write12F61x(int dim)
// write 14 bit PIC
// dim=program size
// vpp before vdd
// DevID@0x2006
// Config@0x2007
// Calib1@0x2008 (save)
// erase: BULK_ERASE_PROG (1001) +10ms
// write: LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 4ms + END_PROG (1010)
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID=0x3fff,calib1=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	if(b->GetCheck()){
		if(dati_hex.GetSize()>0x2008) load_calibword=1;
		else PrintMessage(strings[S_NoCalibW]);	//"Can't find calibration data\r\n"
	}
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write12F61x(%d)\n",dim);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
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
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib1
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib2
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib1=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib1<0x3fff){
		str.Format(strings[S_CalibWord1],calib1);	//"Calib1: 0x%04X\r\n"
		PrintMessage(str);
	}
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(programID||load_calibword){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		if(load_calibword){
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=0x08;
		}
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=EN_VPP_VCC;		//exit program mode
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms tra exit e rientro program mode
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=4000>>8;
	bufferU[j++]=4000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 4ms
			bufferU[j++]=END_PROG;
			bufferU[j++]=WAIT_T2;				//Tdischarge 100us
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-12||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-7;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+5]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+6]<<8)+bufferI[z+7]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=8;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 4ms
			bufferU[j++]=END_PROG;
			bufferU[j++]=WAIT_T2;				//Tdischarge 100us
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG2;			//externally timed, T=3ms min
	bufferU[j++]=WAIT_T3;				//Tprogram 4ms
	bufferU[j++]=END_PROG;
	bufferU[j++]=WAIT_T2;				//Tdischarge 100us
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(load_calibword){
		bufferU[j++]=LOAD_DATA_PROG;			//Calib word 1
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG2;			//externally timed, T=3ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 4ms
		bufferU[j++]=END_PROG;
		bufferU[j++]=WAIT_T2;				//Tdischarge 100us
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=8;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(load_calibword){
		for(z+=8;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(dati_hex[0x2008]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_Calib1Err],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Errore in scrittura Calib1: scritto %04X, letto %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F88x(int dim,int dim2)
// write 14 bit PIC
// dim=program size 	dim2=eeprom size
// vpp before vdd
// DevID@0x2006
// Config@0x2007
// Config2@0x2008
// Calib1@0x2009 (salva)
// eeprom@0x2100
// erase: BULK_ERASE_PROG (1001) +6ms
// write:LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 3ms
// eeprom:	BULK_ERASE_DATA (1011) + 6ms
//			LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 6ms
// verify during write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID=0x3fff,calib1=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2009){
		PrintMessage(strings[S_NoConfigW4]);	//"Impossibile trovare la locazione CONFIG (0x2008)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	if(b->GetCheck()){
		if(dati_hex.GetSize()>0x200A) load_calibword=1;
		else PrintMessage(strings[S_NoCalibW]);	//"Can't find calibration data\r\n"
	}
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(dim2){
		if(dati_hex.GetSize()<0x2100){
			dim2=0;
			PrintMessage(strings[S_NoEEMem]);	//"Can't find EEPROM data\r\n"
		}
		else if(dim2>dati_hex.GetSize()-0x2100) dim2=dati_hex.GetSize()-0x2100;
	}
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F88x(%d,%d)\n",dim,dim2);
		WriteLog(str);
	}
	for(i=0;i<0x200A&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
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
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib1
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=6000>>8;
	bufferU[j++]=6000&0xff;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib1=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib1<0x3fff){
		str.Format(strings[S_CalibWord1],calib1);	//"Calib1: 0x%04X\r\n"
		PrintMessage(str);
	}
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VPP
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(programID||load_calibword){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
		if(load_calibword){
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=9;
		}
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=6ms
	bufferU[j++]=EN_VPP_VCC;		//exit program mode
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			// ritardo T3=6ms tra exit e rientro program mode
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x4;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=3000>>8;
	bufferU[j++]=3000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(40);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 3ms
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-12||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*3+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-5;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+3]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+4]<<8)+bufferI[z+5]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+4]<<8)+bufferI[z+5]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=6;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 3ms
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;				//internally timed, T=3ms min
	bufferU[j++]=WAIT_T3;					//Tprogram 3ms
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=LOAD_DATA_PROG;			//Config word2 0x2008
	bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;				//internally timed, T=3ms min
	bufferU[j++]=WAIT_T3;					//Tprogram 3ms
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(load_calibword){
		bufferU[j++]=LOAD_DATA_PROG;		//Calib word 1
		bufferU[j++]=dati_hex[0x2009]>>8;	//MSB
		bufferU[j++]=dati_hex[0x2009]&0xff;	//LSB
		bufferU[j++]=BEGIN_PROG;			//internally timed, T=3ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 3ms
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(35);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(load_calibword){
		for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2009]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_Calib1Err],dati_hex[0x2009],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Errore in scrittura Calib1: scritto %04X, letto %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** write eeprom ********************
	if(dim2){
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=6000>>8;
		bufferU[j++]=6000&0xff;
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=WAIT_T3;			// ritardo T3=6ms
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x2100-0x2009;		//azzera il contatore EEPROM
		for(w=2,i=k=0x2100;i<0x2100+dim2;i++){
			if(dati_hex[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=dati_hex[i]&0xff;
				bufferU[j++]=BEGIN_PROG;			//internally timed, T=6ms min
				bufferU[j++]=WAIT_T3;				//Tprogram 6ms
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-12||i==0x2100+dim2-1){
				str.Format(strings[S_CodeWriting],(i-0x2100+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*6.5+2);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (dati_hex[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,dati_hex[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							err++;
							if(max_err&&err>max_err){
								str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
								i=0x2200;
								z=DIMBUF;
					}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d \n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		err+=i-k;
		str.Format(strings[S_ComplErr],i-k);	//"completed, %d errors\r\n"
		PrintMessage(str);
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F7x(int dim,int vdd)
// dim=program size
// write 14 bit PIC
// vdd=0  vdd +50ms before vpp
// vdd=1  vdd before vpp
// DevID@0x2006
// Config@0x2007
// Config2@0x2008
// erase: BULK_ERASE_PROG (1001) +30ms
// write:LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 1ms + END_PROG2(1110)
// verify during write
{
	int programID=0,max_err,err=0;
	WORD devID=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F7x(%d,%d)\n",dim,vdd);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(vdd==0){
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(20);
	if(vdd==0) msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	//enter program mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(vdd==0){
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=1000>>8;
	bufferU[j++]=1000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(90);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//externally timed, T=1ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 1ms min
			bufferU[j++]=END_PROG2;
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*1.5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-6;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+4]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+5]<<8)+bufferI[z+6]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+5]<<8)+bufferI[z+6]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=7;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;			//externally timed, T=1ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 1ms
			bufferU[j++]=END_PROG2;
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG;			//externally timed, T=1ms min
	bufferU[j++]=WAIT_T3;				//Tprogram 1ms
	bufferU[j++]=END_PROG2;
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(dati_hex[0x2008]<0x3fff){
		bufferU[j++]=LOAD_DATA_PROG;			//Config word 2 0x2008
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;			//externally timed, T=1ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 1ms
		bufferU[j++]=END_PROG2;
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(dati_hex[0x2008]<0x3fff){
		for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x2008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F71x(int dim,int vdd)
// write 14 bit PIC
// dim=program size
// vdd=0  vdd +50ms before vpp
// vdd=1  vdd before vpp
// DevID@0x2006
// Config@0x2007
// erase: BULK_ERASE_PROG (1001) +6ms
// write:LOAD_DATA_PROG (0010) + BEGIN_PROG2 (11000) + 2ms + END_PROG2(1110)
// verify during write
{
	int programID=0,max_err,err=0;
	WORD devID=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(dati_hex.GetSize()<0x2007){
		PrintMessage(strings[S_NoConfigW3]);	//"Impossibile trovare la locazione CONFIG (0x2007)\r\nFine\r\n"
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write16F71x(%d,%d)\n",dim,vdd);
		WriteLog(str);
	}
	for(i=0;i<0x2009&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=10000>>8;
	bufferU[j++]=10000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//enter program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(vdd==0){
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(20);
	if(vdd==0) msDelay(50);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	//enter program mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(vdd==0){
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(programID){
		bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
		bufferU[j++]=0xFF;				//config fasulla
		bufferU[j++]=0xFF;				//config fasulla
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//ritardo tra exit e enter prog mode
	//enter program mode
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	if(vdd==0){
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
		bufferU[j++]=WAIT_T3;			//ritardo tra vdd e vpp
	}
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(30);
	if(vdd==0) msDelay(100);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	for(w=i=k=0,j=1;i<dim;i++){
		if(dati_hex[i]<0x3fff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed, T=1ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 1ms min
			bufferU[j++]=END_PROG2;
			bufferU[j++]=WAIT_T2;				//Tdischarge 100us
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-11||i==dim-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, add. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*2.5+2);
			w=0;
			read();
			for(z=1;z<DIMBUF-7;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0x3fff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+5]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+6]<<8)+bufferI[z+7]){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+6]<<8)+bufferI[z+7]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
						if(max_err&&err>max_err){
							str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim;
							z=DIMBUF;
						}
					}
					k++;
					z+=8;
				}
			}
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	err+=i-k;
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
//****************** write ID, CONFIG, CALIB ********************
	PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
	int err_c=0;
	bufferU[j++]=LOAD_CONF;			//contatore a 0x2000
	bufferU[j++]=0xFF;				//config fasulla
	bufferU[j++]=0xFF;				//config fasulla
	if(programID){
		for(i=0x2000;i<0x2004;i++){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG2;			//externally timed, T=1ms min
			bufferU[j++]=WAIT_T3;				//Tprogram 1ms
			bufferU[j++]=END_PROG2;
			bufferU[j++]=WAIT_T2;				//Tdischarge 100us
			bufferU[j++]=READ_DATA_PROG;
			bufferU[j++]=INC_ADDR;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=3;
	}
	else{
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=7;
	}
	bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x2007
	bufferU[j++]=dati_hex[0x2007]>>8;		//MSB
	bufferU[j++]=dati_hex[0x2007]&0xff;		//LSB
	bufferU[j++]=BEGIN_PROG2;			//externally timed, T=1ms min
	bufferU[j++]=WAIT_T3;				//Tprogram 1ms
	bufferU[j++]=END_PROG2;
	bufferU[j++]=WAIT_T2;				//Tdischarge 100us
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=INC_ADDR;
	if(dati_hex[0x2008]<0x3fff){
		bufferU[j++]=LOAD_DATA_PROG;			//Config word 2 0x2008
		bufferU[j++]=dati_hex[0x2008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x2008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG2;			//externally timed, T=1ms min
		bufferU[j++]=WAIT_T3;				//Tprogram 1ms
		bufferU[j++]=END_PROG2;
		bufferU[j++]=WAIT_T2;				//Tdischarge 100us
		bufferU[j++]=READ_DATA_PROG;
	}
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	for(i=0,z=0;programID&&i<4;i++){
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if (dati_hex[0x2000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
			str.Format(strings[S_IDErr],i,dati_hex[0x2000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		z+=6;
	}
	for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(!dati_hex[0x2007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
		str.Format(strings[S_ConfigWErr3],dati_hex[0x2007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
		PrintMessage(str);
		err_c++;
	}
	if(dati_hex[0x2008]<0x3fff){
		for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x2008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x2008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
	}
	err+=err_c;
	str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(saveLog){
		str.Format(strings[S_Log9],err);	//"Area config. 	errors=%d \n"
		WriteLog(str);
		WriteLogIO();
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

void COpenProgDlg::Write16F1xxx(int dim,int dim2,int options)
// write 14 bit enhanced PIC
// dim=program size 	dim2=eeprom size   dim3=config size
// options:
//		bit0=0 -> vpp before vdd
//		bit0=1 -> vdd before vpp
//		bit1=1 -> LVP programming
// DevID@0x8006
// Config1@0x8007
// Config2@0x8008
// Calib1@0x8009
// Calib2@0x800A
// eeprom@0x0
// erase: BULK_ERASE_PROG (1001) +5ms
// write:LOAD_DATA_PROG (0010) + BEGIN_PROG (1000) + 2.5ms (8 word algorithm)
// eeprom:	BULK_ERASE_DATA (1011) + 5ms
//			LOAD_DATA_DATA (0011) + BEGIN_PROG (1000) + 2.5ms
// verify after write
{
	int programID=0,max_err,err=0,load_calibword=0;
	WORD devID=0x3fff,calib1=0x3fff,calib2=0x3fff;
	int k=0,k2=0,z=0,i,j,w;
	int saveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(MyDeviceDetected==FALSE) return;
	if(!CheckV33Regulator()){ 
		PrintMessage(strings[S_noV33reg]);	//Can't find 3.3V expnsion board
		return;
	}
	if(dati_hex.GetSize()<0x8009){
		PrintMessage(strings[S_NoConfigW4]);	//"Impossibile trovare la locazione CONFIG (0x2007-0x2008)\r\n"
		PrintMessage(strings[S_End]);
		return;
	}
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_CALIB_LOAD);
	if(b->GetCheck()){
		if(dati_hex.GetSize()>0x800A) load_calibword=1;
		else PrintMessage(strings[S_NoCalibW]);	//"Can't find calibration data\r\n"
	}
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	saveLog=b->GetCheck();
	max_err=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Invalid handle\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(saveLog){
		OpenLogFile(strings[S_LogFile]);	//"Log.txt"
		str.Format("Write16F1xxx(%d,%d,%d)\n",dim,dim2,options);
		WriteLog(str);
	}
	if(dim2>memEE.GetSize()) dim2=memEE.GetSize();
	if((options&2)==0){				//HV entry
		if(!StartHVReg(8.5)){
			PrintMessage(strings[S_HVregErr]); //"HV regulator error\r\n"
			return;
		}
	}
	else StartHVReg(-1);			//LVP mode, turn off HV
	for(i=0;i<0x800B&&i<dati_hex.GetSize();i++) dati_hex[i]&=0x3FFF;
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
	if((options&2)==0){				//HV entry
		if((options&1)==0){				//VPP before VDD
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=4;				//VPP
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=0x5;			//VDD+VPP
		}
		else{							//VDD before VPP without delay
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=1;				//VDD
			bufferU[j++]=EN_VPP_VCC;
			bufferU[j++]=0x5;			//VDD+VPP
		}
	}
	else{			//Low voltage programming
		bufferU[j++]=EN_VPP_VCC;
		bufferU[j++]=4;				//VPP
		bufferU[j++]=WAIT_T3;
		bufferU[j++]=TX16;			//0000 1010 0001 0010 1100 0010 1011 0010 = 0A12C2B2
		bufferU[j++]=2;
		bufferU[j++]=0x0A;
		bufferU[j++]=0x12;
		bufferU[j++]=0xC2;
		bufferU[j++]=0xB2;
		bufferU[j++]=SET_CK_D;		//Clock pulse
		bufferU[j++]=0x4;
		bufferU[j++]=SET_CK_D;
		bufferU[j++]=0x0;
	}
	bufferU[j++]=WAIT_T2;
	bufferU[j++]=WAIT_T2;
	bufferU[j++]=WAIT_T2;
	bufferU[j++]=LOAD_CONF;			//counter at 0x2000
	bufferU[j++]=0xFF;
	bufferU[j++]=0xFF;
	bufferU[j++]=INC_ADDR_N;
	bufferU[j++]=0x06;
	bufferU[j++]=READ_DATA_PROG;	//DevID
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib1
	bufferU[j++]=INC_ADDR;
	bufferU[j++]=READ_DATA_PROG;	//Calib2
	bufferU[j++]=CUST_CMD;
	bufferU[j++]=0x16;		//Reset address
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2500>>8;
	bufferU[j++]=2500&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(3);
	read();
	if(saveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	devID=(bufferI[z+1]<<8)+bufferI[z+2];
	str.Format(strings[S_DevID],devID);	//"DevID: 0x%04X\r\n"
	PrintMessage(str);
	PIC_ID(devID);
	if(dati_hex[0x8006]<0x3FFF&&devID!=dati_hex[0x8006]) PrintMessage(strings[S_DevMismatch]);	//"Warning: the device is different from what specified in source data"
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib1=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib1<0x3fff){
		str.Format(strings[S_CalibWord1],calib1);	//"Calib1: 0x%04X\r\n"
		PrintMessage(str);
	}
	for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	calib2=(bufferI[z+1]<<8)+bufferI[z+2];
	if(calib2<0x3fff){
		str.Format(strings[S_CalibWord2],calib2);	//"Calib2: 0x%04X\r\n"
		PrintMessage(str);
	}
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Erasing ... "
	j=1;
	if(programID){
		bufferU[j++]=LOAD_CONF;			//PC @ 0x8000
		bufferU[j++]=0xFF;
		bufferU[j++]=0xFF;
	}
	bufferU[j++]=BULK_ERASE_PROG;
	bufferU[j++]=WAIT_T3;			// wait 5ms
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=CUST_CMD;
	bufferU[j++]=0x16;		//Reset address
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(8);
	read();
	if(saveLog)WriteLogIO();
	PrintMessage(strings[S_Compl]);	//"completed\r\n"
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Write code ... "
	for(;dim>0&&dati_hex[dim]>=0x3fff;dim--); //skip empty space at end
	dim+=dim%8;		//grow to 8 word multiple
	int valid,inc;
	for(i=k=0,j=1;i<dim;i+=8){
		valid=inc=0;
		for(;i<dim&&!valid;){	//skip empty locations (8 words)
			valid=0;
			for(k=0;k<8;k++) if(dati_hex[i+k]<0x3fff) valid=1;
			if(!valid){
				inc+=8;
				i+=8;
			}
			if(inc&&(valid||inc==248)){	//increase address to skip empty words
				bufferU[j++]=INC_ADDR_N;
				bufferU[j++]=k=inc;
				inc=0;
			}
			if(j>DIMBUF-4||valid){
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(3);
				msDelay(1*k/2);	//wait for long INC_ADDR_N
				read();
				j=1;
				if(saveLog)	WriteLogIO();
			}
		}		
		if(valid){
			k=0;
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i+k]>>8;  		//MSB
			bufferU[j++]=dati_hex[i+k]&0xff;		//LSB
			for(k=1;k<8;k++){
				bufferU[j++]=INC_ADDR;
				bufferU[j++]=LOAD_DATA_PROG;
				bufferU[j++]=dati_hex[i+k]>>8;  		//MSB
				bufferU[j++]=dati_hex[i+k]&0xff;		//LSB
			}
			bufferU[j++]=BEGIN_PROG;			//internally timed, T=2.5ms
			bufferU[j++]=WAIT_T3;
			bufferU[j++]=INC_ADDR;
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(3);
			read();
			j=1;
			if(saveLog){
				str.Format(strings[S_Log7],i,i,0,0);	//"i=%d, k=%d 0=%d\n"
				WriteLog(str);
				WriteLogIO();
			}
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Writing: %d%%, addr. %03X"
			StatusBar.SetWindowText(str);
		}
	}
	PrintMessage(strings[S_Compl]);	//"completed\r\n"
//****************** verify code ********************
	PrintMessage(strings[S_CodeV]);	//"Verifying code ... "
	j=1;
	bufferU[j++]=CUST_CMD;
	bufferU[j++]=0x16;		//Reset address
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(saveLog)	WriteLogIO();
	j=1;
	for(i=k=0;i<dim;i++){
		if(dati_hex[i]<0x3FFF) bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==dim-1){		//2B cmd -> 4B data
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(5);
			read();
			for(z=1;z<DIMBUF-2;z++){
				if(bufferI[z]==INC_ADDR) k++;
				else if(bufferI[z]==READ_DATA_PROG){
					if(dati_hex[k]<0x3FFF&&(dati_hex[k]!=(bufferI[z+1]<<8)+bufferI[z+2])){
						str.Format(strings[S_CodeWError2],k,dati_hex[k],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing address %3X: written %04X, read %04X\r\n"
						PrintMessage(str);
						err++;
					}
					z+=2;
				}
			}
			str.Format(strings[S_CodeV2],i*100/(dim+dim2),i);	//"Verify: %d%%, addr. %04X"
			StatusBar.SetWindowText(str);
			j=1;
			if(saveLog){
				str.Format(strings[S_Log8],i,i,k,k,err);	//"i=%d, k=%d, errors=%d\n"
				WriteLog(str);
				WriteLogIO();
			}
			if(err>=max_err) i=dim;
		}
	}
	if(k<dim){
		str.Format(strings[S_CodeVError3],dim,k);	//"Errore in verifica area programma, richieste %d word, lette %d\r\n"
		PrintMessage(str);
	}
	str.Format(strings[S_ComplErr],err);	//"completed, %d errors\r\n"
	PrintMessage(str);
	if(err>=max_err){
		str.Format(strings[S_MaxErr],err);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
		PrintMessage(str);
	}
//****************** write eeprom ********************
	if(dim2&&err<max_err){
		int errEE=0;
		PrintMessage(strings[S_EEAreaW]);	//"Writing EEPROM ... "
		j=1;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=5000>>8;
		bufferU[j++]=5000&0xff;
		bufferU[j++]=BULK_ERASE_DATA;
		bufferU[j++]=WAIT_T3;			// wait 5ms
		bufferU[j++]=CUST_CMD;
		bufferU[j++]=0x16;		//Reset address
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(7);
		read();
		j=1;
		if(saveLog)WriteLogIO();
		for(w=i=k=0;i<dim2;i++){
			if(memEE[i]<0xff){
				bufferU[j++]=LOAD_DATA_DATA;
				bufferU[j++]=memEE[i];
				bufferU[j++]=BEGIN_PROG;			//internally timed, T=5ms max
				bufferU[j++]=WAIT_T3;				//Tprogram
				bufferU[j++]=READ_DATA_DATA;
				w++;
			}
			bufferU[j++]=INC_ADDR;
			if(j>DIMBUF-12||i==dim2-1){
				str.Format(strings[S_CodeWriting],(i+dim)*100/(dim+dim2),i);	//"Writing: %d%%, add. %03X"
				StatusBar.SetWindowText(str);
				bufferU[j++]=FLUSH;
				for(;j<DIMBUF;j++) bufferU[j]=0x0;
				write();
				msDelay(w*5+2);
				w=0;
				read();
				for(z=1;z<DIMBUF-4;z++){
					if(bufferI[z]==INC_ADDR&&memEE[k]>=0xff) k++;
					else if(bufferI[z]==LOAD_DATA_DATA&&bufferI[z+3]==READ_DATA_DATA){
						if (memEE[k]!=bufferI[z+4]){
							str.Format(strings[S_CodeWError3],k,memEE[k],bufferI[z+4]);	//"Error writing address %4X: written %02X, read %02X\r\n"
							PrintMessage(str);
							errEE++;
							if(max_err&&err+errEE>max_err){
								str.Format(strings[S_MaxErr],err+errEE);	//"Exceeded maximum number of errors (%d), write interrupted\r\n"
								PrintMessage(str);
								PrintMessage(strings[S_IntW]);	//"write interrupted"
								i=dim2;
								z=DIMBUF;
							}
						}
						k++;
						z+=5;
					}
				}
				j=1;
				if(saveLog){
					str.Format(strings[S_Log8],i,i,k,k,errEE);	//"i=%d, k=%d, errors=%d\n"
					WriteLog(str);
					WriteLogIO();
				}
			}
		}
		errEE+=i-k;
		str.Format(strings[S_ComplErr],errEE);	//"completed, %d errors\r\n"
		PrintMessage(str);
		err+=errEE;
	}
//****************** write ID, CONFIG, CALIB ********************
	if(max_err&&err<max_err){
		PrintMessage(strings[S_ConfigAreaW]);	//"Writing CONFIG area ... "
		int err_c=0;
		bufferU[j++]=SET_PARAMETER;
		bufferU[j++]=SET_T3;
		bufferU[j++]=5000>>8;
		bufferU[j++]=5000&0xff;
		bufferU[j++]=LOAD_CONF;			//PC @ 0x8000
		bufferU[j++]=0xFF;
		bufferU[j++]=0xFF;
		if(programID){
			for(i=0x8000;i<0x8004;i++){
				bufferU[j++]=LOAD_DATA_PROG;
				bufferU[j++]=dati_hex[i]>>8;		//MSB
				bufferU[j++]=dati_hex[i]&0xff;		//LSB
				bufferU[j++]=BEGIN_PROG;			//internally timed
				bufferU[j++]=WAIT_T3;				//Tprogram 5ms
				bufferU[j++]=READ_DATA_PROG;
				bufferU[j++]=INC_ADDR;
			}
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=3;
		}
		else{
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=7;
		}
		bufferU[j++]=LOAD_DATA_PROG;			//Config word 0x8007
		bufferU[j++]=dati_hex[0x8007]>>8;		//MSB
		bufferU[j++]=dati_hex[0x8007]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;			//internally timed
		bufferU[j++]=WAIT_T3;				//Tprogram 5ms
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		bufferU[j++]=LOAD_DATA_PROG;			//Config word 2
		bufferU[j++]=dati_hex[0x8008]>>8;		//MSB
		bufferU[j++]=dati_hex[0x8008]&0xff;		//LSB
		bufferU[j++]=BEGIN_PROG;			//internally timed
		bufferU[j++]=WAIT_T3;				//Tprogram 5ms
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		bufferU[j++]=FLUSH;
		for(;j<DIMBUF;j++) bufferU[j]=0x0;
		write();
		msDelay(12);
		if(programID) msDelay(22);
		read();
		for(i=0,z=0;programID&&i<4;i++){
			for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
			if(dati_hex[0x8000+i]!=(bufferI[z+1]<<8)+bufferI[z+2]){
				str.Format(strings[S_IDErr],i,dati_hex[0x8000+i],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing ID%d: written %04X, read %04X\r\n"
				PrintMessage(str);
				err_c++;
			}
			z+=6;
		}
		for(;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x8007]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x8007],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		for(z+=6;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(!dati_hex[0x8008]&((bufferI[z+1]<<8)+bufferI[z+2])){	//error if written 0 and read 1 (!W&R)
			str.Format(strings[S_ConfigWErr3],dati_hex[0x8008],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Error writing config area: written %04X, read %04X\r\n"
			PrintMessage(str);
			err_c++;
		}
		err+=err_c;
		str.Format(strings[S_ComplErr],err_c);	//"completed, %d errors\r\n"
		PrintMessage(str);
		if(saveLog){
			str.Format(strings[S_Log9],err);	//"Config area 	errors=%d \n"
			WriteLog(str);
			WriteLogIO();
		}
	}
//****************** exit ********************
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=NOP;				//exit program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	str.Format(strings[S_EndErr],(stop-start)/1000.0,err,err!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nEnd (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	if(saveLog){
		WriteLog(str);
		CloseLogFile();
	}
	StatusBar.SetWindowText("");
}

