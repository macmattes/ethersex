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
double mq2_defaultro;
double mq2_ro = 0;
double mq2_res = 0;
double mq2_adc = 0;
uint16_t mq2_avg=MQ2_SMOOTHING;

#ifdef MQ2_AUTOTUNE_SUPPORT
double mq2_res_max;
#endif
/*
 * init mq2
 */
void mq2_init(void) {
  	eeprom_restore_long(mq2_calibration, &mq2_defaultro);
	if (mq2_defaultro == 0)
		mq2_defaultro = MQ2_DEFAULTRO;
}

/*
 * get resistence for given voltage
 */
double mq2_getrs(double adc) {
	return ((((long)adc_get_vref * MQ2_BALANCERESISTOR) / adc - MQ2_BALANCERESISTOR));
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
double mq2_getro(double res, double ppm) {
	return (double)(res * exp( log(MQ2_SCALINGFACTOR/ppm) / MQ2_EXPONENT ));
}

/*
 * get the ppm concentration
 */
long double mq2_getppm(double res, double ro) {
	long double ret = 0;
	double validinterval = 0;
	validinterval = res/ro;
	//if(validinterval<MQ2_MAXRSRO && validinterval>MQ2_MINRSRO) {
	    ret = (long double)((long)MQ2_SCALINGFACTOR * pow((res/ro), MQ2_EXPONENT));
	//}
	return ret;
}

long 
mq2_calibrate(void)
{
    //write to eerom
    eeprom_save_long(mq2_calibration, mq2_defaultro);
    eeprom_update_chksum();
    mq2_defaultro = mq2_ro;

#ifdef MQ2_AUTOTUNE_SUPPORT
    mq2_res_max = 0;
#endif

    return mq2_defaultro;
}

void 
mq2_calculation(void) {
		//get res
		mq2_res = mq2_getrs(mq2_adc);

		//get ro
		mq2_ro = mq2_getro(mq2_res, MQ2_DEFAULTPPM);

		//convert to ppm (using defaultro) average
                mq2_ppm = mq2_getppm(mq2_res, mq2_defaultro);

#ifdef MQ2_DEBUG_SUPPORT
	char printbuff[100];
		ltoa(mq2_adc, printbuff, 10);
		debug_printf("adc     : %s \n", printbuff);

		ltoa(mq2_res, printbuff, 10);
		debug_printf("rs     : %s \n", printbuff);

		ltoa(mq2_ro, printbuff, 10);
		debug_printf("ro     : %s \n", printbuff);

		ltoa(mq2_defaultro, printbuff, 10);
		debug_printf("defaultro     : %s \n", printbuff);

		ltoa((int32_t)mq2_ppm, printbuff, 10);
		debug_printf("ppm     : %s \n", printbuff);
#endif
#ifdef MQ2_AUTOTUNE_SUPPORT
		if ( mq2_res > mq2_res_max ) {
			mq2_calibrate();
			mq2_res_max=mq2_res;
		}
#endif
}

void 
mq2_main(void) {
	//get adc average
	mq2_adc = (mq2_avg*mq2_adc+adc_get_voltage(MQ2_ADCPORT))/(mq2_avg+1);
}
#endif

/*
  -- Ethersex META --
  init(mq2_init)
  timer(50, mq2_calculation())
  mainloop(mq2_main)
*/
