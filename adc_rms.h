/* Project:	SmartModule
 * File:	adc_rms.h
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 12:33 PM
 */

#ifndef ADC_RMS_H
#define ADC_RMS_H

#include "buffer.h"

// DEFINITIONS-----------------------------------------------------------------
#define ADC_DC_OFFSET	3103	//((x steps/4096) * 3.3V = offset in volts)
#define ADC_WINDOW_SIZE	128

// TYPE DEFINITIONS------------------------------------------------------------

typedef struct AdcRmsInfo
{
	RingBuffer samples;
	unsigned char pinFloatAnimation;
} AdcRmsInfo;

// FUNCTION PROTOTYPES---------------------------------------------------------
void InitializeLoadMeasurement(void);
double CalculateCurrentRMS(void);

#endif