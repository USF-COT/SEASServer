/*
  GUIprotocol.h

  Mini SEAS GUI <-> Mini SEAS protocol definition header file.

  Revision History:

  3/10/2009   Jim Patten        Created.
*/


/* GUI to Mini SEAS commands */

#define		ERR	0x00 // Error, may be followed by \0 terminated string

/* System command identifier mnemonics */
#define         STM     0x01            // Set Time
#define         RTM     0x02     // Read Time
#define         RSD     0x03            // Read Directory
#define         RSF     0x04            // Read Data File
#define         DSF     0x05            // Delete Data File

/* Benchtop mode command identifier mnemonics */
#define         PMW     0x06            // Set Pump RPM
#define         PMC     0x07            // Pump Control
#define         HTP     0x08            // Set Heater Temperature
#define         HTC     0x09            // Heater Control
#define         LTC     0x0A            // Light Source Control
#define         LSD     0x0B            // Load Slope Data for Wavelength Concentration Calculation
#define         SSP     0x0C            // Set Spectrometer Parameters
#define     SVP      0x1E     // Reserved
#define         SCP     0x0D            // Set Absorbance Computational Data
#define         RFS     0x0E            // Read Full Spectrum
#define     RCD      0x0F     // Read Spectrometer Calibration Data
#define     RSP      0x10     // Read Spectrometer Parameters
#define     RCP      0x11     // Read Spectrometer Computation Data
#define     SVC      0x12     // Save configuration
#define     RAD      0x13     // Read absorbance data
#define     RCC      0x14     // Read concentration data
#define     RDS      0x15     // Record Dark Spectra
#define     RRS      0x16     // Record Reference Spectra
#define     RSS      0x17     // Record Sample Spectra

/* Program mode command identifier mnemonics */
#define         EXM             0x18     // Execute Method
#define     TRM         0x19     // Terminate Method

/* Method transfer command identifier mnemonics */
#define     LRM         0x1A     // Load Method
#define     LMT         0x1B     // List Method Files
#define     RMF         0x1C     // Read Method File
#define         SMF     0x1D            // Set Current Method File
#define     DLM      0x1E     // Delete Method

#define     RAS      0x1F     // Record Absorbance Spectra

#define     SAM      0x20     // Send active filename

/* Status Report Command Mnemonics */
#define     RPS         0x21     // Report Pump State
#define     RHS         0x22     // Report Heater State
#define     RLS         0x23     // Report Light Source State
#define     RBS         0x24     // Report Battery Voltage
#define     RTD         0x25     // Report CTD Values


/* Mini SEAS to GUI commands */

/* System command identifier mnemonics */

#define     RPT      0x61     // Report Time
#define     SDC      0x62     // Send Directory Count
#define     SDE         0x63     // Send Directory Entry
#define     SFR         0x64     // Send Data File Record

/* Benchtop mode command identifier mnemonics */
#define     SFS         0x65     // Send Full Spectrum
#define     SWL         0x66     // Send Wavelengths
#define     DSR      0x67     // Dark Spectra Recorded
#define     RSR      0x68     // Reference Spectra Recorded
#define     SSR      0x69     // Sample Spectra Recorded

