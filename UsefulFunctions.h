/*
 *  UsefulFunctions.h
 *  Untitled
 *
 *  Created by Steven Christe on 7/22/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *
 */

#ifndef _UsefulFunctions_h_     // prevent multiple includes
#define _UsefulFunctions_h_

#include <iostream>
#include <stdio.h>
#include <sstream>
#include <string>

using namespace std;

//converts a number from one integer base to another
string convertBase(unsigned long v, long base);

//returns the bits in an unsigned, 
unsigned getbits(unsigned x, int p, int n);

//reverses bit order in an unsigned.
unsigned reversebits(unsigned x, int n);

void* testfunction(void *p);
void* watchfunction(void *p);
void startfile(void);

#endif