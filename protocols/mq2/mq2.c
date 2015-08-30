/*
mq2 0x01

copyright (c) Davide Gironi, 2012

Released under GPLv3.
Please refer to LICENSE file for licensing information.
*/


#include <stdio.h>
#include "config.h"
#include "core/bit-macros.h"
#include "core/debug.h"
#include "core/eeprom.h"
#include <math.h> //include libm
#include "hardware/adc/adc.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/mq2/mq2.h"

#ifdef MQ2_SUPPORT
/* ----------------------------------------------------------------------------
 *global variables
 */
long mq2_defaultro;

/*
 * init mq2
 */
void 
mq2_init(void) {
	mq2_readeep();
}

/*
 * get resistence for given voltage
 */
long 
mq2_getrs(void) {
	return ((((long)adc_get_vref() * MQ2_BALANCERESISTOR) / (long)adc_get_voltage(MQ2_ADCPORT) - MQ2_BALANCERESISTOR));
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long 
mq2_getro(void) {
	return ((float)mq2_getrs() * exp( log(MQ2_SCALINGFACTOR/MQ2_DEFAULTPPM) / MQ2_EXPONENT ));
}

long 
mq2_calibrate(void)
{
    mq2_defaultro = mq2_getro();
    return mq2_defaultro;
}

/*
 * get the ppm concentration
 */
long 
mq2_getppm() {
	long val = (MQ2_SCALINGFACTOR * pow(((float)mq2_getrs()/mq2_defaultro), MQ2_EXPONENT));
	return val;
}


/*
 * set / get the params
 */
void
mq2_readeep(void)
{
    //read from eeprom
    eeprom_restore_long(mq2_calibration, &mq2_defaultro);
}


#endif

/*
  -- Ethersex META --
  init(mq2_init)
*/
