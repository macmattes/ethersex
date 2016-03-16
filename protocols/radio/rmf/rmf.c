#include <avr/io.h>
#include <stdio.h>
#include <inttypes.h>
#ifndef F_CPU
#define F_CPU 8000000UL     /* Internal Quarz 8 Mhz*/
#endif
#include <util/delay.h>


int PIN_off()
{
PORTB &= ~ (1 << DDB0);
return 0;
}

int PIN_on()
{
PORTB |= (1 << DDB0);
return 0;
}

int bit_null()
  {
  PIN_on();
  _delay_us(450);
  PIN_off();
  _delay_us(650);
}

int bit_eins()
  {
  PIN_on();
  _delay_us(800);
  PIN_off();
  _delay_us(300);
}

int bit_anfang()
  {
  PIN_on();
  _delay_us(5000);
  PIN_off();
  _delay_us(1500);
}

int main()
{


  DDRB = 0b00000001; //Port B 0 = Ausgang


while(1)    
  {
  bit_anfang();
  char *p = "0001111100110000001011101111000101010101"; //FB 1 Stop 
  while( *p )
     {  
       if (*p=='0') bit_null();
       if (*p=='1') bit_eins();
     p++;
       }


  }
}
