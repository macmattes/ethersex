/* port the enc28j60 is attached to */
pin(SPI_CS_NET, SPI_CS_HARDWARE)

ifdef(`conf_SD_READER', `dnl
  /* port the sd-reader CS is attached to */
  pin(SPI_CS_SD_READER, PB3, OUTPUT)
')dnl

ifdef(`conf_ONEWIRE', `dnl
  /* onewire port range */
  ONEWIRE_PORT_RANGE(PA7, PA7)
')dnl

ifdef(`conf_STELLA', `dnl
  /* stella port 1 */
  STELLA_PORT1_RANGE(PC0, PC7)
')dnl

dnl STELLA_PORT2_RANGE(PA0, PA3)

ifdef(`conf_STATUSLED_POWER', `dnl
pin(STATUSLED_POWER, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_BOOTED', `dnl
pin(STATUSLED_BOOTED, PA3, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_NETLINK', `dnl
pin(STATUSLED_NETLINK, PA2, OUTPUT)
')dnl

ifdef(`conf_STATUSLED_HB_ACT', `dnl
pin(STATUSLED_HB_ACT, PA0, OUTPUT)
')dnl

ifdef(`conf_FS20', `dnl
/* fs20 support */
pin(FS20_SEND, PD7)
ifdef(`conf_FS20_RECEIVE', `dnl
/* DO NOT CHANGE PIN!  USES INTERNAL COMPARATOR! */
pin(FS20_RECV, PB3)
')dnl
')dnl

ifdef(`conf_RFM12', `dnl
/* port the rfm12 module CS is attached to */
pin(SPI_CS_RFM12_0, PD5, OUTPUT)
/* port the LEDS for rfm12 tx attached to */
pin(STATUSLED_RFM12_TX, PB0, OUTPUT)
pin(STATUSLED_RFM12_RX, PB3, OUTPUT)
RFM12_USE_INT(1)
RFM12_ASK_SENSE_USE_INT(1)
')

ifdef(`conf_ZBUS', `dnl
/* port config for zbus */
pin(ZBUS_RXTX_PIN, PD2)
')

ifdef(`conf_ECMD_SERIAL_USART_RS485', `dnl
pin(ECMD_SERIAL_USART_TX, PC2)
')

ifdef(`conf_RC5', `
pin(RC5_SEND, PD4)
')

ifdef(`conf_LTC1257', `
  pin(LTC1257_CLK, PA2, OUTPUT)
  pin(LTC1257_DATA, PA1, OUTPUT)
  pin(LTC1257_LOAD, PA0, OUTPUT)
')

ifdef(`conf_USTREAM', `
  pin(VS1053_CS, PB0, OUTPUT)
  pin(VS1053_DREQ, PB3, INPUT)
')

ifdef(`conf_FREQCOUNT',`dnl
 pin(FREQCOUNT_PIN, PD6, INPUT)
 pin(FREQCOUNT_CHANNEL_MULTIPLEX_BIT1, PD7, OUTPUT) dnl >1
 pin(FREQCOUNT_CHANNEL_MULTIPLEX_BIT2, PB0, OUTPUT) dnl >2
 pin(FREQCOUNT_CHANNEL_MULTIPLEX_BIT3, PB3, OUTPUT) dnl >4
 dnl pin(FREQCOUNT_CHANNEL_MULTIPLEX_CS_A, PB4, OUTPUT) dnl >8
 dnl pin(FREQCOUNT_CHANNEL_MULTIPLEX_CS_B, PB5, OUTPUT) dnl >8
')

ifelse(`conf_HD44780',`HD44780_DIRECTSUPPORT',`dnl
  pin(HD44780_RS, PD3)
  pin(HD44780_RW, PD2)
  pin(HD44780_EN1, PB0)
  pin(HD44780_D4, PD4)
  pin(HD44780_D5, PD5)
  pin(HD44780_D6, PD6)
  pin(HD44780_D7, PD7)
  ifdef(`conf_HD44780_BACKLIGHT', `
    pin(HD44780_BL, PB1, OUTPUT)
  ')
')


ifelse(value_HD44780_CONNECTION,`HD44780_I2CSUPPORT',`dnl
 dnl HD44780_PCF8574x_MAPPING(ADR,RS,RW,EN,DB4,DB5,DB6,DB7,BL)
 HD44780_PCF8574x_MAPPING(0x27, 0, 1, 2, 4, 5, 6, 7, 3)
')dnl

ifdef(`conf_WS2801', `
	pin(WS2801_CLOCK, PD2, OUTPUT)
	pin(WS2801_DATA, PD3, OUTPUT)
')

ifdef(`conf_BUTTONS_INPUT', `
  pin(BTN_GARAGE, PA0, INPUT)		dnl 0
  pin(BTN_TERRASSE_RECHTS, PA1, INPUT)  dnl 1
  pin(BTN_TERRASSE_LINKS, PA2, INPUT)	dnl 2


  #define BUTTONS_COUNT 3

  #define BUTTONS_CONFIG(_x) \
  _x(BTN_GARAGE)\
  _x(BTN_TERRASSE_RECHTS)\
  _x(BTN_TERRASSE_LINKS)


')


