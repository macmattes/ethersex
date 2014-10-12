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
uint16_t i,green,red,blue,clr,ctl,R,G,B;
uint16_t Illuminance;
double TCS3414tristimulus[3]; // [tri X, tri Y, tri Z]
double TCS3414chromaticityCoordinates[2]; //chromaticity coordinates // [x, y]

#ifndef I2C_TCS3414CS_GAIN
#define I2C_TCS3414CS_GAIN gain
#endif

void TCS3414CS_init()
{  	
	setIntegrationTime(I2C_TCS3414CS_INTEGRATION_TIME);
	setGain(I2C_TCS3414CS_GAIN|I2C_TCS3414CS_PRESCALER);
	setEnableADC();
        _delay_ms(400);
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
}

void setGain(int x)
{
if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  TWDR = REG_GAIN;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;
  TWDR = x;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_NACK)
    goto end;
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

  if (!i2c_master_select(COLOR_SENSOR_ADDR, TW_WRITE))
    goto end;
  TWDR = REG_BLOCK_READ;
  if (i2c_master_transmit_with_ack() != TW_MT_DATA_ACK)
    goto end;

  _delay_ms(10);

  if (i2c_master_start() != TW_REP_START)
    goto end;
  TWDR = (COLOR_SENSOR_ADDR << 1) | TW_READ;
  if (i2c_master_transmit() != TW_MR_SLA_ACK)
    goto end;
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[0] = TWDR; //DATA1LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[1] = TWDR; //DATA1HIGH
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[2] = TWDR; //DATA2LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[3] = TWDR; //DATA2HIGH
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[4] = TWDR; //DATA3LOW
  if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[5] = TWDR; //DATA3HIGH
 if (i2c_master_transmit_with_ack() != TW_MR_DATA_ACK)
    goto end;
  data[6] = TWDR; //DATA4LOW
  if (i2c_master_transmit() != TW_MR_DATA_NACK)
    goto end;
  data[7] = TWDR; //DATA4HIGH

	green=data[1]*256+data[0];
	red=data[3]*256+data[2];
	blue=data[5]*256+data[4];
	clr=data[7]*256+data[6];

    #ifdef DEBUG_I2C
        //debug_printf("I2C: i2c_tcs3414cs: 0x%X%X 0x%X%X 0x%X%X 0x%X%X\n",data[7],data[6],data[5],data[4],data[3],data[2],data[1],data[0]);
        debug_printf("I2C: i2c_tcs3414cs: red %.0d green %.0d blue %.0d clear %.0d\n",red,green,blue,clr);
    #endif	
end:
  i2c_master_stop();

  CCTCalc();

}

/* takes the raw values from the sensors and converts them to
Correlated Color Temperature. Returns a float with CCT ***/
void CCTCalc(void){

//calculate tristimulus values (chromaticity coordinates)
//The tristimulus Y value represents the illuminance of our source
TCS3414tristimulus[0] = (-0.14282 * red) + (1.54924 * green) + (-0.95641 * blue); //X
TCS3414tristimulus[1] = (-0.32466 * red) + (1.57837 * green) + (-0.73191 * blue); //Y // = Illuminance
TCS3414tristimulus[2] = (-0.68202 * red) + (0.77073 * green) + (0.56332 * blue); //Z

Illuminance = (uint16_t)TCS3414tristimulus[1];

double XYZ = TCS3414tristimulus[0] + TCS3414tristimulus[1] + TCS3414tristimulus[2];

        #ifdef DEBUG_I2C
                debug_printf("I2C: i2c_tcs3414cs: XYZ %d\n",XYZ);
        	debug_printf("I2C: i2c_tcs3414cs: lux %.0d\n",(uint16_t)TCS3414tristimulus[1]);
    	#endif

//calculate the chromaticiy coordinates
TCS3414chromaticityCoordinates[0] = TCS3414tristimulus[0] / XYZ; //x
TCS3414chromaticityCoordinates[1] = TCS3414tristimulus[1] / XYZ; //y

    #ifdef DEBUG_I2C
        debug_printf("I2C: i2c_tcs3414cs: ColorTemp %.0dK\n",Colortemp());
    #endif

}

uint16_t Colortemp(void)
{
  uint16_t CCT = 0;
  if ((TCS3414chromaticityCoordinates[0] > 0.25)&&(TCS3414chromaticityCoordinates[1] > 0.25)) {
    double n = (TCS3414chromaticityCoordinates[0] - 0.3320) / (0.1858 - TCS3414chromaticityCoordinates[1]);
    CCT = (uint16_t)( (449*pow(n,3)) + (3525*pow(n,2)) + (6823.3 * n) + 5520.33 );
  }
return CCT;
}

/*
 -- Ethersex META --
 header(hardware/i2c/master/i2c_TCS3414CS.h)
 init(TCS3414CS_init)
 timer(500, readRGB())
 */

#endif /* I2C_TCS3414CS_SUPPORT */
