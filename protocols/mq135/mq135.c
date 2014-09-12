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
#include "core/periodic.h"
#include <util/delay.h>
//#include "services/clock/clock.h"
#include <math.h> //include libm
#include "hardware/adc/adc.h"
#include "protocols/ecmd/ecmd-base.h"
#include "protocols/mq135/mq135.h"

#ifdef MQ135_SUPPORT
/* ----------------------------------------------------------------------------
 *global variables
 */
double mq135_defaultro;
double mq135_ro = 0;
double mq135_res = 0;
double mq135_adc = 0;
uint16_t mq135_avg=MQ135_SMOOTHING;

#ifdef MQ135_AUTOTUNE_SUPPORT
double mq135_res_max;
#endif
/*
 * init mq135
 */
void mq135_init(void) {
  	eeprom_restore_long(mq135_calibration, &mq135_defaultro);
	if (mq135_defaultro == 0)
		mq135_defaultro = MQ135_DEFAULTRO;
}

/*
 * get resistence for given voltage
 */
double mq135_getrs(double adc) {
	return ((((long)adc_get_vref * MQ135_BALANCERESISTOR) / adc - MQ135_BALANCERESISTOR));
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
double mq135_getro(double res, double ppm) {
	return (double)(res * exp( log(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT ));
}

/*
 * get the ppm concentration
 */
long double mq135_getppm(double res, double ro) {
	long double ret = 0;
	double validinterval = 0;
	validinterval = res/ro;
	//if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
	    ret = (long double)((long)MQ135_SCALINGFACTOR * pow((res/ro), MQ135_EXPONENT));
	//}
	return ret;
}

long 
mq135_calibrate(void)
{
    //write to eerom
    eeprom_save_long(mq135_calibration, mq135_defaultro);
    eeprom_update_chksum();
    mq135_defaultro = mq135_ro;

#ifdef MQ135_AUTOTUNE_SUPPORT
    mq135_res_max = 0;
#endif

    return mq135_defaultro;
}

void 
mq135_calculation(void) {
		//get res
		mq135_res = mq135_getrs(mq135_adc);

		//get ro
		mq135_ro = mq135_getro(mq135_res, MQ135_DEFAULTPPM);

		//convert to ppm (using defaultro) average
                mq135_ppm = mq135_getppm(mq135_res, mq135_defaultro);

#ifdef MQ135_DEBUG_SUPPORT
	char printbuff[100];
		ltoa(mq135_adc, printbuff, 10);
		debug_printf("adc     : %s \n", printbuff);

		ltoa(mq135_res, printbuff, 10);
		debug_printf("rs     : %s \n", printbuff);

		ltoa(mq135_ro, printbuff, 10);
		debug_printf("ro     : %s \n", printbuff);

		ltoa(mq135_defaultro, printbuff, 10);
		debug_printf("defaultro     : %s \n", printbuff);

		ltoa((int32_t)mq135_ppm, printbuff, 10);
		debug_printf("ppm     : %s \n", printbuff);
#endif
#ifdef MQ135_AUTOTUNE_SUPPORT
		if ( mq135_res > mq135_res_max ) {
			mq135_calibrate();
			mq135_res_max=mq135_res;
		}
#endif
}

void 
mq135_main(void) {
	//get adc average
	mq135_adc = (mq135_avg*mq135_adc+adc_get_voltage(MQ135_ADCPORT))/(mq135_avg+1);
}
#endif

/*
  -- Ethersex META --
  init(mq135_init)
  timer(50, mq135_calculation())
  mainloop(mq135_main)
*/
