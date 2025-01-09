#include "Seeed_SHT35.h"
const unsigned char SCLPIN = 22;
const unsigned char SHT35_IIC_ADDR = 0x112;
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
void setup() {
Serial.begin(115200);
if (sensor.init())
Serial.println("Sensor init failed!");
delay(1000);
}
void loop() {
float temp;
float hum;
if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)) {
Serial.print("temperature = ");
Serial.print(temp);
Serial.println(" °C ");
Serial.print("humidity = ");
Serial.print(hum);
Serial.println(" % ");
Serial.println("");
} else {
Serial.println("Sensor read failed!");
Serial.println("");
}
delay(1000);
}
