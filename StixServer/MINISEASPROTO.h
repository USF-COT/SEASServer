/* 
 * File:   MINISEASPROTO.h
 * Author: seraph
 *
 * Created on June 3, 2009, 1:36 PM
 */

#ifndef _MINISEASPROTO_H
#define	_MINISEASPROTO_H

#ifdef	__cplusplus
extern "C" {
#endif

// System Settings
#define STM 0x01 // Set Time
#define RTM 0x02 // Read Time
#define RSD 0x03 // Read Directory
#define RSF 0x04 // Read Data File
#define DSF 0x05 // Delete Data File

// Pump Settings
#define PMW 0x06 // Set Pump RPM
#define PMC 0x07 // Pump Control (On or Off)

// Heater Settings
#define HTP 0x08 // Set Heater Temperature
#define HTC 0x09 // Heater Control (On or Off)

// Light Source Control
#define LTC 0x0A // Light Source Control (On or Off)

// Valve Control
#define SVS 0x0B // Set Valve State

// Wavelength Settings
#define SAW 0x0C // Set Absorbance Wavelengths
#define SNW 0x0D // Set Non-absorbance Wavelengths

// Spectrum Commands
#define RFS 0x0E // Read Full Specturm
#define RWL 0x0F // Read Wavelengths

// Reading Settings
#define SIT 0x10 // Set Integration Time
#define SBX 0x11 // Set Boxcar
#define SSA 0x12 // Set Scans to Average

// Program Mode
#define EXM 0x13 // Execute Method
#define TRM 0x14 // Terminate Method

// Method Transfer
#define LDM 0x15 // Load Method
#define RDM 0x16 // Read Method
#define SMR 0x17 // Send Method Record
#define SMF 0x18 // Set Current Method File

// Mini SEAS to GUI
#define SDC 0x19 // Send Directory Count
#define SDE 0x1A // Send Directory Entry
#define SFR 0x1B // Send Data File Record
#define SFS 0x1C // Send Full Spectrum
#define SWL 0x1D // Send Wavelengths
#define RMW 0x1E // Report Pump RPM
#define RPS 0x1F // Report Pump State
#define RTP 0x20 // Report Heater Temperature
#define RHS 0x21 // Report Heater Status
#define RLS 0x22 // Report Light Source State
#define RVS 0x23 // Report Valve State

// Gumstix to LON Nodes
// Globals
#define ENA 0x24
#define DIS 0x25
#define OPN 0x26
#define CLS 0x27

// Pump Control
#define PMP 0x28 // Pump Device
#define PWL 0x29 // Pump Power Level
#define PWR 0x2A // Pump Power

// Heater Control
#define HTR 0x2B // Heater Device
#define TMP 0x2C // Heater Temperature
// PWR Valid for Heater Too

// Light Source Control
#define LGT 0x2D // Light Device
// PWR Valid for Light Too

// Valve Control
#define VLV 0x2E // Valve Device
#define STE 0x2F // Shutoff Valve

// Battery Status
#define BAT 0x30 // Battery Device
#define BVR 0x31 // Request Voltage

// CTD Control
#define CTD 0x32 // CTD Device
#define CTR 0x33 // Request CTD Values

#ifdef	__cplusplus
}
#endif

#endif	/* _MINISEASPROTO_H */

