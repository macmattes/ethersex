static const char sensor_name_avrnetio11_DHT0[] PROGMEM = "avrnetio11_DHT0";
static const char sensor_name_avrnetio11_DHT1[] PROGMEM = "avrnetio11_DHT1";

dht_sensor_t dht_sensors[] = {
	{
		.port = &PORTA,
		.pin = PA6,
		.name = sensor_name_avrnetio11_DHT0
	},
	{
		.port = &PORTA,
		.pin = PA7,
		.name = sensor_name_avrnetio11_DHT1
	}
};

