/*
mq135 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/
#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "config.h"

#ifndef _MQ135_H_
#define _MQ135_H_

#ifdef MQ135_SUPPORT
double mq135_defaultro;
long mq135_ppm;
double mq135_adc;

//define sensor resistance at 1000ppm of CH4 in the clean air
//calibrate your sensor to obtain precise value
//Ro = Rs * sqrt(MQ135_SCALINGFACTOR/ppm, MQ135_EXPONENT)
//   = Rs * exp( ln(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT )
//seems a power function y = a*x^b
//so ppm = a*(Rs/Ro)^b
//using power regression, you can obtain scaling factor (a), and exponent (b) !for a specific GAS!
//points: (1.5 , 10) (0.75 , 100) (0.59 , 200)

//define the Rs/Ro valid interval, MQ135 detect from 10ppm to 300ppm
//look at the datasheet and use the helper to define those values
#define MQ135_SCALINGFACTOR 116.6020682 //CO2 gas value
#define MQ135_EXPONENT -2.769034857 //CO2 gas value
#define MQ135_MAXRSRO 2.428 //for CO2
#define MQ135_MINRSRO 0.358 //for CO2
#define MQ135_DEFAULTPPM 392.000 //for CO2

//functions
double mq135_getrs(double adc);
double mq135_getro(double resvalue, double ppm);
long double mq135_getppm(double resvalue, double ro);
long mq135_calibrate(void);
void mq135_calculation(void);
void mq135_main(void);

#endif
#endif
