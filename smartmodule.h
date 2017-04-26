/* Project:	SmartModule
 * File:	smartmodule.h
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 12:33 PM
 */

#ifndef ADC_RMS_H
#define ADC_RMS_H

#include "buffer.h"

// DEFINITIONS-----------------------------------------------------------------
#define ADC_DC_OFFSET		3103	// ((x steps/4096) * 3.3V = offset in volts)
#define ADC_WINDOW_SIZE		128
#define TIMER0_START_VALUE	0xDB60	// 100ms (Higher values = SHORTER timer period)

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct AdcRmsInfo
{
	RingBuffer samples;
	unsigned char pinFloatAnimation;
} AdcRmsInfo;

typedef struct ProxDetectInfo
{
	bool isTripped;
	unsigned int count;
	unsigned long int lastTripped;
} ProxDetectInfo;

// GLOBAL VARIABLES------------------------------------------------------------
extern struct AdcRmsInfo _adc;
extern struct ProxDetectInfo _prox;
extern unsigned char _relayState;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Load Measurement
void InitializeLoadMeasurement(void);
double CalculateCurrentRMS(void);
// Relay Control
void RelayControl(unsigned char state);

#endif