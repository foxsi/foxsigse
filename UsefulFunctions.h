/*
 *  UsefulFunctions.h
 *  Untitled
 *
 *  Created by Steven Christe on 7/22/09.
 *  Copyright 2009 FOXSI. All rights reserved.
 *
 *  Description
 *  -----------
 *
 *  Holds random miscellaneous useful functions.
 *
 */

#ifndef _UsefulFunctions_h_     // prevent multiple includes
#define _UsefulFunctions_h_

#include <iostream>
#include <stdio.h>
#include <sstream>

using namespace std;

// converts a number from one integer base to another
string convertBase(unsigned long v, long base);

// returns the bits in an unsigned, 
unsigned getbits(unsigned x, int p, int n);

// reverses bit order in an unsigned.
unsigned reversebits(unsigned x, int n);

// find the media of an array of size
unsigned int median(unsigned int *array, int n);

// find the maximum value of an array of size n
// ignore elements below min_index
unsigned long maximumValue(unsigned long *array, int n, int min_index);

void get_version(char *text);

#endif