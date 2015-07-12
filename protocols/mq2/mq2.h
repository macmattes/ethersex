/*
mq2 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"

#ifndef _MQ2_H_
#define _MQ2_H_
#endif

#ifdef MQ2_SUPPORT
long mq2_defaultro;

//define sensor resistance at 1000ppm of CH4 in the clean air
//calibrate your sensor to obtain precise value
//Ro = Rs * sqrt(MQ2_SCALINGFACTOR/ppm, MQ2_EXPONENT)
//   = Rs * exp( ln(MQ2_SCALINGFACTOR/ppm) / MQ2_EXPONENT )
//seems a power function y = a*x^b
//so ppm = a*(Rs/Ro)^b
//using power regression, you can obtain scaling factor (a), and exponent (b) !for a specific GAS!
//points: (1.5 , 10) (0.75 , 100) (0.59 , 200)

//define the Rs/Ro valid interval, MQ2 detect from 10ppm to 300ppm
//look at the datasheet and use the helper to define those values
#define MQ2_SCALINGFACTOR 3900 //CH4 gas value
#define MQ2_EXPONENT -2.7 //CH4 gas value

//functions
double mq2_getrs(void);
long mq2_getro(void);
long mq2_calibrate(void);
long mq2_getppm(void);
void mq2_readeep(void);
void mq2_writeeep(void);
#endif
