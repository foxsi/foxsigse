/*
 *  telemetry.h
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/7/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

float telemetry_ysi44031_convert_temperature(int voltage);
float telemetry_hvvalue_convert_voltage(int value);
int telemetry_voltage_convert_hvvalue(float value);

