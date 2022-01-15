



//version 27 june 2019 
// should become a safe arduino version of w1209 thermostat
//
// but far from finished
// i think in this rotary encoder i2c oled 128x64 version all is working now
// should become a safe arduino version of w1209 thermostat
// but far from finished
// parts of code used from
// http://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
// https://github.com/tehniq3/DS18B20_thermostat_4digit_7segment_led_display/blob/master/4dig7segm_ac_18b20_thermostat_ver4m7.ino
// Robust Rotary encoder reading
// Copyright John Main - best-microcontroller-projects.com
// https://www.best-microcontroller-projects.com/rotary-encoder.html
//
// this code came from
// http://sticker.tk/forum/index.php?action=view&id=296
// copyright then, now, and forever, luberth dijkman bangert 30 andijk the netherlands
// bob-a-job
// a nickle or dime would be apreciated Http://paypal.me/LDijkman
// if you gave me 5 cents for every device you program with this code
// i will be verry happy
// it doesnt hurt
// Http://paypal.me/LDijkman
//
// I Feel so lonely in this Universe ;-(
// if you change the program to make it do more or better
// would like to know/share
// https://m.facebook.com/luberth.dijkman
// http://sticker.tk/forum/index.php?action=view&id=296


#include <EEPROM.h>
//#include <LiquidCrystal_I2C.h>                     // https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
//LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7);
#include <Arduino.h>
#include <U8x8lib.h>     //Using u8g2 library => search librarymanager for u8g2 and install
#include "pitches.h"     //https://www.arduino.cc/en/Tutorial/toneMelody


// die zien we nooit meer terug cocktail trio vlooiencircus 1965 https://www.youtube.com/watch?v=DizwZ0uOX5A
// notes in the melody:  //from #include "pitches.h"
int melody[] = {
  NOTE_C4, NOTE_G3, NOTE_G3, NOTE_A3, NOTE_G3, 0, NOTE_B3, NOTE_C4
};

// note durations: 4 = quarter note, 8 = eighth note, etc.:
int noteDurations[] = {
  4, 8, 8, 4, 4, 4, 4, 4
};


//U8X8_SSD1306_128X64_NONAME_SW_I2C u8x8(12, 11, U8X8_PIN_NONE);
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(U8X8_PIN_NONE);
float Vo;
float R1 = 10000;
float logR2, R2, T, Tc, Tf;
float c1 = 1.009249522e-03, c2 = 2.378405444e-04, c3 = 2.019202697e-07;
float CalibrationOffset = 0; //w1209 thermostat has -7 to +7 0.1 steps

float SwitchOnTemp = 30.0f;
byte TempByte;
float TempFloat;
int TempInt;
unsigned long TempLong;
byte val;
int RelaisState = 0;
byte CoolorHeat = 2; //1=cool 2=heat

int delayval;
float MaxTemp = 110;
float MinTemp = -40;
int LowTempAlarmVal = 0 ;
int HighTempAlarmVal = 100;

float relayonpointbelowsetpoint = -0.2;
float relayoffabovesetpoint = 0.1;

int menu = 0;
unsigned long starttime;
unsigned long timeon;

int MaxTimeRelayMayBeonInSeconds = 600;
int MaxTime2SetPoint = 300;

// Robust Rotary encoder reading
// Copyright John Main - best-microcontroller-projects.com
#define CLK 3
#define DATA 4
static uint8_t prevNextCode = 0;
static uint16_t store = 0;

int yesorno; //factory reset




void setup() {

  //Serial.begin(115200);
  Serial.begin(9600);

  //u8x8.begin(20, 4);
  // u8x8.setBacklightPin(3, POSITIVE);
  // u8x8.setBacklight(HIGH);
  u8x8.begin();
  u8x8.setPowerSave(0);
  u8x8.setFont(u8x8_font_pxplusibmcgathin_f);

  // first time i use eeprom, thought it would be very hard to use
  // i make steps of 5 in eeprom adres because i did not know what to use
  // 5 is a nice step when i look with the eeprom read example from arduino
  // not optimal use of eeprom but have eeprom more then i use

  /*
    // next is for test => normally commented
    // erase eeprom all to 0
        for (int i = 0 ; i < EEPROM.length() ; i++){EEPROM.put(i, 0);
                            //was eeprom.write but i think put is better   put only writes when val not the same
            u8x8.setCursor(2, 1);u8x8.print(i);u8x8.print(" Erase EEPROM");
        } // erase eeprom all to 0
       for (int i = 30 ; i > 0 ; i--){u8x8.setCursor(9, 3);u8x8.print(i);u8x8.print(" ");delay(500);}
    // erase eeprom all to 0
  */


  // first run ??? write some val to eeprom if value at eepromadres 666 not is 666
  //if this is first run then val will not be 666 at eeprom adres 666 so next will be run
  EEPROM.get(666, TempInt);
  if (TempInt != 666) {           // IF this is the first run THEN val at eeprom adres 666 is -1???
    EEPROM.put(0, 31.00);         // setpoint
    EEPROM.put(5, 0.00);          // callibration offset
    EEPROM.put(10, -0.3);         // below on
    EEPROM.put(15, 0.3);          // above off
    EEPROM.put(20, 600);          // max time in seconds relay on
    EEPROM.put(25, 2);            // 1 cool 2 heat
    EEPROM.put(30, 90);           // high temp alarm
    EEPROM.put(35, 20);           // low temp alarm
    EEPROM.put(40, 300);
    EEPROM.put(666, 666);        // set eepromadres 666 to val 666 no need to call / run this anymore in future
    u8x8.clear();
    u8x8.setCursor(0, 0);
    u8x8.print(F("Hi There! First run"));
    u8x8.setCursor(0, 1);
    u8x8.print(F("Have written value's"));
    u8x8.setCursor(0, 2);
    u8x8.print(F("to EEPROM "));
    u8x8.setCursor(0, 3);
    u8x8.print(F("Thanks for trying"));
    for (int i = 30 ; i > 0 ; i--) {
      u8x8.setCursor(17, 2);
      u8x8.print(i);
      u8x8.print(" ");
      delay(500);
    }
    u8x8.clear();
  }


  // is written in eeprom like this
  // 665 255
  // 666 154       2x256+154=666
  // 667 2
  // 668 255
  /*
    0 30
    1 0
    2 240
    3 65
    4 255
    5 0
    6 0
    7 128
    8 178
    9 255
    10  154
    11  153
    12  153
    13  190
    14  255
    15  154
    16  153
    17  153
    18  62
    19  255
    20  88
    21  2
    22  255
    23  255
    24  255
    25  255
    26  255
    27  255
    28  255
    29  255
    30  90
    31  0
    32  255
    33  255
    34  255
    35  20
    36  0
    37  255
    38  255
    39  255
    40  255
  */




  // read stored valeus from eeprom
  EEPROM.get(0, SwitchOnTemp);
  EEPROM.get(5, CalibrationOffset);
  EEPROM.get(10, relayonpointbelowsetpoint);
  EEPROM.get(15, relayoffabovesetpoint);
  EEPROM.get(20, MaxTimeRelayMayBeonInSeconds);
  EEPROM.get(25, CoolorHeat);
  EEPROM.get(30, HighTempAlarmVal);
  EEPROM.get(35, LowTempAlarmVal);
  EEPROM.get(40, MaxTime2SetPoint);

  // A0 NTC thermistor
  //  pinMode(2, INPUT_PULLUP);       // set button
  //  pinMode(3, INPUT_PULLUP);       // + button
  //  pinMode(4, INPUT_PULLUP);       // - button

  //rotary encoder
  pinMode(2, INPUT_PULLUP);         // D2 rotary encoder button
  pinMode(CLK, INPUT);              // D3
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);             // D4
  pinMode(DATA, INPUT_PULLUP);

  pinMode (8, OUTPUT);            // D8 passive buzzer module
  pinMode(10, OUTPUT);            // relais
  pinMode(11, OUTPUT);            // lowtemp alarm relais
  pinMode(12, OUTPUT);            // hightemp alarm relais
  pinMode(13, OUTPUT);            // another relais
}




void loop() {

  // int SetButton() = SetButton();
  // int PlusButton = PlusButton();
  // int MinButton = MinButton();

  if (!SetButton()) {                       //if !=not setbutton pressed
    menu = 1;
    while (SetButton() == LOW) {
      // loop until button released
      // maybe a timer here
      // alarm if button never released
      u8x8.setCursor(0, 0);
      u8x8.print(F("                "));
      u8x8.setCursor(0, 1);
      u8x8.print(F("  In Menu Now   "));
      u8x8.setCursor(0, 2);
      u8x8.print(F(" Release Button "));
      u8x8.setCursor(0, 3);
      u8x8.print(F("                "));
      u8x8.setCursor(0, 4);
      u8x8.print(F("                "));
    }
    u8x8.clear();
  }






  //1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
  //setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint
  TempLong = millis();  //reset innactive time counter
  if (menu == 1) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("1 SetTemp"));
    u8x8.setCursor(0, 1);
    u8x8.print(SwitchOnTemp);
  }
  while (menu == 1) {
    if ((millis() - TempLong)  > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      SwitchOnTemp = SwitchOnTemp + (rval / 10);
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(SwitchOnTemp);
      u8x8.print(F(" "));
      if (SwitchOnTemp > MaxTemp) SwitchOnTemp = MaxTemp;
      if (SwitchOnTemp < MinTemp) SwitchOnTemp = MinTemp;
    }

    if (!SetButton()) {                         //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 2;
      u8x8.clear();
      delay(250);
    }
  }


  EEPROM.get(0, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (SwitchOnTemp != TempFloat) {                  // only write to eeprom if value is different
    EEPROM.put(0, SwitchOnTemp);                    // put already checks if val is needed to write
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempFloat);
    u8x8.print(F(" new= "));
    u8x8.print(SwitchOnTemp);

    for (int i = 0; i < 100; i++)Serial.println(F("SwitchOnTemp DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }






  //2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
  //calibration calibration calibration calibration calibration calibration calibration calibration calibration calibration calibration
  TempLong = millis();  //reset innactive time counter
  if (menu == 2) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("2 Cal. Offset"));
    u8x8.setCursor(0, 1);
    u8x8.print(CalibrationOffset);
  }
  while (menu == 2) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }


    float rval;
    if ( rval = read_rotary() ) {
      CalibrationOffset = CalibrationOffset + (rval / 10);
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(CalibrationOffset);
      u8x8.print(F(" "));
      if (CalibrationOffset > 7)CalibrationOffset = 7;
      if (CalibrationOffset < -7)CalibrationOffset = -7;
    }

    if (!SetButton()) {                       //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 3;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(5, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (CalibrationOffset != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(5, CalibrationOffset);               // i have no idea wat eeprom adress to use just jump to 5
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempFloat);
    u8x8.print(F(" new= "));
    u8x8.print(CalibrationOffset);
    for (int i = 0; i < 100; i++)Serial.println(F("CalibrationOffset DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }







  //3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3
  //below set below set below set below set below set below set below set below set below set below set below set below set below set
  TempLong = millis();  //reset innactive time counter
  if (menu == 3) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("3 Below Set"));
    u8x8.setCursor(0, 1);
    u8x8.print(relayonpointbelowsetpoint);
  }
  while (menu == 3) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      relayonpointbelowsetpoint  = relayonpointbelowsetpoint  + (rval / 10);
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(relayonpointbelowsetpoint);
      u8x8.print(F(" "));
      if (relayonpointbelowsetpoint > -0.1)relayonpointbelowsetpoint = -0.1;
      if (relayonpointbelowsetpoint < -2)relayonpointbelowsetpoint = -2;
    }

    if (!SetButton()) {                   //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 4;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(10, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (relayonpointbelowsetpoint != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(10, relayonpointbelowsetpoint);               // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempFloat);
    u8x8.print(F(" new= "));
    u8x8.print(relayonpointbelowsetpoint);
    for (int i = 0; i < 100; i++)Serial.println(F("relayonpointbelowsetpoint DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }






  //4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4
  //above set above set above set above set above set above set above set above set above set above set above set above set
  TempLong = millis();  //reset innactive time counter
  if (menu == 4) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("4 Above Set"));
    u8x8.setCursor(0, 1);
    u8x8.print(relayoffabovesetpoint);
  }
  while (menu == 4) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      relayoffabovesetpoint  = relayoffabovesetpoint  + (rval / 10);
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(relayoffabovesetpoint);
      u8x8.print(F(" "));
      if (relayoffabovesetpoint > 2)relayoffabovesetpoint = 2;
      if (relayoffabovesetpoint < 0)relayoffabovesetpoint = 0;
    }

    if (!SetButton()) {              //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 5;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(15, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (relayoffabovesetpoint != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(15, relayoffabovesetpoint);               // i have no idea wat eeprom adress to use just jump to 5
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempFloat);
    u8x8.print(F(" new= "));
    u8x8.print(relayoffabovesetpoint);
    for (int i = 0; i < 100; i++)Serial.println(F("relayoffabovesetpoint DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }





  //5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-5-
  TempLong = millis();  //reset innactive time counter
  if (menu == 5) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("5 MaxT 2SetPoint"));
    u8x8.setCursor(0, 1);
    u8x8.print(MaxTime2SetPoint );
    u8x8.print(F(" "));
  }
  while (menu == 5) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      MaxTime2SetPoint  = MaxTime2SetPoint  + rval;
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(MaxTime2SetPoint );
      u8x8.print(F(" "));
      if (MaxTime2SetPoint > MaxTimeRelayMayBeonInSeconds)MaxTime2SetPoint = MaxTimeRelayMayBeonInSeconds;
      if (MaxTime2SetPoint < 0)MaxTime2SetPoint = 0;
    }

    if (!SetButton()) {
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 6;
      u8x8.clear();
      delay(250);
    }
  }


  EEPROM.get(40, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (MaxTime2SetPoint != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(40, MaxTime2SetPoint);               // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempInt);
    u8x8.print(F(" new= "));
    u8x8.print(MaxTime2SetPoint);
    for (int i = 0; i < 100; i++)Serial.println(F("MaxTime2SetPoint DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }







  //6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-6-
  //max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay
  TempLong = millis();  //reset innactive time counter
  if (menu == 6) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("6 Max T Relay on"));
    u8x8.setCursor(0, 1);
    u8x8.print(MaxTimeRelayMayBeonInSeconds);
  }
  while (menu == 6) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      MaxTimeRelayMayBeonInSeconds  = MaxTimeRelayMayBeonInSeconds  + rval;
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(MaxTimeRelayMayBeonInSeconds);
      u8x8.print(F(" "));
      if (MaxTimeRelayMayBeonInSeconds < 30)MaxTimeRelayMayBeonInSeconds = 30;
      if (MaxTimeRelayMayBeonInSeconds > 3600)MaxTimeRelayMayBeonInSeconds = 3600;
    }

    if (!SetButton()) {                //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 7;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(20, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (MaxTimeRelayMayBeonInSeconds != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(20, MaxTimeRelayMayBeonInSeconds);               // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempInt);
    u8x8.print(F(" new= "));
    u8x8.print(MaxTimeRelayMayBeonInSeconds);
    for (int i = 0; i < 100; i++)Serial.println(F("MaxTimeRelayMayBeonInSeconds DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }







  //7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-7-
  //CoolorHeat CoolorHeat CoolorHeat CoolorHeat CoolorHeat CoolorHeat
  TempLong = millis();  //reset innactive time counter
  if (menu == 7) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("7 Cool / Heat"));
    u8x8.setCursor(0, 1);
    if (CoolorHeat == 1)u8x8.print(F("    COOL    "));
    if (CoolorHeat == 2)u8x8.print(F("    HEAT    "));
  }
  while (menu == 7) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      CoolorHeat  = CoolorHeat  + rval;
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(CoolorHeat);
      u8x8.print(F(" "));
      if (CoolorHeat < 1)CoolorHeat = 2;
      if (CoolorHeat > 2)CoolorHeat = 1;
      u8x8.setCursor(0, 1);
      if (CoolorHeat == 1)u8x8.print(F("    COOL    "));
      if (CoolorHeat == 2)u8x8.print(F("    HEAT    "));
    }

    if (!SetButton()) { //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 8;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(25, TempByte);                         // limmited write to eeprom = read is unlimmited
  if (CoolorHeat != TempByte) {                     // only write to eeprom if value is different
    EEPROM.put(25, CoolorHeat);                     // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempByte);
    u8x8.print(F(" new= "));
    if (CoolorHeat == 1)u8x8.print(F("COOL"));
    if (CoolorHeat == 2)u8x8.print(F("HEAT"));
    for (int i = 0; i < 100; i++)Serial.println(F("Cool/Heat DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }






  //8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-8-
  TempLong = millis();  //reset innactive time counter
  if (menu == 8) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("8 Low Temp Alarm"));
    u8x8.setCursor(0, 1);
    u8x8.print(LowTempAlarmVal );
    u8x8.print(F(" "));
  }
  while (menu == 8) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      LowTempAlarmVal  = LowTempAlarmVal  + rval;
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(LowTempAlarmVal );
      u8x8.print(F(" "));
      if (LowTempAlarmVal > MaxTemp)LowTempAlarmVal = MaxTemp;
      if (LowTempAlarmVal < MinTemp)LowTempAlarmVal = MinTemp;
    }

    if (!SetButton()) {
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 9;
      u8x8.clear();
      delay(250);
    }
  }


  EEPROM.get(35, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (LowTempAlarmVal != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(35, LowTempAlarmVal);               // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempInt);
    u8x8.print(F(" new= "));
    u8x8.print(LowTempAlarmVal);
    for (int i = 0; i < 100; i++)Serial.println(F("LowTempAlarmVal DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }






  //9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-9-
  TempLong = millis();  //reset innactive time counter
  if (menu == 9) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("9 HighTemp Alarm"));
    u8x8.setCursor(0, 1);
    u8x8.print(HighTempAlarmVal );
  }
  while (menu == 9) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      HighTempAlarmVal  = HighTempAlarmVal  + rval;
      TempLong = millis();  //reset innactive time counter
      u8x8.setCursor(0, 1);
      u8x8.print(HighTempAlarmVal );
      u8x8.print(F(" "));
      if (HighTempAlarmVal > MaxTemp)HighTempAlarmVal = MaxTemp;
      if (HighTempAlarmVal < MinTemp)HighTempAlarmVal = MinTemp;
    }

    if (!SetButton()) {                     //if !=not setbutton pressed
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 10;
      u8x8.clear();
      delay(250);
    }
  }

  EEPROM.get(30, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (HighTempAlarmVal != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(30, HighTempAlarmVal);               // i have no idea wat eeprom adress to use just jump to 10
    u8x8.setCursor(0, 0);
    u8x8.print(F("Saving to EEPROM"));
    u8x8.setCursor(0, 2);
    u8x8.print(TempInt);
    u8x8.print(F(" new= "));
    u8x8.print(HighTempAlarmVal);
    for (int i = 0; i < 100; i++)Serial.println(F("HighTempAlarmVal DATA WRITEN / SAVED TO EEPROM "));
    u8x8.clear();
  }





  //10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10
  TempLong = millis();  //reset innactive time counter
  yesorno = 2;
  if (menu == 10) {
    u8x8.setCursor(0, 0);
    u8x8.print(F("10 Factory Reset"));
    u8x8.setCursor(6, 2);
    if (yesorno == 1)u8x8.print(F("YES"));
    if (yesorno == 2)u8x8.print(F("NO "));
  }
  while (menu == 10) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      yesorno  = yesorno  + rval;
      TempLong = millis();  //reset innactive time counter
      if (yesorno < 1)yesorno = 2;
      if (yesorno > 2)yesorno = 1;
      u8x8.setCursor(6, 2);
      if (yesorno == 1)u8x8.print(F("YES"));
      if (yesorno == 2)u8x8.print(F("NO "));
    }


    if (!digitalRead(2) && yesorno == 1) {
      EEPROM.put(0, 30.00);         // setpoint
      EEPROM.put(5, 0.00);          // callibration offset
      EEPROM.put(10, -0.3);         // below on
      EEPROM.put(15, 0.3);          // above off
      EEPROM.put(20, 600);          // max time in seconds relay on
      EEPROM.put(25, 2);            // 1=cool 2=heat
      EEPROM.put(30, 90);           // high temp alarm
      EEPROM.put(35, 20);           // low temp alarm
      EEPROM.put(40, 300);
      u8x8.clear();
      u8x8.setCursor(0, 0);
      u8x8.print(F("Saving to EEPROM"));
      for (int i = 0; i < 100; i++)Serial.println(F(" valeus restored to factory settings "));
      // read stored valeus from eeprom
      EEPROM.get(0, SwitchOnTemp);
      EEPROM.get(5, CalibrationOffset);
      EEPROM.get(10, relayonpointbelowsetpoint);
      EEPROM.get(15, relayoffabovesetpoint);
      EEPROM.get(20, MaxTimeRelayMayBeonInSeconds);
      EEPROM.get(25, CoolorHeat);
      EEPROM.get(30, HighTempAlarmVal);
      EEPROM.get(35, LowTempAlarmVal);
      EEPROM.get(40, MaxTime2SetPoint);
      delay(1000);
      menu = 11;
      u8x8.clear();

    }

    if (!digitalRead(2) && yesorno == 2) { //no + button
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 11;
      u8x8.clear();
      delay(250);
    }

  }





  //11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11
  TempLong = millis();  //reset innactive time counter
  while (menu == 11) {
    if ((millis() - TempLong)  > 1200000) {
      TimeOut();
      break;
    }
    
    // Arduino UpTime RunTime
    // runtime since boot/start days hours minutes seconds
    // micros has a 70 minut overflow faster for testing
    // millis has a 50 day overflow
    // ISR timer 1 overflow counter?
    // interrupt service routine to count the number of overflows of timer 0 1 2 ???
    // is there something to count the number of  overflows
    // i do not know i am a beginner newbie
    
    Serial.println(F(" rumtime menu"));
    u8x8.setCursor(0, 0);
    u8x8.print(F("11 RunTime"));
    u8x8.setCursor(3, 2);
    u8x8.print((micros() / 86400000) % 365);   //should be millis() micros overflow is faster
    u8x8.print(" ");
    u8x8.print((micros() / 3600000) % 24);    //should be millis() micros overflow is faster
    u8x8.print(":");
    u8x8.print((micros() / 60000) % 60);       //should be millis() micros overflow is faster
    u8x8.print(":");
    u8x8.print((micros() / 1000) % 60);       //should be millis() micros overflow is faster
    u8x8.print("  ");
    u8x8.setCursor(0, 4);
    u8x8.print(micros());
    u8x8.setCursor(0, 5);
    u8x8.print(F("Copyright DLD"));
    u8x8.setCursor(0, 6);
    u8x8.print(F("Luberth Dijkman"));
    u8x8.setCursor(0, 7);
    u8x8.print(F("The Netherlands"));
    if (SetButton() == LOW) {
      while (SetButton() == LOW); // wait for release button do not fly trough all the menus
      menu = 0;
      u8x8.clear();
      delay(250);
    }

  }





  Read_NTC_Thermistor();

  //Vo = analogRead(ThermistorPin);
  R2 = R1 * (1023.0 / (float)Vo - 1.0);
  logR2 = log(R2);
  T = (1.0 / (c1 + c2 * logR2 + c3 * logR2 * logR2 * logR2));
  Tc = T - 273.15 + CalibrationOffset;

  //Tf = (Tc * 9.0)/ 5.0 + 32.0; // fahrenheit


  // compare actualtemp to switchpoint with offsets
  //COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL COOL
  if (CoolorHeat == 1) {      // 1=cool 2=heat
    u8x8.setCursor(0, 7);
    u8x8.print(F("    Cool Mode   "));
    if (Tc < SwitchOnTemp + relayonpointbelowsetpoint) {
      RelaisState = 0;
    }
    if (Tc > SwitchOnTemp + relayoffabovesetpoint) {
      RelaisState = 1;
    }
  }
  //HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT HEAT
  if (CoolorHeat == 2) {      // 1=cool 2=heat
    u8x8.setCursor(0, 7);
    u8x8.print(F("   Heat Mode    "));
    if (Tc < SwitchOnTemp + relayonpointbelowsetpoint) {
      RelaisState = 1;
    }
    if (Tc > SwitchOnTemp + relayoffabovesetpoint) {
      RelaisState = 0;
    }
  }

  if (Tc < LowTempAlarmVal) {
    for (int i = 0; i < 10; i++)Serial.println(F(" Alarm Temperature LOW "));
    u8x8.setCursor(0, 2);
    u8x8.print(F("Alarm Temp. LOW"));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(0, 2);
    u8x8.print(F("               "));
    digitalWrite(11, 1);
  } else {
    digitalWrite(11, 0);

  }

  if (Tc > HighTempAlarmVal) {
    for (int i = 0; i < 10; i++)Serial.println(F(" Alarm Temperature HIGH "));
    digitalWrite(12, 1);
    u8x8.setCursor(0, 2);
    u8x8.print(F("Alarm Temp. High"));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(0, 2);
    u8x8.print(F("                "));
  } else {
    digitalWrite(12, 0);
  }




  Serial.print(F("D10 "));
  Serial.print(RelaisState);

  Serial.print(F(" A0 "));
  Serial.print(Vo);
  Serial.print(F(" Temp "));
  Serial.print(Tc, 1);  // 1 decimal

  u8x8.setCursor(2, 1);
  u8x8.print(Tc, 1);
  u8x8.print(" C");//\337C Cdegree sign


  if (Vo <= 60) {
    for (int i = 0; i < 10; i++)Serial.println(F(" analogread < 60 ERROR LOW "));
    RelaisState = 0; //turn off relays
    u8x8.setCursor(0, 2);
    u8x8.print(F("  Sensor Loss  "));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(0, 2);
    u8x8.print(F("               "));
  }
  if (Vo >= 960) {
    for (int i = 0; i < 10; i++)Serial.println(F(" analogread > 960 ERROR HIGH "));
    RelaisState = 0; //turn off relays
    u8x8.setCursor(0, 2);
    u8x8.print(F(" Sensor Shorted "));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(0, 2);
    u8x8.print(F("                "));
  }



  //Serial.print("menu nr ");
  //Serial.print(menu);

  Serial.print(F(" Set "));
  Serial.print(SwitchOnTemp);
  u8x8.setCursor(10, 1);
  u8x8.print(SwitchOnTemp, 1);
  u8x8.print(" C");   //\337C Cdegree sign


  //Serial.print(" TempFloat ");
  //Serial.print(TempFloat);
  //Serial.print(" TempInt ");
  //Serial.print(TempInt);
  //Serial.print(" TempLong ");
  //Serial.print(TempLong);

  Serial.print(F(" Below "));
  Serial.print(relayonpointbelowsetpoint);
  Serial.print(F(" Above "));
  Serial.print(relayoffabovesetpoint);


  Serial.print(F(" Offset "));
  Serial.print(CalibrationOffset);

  Serial.print(F(" Lalarm "));
  Serial.print(LowTempAlarmVal);
  Serial.print(F(" Halarm "));
  Serial.print(HighTempAlarmVal);

  if (RelaisState == 0) {
    digitalWrite(10, 0);
    starttime = millis();
  }
  if (RelaisState == 1 && digitalRead(10) == 0) {
    digitalWrite(10, 1);
    starttime = millis();
  }

  timeon =  (millis() - starttime) / 1000;
  Serial.print(F(" MaxT "));
  Serial.print(MaxTimeRelayMayBeonInSeconds);

  Serial.print(F(" Ton "));
  Serial.print(timeon);

  if (RelaisState == 1 ) {
    u8x8.setCursor(0, 4);
    u8x8.print(timeon);
    u8x8.print(F(" Sec. ON "));
  } else {
    u8x8.setCursor(0, 4);
    u8x8.print(F("Relay OFF      "));
  }

  EEPROM.get(666, TempInt);
  Serial.print(F(" Devil "));
  Serial.println(TempInt);


  if (timeon > MaxTimeRelayMayBeonInSeconds) {            //if there is a sensor fail or heating fail
    for (int i = 0; i < 10; i++)Serial.println(F(" >>>>>>>> ERROR max time relay ON <<<<<<<<<"));
    //shutdown more??? relay should not be on so long
    u8x8.setCursor(0, 2);
    u8x8.print(F("MaxTime Relay ON"));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(1, 2);
    u8x8.print(F("                 "));
  }

  if (timeon > MaxTime2SetPoint) {            //
    for (int i = 0; i < 10; i++)Serial.println(F(" >>>>>>>> time on 2 desired setpoint to long <<<<<<<<<"));
    //shutdown more??? relay should not be on so long
    u8x8.setCursor(0, 2);
    u8x8.print(F("Time2 setpoint >"));
    buzzer();   //passive buzzer on D8
    u8x8.setCursor(1, 2);
    u8x8.print(F("                   "));

  }



  //delay(50);

  // if you change the program to make it do more or better
  // would like to know/share
  // https://m.facebook.com/luberth.dijkman
  // http://sticker.tk/forum/index.php?action=view&id=296

}
//end loop







//maybe above code is better to read if i do button digitalread like this
boolean SetButton() {
  boolean sval;
  sval = digitalRead(2);
  if (sval == 0) {                 // make a buzz when button pressed
    tone(8, 200); //output D8 Hz
    delay(100);
    noTone(8);
    delay(250);
  }
  //Serial.print(F("SetButton="));
  //Serial.println(sval);
  return sval;
}

boolean PlusButton() {
  boolean sval;
  sval = digitalRead(3);
  Serial.print(F("PlusButton="));
  Serial.println(sval);
  return sval;
}

boolean MinButton() {
  boolean sval;
  sval = digitalRead(4);
  Serial.print(F("MinButton="));
  Serial.println(sval);
  return sval;
}

int Read_NTC_Thermistor() {
  for (int i = 0; i < 500; i++) {
    Vo = Vo + analogRead(A0);             // found temp value displayed bouncy noisy fluctuating
    delay(1);                             // do 500 analog ntc readings and divide by 200 to get an average
  }
  Vo = Vo / 500;
  return Vo;
}


void TimeOut() {
  u8x8.clear();  //exit menu if 20 seconds innactive
  u8x8.setCursor(0, 1);
  u8x8.print(F("     TimeOut    "));
  u8x8.setCursor(0, 2);
  u8x8.print(F("  Start Screen  "));
  delay(2000);
  u8x8.clear();
  menu = 0;
}





int8_t read_rotary() {
  static int8_t rot_enc_table[] = {0, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0};

  // Robust Rotary encoder reading
  // Copyright John Main - best-microcontroller-projects.com
  // https://www.best-microcontroller-projects.com/rotary-encoder.html

  prevNextCode <<= 2;
  if (digitalRead(DATA)) prevNextCode |= 0x02;
  if (digitalRead(CLK)) prevNextCode |= 0x01;
  prevNextCode &= 0x0f;

  // If valid then store as 16 bit data.
  if  (rot_enc_table[prevNextCode] ) {
    store <<= 4;
    store |= prevNextCode;
    //if (store==0xd42b) return 1;
    //if (store==0xe817) return -1;
    if ((store & 0xff) == 0x2b) return -1;
    if ((store & 0xff) == 0x17) return 1;
  }
  return 0;

  // Robust Rotary encoder reading
  // Copyright John Main - best-microcontroller-projects.com
  // https://www.best-microcontroller-projects.com/rotary-encoder.html
}

void buzzer() {
  /* int i, t;
    for (t = 0; t < 1; t = t + 1) {
     //for (i = 7000; i > 1; i=i-1)tone(zoemer, i);
     //for (i = 1; i < 7000; i = i + 1)tone(8, i);   //passive buzzer on D8
     tone(8, 1000, 300);  //output D8 1000Hz 300Ms
     delay(500);
     tone(8, 500, 300); //output D8 500Hz 300Ms
     delay(500);
     noTone(8);
     delay(200);
    }
    noTone(8);
    //return 0;
  */
  // iterate over the notes of the melody:
  for (int thisNote = 0; thisNote < 8; thisNote++) {

    // to calculate the note duration, take one second divided by the note type.
    //e.g. quarter note = 1000 / 4, eighth note = 1000/8, etc.
    int noteDuration = 1000 / noteDurations[thisNote];
    tone(8, melody[thisNote], noteDuration);

    // to distinguish the notes, set a minimum time between them.
    // the note's duration + 30% seems to work well:
    int pauseBetweenNotes = noteDuration * 1.30;
    delay(pauseBetweenNotes);
    // stop the tone playing:
    noTone(8);
  }
}












// it is mostly air i live on
// but i could use your support
// a nickle or dime for every device you put this code on
// would be appreciated Http://paypal.me/LDijkman
// more is allowed
