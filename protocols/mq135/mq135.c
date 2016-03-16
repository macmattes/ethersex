/*
mq135 0x01

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
#include "protocols/mq135/mq135.h"

#ifdef MQ135_SUPPORT
/* ----------------------------------------------------------------------------
 *global variables
 */
long mq135_defaultro;

/*
 * init mq135
 */
void 
mq135_init(void) {
	mq135_readeep();
}

/*
 * get resistence for given voltage
 */
long 
mq135_getrs(void) {
	return ((((long)adc_get_vref() * MQ135_BALANCERESISTOR) / (long)adc_get_voltage(MQ135_ADCPORT) - MQ135_BALANCERESISTOR));
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long 
mq135_getro(void) {
	return ((float)mq135_getrs() * exp( log(MQ135_SCALINGFACTOR/MQ135_DEFAULTPPM) / MQ135_EXPONENT ));
}

long 
mq135_calibrate(void)
{
    mq135_defaultro = mq135_getro();
    return mq135_defaultro;
}

/*
 * get the ppm concentration
 */
long 
mq135_getppm() {
	long val = (MQ135_SCALINGFACTOR * pow(((float)mq135_getrs()/mq135_defaultro), MQ135_EXPONENT));
	return val;
}


/*
 * set / get the params
 */
void
mq135_writeeep(void)
{
    //write to eeprom
    eeprom_save_long(mq135_calibration, mq135_defaultro);
    eeprom_update_chksum();
}
void
mq135_readeep(void)
{
    //read from eeprom
    eeprom_restore_long(mq135_calibration, &mq135_defaultro);
}


#endif

/*
  -- Ethersex META --
  init(mq135_init)
*/
