static const char sensor_name_pa6[] PROGMEM = "pa6";

dht_sensor_t dht_sensors[] = {
	{
		.port = &PORTA,
		.pin = PA6,
		.name = sensor_name_pa6
	}
};

