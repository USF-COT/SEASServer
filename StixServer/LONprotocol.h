/*
   LONProtocol.h
    
   Mini SEAS Gumstix <-> LON protocol definition header file.
  
   Revision History:
   
   2/10/2009   Jim Patten        Created.
   9/28/2010   Michael Lindemuth Renamed from protocol.h to LONProtocol.h to avoid confusion with GUI protocol.
*/


/* Device Identifier mnemonics */

#define		LGT		0x19		// Lon Gateway Node
#define		PMP		0x20		// Pump Control Node
#define		HTR		0x21		// Heater Control  Node
#define		LTE		0x22		// Light Source Control Node
#define		VLV		0x23		// Shutoff Valve Control Node
#define		CTD		0x24		// CTD I/O Node
#define     BAT      0x25     // Battery Node

/* Command identifier mnemonics */
#define  	PWR    	0x40		// Set power
#define  	PWL    	0x41		// Set power level
#define  	CTR    	0x42		// CTD values
#define     TMP      0x43     // Heater temperature
#define     BVL      0x44     // Battery voltage



/* Data field identifier mnemonics */
#define  	ENA    	0x30		// Enable
#define  	DIS    	0x31		// Disable
#define     OPN      0x32     // Open
#define     CLS      0x33     // Close

/* LON -> Gumstix */
#define    ACK    0x51
#define    NAK    0x52
