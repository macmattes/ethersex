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
long mq135_defaultro;
double mq135_ppm=0;
uint16_t mq135_adc=0;
long mq135_ro = 0;

/*
 * init mq135
 */
void mq135_init(void) {
   #ifdef MQ135_SUPPORT
  	eeprom_restore_long(mq135_calibration, &mq135_defaultro);
	if (mq135_defaultro == 0)
		mq135_defaultro = MQ135_DEFAULTRO;
   #endif
}

/*
 * get resistence for given voltage
 */
long mq135_getres(uint16_t adc) {	
	return (long)(((long)adc_get_vref*MQ135_BALANCERESISTOR)/adc-MQ135_BALANCERESISTOR);
}

/*
 * get the calibrated ro based upon read resistance, and a know ppm
 */
long mq135_getro(long resvalue, double ppm) {
	return (long)(resvalue * exp( log(MQ135_SCALINGFACTOR/ppm) / MQ135_EXPONENT ));
}

/*
 * get the ppm concentration
 */
double mq135_getppm(long resvalue, long ro) {
	double ret = 0;
	double validinterval = 0;
	validinterval = resvalue/(double)ro;
	if(validinterval<MQ135_MAXRSRO && validinterval>MQ135_MINRSRO) {
		ret = (double)MQ135_SCALINGFACTOR * pow( ((double)resvalue/ro), MQ135_EXPONENT);
	}
	return ret;
}

long 
mq135_calibrate(void)
{
    eeprom_save_long(mq135_calibration, mq135_ro);
    eeprom_update_chksum();
    mq135_defaultro = mq135_ro;
    return mq135_defaultro;
}

void 
mq135_main(void) {
	//char printbuff[100];
		//get adc
		mq135_adc = (3*mq135_adc+adc_get_voltage(MQ135_ADCPORT))/4;

		long res = mq135_getres(mq135_adc);
		//get ro
		mq135_ro = mq135_getro(res, MQ135_DEFAULTPPM);
		//convert to ppm (using default ro)
		mq135_ppm = (3*mq135_ppm + mq135_getppm(res, mq135_defaultro))/4;

		//ltoa(mq135_adc, printbuff, 10);
		//debug_printf("ADC     : %s \n", printbuff);

		//ltoa(res, printbuff, 10);
		//debug_printf("RES     : %s \n", printbuff);

		//ltoa(mq135_ro, printbuff, 10);
		//debug_printf("ro     : %s \n", printbuff);

		//ltoa(mq135_defaultro, printbuff, 10);
		//debug_printf("defaultro     : %s \n", printbuff);

		//ltoa((int16_t)mq135_ppm, printbuff, 10);
		//debug_printf("ppm     : %s \n", printbuff);

}
#endif

/*
  -- Ethersex META --
  init(mq135_init)
  timer(50, mq135_main())
*/
