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
long mq2_defaultro = MQ2_DEFAULTRO;
double mq2_ppm=0;
uint16_t mq2_adc=0;

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

void mq2_main(void) {
	char printbuff[100];
	long mq2_ro = 0;

		//get adc
		mq2_adc = (3*mq2_adc+adc_get_voltage(MQ2_ADCPORT))/4;

		long res = mq2_getres(mq2_adc);
		//get ro
		mq2_ro = mq2_getro(res, MQ2_DEFAULTPPM);
		//convert to ppm (using default ro)
		mq2_ppm = (3*mq2_ppm + mq2_getppm(res, mq2_defaultro))/4;

		ltoa(mq2_adc, printbuff, 10);
		debug_printf("ADC     : %s \n", printbuff);

		ltoa(res, printbuff, 10);
		debug_printf("RES     : %s \n", printbuff);

		ltoa(mq2_ro, printbuff, 10);
		debug_printf("ro     : %s \n", printbuff);

		ltoa(mq2_defaultro, printbuff, 10);
		debug_printf("defaultro     : %s \n", printbuff);

		ltoa((int16_t)mq2_ppm, printbuff, 10);
		debug_printf("ppm     : %s \n", printbuff);

}
#endif

/*
  -- Ethersex META --
  timer(50, mq2_main())
*/
