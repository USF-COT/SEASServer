/*
   protocol.h
    
   Mini SEAS Gumstix <-> LON protocol definition header file.
  
   Revision History:
   
   2/10/2009   Jim Patten        Created.
   4/30/2009		Added ACK command
*/


/* Device Identifier mnemonics */

#define		LGT		0x19		// Lon Gateway Node
#define		PMP		0x20		// Pump Control Node
#define		HTR		0x21		// Heater Control  Node
#define		LTE		0x22		// Light Source Control Node
#define		VLV		0x23		// Shutoff Valve Control Node
#define		CTD		0x24		// CTD I/O Node
#define     BAT     0x25     	// Battery Node

/* Command identifier mnemonics */
#define  	PWR    	0x40		// Set power
#define  	PWL    	0x41		// Set power level
#define  	CTR    	0x42		// CTD values
#define     TMP     0x43     	// Heater temperature
#define     BVR     0x44     	// Battery voltage
#define		LPG 	0x4E		// Lon Ping Command	
#define		STS 	0x4F		// Status
#define		ACK		0x50		// Generic acknowledge
#define		NAK		0x70		// Generic unacknowledge
#define		RCT 	0x71		// Report current temperature


/* Data field identifier mnemonics */
#define  	ENA    	0x30		// Enable
#define  	DIS    	0x31		// Disable
#define     OPN     0x32     	// Open
#define     CLS     0x33     	// Close

