#include "Wire.h"
#include <SoftwareSerial.h>

#define bl03_address 0x0b

#define RW_DELAY_USEC   250

int currentSOC = 0;
const int softwareTx = 8;
const int softwareRx = 7;
String stringSOC = "";

SoftwareSerial s7s(softwareRx, softwareTx);

unsigned int counter = 0;  // This variable will count up to 65k
char tempString[10];  // Will be used with sprintf to create strings

void setup()
{
  Wire.begin();
  delayMicroseconds(RW_DELAY_USEC);

  Serial.begin(9600);
  // Must begin s7s software serial at the correct baud rate.
  //  The default of the s7s is 9600.
  s7s.begin(9600);

  // Clear the display, and then turn on all segments and decimals
  clearDisplay();  // Clears display, resets cursor
  s7s.print("-HI-");  // Displays -HI- on all digits
  setDecimals(0b111111);  // Turn on all decimals, colon, apos

  // Flash brightness values at the beginning
  setBrightness(0);  // Lowest brightness
  delay(1500);
  setBrightness(127);  // Medium brightness
  delay(1500);
  setBrightness(255);  // High brightness
  delay(1500);

  // Clear the display before jumping into loop
  clearDisplay();  
  
int currentSOC = 0;
setDecimals(2);
}

void checkErrorCode()
{
  byte errorcode = Wire.endTransmission(bl03_address);
  Serial.print("Errorcode="); Serial.println(errorcode);
}

double getvoltage()
{
  Wire.beginTransmission(bl03_address);
  char cmd[] = { 0x09, 0x00, 0x00 };
  size_t cmdArraySize = sizeof(cmd); 
  size_t bytesWritten = Wire.write(cmd, cmdArraySize);
  byte errorcode = Wire.endTransmission(bl03_address);

  delayMicroseconds(RW_DELAY_USEC);

  size_t byteCount = 2;
  size_t currentByte = 0;
  byte data[byteCount];
  Wire.requestFrom(bl03_address, byteCount); // request 2 bytes
  while(Wire.available())    // slave may send less than requested
  { 
    data[currentByte] = Wire.read(); // receive the byte
    currentByte++;
  }

  double c = (data[1] * 256) + data[0];
  c = c / 1000;

  delayMicroseconds(RW_DELAY_USEC);
  
  return c;
}

int getSOC()
{
  Wire.beginTransmission(bl03_address);
  char cmd[] = { 0x0d, 0x00, 0x00 };
  size_t cmdArraySize = sizeof(cmd); 
  size_t bytesWritten = Wire.write(cmd, cmdArraySize);
  byte errorcode = Wire.endTransmission(bl03_address);

//  Serial.print("Wrote ");
//  Serial.print(bytesWritten);
//  Serial.print(" bytes. Errorcode=");
//  Serial.println(errorcode);

  delayMicroseconds(RW_DELAY_USEC);

  size_t byteCount = 2;
  size_t currentByte = 0;
  byte data[byteCount];
  Wire.requestFrom(bl03_address, byteCount); // request 2 bytes
  while(Wire.available())    // slave may send less than requested
  { 
    data[currentByte] = Wire.read(); // receive the byte
    currentByte++;
//    Serial.println(data[currentByte], HEX);
  }

  int c = data[0];

  delayMicroseconds(RW_DELAY_USEC);
  
  return c;
}

void gettemperature()
{
  Wire.beginTransmission(bl03_address);
  Wire.write(0x08);
  byte one = 0, second = 0;
  Wire.requestFrom(bl03_address, 2);
  one = Wire.read(); //should get 97; got 74
  second = Wire.read(); //should get B; got 30
  Serial.println(one, HEX);
  Serial.println(second, HEX);
  int c = (second * 256) + one;
  Serial.print("Temp: ");
  Serial.println(c);
//  Serial.println(((c / 10) - 273.15), 2);
  int errorcode = Wire.endTransmission(bl03_address);
  Serial.print("Errorcode="); Serial.println(errorcode);
  //setDecimals(4);
}

void loop()
{
  int newSOC = getSOC();

  if(currentSOC != newSOC || true)
  {
    currentSOC = newSOC;
    stringSOC = String(currentSOC) + "00";

    Serial.println("Battery Info");
    Serial.println("------------------------");
    
    Serial.print("SOC: ");
    Serial.print(currentSOC);
    Serial.println("%");
    //sprintf(currentSOC, "%2d");
    s7s.print(stringSOC);
  
    Serial.print("Voltage: ");
    Serial.println(getvoltage());
  
    Serial.println();
    Serial.println();
  }
  //sprintf(getSOC, "%4d", counter);

  delay(1000);
}

void clearDisplay()
{
  s7s.write(0x76);  // Clear display command
}

// Set the displays brightness. Should receive byte with the value
//  to set the brightness to
//  dimmest------------->brightest
//     0--------127--------255
void setBrightness(byte value)
{
  s7s.write(0x7A);  // Set brightness command byte
  s7s.write(value);  // brightness data byte
}

// Turn on any, none, or all of the decimals.
//  The six lowest bits in the decimals parameter sets a decimal 
//  (or colon, or apostrophe) on or off. A 1 indicates on, 0 off.
//  [MSB] (X)(X)(Apos)(Colon)(Digit 4)(Digit 3)(Digit2)(Digit1)
void setDecimals(byte decimals)
{
  s7s.write(0x77);
  s7s.write(decimals);
}
