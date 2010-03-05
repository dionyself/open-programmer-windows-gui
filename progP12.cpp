/*
 * progP12.cpp - algorithms to program the PIC12 (12 bit word) family of microcontrollers
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

void COpenProgDlg::Read12F5xx(int dim, int dim2)
// read 12 bit PIC
// dim=program size 	dim2=config size
// vdd before vpp
// CONFIG @ 0x7FF (upon entering in program mode)
// OSCCAL in last memory location
// 4 ID + reserved area beyond code memory
{
	int k=0,z=0,i,j;
	int SaveLog;
	CString str;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(dim2<4) dim2=4;
	CButton* b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	SaveLog=b->GetCheck();
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	dati_hex.RemoveAll();
	dati_hex.SetSize(0x1000);
	if(SaveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Read12F5xx(%d,%d)\n",dim,dim2);
		WriteLog(str);
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
	bufferU[j++]=2000>>8;
	bufferU[j++]=2000&0xff;
	bufferU[j++]=EN_VPP_VCC;		//entrata program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	bufferU[j++]=READ_DATA_PROG;	//configuration word
	bufferU[j++]=INC_ADDR;			// 7FF->000
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(2);
	read();
	if(SaveLog)WriteLogIO();
	for(z=0;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if(z<DIMBUF-2){
		dati_hex[0xfff]=(bufferI[z+1]<<8)+bufferI[z+2];
		PrintMessage("\r\n");
		str.Format(strings[S_ConfigWord],dati_hex[0xfff]);	//"\r\nConfiguration word: 0x%03X\r\n"
		PrintMessage(str);
		switch(dati_hex[0xfff]&0x03){
			case 0:
				PrintMessage(strings[S_LPOsc]);	//"LP oscillator\r\n"
				break;
			case 1:
				PrintMessage(strings[S_XTOsc]);	//"XT oscillator\r\n"
				break;
			case 2:
				PrintMessage(strings[S_IntOsc]);	//"Internal osc.\r\n"
				break;
			case 3:
				PrintMessage(strings[S_RCOsc]);	//"RC oscillator\r\n"
				break;
		}
		if(dati_hex[0xfff]&0x04) PrintMessage(strings[S_WDTON]);	//"WDT ON\r\n"
		else PrintMessage(strings[S_WDTOFF]);	//"WDT OFF\r\n"
		if(dati_hex[0xfff]&0x08) PrintMessage(strings[S_CPOFF]);	//"Code protection OFF\r\n"
		else PrintMessage(strings[S_CPON]);	//"Code protection ON\r\n"
		if(dati_hex[0xfff]&0x10) PrintMessage(strings[S_MCLRON]);	//"Master clear ON\r\n"
		else PrintMessage(strings[S_MCLROFF]);	//"Master clear OFF\r\n"
	}
	else PrintMessage(strings[S_NoConfigW]);	//"Impossibile leggere la config word\r\n"
//****************** read code ********************
	PrintMessage(strings[S_CodeReading1]);		//lettura codice ...
	for(i=0,j=1;i<dim+dim2;i++){
		bufferU[j++]=READ_DATA_PROG;
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF*2/4-2||i==dim+dim2-1){		//2 istruzioni -> 4 risposte
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
			str.Format(strings[S_CodeReading],i*100/(dim+64),i);	//"Lettura: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			j=1;
			if(SaveLog){
				str.Format(strings[S_Log7],i,i,k,k);	//"i=%d(0x%X), k=%d(0x%X)\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	bufferU[j++]=NOP;				//uscita program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(1);
	read();
	unsigned int stop=GetTickCount();
	if(SaveLog)CloseLogFile();
	for(i=k;i<0xfff;i++) dati_hex[i]=0xfff;
	if(k!=dim+dim2){
		str.Format(strings[S_ReadErr],dim+dim2,k);	//"Errore in lettura: word richieste=%d, lette=%d\r\n"
		PrintMessage(str);
	}
	else PrintMessage(strings[S_Compl]);
	StatusBar.SetWindowText(strings[S_Ready]);	//"pronto"
//****************** visualize ********************
	for(i=0;i<4;i+=2){
		str.Format(strings[S_ChipID],i,dati_hex[dim+i],i+1,dati_hex[dim+i+1]);	//"ID%d: 0x%03X   ID%d: 0x%03X\r\n"
		PrintMessage(str);
	}
	if(dim2>4){
		str.Format(strings[S_BKOsccal],dati_hex[dim+4]);	//"Backup OSCCAL: 0x%03X\r\n"
		PrintMessage(str);
	}
	s.Empty();
	PrintMessage("\r\n");
	PrintMessage(strings[S_CodeMem]);	//"\r\nMemoria programma:\r\n"
	CString aux;
	for(i=0;i<dim;i+=COL){
		int valid=0;
		for(j=i;j<i+COL&&j<dim;j++){
			t.Format("%03X ",dati_hex[j]);
			s+=t;
			if(dati_hex[j]<0xfff) valid=1;
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
	if(dim2>5){
		PrintMessage(strings[S_ConfigResMem]);	//"\r\nMemoria configurazione e riservata:\r\n"
		for(i=dim;i<dim+dim2;i+=COL){
			int valid=0;
			for(j=i;j<i+COL&&j<dim+64;j++){
				t.Format("%03X ",dati_hex[j]);
				s+=t;
				if(dati_hex[j]<0xfff) valid=1;
			}
			if(valid){
				t.Format("%04X: %s\r\n",i,s);
				aux+=t;
			}
			s.Empty();
		}
	}
	PrintMessage(aux);
	StatusBar.SetWindowText("");
	PrintMessage("\r\n");
	str.Format(strings[S_End],(stop-start)/1000.0);	//"\r\nFine (%.2f s)\r\n"
	PrintMessage(str);
}

void COpenProgDlg::Write12F5xx(int dim,int OscAddr)
{
// write 12 bit PIC
// dim=program size     max~4300=10CC
// OscAddr=OSCCAL address (saved at the beginning), -1 not to use it
// vdd before vpp
// CONFIG @ 0x7FF upon entering program mode
// BACKUP OSCCAL @ dim+5 (saved at the beginning)
// erase: BULK_ERASE_PROG (1001) +10ms
// write: BEGIN_PROG (1000) + Tprogram 2ms + END_PROG2 (1110);
	int k=0,z=0,i,j,SaveLog,w;
	int programID,max_errori,errori=0,usa_BKosccal,usa_osccal,load_osccal;
	WORD osccal=-1,BKosccal=-1;
	CString str,err;
	CString s,t;
	DWORD BytesWritten=0;
	ULONG Result;
	if(OscAddr>dim) OscAddr=dim-1;
	CButton* b=(CButton*)m_DispoPage.GetDlgItem(IDC_IDPROG);
	programID=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_LOAD);
	load_osccal=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_OSCCAL);
	usa_osccal=b->GetCheck();
	b=(CButton*)m_DispoPage.GetDlgItem(IDC_OSC_BK);
	usa_BKosccal=b->GetCheck();
	b=(CButton*)m_OpzioniPage.GetDlgItem(IDC_REGISTRO);
	SaveLog=b->GetCheck();
	max_errori=m_OpzioniPage.GetDlgItemInt(IDC_ERRMAX);
	if(OscAddr==-1) usa_BKosccal=usa_osccal=0;
	if(MyDeviceDetected==FALSE) return;
	if (ReadHandle == INVALID_HANDLE_VALUE){
		PrintMessage(strings[S_InvHandle]);	//"Handle invalido\r\n"
		return;
	}
	if(dati_hex.GetSize()<0x1000){
		PrintMessage(strings[S_NoConfigW2]);	//"Impossibile trovare la locazione CONFIG (0xFFF)\r\n"
		return;
	}
	CancelIo(ReadHandle);
	if(SaveLog){
		OpenLogFile(strings[S_LogFile]);	//"Registro.txt"
		str.Format("Write12F5xx(%d)\n",dim);
		WriteLog(str);
	}
	for(i=0;i<dati_hex.GetSize();i++) dati_hex[i]&=0xFFF;
	PrintMessage(strings[S_Writing]);	//"Inizio scrittura...\r\n"
	unsigned int start=GetTickCount();
	bufferU[0]=0;
	j=1;
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T1T2;
	bufferU[j++]=1;						//T1=1u
	bufferU[j++]=100;					//T2=100u
	bufferU[j++]=EN_VPP_VCC;		//entrata program mode
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=EN_VPP_VCC;		//VDD
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;		//VDD+VPP
	bufferU[j++]=0x5;
	bufferU[j++]=NOP;
	if(OscAddr!=-1){
		for(i=-1;i<OscAddr-0xff;i+=0xff){
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=0xff;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=OscAddr-i;
		bufferU[j++]=READ_DATA_PROG;	// OSCCAL
		if(OscAddr<dim){
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=dim-OscAddr;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=0x4;				// 400->404
		bufferU[j++]=READ_DATA_PROG;	// backup OSCCAL
	}
	bufferU[j++]=NOP;				//uscita program mode
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
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(15);
	read();
	if(SaveLog)WriteLogIO();
	if(OscAddr!=-1){
		for(z=4;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(z<DIMBUF-2) osccal=(bufferI[z+1]<<8)+bufferI[z+2];
		for(z+=3;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
		if(z<DIMBUF-2) BKosccal=(bufferI[z+1]<<8)+bufferI[z+2];
		if(osccal==-1||BKosccal==-1){
			PrintMessage(strings[S_ErrOsccal]);	//"Errore in lettura OSCCAL e BKOSCCAL"
			return;
		}
		str.Format(strings[S_Osccal],osccal);	//"OSCCAL: 0x%03X\r\n"
		PrintMessage(str);
		str.Format(strings[S_BKOsccal],BKosccal);	//"Backup OSCCAL: 0x%03X\r\n"
		PrintMessage(str);
	}
//****************** erase memory ********************
	PrintMessage(strings[S_StartErase]);	//"Cancellazione ... "
	j=1;
	bufferU[j++]=EN_VPP_VCC;			// entrata program mode
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	if(dim>OscAddr+1){				//12F519 (Flash+EEPROM)
		bufferU[j++]=BULK_ERASE_PROG;	// Bulk erase
		bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
		for(i=-1;i<dim-0xff;i+=0xff){	// 0x43F
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=0xff;
		}
		bufferU[j++]=INC_ADDR_N;
		bufferU[j++]=dim-i-1;
		bufferU[j++]=BULK_ERASE_PROG;	// Bulk erase EEPROM
		bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
		if(programID){
			bufferU[j++]=INC_ADDR;
			bufferU[j++]=BULK_ERASE_PROG;	// Bulk erase
			bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
		}
	}
	else{							//12Fxxx
		if(programID){
			for(i=-1;i<dim-0xff;i+=0xff){
				bufferU[j++]=INC_ADDR_N;
				bufferU[j++]=0xff;
			}
			bufferU[j++]=INC_ADDR_N;
			bufferU[j++]=dim-i;
			bufferU[j++]=BULK_ERASE_PROG;	// Bulk erase
			bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
		}
		else{
			bufferU[j++]=BULK_ERASE_PROG;	// Bulk erase
			bufferU[j++]=WAIT_T3;			// ritardo T3=10ms
		}
	}
	bufferU[j++]=EN_VPP_VCC;		// uscita program mode
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			// ritardo T3=10ms tra uscita e rientro program mode
	bufferU[j++]=EN_VPP_VCC;		// entrata program mode
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=INC_ADDR;				// 7FF->000
	bufferU[j++]=SET_PARAMETER;
	bufferU[j++]=SET_T3;
	bufferU[j++]=2000>>8;				//T3=2ms
	bufferU[j++]=2000&0xff;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(30);
	if(dim>OscAddr+1) msDelay(20);
	read();
	PrintMessage(strings[S_Compl]);	//"completata\r\n"
	if(SaveLog)WriteLogIO();
//****************** write code ********************
	PrintMessage(strings[S_StartCodeProg]);	//"Scrittura codice ... "
	int dim1=dim;
	if(programID) dim1=dim+5;
	if(usa_BKosccal) dati_hex[OscAddr]=BKosccal;
	else if(usa_osccal) dati_hex[OscAddr]=osccal;
	for(i=k=w=0,j=1;i<dim1;i++){
		if(dati_hex[i]<0xfff){
			bufferU[j++]=LOAD_DATA_PROG;
			bufferU[j++]=dati_hex[i]>>8;		//MSB
			bufferU[j++]=dati_hex[i]&0xff;		//LSB
			bufferU[j++]=BEGIN_PROG;
			bufferU[j++]=WAIT_T3;				//Tprogram 2ms
			bufferU[j++]=END_PROG2;
			bufferU[j++]=WAIT_T2;				//Tdischarge
			bufferU[j++]=READ_DATA_PROG;
			w++;
		}
		bufferU[j++]=INC_ADDR;
		if(j>DIMBUF-10||i==dim1-1){
			str.Format(strings[S_CodeWriting],i*100/dim,i);	//"Scrittura: %d%%, ind. %03X"
			StatusBar.SetWindowText(str);
			bufferU[j++]=FLUSH;
			for(;j<DIMBUF;j++) bufferU[j]=0x0;
			write();
			msDelay(w*3+3);
			w=0;
			read();
			for(z=1;z<DIMBUF-7;z++){
				if(bufferI[z]==INC_ADDR&&dati_hex[k]>=0xfff) k++;
				else if(bufferI[z]==LOAD_DATA_PROG&&bufferI[z+5]==READ_DATA_PROG){
					if (dati_hex[k]!=(bufferI[z+6]<<8)+bufferI[z+7]){
						str.Format(strings[S_CodeWError],k,dati_hex[k],(bufferI[z+6]<<8)+bufferI[z+7]);	//"Errore in scrittura all'indirizzo %3X: scritto %03X, letto %03X\r\n"
						PrintMessage(str);
						errori++;
						if(max_errori&&errori>max_errori){
							str.Format(strings[S_MaxErr],errori);	//"Superato il massimo numero di errori (%d), scrittura interrotta\r\n"
							PrintMessage(str);
							PrintMessage(strings[S_IntW]);	//"Scrittura interrotta"
							i=dim1;
							z=DIMBUF;
						}
					}
					k++;
					z+=8;
				}
			}
			j=1;
			if(SaveLog){
				str.Format(strings[S_Log8],i,i,k,k,errori);	//"i=%d, k=%d, errori=%d,\n"
				WriteLog(str);
				WriteLogIO();
			}
		}
	}
	errori+=i-k;
	str.Format(strings[S_ComplErr],errori);	//"completata, %d errori\r\n"
	PrintMessage(str);
//****************** write CONFIG ********************
	PrintMessage(strings[S_ConfigW]);	//"Scrittura CONFIG ... "
	int err_c=0;
	bufferU[j++]=NOP;				//uscita program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=WAIT_T3;			//10 ms tra uscita e rientro prog. mode
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=WAIT_T3;
	bufferU[j++]=EN_VPP_VCC;		//entrata program mode
	bufferU[j++]=0x1;
	bufferU[j++]=NOP;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x5;
	bufferU[j++]=LOAD_DATA_PROG;	//config word
	bufferU[j++]=dati_hex[0xfff]>>8;			//MSB
	bufferU[j++]=dati_hex[0xfff]&0xff;			//LSB
	bufferU[j++]=BEGIN_PROG;
	bufferU[j++]=WAIT_T3;			//Tprogram 2ms
	bufferU[j++]=END_PROG2;
	bufferU[j++]=WAIT_T2;			//Tdischarge
	bufferU[j++]=READ_DATA_PROG;
	bufferU[j++]=NOP;				//uscita program mode
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x1;
	bufferU[j++]=EN_VPP_VCC;
	bufferU[j++]=0x0;
	bufferU[j++]=SET_CK_D;
	bufferU[j++]=0x0;
	bufferU[j++]=FLUSH;
	for(;j<DIMBUF;j++) bufferU[j]=0x0;
	write();
	msDelay(20);
	read();
	unsigned int stop=GetTickCount();
	for(z=10;z<DIMBUF-2&&bufferI[z]!=READ_DATA_PROG;z++);
	if (z<(DIMBUF-2)&&dati_hex[0xfff]!=(bufferI[z+1]<<8)+bufferI[z+2]){
		str.Format(strings[S_ConfigWErr],dati_hex[0xfff],(bufferI[z+1]<<8)+bufferI[z+2]);	//"Errore in scrittura config:\r\ndato scritto %03X, letto %03X\r\n"
		PrintMessage(str);
		err_c++;
	}
	errori+=err_c;
	if (z>DIMBUF-2) PrintMessage(strings[S_ConfigWErr2]);	//"Errore in scrittura CONFIG"
	str.Format(strings[S_ComplErr],err_c);	//"completata, %d errori\r\n"
	PrintMessage(str);
	if(SaveLog){
		str.Format(strings[S_Log8],i,i,k,k,errori);	//"i=%d, k=%d, errori=%d\n"
		WriteLog(str);
		WriteLogIO();
		CloseLogFile();
	}
	str.Format(strings[S_EndErr],(stop-start)/1000.0,errori,errori!=1?strings[S_ErrPlur]:strings[S_ErrSing]);	//"\r\nFine (%.2f s) %d %s\r\n\r\n"
	PrintMessage(str);
	StatusBar.SetWindowText("");
}

