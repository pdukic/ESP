#include "Seeed_SHT35.h"
#include <U8g2lib.h>
#include <Wire.h>
const unsigned char SCLPIN = 22;
const unsigned char SHT35_IIC_ADDR = 0x45;
SHT35 sensor(SCLPIN, SHT35_IIC_ADDR);
// Init display:
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);
void setup()
{
Serial.begin(115200);
u8g2.begin();
u8g2.setFont(u8g2_font_ncenB08_tr);
if (sensor.init())
Serial.println("Sensor init failed!");
delay(1000);
}
void loop()
{
float temp;
float hum;
if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)){
u8g2.clearBuffer();
String msg = "Temperature: " + String(temp) + " C ";
u8g2.drawStr(0, 10, msg.c_str());
Serial.println(msg);
msg = "Humidity: " + String(hum) + " % ";
u8g2.drawStr(0, 28, msg.c_str());
Serial.println(msg);
u8g2.sendBuffer();
} else {
Serial.println("Sensor read failed!");
Serial.println("");
}
delay(1000);
}

