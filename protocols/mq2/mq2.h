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

#ifdef MQ2_SUPPORT
extern long mq2_defaultro;
double mq2_ppm;
uint16_t mq2_adc;

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
#define MQ2_MAXRSRO 3.000 //for CH4
#define MQ2_MINRSRO 0.600 //for CH4

//functions
long mq2_getres(uint16_t adc);
long mq2_getro(long resvalue, double ppm);
double mq2_getppm(long resvalue, long ro);
long mq2_calibrate(void);
void mq2_main(void);

#endif
#endif
