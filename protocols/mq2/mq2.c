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
#include "core/periodic.h"
#include <util/delay.h>
//#include "services/clock/clock.h"
#include <math.h> //include libm
#include "hardware/adc/adc.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/mq2/mq2.h"

#ifdef MQ2_SUPPORT
/* ----------------------------------------------------------------------------
 *global variables
 */
long mq2_defaultro;
double mq2_ppm=0;
uint16_t mq2_adc=0;
long mq2_ro = 0;
uint16_t mq2_avg=MQ2_SMOOTHING;

/*
 * init mq2
 */
void mq2_init(void) {
   #ifdef MQ2_SUPPORT
  	eeprom_restore_long(mq2_calibration, &mq2_defaultro);
	if (mq2_defaultro == 0)
		mq2_defaultro = MQ2_DEFAULTRO;
   #endif
}

/*
 * get resistence for given voltage
 */
long mq2_getres(uint16_t adc) {	
	return (long)(((long)adc_get_vref*MQ2_BALANCERESISTOR)/adc-MQ2_BALANCERESISTOR);
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long mq2_getro(long resvalue, double ppm) {
	return (long)(resvalue * exp( log(MQ2_SCALINGFACTOR/ppm) / MQ2_EXPONENT ));
}

/*
 * get the ppm concentration
 */
double mq2_getppm(long resvalue, long ro) {
	double ret = 0;
	double validinterval = 0;
	validinterval = resvalue/(double)ro;
	if(validinterval<MQ2_MAXRSRO && validinterval>MQ2_MINRSRO) {
		ret = (double)MQ2_SCALINGFACTOR * pow( ((double)resvalue/ro), MQ2_EXPONENT);
	}
	return ret;
}

long 
mq2_calibrate(void)
{
    eeprom_save_long(mq2_calibration, mq2_ro);
    eeprom_update_chksum();
    mq2_defaultro = mq2_ro;
    return mq2_defaultro;
}

void 
mq2_main(void) {
		//get adc average
		mq2_adc = (mq2_avg*mq2_adc+adc_get_voltage(MQ2_ADCPORT))/(mq2_avg+1);

		long res = mq2_getres(mq2_adc);
		//get ro
		mq2_ro = mq2_getro(res, MQ2_DEFAULTPPM);
		//convert to ppm (using defaultro) average
		mq2_ppm = (mq2_avg*mq2_ppm + mq2_getppm(res, mq2_defaultro))/(mq2_avg+1);
#ifdef MQ2_AUTOCALIBRATE
	if (mq2_ppm < MQ2_DEFAULTPPM)
		mq2_calibrate();
#endif
}
#endif

/*
  -- Ethersex META --
  init(mq2_init)
  timer(50, mq2_main())
*/
