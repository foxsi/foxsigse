/*
 *  commands.h
 *  FOXSI_GSE
 *
 *  Created by Steven Christe on 10/26/11.
 *  Copyright 2011 NASA GSFC. All rights reserved.
 *
 */

void command_attenuator_state(bool state);
void command_voltage_set(int hv_value);
void command_clock_set(int clockhi, int clocklo);
void create_cmd_attenuator(bool state);
void create_cmd_hv(int hvvalue);
void set_clockhi(int clockvalue);
void set_clock_f(int clock_value);
int command_initialize_serial(void);