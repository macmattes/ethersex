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

//define sensor resistance at 100ppm of CH4 in the clean air
//calibrate your sensor to obtain precise value
//Ro = Rs * sqrt(MQ2_SCALINGFACTOR/ppm, MQ2_EXPONENT)
//   = Rs * exp( ln(MQ2_SCALINGFACTOR/ppm) / MQ2_EXPONENT )

//the graphic at fig.2 of datasheet (sensitivity characteristics of the MQ-2)
//seems a power function y = a*x^b
//so ppm = a*(Rs/Ro)^b
//using power regression, you can obtain scaling factor (a), and exponent (b) !for a specific GAS!
//points: (1.5 , 10) (0.75 , 100) (0.59 , 200)

#define MQ2_SCALINGFACTOR 3900 //CH4 gas value
#define MQ2_EXPONENT -2.7 //CH4 gas value
#define MQ2_MAXRSRO 3.0 //for CH4
#define MQ2_MINRSRO 0.7 //for CH4
//#define MQ2_BALANCERESISTOR 22000 //for CH4


//functions
extern long mq2_getres(uint16_t adc);
extern long mq2_getro(long resvalue, double ppm);
extern double mq2_getppm(long resvalue, long ro);
void mq2_main(void);

#endif
#endif
