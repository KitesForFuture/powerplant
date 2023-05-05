float batteryVoltage = 2750.0;

void updateBatteryPercentage(){
	// BATTERY VOLTAGE (smoothed)
	batteryVoltage = 0.997*batteryVoltage + 0.003*(float)getSensor(BATTERY_VOLTAGE);
}

float getBatteryPercentage(){
	// calculate percentage from voltage
	return (batteryVoltage - 2550)*0.005;
}
