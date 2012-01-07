/*
 *  telemetry.h
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/7/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

#ifndef _telemetry_h_     // prevent multiple includes
#define _telemetry_h_

float temperature_convert_ysi44031(int voltage);
float telemetry_hvvalue_convert_voltage(int value);
int telemetry_voltage_convert_hvvalue(float value);

#endif