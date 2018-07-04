#include <LiquidCrystal.h>
#include <OneWire.h>

const int RS_Pin = 8;
const int E_Pin = 10;
const int D4_Pin = 44;
const int D5_Pin = 46;
const int D6_Pin = 48;
const int D7_Pin = 50;
const int TEMP_Pin = 12;

int internalTemperature = 0xFFFF;
int externalTemperature = 0xFFFF;
const byte INTERNAL_THERMOMETER[] = { 0x28, 0x9E, 0x9C, 0x1F, 0x00, 0x00, 0x80, 0x04 };
const byte EXTERNAL_THERMOMETER[] = { 0x28, 0x9E, 0x9C, 0x1F, 0x00, 0x00, 0x80, 0x05 };

LiquidCrystal lcd(RS_Pin, E_Pin, D4_Pin, D5_Pin, D6_Pin, D7_Pin);
OneWire thermometer(TEMP_Pin);

void setup() {
  // initialize serial
  Serial.begin(115200);
  
  // initialize lcd
  lcd.begin(16, 2);
  lcd.clear();
  // write UI text
  lcd.setCursor(0, 0);
  lcd.print("EXTERNAL: " + formatTemperature(externalTemperature));
  lcd.setCursor(0, 1);
  lcd.print("INTERNAL: " + formatTemperature(internalTemperature));
}

void loop() {
  // get temperature
  internalTemperature = (int)getTemperature(INTERNAL_THERMOMETER);
  externalTemperature = (int)getTemperature(EXTERNAL_THERMOMETER);
  
  // wait
  delay(800);
  // write top temp
  lcd.setCursor(10, 0);
  lcd.print(formatTemperature(externalTemperature));
  // write bottom temp
  lcd.setCursor(10, 1);
  lcd.print(formatTemperature(internalTemperature));
  
}

float getTemperature(const byte addr[]) {
  // the incoming data
  byte data[9];
  
  // resets the 1-wire bus, selects the sensor
  thermometer.reset();
  thermometer.select(addr);
  // asks for a measure and waits for the end of it
  thermometer.write(0x44, 1);
  delay(800);
  // resets the bus, select the sensor and asks for a data read
  thermometer.reset();
  thermometer.select(addr);
  thermometer.write(0xBE);
  // reading the data
  for (byte i = 0; i < 9; i++) {
    data[i] = thermometer.read();
  }
  // computing temperature
  return (int16_t) ((data[1] << 8) | data[0]) * 0.0625; 
}

String formatTemperature(int temperature) {
  // create string
  String text;
  // write sign
  text += (temperature == 0xFFFF ? "" : (temperature > 0 ? "+" : (temperature < 0 ? "-" : "")));
  // write temperature
  text += (temperature == 0xFFFF ? "??" : String(abs(temperature)));
  // write unit
  text += ((char)223);
  text += "C";
  // pad, length should be 5
  while (text.length() < 5) {
    text += " ";
  }
  // return
  return text;
}

