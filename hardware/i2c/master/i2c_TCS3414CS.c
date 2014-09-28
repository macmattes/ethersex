/*
 * arduino library for TCS3414CS color sensor
 */

#include <avr/io.h>
#include <util/twi.h>
#include <util/delay.h>
#include <math.h> 

#include "config.h"
#include "autoconf.h"
#include "core/debug.h"
#include "core/bit-macros.h"
#include "i2c_master.h"
#include "i2c_TCS3414CS.h"

#ifdef I2C_TCS3414CS_SUPPORT

/* global variables, accessible for this module only */ 
//static color_t current_color = RED;
uint16_t i,green,red,blue,clr,ctl;
uint16_t TCS3414values[4]; // [Clear,Red,Green,Blue]
uint16_t integrationTime = 12;
uint16_t gain = 1;
uint8_t gainstate = 0;

double ColorTemperature = 0;
double Illuminance = 0;

#ifndef I2C_TCS3414CS_GAIN
#define I2C_TCS3414CS_GAIN gain
#endif

void TCS3414CS_init()
{  	
	//setTimingReg(INTEG_MODE_FREE);//Set free mode
	setIntegrationTime(I2C_TCS3414CS_INTEGRATION_TIME);
	//setGain(GAIN_16|PRESCALER_2);
	setGain(I2C_TCS3414CS_GAIN|I2C_TCS3414CS_PRESCALER);
	setEnableADC();//Start ADC of the color sensor
	//_delay_ms(integrationTime);
        _delay_ms(12);
    #ifdef DEBUG_I2C
        debug_printf("I2C: i2c_tcs3414cs: init done\n");
    #endif
}
   
/************************************/
/*
 * set of sensor's configuration functions 
 */
void setTimingReg(int x)
{
  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  TWDR = REG_TIMING;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
  TWDR = x;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_NACK)
    goto end;
end:
  i2c_master_stop();   
}

void setIntegrationTime(int x)
{
  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  TWDR = REG_TIMING;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
  TWDR = x;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_NACK)
    goto end;
end:
  i2c_master_stop();
/*
	switch(x) 
	{
		case INTEGRATION_TIME_12ms:
			integrationTime = 12;
			break;
		case INTEGRATION_TIME_100ms:
			integrationTime = 100;
			break;
		case INTEGRATION_TIME_400ms:
			integrationTime = 400;
			break;
		default:
			integrationTime = 12;
			break;
	}
*/
}

void setGain(int x)
{

   //Wire.beginTransmission(COLOR_SENSOR_ADDR);
  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
   //Wire.write(REG_GAIN);
  TWDR = REG_GAIN;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
   //Wire.write(x);
  TWDR = x;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_NACK)
    goto end;
   //Wire.endTransmission();
end:
  i2c_master_stop();
}

void setEnableADC()
{
  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  TWDR = REG_CTL;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
  TWDR = CTL_DAT_INIITIATE;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_NACK)
    goto end;
end:
  i2c_master_stop();   
}

/************************************/
/*
 * Read the current sensor reading and store to our current color bin.
 * An integration time is assumed (not included in this routine) between 
 * reading.
 */
void readRGB()
{
  uint8_t data[8];

  /*select slave in write mode */
  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  /*send the dataaddress */
  TWDR = REG_BLOCK_READ;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  _delay_ms(10);		// for slow devices

  /* Do an repeated start condition */
  if (i2c_master_start() != TW_REP_START)
    goto end;
  /*select the slave in read mode */
  TWDR = (COLOR_SENSOR_ADDR << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;

  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[0] = TWDR; //Byte Count
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[1] = TWDR; //DATA1LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[2] = TWDR; //DATA1HIGH
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[3] = TWDR; //DATA2LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[4] = TWDR; //DATA2HIGH
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[5] = TWDR; //DATA3LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[6] = TWDR; //DATA3HIGH
 if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[7] = TWDR; //DATA4LOW
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;
  data[8] = TWDR; //DATA4HIGH

	green=data[2]*256+data[1];
	red=data[4]*256+data[3];
	blue=data[6]*256+data[5];
	clr=data[8]*256+data[7];

    #ifdef DEBUG_I2C
        debug_printf("I2C: i2c_tcs3414cs: red %.0d green %.0d blue %.0d clear %.0d\n",red,green,blue,clr);
    #endif

        TCS3414values[0] = clr;
        TCS3414values[1] = red;
	TCS3414values[2] = green;
	TCS3414values[3] = blue;

    ColorTemperature = CCTCalc(TCS3414values);
    #ifdef DEBUG_I2C
        debug_printf("I2C: i2c_tcs3414cs: ColorTemp %.0dK\n",(uint16_t)ColorTemperature);
    #endif	
end:
  i2c_master_stop();
    #ifdef I2C_TCS3414CS_AUTOTUNE_SUPPORT
	uint8_t g;
	if (clr < 500) {
		g = 3;
	} else if (clr < 2000) {
		g = 2;
	} else if (clr < 8000) {
		g = 1;
	} else {
		g = 0;
	};
	if (gainstate != g) {
		gainstate = g;
		gain = pow(4,g)+1;
		setGain(gain|I2C_TCS3414CS_PRESCALER);
        #ifdef DEBUG_I2C
        	debug_printf("I2C: i2c_tcs3414cs: gainstate %.0d gain %.0d\n",gainstate*10,gain);
    	#endif
	}
    #endif
}

/*** takes the raw values from the sensors and converts them to
Correlated Color Temperature. Returns a float with CCT ***/
double CCTCalc(uint16_t allcolors[]){
double TCS3414tristimulus[3]; // [tri X, tri Y, tri Z]
double TCS3414chromaticityCoordinates[2]; //chromaticity coordinates // [x, y]

//calculate tristimulus values (chromaticity coordinates)
//The tristimulus Y value represents the illuminance of our source
TCS3414tristimulus[0] = (-0.14282 * allcolors[1]) + (1.54924 * allcolors[2]) + (-0.95641 * allcolors[3]); //X
TCS3414tristimulus[1] = (-0.32466 * allcolors[1]) + (1.57837 * allcolors[2]) + (-0.73191 * allcolors[3]); //Y // = Illuminance
TCS3414tristimulus[2] = (-0.68202 * allcolors[1]) + (0.77073 * allcolors[2]) + (0.56332 * allcolors[3]); //Z

Illuminance = TCS3414tristimulus[1];

double XYZ = TCS3414tristimulus[0] + TCS3414tristimulus[1] + TCS3414tristimulus[2];

        #ifdef DEBUG_I2C
        	debug_printf("I2C: i2c_tcs3414cs: xyz %.0d\n",(uint16_t)XYZ);
        	debug_printf("I2C: i2c_tcs3414cs: lux %.0d\n",(uint16_t)TCS3414tristimulus[1]);
    	#endif

//calculate the chromaticiy coordinates
TCS3414chromaticityCoordinates[0] = TCS3414tristimulus[0] / XYZ; //x
TCS3414chromaticityCoordinates[1] = TCS3414tristimulus[1] / XYZ; //y

double n = (TCS3414chromaticityCoordinates[0] - 0.3320) / (0.1858 - TCS3414chromaticityCoordinates[1]);


        #ifdef DEBUG_I2C
        	debug_printf("I2C: i2c_tcs3414cs: colortemp %.0d\n",(uint32_t)n);
    	#endif


double CCT = ( (449*pow(n,3)) + (3525*pow(n,2)) + (6823.3 * n) + 5520.33 );

return CCT;
}


/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_TCS3414CS.h)
 init(TCS3414CS_init)
 timer(200, readRGB())
 */

#endif /* I2C_TCS3414CS_SUPPORT */
