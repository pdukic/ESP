#include "header.h"

float temp;
float press;
float hum;
uint8_t oversampling = 7;
int16_t ret;
String msg = ""; //priprema poruke koja ce se poslati na serial i u buffer za OLED (SSD1306)
Preferences prefs;

float avgTemp = 0.0;
float avgHum = 0.0;
float avgPress = 0.0;
int brojMjerenja = 0;


void setup() {
  Serial.begin(115200);

  Alarms alarms;
  prefs.begin("Veleri-OI-meteo", false);
  bool hasPrefs = prefs.getBool("valid", false);

  if (hasPrefs){
    Serial.println("Saved settings found. ");
    float tmp;
    // Read settings into a temporary variable and store if no error.
    // In case of missing key-value pair or other error, getFloat returns NaN
    tmp = prefs.getFloat("humidity_low");
    if (!isnan(tmp)) alarms.humidity_low = tmp;
    tmp = prefs.getFloat("humidity_high");
    if (!isnan(tmp)) alarms.humidity_high = tmp;
    tmp = prefs.getFloat("temperature_low");
    if (!isnan(tmp)) alarms.temperature_low = tmp;
    tmp = prefs.getFloat("temperature_high");
    if (!isnan(tmp)) alarms.temperature_high = tmp;
  }
  else{
    Serial.println("Saved settings not found, using default values."); 
    prefs.putFloat("humidity_low", alarms.humidity_low); 
    prefs.putFloat("humidity_high", alarms.humidity_high); 
    prefs.putFloat("temperature_low", alarms.temperature_low); 
    prefs.putFloat("temperature_high", alarms.temperature_high); 
    prefs.putBool("valid", true);
  }
  Serial.println("Configured alarms: ");
  Serial.print("Min. temp.:");
  Serial.println(alarms.temperature_low);
  Serial.print("Max. temp.:");
  Serial.println(alarms.temperature_high);
  Serial.print("Min. humidity.:");
  Serial.println(alarms.humidity_low);
  Serial.print("Max. humidity.:");
  Serial.println(alarms.humidity_high);

  select_channel_i2c(OLED_IIC_CH);
  u8g2.begin();
  u8g2.setFont(u8g2_font_ncenB08_tr);//8px font

  select_channel_i2c(DPS310_IIC_CH);
  Dps3xxPressureSensor.begin(Wire);

  select_channel_i2c(SHT35_IIC_CH);
  if (sensor.init())
    Serial.println("SHT35 init failed!");

  delay(1000);
  Serial.println("Init complete!");

  if (sensor.init())
Serial.println("Sensor init failed!");
if (!SPIFFS.begin(FORMAT_SPIFFS_IF_FAILED)) {
Serial.println("SPIFFS Mount Failed");
return;
}
delay(1000);
displayFile(SPIFFS, u8g2, LOGFILE);
  


  /*  if(!LittleFS.begin()){
    Serial.println("An Error has occurred while mounting LittleFS");
    return;}
  
  
   File file = LittleFS.open("/test.txt", "r");
    if(!file){
    Serial.println("Failed to open file for reading");
    return;}
  
  Serial.println("File Content:");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();}
*/


}






void loop() {

  uint8_t oversampling = 7;
  int16_t ret;
  String msg = ""; //priprema poruke koja ce se poslati na serial i u buffer za OLED (SSD1306)

  Serial.println();

  u8g2.clearBuffer(); // pocisti buffer koji sadrzi ono sto ce se prikazati na OLED. Buffer je u RAM MCU-a.






  mjerenjeDPS();
  mjerenjeSHT();


// kad imam sve poruke u bufferu - cijeli izgled ekrana u bufferu, posalji na OLED (posalji SSD1306 MCU-u)
  select_channel_i2c(OLED_IIC_CH);//odaberi na mux-u kanal
  u8g2.sendBuffer();//posalji na SSD1306

  // Wait some time
  delay(1000);
}




void select_channel_i2c(uint8_t channel) {
  Wire.beginTransmission(TCA9548A_DEF_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}


void mjerenjeDPS(){


  select_channel_i2c(DPS310_IIC_CH);
  /*
   * lets the Dps3xx perform a Single temperature measurement with the last (or standard) configuration
   * The result will be written to the parameter temperature
   * ret = Dps3xxPressureSensor.measureTempOnce(temperature);
   * the commented line below does exactly the same as the one above, but you can also config the precision
   * oversampling can be a value from 0 to 7
   * the Dps 3xx will perform 2^oversampling internal temperature measurements and combine them to one result with higher precision
   * measurements with higher precision take more time, consult datasheet for more information
   */
  ret = Dps3xxPressureSensor.measureTempOnce(temp, oversampling); //mjeri temperaturu, oversampling - uzmi vise uzoraka (2^n, n=0..7)

  if (ret != 0) {
    msg = "Temp@DPS FAIL! ret = " + String(ret);
  } else {
    msg = "Temp@DPS: " + String(temp) + " *C"; //napravi poruku
  }
  Serial.println(msg);
  u8g2.drawStr(0, OLED_ROW_0, msg.c_str()); // stavi poruku u buffer, ovo je jos u RAM MCU-a


  ret = Dps3xxPressureSensor.measurePressureOnce(press, oversampling);
  if (ret != 0) {

    msg = "Pressure read FAIL! ret = " + String(ret);
  } else {
    msg = "Pressure: " + String(press / 100) + " hPa";
  }
  Serial.println(msg);
  u8g2.drawStr(0, OLED_ROW_1, msg.c_str());


}


void mjerenjeSHT(){


  select_channel_i2c(SHT35_IIC_CH);
  if (NO_ERROR == sensor.read_meas_data_single_shot(HIGH_REP_WITH_STRCH, &temp, &hum)) {
    msg = "Temp@SHT: " + String(temp) + " *C ";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    msg = "Humidity: " + String(hum) + " % ";
    u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
    Serial.println(msg);
  } else {
    msg = "Temp@SHT35 FAIL!";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());
    msg = "Humid@SHT35 FAIL!";
    Serial.println(msg);
    u8g2.drawStr(0, OLED_ROW_3, msg.c_str());
  }
}



void ispisProsjeka() {

  float sumaTemp=0.0;
  float sumaHum=0.0;
  float sumaPress=0.0; 
  int brojac=0;

  const long zadaniInterval=60000;
  unsigned long prethodniInterval=0;
  

 unsigned long interval=millis();
  
    if(interval - prethodniInterval >= zadaniInterval){
    
  prethodniInterval=interval;

 float  temperature = sht31.readTemperature();

 float  humidity = sht31.readHumidity();

 float pressure = pressureSensor.readPressure();

  sumaTemp+=temperature;
  sumaHum+=humidity;
  sumaPress+=pressure;

 brojac++;

  float tempAvg=sumaTemp/(float)brojac;
  float humAvg=sumaHum/(float)brojac;
  float pressAvg=sumaPress/(float)brojac;
  
  Serial.print("Avg. temp.:");
  Serial.println(tempAvg);
  Serial.print("Avg. humidity.:");
  Serial.println(humAvg);
  Serial.print("Avg pressure:");
  Serial.println(pressAvg);

  msg = "TempAVG: " + String(tempAvg) + " *C ";
  u8g2.drawStr(0, OLED_ROW_0, msg.c_str());
  msg = "HumAVG: " + String(humAvg) + " % ";
    u8g2.drawStr(0, OLED_ROW_1, msg.c_str());
    msg = "PressAVG: " + String(pressAvg/100) + " hPa ";
    u8g2.drawStr(0, OLED_ROW_2, msg.c_str());



  }


}

/*
Appends /message/ to file /path/ on filesystem /fs/, creating if necessary.
*/
void appendFile(fs::FS &fs, const char * path, const char * message) {
File file = fs.open(path, FILE_APPEND);
if (!file) {
Serial.print("File ");
Serial.print(path);
Serial.println(" not found. Attempting to create new file.");
file = fs.open(path, FILE_WRITE);
if (!file) {
Serial.print("Unable to create file ");
Serial.println(path);
return;
}
Serial.println("File created.");
}
if (!file.print(message)) {
Serial.print("Failed to append to file ");
Serial.println(path);
}
file.close();
}

/*
Reads file /path/ on filesystem /fs/ and sends it over serial port
and displays on screen.
*/
void displayFile(fs::FS &fs, U8G2& disp, const char * path) {
Serial.printf("Reading file: %s\r\n", path);
File file = fs.open(path);
if (!file || file.isDirectory()) {
Serial.println("Failed to open file for reading!");
return;
}
String buffer;
while (file.available()) {
char next = file.read();
buffer += next;
if (next == '\n'){
Serial.print(buffer);
disp.clearBuffer();
disp.setFont(u8g2_font_profont12_tr);
disp.drawStr(0, 10, "Log file:");
disp.drawStr(0, 28, buffer.c_str());
disp.sendBuffer();
buffer = "";
}
}
file.close();
}

/*
Reads file /path/ on filesystem /fs/ and sends it over serial port
and displays on screen.
*/
void displayFile(fs::FS &fs, U8G2& disp, const char * path) {
Serial.printf("Reading file: %s\r\n", path);
File file = fs.open(path);
if (!file || file.isDirectory()) {
Serial.println("Failed to open file for reading!");
return;
}
String buffer;
while (file.available()) {
char next = file.read();
buffer += next;
if (next == '\n'){
Serial.print(buffer);
disp.clearBuffer();
disp.setFont(u8g2_font_profont12_tr);
disp.drawStr(0, 10, "Log file:");
disp.drawStr(0, 28, buffer.c_str());
disp.sendBuffer();
buffer = "";
}
}
file.close();
}






/*
  if (!isnan(temperature) && !isnan(humidity) && !isnan(pressure)) {
    // Ažuriraj minimum, maksimum i prosjek
    if (temperature < tempMin) tempMin = temperature;
    if (temperature > tempMax) tempMax = temperature;
    tempAvg = (tempAvg * readingsCount + temperature) / (readingsCount + 1);

    if (humidity < humMin) humMin = humidity;
    if (humidity > humMax) humMax = humidity;
    humAvg = (humAvg * readingsCount + humidity) / (readingsCount + 1);

    if (pressure < pressMin) pressMin = pressure;
    if (pressure > pressMax) pressMax = pressure;
    pressAvg = (pressAvg * readingsCount + pressure) / (readingsCount + 1);

    readingsCount++;
  }
 }
 */