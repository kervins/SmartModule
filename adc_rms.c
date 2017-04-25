/* Project:	SmartModule
 * File:	adc_rms.c
 * Author:	Jonathan Ruisi
 * Created:	April 13, 2017, 12:33 PM
 */

#include <xc.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>
#include "adc_rms.h"
#include "buffer.h"
#include "serial_comm.h"
#include "shell.h"

// GLOBAL VARIABLES------------------------------------------------------------
AdcRmsInfo _adc;
uint16_t _adcData[ADC_WINDOW_SIZE];

// FUNCTIONS-------------------------------------------------------------------

void InitializeLoadMeasurement(void)
{
	InitializeRingBuffer(&_adc.samples, ADC_WINDOW_SIZE, 2, &_adcData);
	_adc.pinFloatAnimation = 0;
}

double CalculateCurrentRMS(void)
{
	if(_adc.samples.length != _adc.samples.capacity)
		return 0.0;

	// Pause ADC sampling
	PIE5bits.TMR6IE	= 0;

	unsigned int i;
	double result = 0.0;
	// Calculate the sum of the squares of each sample
	for(i = 0; i < ADC_WINDOW_SIZE; i++)
	{
		unsigned int sample = 0;
		RingBufferDequeue(&_adc.samples, &sample);
		double adjSample1 = sample * 0.000805860806;
		double adjSample2 = adjSample1 - 2.474;
		double adjSample3 = adjSample2 / 0.04;
		result += adjSample3 * adjSample3;
	}

	// Divide the sum by the window size,
	// then take the square root of this value to obtain the RMS voltage
	result /= (double) ADC_WINDOW_SIZE;
	result = sqrt(result);
	result *= 120.0;

	_adc.samples.head = 0;
	_adc.samples.tail = 0;
	_adc.samples.length = 0;

	// Resume ADC sampling
	PIE5bits.TMR6IE	= 1;
	return result;
}