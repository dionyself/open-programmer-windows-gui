OpenProg: control program for open programmer, 
an USB programmer for PIC and ATMEL micros, I2C/SPI/MicroWire memories, and other I2C/SPI devices.
Website: http://openprog.altervista.org
These sources are given free of charge under the GNU General Public License version 2 

Changelog (to use new features the corresponding firmware is required):

V 0.9.0 march 2014:		Write16F72x requires only config-word 1;
						added HV serial programming for ATtiny11-12-13-24-25-44-45-84-85;
						added 24FJ64GA3xx-GCxx,24FJ128GA3xx-GB2xx-GCxx-DAxx,24FJ256DAxx,24EPx,33EPx;
						added 95xx SPI EEPROM,25X05 FLASH;
						added 12F1571-72,16F527,16F753,16F1454-55-59;
						some code rework & minor improvements

V 0.8.1 june 2013:		various user interface improvements; skip LV check;
						fixed EEPROM write when code protection is active (16F83-84,12F629,
						12F675,16F627-28,16F630,16F676,16F870-1-2,16F873-74,16F876-77);
						fixed read of files > 1MB;
						modified prog mode entry for AVR;
						fixed write of 93Sx6 with protection 

V 0.8.0 june 2012:		added one-wire memories DS2430,DS2431,DS2433,DS28EC20, thermometer DS1820;
						added UNIO memories 11010-20-40-80-160;
						fixed algorithms for 24x1024/5 and 251024, 
						added SPI FLASH 251005,252005,254005,258005,251605,25X10,25X20,25X40,25X80,
						25X16,25X32,25X64;
						added 16F1782-3-4-6-7,12C508-9

V 0.7.10 january 2012:	added 12F617,12F1501,16F1503-7-8-9,16F720-21,16F72,16F707,
						18F13K22,18F14K22,18F23K22,18F43K22,18F24K22,18F44K22,18F25K22,
						18F45K22,18F26K22,18F46K22,18F8520,18F66J60,18F66J65,18F67J60,
						18F86J60,18F86J65,18F87J60,18F96J60,18F96J65,18F97J60,
						fixed read/write binary files, various corrections

V 0.7.9	june 2011:		added ATtiny11-12-24-26-261-44-48-461-4313-84-88-861, 
						ATmega48-88-164A-168-324A-328-644A-1284, 16LF1902-3-4-6-7,
						EEPROM write optimization, 
						various minor modifications and fixes

V 0.7.8	april 2011:		updated algorithms for 16F87xA and 16F62xA,
						16F1822 becomes 12F1822, added 30F1010, 30F2020, 30F2023, 16F1847, 12F1840
						16F1516, 16F1517, 16F1518, 16F1519, 16F1526, 16F1527;
						greater modularity for source code, corrcted some bugs

V 0.7.7	august 2010:	added 16F72x,
						better compatibility with non standard hex files

V 0.7.6	july 2010:		modified 93xxx algorithms with byte by byte read for better compatibility,
						added support for programmers with 12 bit ADC (18F2553 etc.) 

V 0.7.5	june 2010:		corrected write bug for 18Fx config (introduced in v.0.7.4),
						added "goto ICD" write for 16Fx,
						various minor fixes

V 0.7.4	may 2010:		added PIC24H, dsPIC30, dsPIC33, 18FxxKxx, 18FxxJxx,
						eliminated many MFC objects,
						fixed: config word verify, 24F and 18F algorithm, various

V 0.7.3	april 2010:		added 16F1xxx, some code polishing, 
						separated fileIO.cpp and made similar to command-line version

V 0.7.2	march 2010:		added some uW memories, corrected 16F7xx algo, various bugfixes

V 0.7.1	february 2010:	added support for some PIC18 and Atmel micros

V 0.7.0	january 2010:	added support for I2C EEPROMs and some PIC24F micros

V 0.6.2	october 2009:	corrected algorithms for 16F628 16F87/88 and 81X; bin file support with EE memories

V 0.6.1	september 2009:	added support for 93Cx6 EEPROMs and some Atmel micros, corrected some bugs

V 0.6.0	june 2009:  	added support for 93Sx6 MicroWire EEPROMs

V 0.5.3 march 2009: 	corrected a bug with PIC18 config words

V 0.5.2 march 2009: 	added some PIC and Atmel micros, corrected some bugs

V 0.5.1 january 2009: 	added some PIC micros, corrected some bugs

V 0.5.0	november 2008: 	added some ATMEL micros(90S8515, 90S1200), universal support for I2C/SPI

V 0.4.0	august 2008: 	added I2C memories (24XX).

V 0.3.0	july 2008:		first public release, supports PIC 10,12,16,18.
