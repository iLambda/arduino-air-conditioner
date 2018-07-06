#include <LiquidCrystal.h>
#include <OneWire.h>

#define CRITICAL_THRESH   7
#define OK_THRESH         18     

#define TEMPERATURE_UNDEFINED -32768

const int RS_Pin = 2;
const int E_Pin = 3;
const int D4_Pin = 4;
const int D5_Pin = 5;
const int D6_Pin = 6;
const int D7_Pin = 7;
const int TEMP_Pin = 12;

const int USELESSLED_Pin = A2;
const int CRITLED_Pin = A1;
const int OKLED_Pin = A0;

const byte CHAR_DELTA[8] =
{
  B00100,
  B00100,
  B01010,
  B01010,
  B10001,
  B10001,
  B11111,
  B00000
};

int internalTemperature = TEMPERATURE_UNDEFINED;
int externalTemperature = TEMPERATURE_UNDEFINED;
const byte INTERNAL_THERMOMETER[] = { 0x28, 0x73, 0xA6, 0xA2, 0x0A, 0x00, 0x00, 0xB4 }; // edit with right 1-wire address
const byte EXTERNAL_THERMOMETER[] = { 0x28, 0xAA, 0x39, 0xA3, 0x0A, 0x00, 0x00, 0xB1 };

LiquidCrystal lcd(RS_Pin, E_Pin, D4_Pin, D5_Pin, D6_Pin, D7_Pin);
OneWire thermometer(TEMP_Pin);

enum INDICATOR_STATE {
  INDICATOR_USELESS,
  INDICATOR_CRITICAL,
  INDICATOR_OK,
  INDICATOR_OFF
};

void setup() {
  // initialize serial
  Serial.begin(115200);

  // set led pins as OUTPUT
  pinMode(USELESSLED_Pin, OUTPUT);
  pinMode(CRITLED_Pin, OUTPUT);
  pinMode(OKLED_Pin, OUTPUT);
  // set their default value
  setIndicator(INDICATOR_OFF);
  
  // initialize lcd
  lcd.begin(16, 2);
  lcd.clear();

  // create special character
  lcd.createChar(0, CHAR_DELTA);
  
  // write UI text
  lcd.setCursor(0, 0);
  lcd.print("EXT=" + formatTemperature(externalTemperature));
  lcd.setCursor(0, 1);
  lcd.print("INT=" + formatTemperature(internalTemperature));
  
  lcd.setCursor(9, 0);
  lcd.write(byte(0));
  lcd.print("T=" + formatTemperature(0xFFFF));

  internalTemperature = -12;
}

void loop() {
byte address[8];
   
  // get temperature
  internalTemperature = (int)getTemperature(INTERNAL_THERMOMETER);
  externalTemperature = (int)getTemperature(EXTERNAL_THERMOMETER);
  
  // check delta temperature and set indicator
  int deltaTemp = externalTemperature - internalTemperature;
  if (deltaTemp >= OK_THRESH) {
    setIndicator(INDICATOR_OK);
  } else if (deltaTemp >= CRITICAL_THRESH) {
    setIndicator(INDICATOR_CRITICAL);    
  } else {
    setIndicator(INDICATOR_USELESS);    
  }
  
  // wait
  delay(800);
  // write top temp
  lcd.setCursor(4, 0);
  lcd.print(formatTemperature(externalTemperature));
  // write bottom temp
  lcd.setCursor(4, 1);
  lcd.print(formatTemperature(internalTemperature));
  // write delta
  lcd.setCursor(12, 0);
  lcd.print(formatTemperature(deltaTemp));
  
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
  text += (temperature == TEMPERATURE_UNDEFINED ? "" : (temperature > 0 ? "+" : (temperature < 0 ? "-" : "")));
  // write temperature
  text += (temperature == TEMPERATURE_UNDEFINED ? "??" : String(abs(temperature)));
  // write unit
  text += ((char)223);
  //text += "C";
  // pad, length should be 5
  while (text.length() < 5) {
    text += " ";
  }
  // return
  return text;
}

void setIndicator(INDICATOR_STATE state) {
  // set leds
  digitalWrite(USELESSLED_Pin, state == INDICATOR_USELESS);
  digitalWrite(CRITLED_Pin, state == INDICATOR_CRITICAL);
  digitalWrite(OKLED_Pin, state == INDICATOR_OK);
}

