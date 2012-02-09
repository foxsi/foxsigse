/*
 *  telemetry.cpp
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/7/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

#include "telemetry.h"
#include "math.h"

float temperature_convert_ysi44031(int value)
{
	// Take the reading of A/D Voltage from the YSI 44031 temperature
	// sensor and convert to a temperature (in Celsius).
	
	// Test input 1
	// int value = 0x0FF4;
	// answer should be -80.0
	
	// Test input 2
	// int value = 0x070D;
	// answer should be 31.0
	
	double resistance;
	double temperature;
	double frac;
	double voltage;
	
	const float c1 = 0.00099542549430017;
	const float c2 = 0.000234788987236607;
	const float c3 = 1.13519068800246E-07;
	
	frac = value/4095.0;
	voltage = 2.5*frac;
	resistance = 10000.0 * frac/ ( 1.0 - frac );
	//printf("%f\n",resistance);
	temperature = 1 / (c1 + c2 * log(resistance) + c3 * (pow(log(resistance),3))) - 273.15 - 9.73;
	//printf("%f\n", temperature);
	return temperature;
}

float telemetry_hvvalue_convert_voltage(int value)
{
	// Take the reading of the HV value and convert it to an actual voltage
	// 256 is 31 V
	// 1000 hv_value is 125 volts
	// 2000 is 250 volts
	
	return (float) value/8.0;
}

int telemetry_voltage_convert_hvvalue(float value)
{
	// Take an actual voltage value and convert it to an HV value to send
	// as a command
	
	return (int) value*8;
}