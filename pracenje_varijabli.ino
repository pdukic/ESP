#include <Wire.h>
#include <Adafruit_SHT31.h>
#include <SparkFun_DPS310.h>
#include <Adafruit_SSD1306.h>
#include <CapacitiveSensor.h>

// Senzori
Adafruit_SHT31 sht31 = Adafruit_SHT31();  // Senzor za temperaturu i vlagu
DPS310 pressureSensor;  // Senzor za tlak
Adafruit_SSD1306 display(128, 64, &Wire, -1);  // OLED zaslon

// Kapacitivni senzori (priključeni na analogne pinove)
CapacitiveSensor touchSensor1 = CapacitiveSensor(4, 2);  // Pinovi za odabir prikaza
CapacitiveSensor touchSensor2 = CapacitiveSensor(4, 3);  
CapacitiveSensor touchSensor3 = CapacitiveSensor(4, 4);

// Varijable za pohranu trenutnih podataka
float temperature, humidity, pressure;
float tempMin = 1000.0, tempMax = -1000.0;
float humMin = 100.0, humMax = -100.0;
float pressMin = 1000.0, pressMax = -1000.0;
float tempAvg = 0.0, humAvg = 0.0, pressAvg = 0.0;
unsigned long readingsCount = 0;  // Broj očitanja

// Varijable za odabir prikaza
int displayMode = 0;  // 0 = Trenutne vrijednosti, 1 = Statistika, 2 = Reset

void setup() {
  Serial.begin(115200);

  // Inicijalizacija senzora
  if (!sht31.begin(0x44)) {
    Serial.println("SHT35 senzor nije pronađen!");
    while (1);
  }
  
  if (!pressureSensor.begin()) {
    Serial.println("DPS310 senzor nije pronađen!");
    while (1);
  }

  // Inicijalizacija OLED zaslona
  if (!display.begin(SSD1306_I2C_ADDRESS, OLED_RESET)) {
    Serial.println(F("Ne mogu inicijalizirati OLED zaslon"));
    while (1);
  }
  display.display();
  delay(2000); // Pričekaj da OLED zaslon pokrene
}

void loop() {
  checkTouchSensors();
  readSensors();
  displayData();
  delay(1000);  // Očitaj senzore i osvježi zaslon svake sekunde
}

// Funkcija za praćenje dodira i promjenu prikaza
void checkTouchSensors() {
  long sensor1Value = touchSensor1.capacitiveSensor(30);
  long sensor2Value = touchSensor2.capacitiveSensor(30);
  long sensor3Value = touchSensor3.capacitiveSensor(30);

  // Ako je prvi senzor aktiviran (odabir trenutnih vrijednosti)
  if (sensor1Value > 1000) {
    displayMode = 0;
    delay(300);  // Debounce delay
  }

  // Ako je drugi senzor aktiviran (odabir statistike)
  if (sensor2Value > 1000) {
    displayMode = 1;
    delay(300);  // Debounce delay
  }

  // Ako je treći senzor aktiviran (resetiranje podataka)
  if (sensor3Value > 1000) {
    resetStatistics();
    displayMode = 2;
    delay(300);  // Debounce delay
  }
}

// Funkcija za očitanja i statistiku
void readSensors() {
  temperature = sht31.readTemperature();
  humidity = sht31.readHumidity();
  pressure = pressureSensor.readPressure();

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

// Funkcija za resetiranje statistike
void resetStatistics() {
  tempMin = 1000.0;
  tempMax = -1000.0;
  humMin = 100.0;
  humMax = -100.0;
  pressMin = 1000.0;
  pressMax = -1000.0;
  tempAvg = 0.0;
  humAvg = 0.0;
  pressAvg = 0.0;
  readingsCount = 0;
}

// Funkcija za prikaz podataka na OLED zaslonu
void displayData() {
  display.clearDisplay();
  
  if (displayMode == 0) {
    // Trenutne vrijednosti
    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temperature);
    display.println(" C");

    display.print("Humidity: ");
    display.print(humidity);
    display.println(" %");

    display.print("Pressure: ");
    display.print(pressure);
    display.println(" hPa");
  } else if (displayMode == 1) {
    // Statistika
    display.setCursor(0, 0);
    display.print("Temp: Min ");
    display.print(tempMin);
    display.print(" Max ");
    display.print(tempMax);
    display.print(" Avg ");
    display.println(tempAvg);

    display.print("Humidity: Min ");
    display.print(humMin);
    display.print(" Max ");
    display.print(humMax);
    display.print(" Avg ");
    display.println(humAvg);

    display.print("Pressure: Min ");
    display.print(pressMin);
    display.print(" Max ");
    display.print(pressMax);
    display.print(" Avg ");
    display.println(pressAvg);
  } else if (displayMode == 2) {
    // Reset
    display.setCursor(0, 0);
    display.println("Data Reset");
  }

  display.display();
}
