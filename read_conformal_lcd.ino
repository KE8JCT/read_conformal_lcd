/* Conformable Wearable Battery Display Screen
 *  CWB read script by Dan Miliken
 *  LCD progress bar script found here: https://www.electronicsblog.net/arduino-lcd-horizontal-progress-bar-using-custom-characters/
 *  Implemented/Slightly changed by nolan pearce
 *  Uses Inventus Power CWB, Arduino uno, 
 *  Wiring for LCD found here: https://www.hacktronics.com/Tutorials/arduino-character-lcd-tutorial.html
 *  CONNECT GREEN WIRE - DATA - A4/SDA
 *  CONNECT WHITE WIRE - CLOCK - A5/SCL
 *  CONNECT GND - GND
 *  PULLUP RESISTORS 4.7K OHM TO +5V ON DAT AND CLK
 *  
 */

#include "Wire.h"

#define bl03_address 0x0b

#define RW_DELAY_USEC   250

#include <LiquidCrystal.h>

LiquidCrystal lcd(12, 11, 10, 5, 4, 3, 2);
int backLight = 13;  

int currentSOC = 0;

void setup()
{
  pinMode(backLight, OUTPUT);
  digitalWrite(backLight, HIGH); 
  Wire.begin();
  delayMicroseconds(RW_DELAY_USEC);

  Serial.begin(9600);
  lcd.begin(16, 2);
  lcd.print("hello, world!");
  delay(500);
  lcd.clear();
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
}

void loop()
{
  int newSOC = getSOC();

  if(currentSOC != newSOC || true)
  {
    currentSOC = newSOC;

    Serial.println("Battery Info");
    Serial.println("------------------------");
    
    Serial.print("SOC: ");
    Serial.print(currentSOC);
    Serial.println("%");
  
    Serial.print("Voltage: ");
    Serial.println(getvoltage());
    LCD_progress_bar(2, currentSOC, 0, 100);
    lcd.setCursor(0,0);
    lcd.print(getvoltage());
    lcd.print("V , ");
    lcd.print(currentSOC);
    lcd.print(" % ");
    delay(500);
    Serial.println();
    Serial.println();
  }
  
  delay(1000);
}
void LCD_progress_bar (int row, int var, int minVal, int maxVal)
{
  int block = map(var, minVal, maxVal, 0, 16);   // Block represent the current LCD space (modify the map setting to fit your LCD)
  int line = map(var, minVal, maxVal, 0, 80);     // Line represent the theoretical lines that should be printed
  int bar = (line-(block*5));                             // Bar represent the actual lines that will be printed
 
  /* LCD Progress Bar Characters, create your custom bars */

  byte bar1[8] = { 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10, 0x10};
  byte bar2[8] = { 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18};
  byte bar3[8] = { 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C, 0x1C};
  byte bar4[8] = { 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E};
  byte bar5[8] = { 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F, 0x1F};
  lcd.createChar(1, bar1);
  lcd.createChar(2, bar2);
  lcd.createChar(3, bar3);
  lcd.createChar(4, bar4);
  lcd.createChar(5, bar5);
 
  for (int x = 0; x < block; x++)                        // Print all the filled blocks
  {
    lcd.setCursor (x, row);
    lcd.write (1023);
  }
 
  lcd.setCursor (block, row);                            // Set the cursor at the current block and print the numbers of line needed
  if (bar != 0) lcd.write (bar);
  if (block == 0 && line == 0) lcd.write (1022);   // Unless there is nothing to print, in this case show blank
 
  for (int x = 16; x > block; x--)                       // Print all the blank blocks
  {
    lcd.setCursor (x, row);
    lcd.write (1022);
  }
}
