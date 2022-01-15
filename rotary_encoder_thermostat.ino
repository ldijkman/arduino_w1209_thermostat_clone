




Arduino w1209 thermostat clone<br>
<b>this is the rotary encoder button version<br>
in the future i will use a rotary encoder button<br>
just a 1 euro pcb module is easier</b>

a quick change to rotary encoder push button <br>
most should work

//version 24june 2019
//
// uno or nano 
// 20x4 i2c lcd
// rotary encoder with pushbutton on axis
// 10k NTC thermistor with 10k resistor
//
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
// a nickle or dime would be apreciated http://paypal.me/pools/c/8amUN5rgb9
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
#include <LiquidCrystal_I2C.h>                     // https://bitbucket.org/fmalpartida/new-liquidcrystal/wiki/Home
LiquidCrystal_I2C  lcd(0x27, 2, 1, 0, 4, 5, 6, 7);

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

  lcd.begin(20, 4);
  lcd.setBacklightPin(3, POSITIVE);
  lcd.setBacklight(HIGH);

  // first time i use eeprom, thought it would be very hard to use
  // i make steps of 5 in eeprom adres because i did not know what to use
  // 5 is a nice step when i look with the eeprom read example from arduino
  // not optimal use of eeprom but have eeprom more then i use

  /*
    // next is for test => normally commented
    // erase eeprom all to 0
        for (int i = 0 ; i < EEPROM.length() ; i++){EEPROM.put(i, 0);
                            //was eeprom.write but i think put is better   put only writes when val not the same
            lcd.setCursor(2, 1);lcd.print(i);lcd.print(" Erase EEPROM");
        } // erase eeprom all to 0
       for (int i = 30 ; i > 0 ; i--){lcd.setCursor(9, 3);lcd.print(i);lcd.print(" ");delay(500);}
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
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print(F("Hi There! First run"));
    lcd.setCursor(0, 1);
    lcd.print(F("Have written value's"));
    lcd.setCursor(0, 2);
    lcd.print(F("to EEPROM "));
    lcd.setCursor(0, 3);
    lcd.print(F("Thanks for trying"));
    for (int i = 30 ; i > 0 ; i--) {
      lcd.setCursor(17, 2);
      lcd.print(i);
      lcd.print(" ");
      delay(500);
    }
    lcd.clear();
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
  pinMode(CLK, INPUT);
  pinMode(CLK, INPUT_PULLUP);
  pinMode(DATA, INPUT);
  pinMode(DATA, INPUT_PULLUP);


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
      lcd.setCursor(0, 0);
      lcd.print(F("                    "));
      lcd.setCursor(0, 1);
      lcd.print(F(" In Menu System Now "));
      lcd.setCursor(0, 2);
      lcd.print(F("   Release Button   "));
      lcd.setCursor(0, 3);
      lcd.print(F("                    "));
    }
    lcd.clear();
  }



  //1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1
  //setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint
  TempLong = millis();  //reset innactive time counter
  if (menu == 1) {
    lcd.setCursor(0, 0);
    lcd.print(F("1 SetTemp"));
    lcd.setCursor(0, 1);
    lcd.print(SwitchOnTemp);
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
      lcd.setCursor(0, 1);
      lcd.print(SwitchOnTemp);
      lcd.print(F(" "));
      if (SwitchOnTemp > MaxTemp) SwitchOnTemp = MaxTemp;
      if (SwitchOnTemp < MinTemp) SwitchOnTemp = MinTemp;
    }

    if (!SetButton()) {                         //if !=not setbutton pressed
      menu = 2;
      lcd.clear();
      delay(250);
    }
  }


  EEPROM.get(0, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (SwitchOnTemp != TempFloat) {                  // only write to eeprom if value is different
    EEPROM.put(0, SwitchOnTemp);                    // put already checks if val is needed to write
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempFloat);
    lcd.print(F(" new= "));
    lcd.print(SwitchOnTemp);

    for (int i = 0; i < 100; i++)Serial.println(F("SwitchOnTemp DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }






  //2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2 2
  //calibration calibration calibration calibration calibration calibration calibration calibration calibration calibration calibration
  TempLong = millis();  //reset innactive time counter
  if (menu == 2) {
    lcd.setCursor(0, 0);
    lcd.print(F("2 Cal. Offset"));
    lcd.setCursor(0, 1);
    lcd.print(CalibrationOffset);
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
      lcd.setCursor(0, 1);
      lcd.print(CalibrationOffset);
      lcd.print(F(" "));
      if (CalibrationOffset > 7)CalibrationOffset = 7;
      if (CalibrationOffset < -7)CalibrationOffset = -7;
    }

    if (!SetButton()) {                       //if !=not setbutton pressed
      menu = 3;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(5, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (CalibrationOffset != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(5, CalibrationOffset);               // i have no idea wat eeprom adress to use just jump to 5
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempFloat);
    lcd.print(F(" new= "));
    lcd.print(CalibrationOffset);
    for (int i = 0; i < 100; i++)Serial.println(F("CalibrationOffset DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }

  //3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3 3
  //below set below set below set below set below set below set below set below set below set below set below set below set below set
  TempLong = millis();  //reset innactive time counter
  if (menu == 3) {
    lcd.setCursor(0, 0);
    lcd.print(F("3 Below Set"));
    lcd.setCursor(0, 1);
    lcd.print(relayonpointbelowsetpoint);
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
      lcd.setCursor(0, 1);
      lcd.print(relayonpointbelowsetpoint);
      lcd.print(F(" "));
      if (relayonpointbelowsetpoint > -0.1)relayonpointbelowsetpoint = -0.1;
      if (relayonpointbelowsetpoint < -2)relayonpointbelowsetpoint = -2;
    }

    if (!SetButton()) {                   //if !=not setbutton pressed
      menu = 4;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(10, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (relayonpointbelowsetpoint != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(10, relayonpointbelowsetpoint);               // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempFloat);
    lcd.print(F(" new= "));
    lcd.print(relayonpointbelowsetpoint);
    for (int i = 0; i < 100; i++)Serial.println(F("relayonpointbelowsetpoint DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }


  //4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4 4
  //above set above set above set above set above set above set above set above set above set above set above set above set
  TempLong = millis();  //reset innactive time counter
  if (menu == 4) {
    lcd.setCursor(0, 0);
    lcd.print(F("4 Above Set"));
    lcd.setCursor(0, 1);
    lcd.print(relayoffabovesetpoint);
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
      lcd.setCursor(0, 1);
      lcd.print(relayoffabovesetpoint);
      lcd.print(F(" "));
      if (relayoffabovesetpoint > 2)relayoffabovesetpoint = 2;
      if (relayoffabovesetpoint < 0)relayoffabovesetpoint = 0;
    }

    if (!SetButton()) {              //if !=not setbutton pressed
      menu = 5;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(15, TempFloat);                         // limmited write to eeprom = read is unlimmited
  if (relayoffabovesetpoint != TempFloat) {             // only write to eeprom if value is different
    EEPROM.put(15, relayoffabovesetpoint);               // i have no idea wat eeprom adress to use just jump to 5
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempFloat);
    lcd.print(F(" new= "));
    lcd.print(relayoffabovesetpoint);
    for (int i = 0; i < 100; i++)Serial.println(F("relayoffabovesetpoint DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }

  //5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5 5
  //max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay max t relay
  TempLong = millis();  //reset innactive time counter
  if (menu == 5) {
    lcd.setCursor(0, 0);
    lcd.print(F("5 Max T Relay on Sec"));
    lcd.setCursor(0, 1);
    lcd.print(MaxTimeRelayMayBeonInSeconds);
  }
  while (menu == 5) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      MaxTimeRelayMayBeonInSeconds  = MaxTimeRelayMayBeonInSeconds  + rval;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 1);
      lcd.print(MaxTimeRelayMayBeonInSeconds);
      lcd.print(F(" "));
      if (MaxTimeRelayMayBeonInSeconds < 30)MaxTimeRelayMayBeonInSeconds = 30;
      if (MaxTimeRelayMayBeonInSeconds > 3600)MaxTimeRelayMayBeonInSeconds = 3600;
    }

    if (!SetButton()) {                //if !=not setbutton pressed
      menu = 6;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(20, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (MaxTimeRelayMayBeonInSeconds != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(20, MaxTimeRelayMayBeonInSeconds);               // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempInt);
    lcd.print(F(" new= "));
    lcd.print(MaxTimeRelayMayBeonInSeconds);
    for (int i = 0; i < 100; i++)Serial.println(F("MaxTimeRelayMayBeonInSeconds DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }

  //6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6 6
  //CoolorHeat CoolorHeat CoolorHeat CoolorHeat CoolorHeat CoolorHeat
  TempLong = millis();  //reset innactive time counter
  if (menu == 6) {
    lcd.setCursor(0, 0);
    lcd.print(F("6 Cool / Heat"));
    lcd.setCursor(0, 1);
    if (CoolorHeat == 1)lcd.print(F("Not yet! = COOL"));
    if (CoolorHeat == 2)lcd.print(F("Not yet! = HEAT"));
  }
  while (menu == 6) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      CoolorHeat  = CoolorHeat  + rval;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 1);
      lcd.print(CoolorHeat);
      lcd.print(F(" "));
      if (CoolorHeat < 1)CoolorHeat = 2;
      if (CoolorHeat > 2)CoolorHeat = 1;
      lcd.setCursor(0, 1);
      if (CoolorHeat == 1)lcd.print(F("Not yet! = COOL"));
      if (CoolorHeat == 2)lcd.print(F("Not yet! = HEAT"));
    }

    if (!SetButton()) { //if !=not setbutton pressed
      menu = 7;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(25, TempByte);                         // limmited write to eeprom = read is unlimmited
  if (CoolorHeat != TempByte) {                     // only write to eeprom if value is different
    EEPROM.put(25, CoolorHeat);                     // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempByte);
    lcd.print(F(" new= "));
    if (CoolorHeat == 1)lcd.print(F("COOL"));
    if (CoolorHeat == 2)lcd.print(F("HEAT"));
    for (int i = 0; i < 100; i++)Serial.println(F("Cool/Heat DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }







  //777777777777777777777777777777777777777777777777777777777777777777777
  TempLong = millis();  //reset innactive time counter
  if (menu == 7) {
    lcd.setCursor(0, 0);
    lcd.print(F("7 High Temp Alarm"));
    lcd.setCursor(0, 1);
    lcd.print(HighTempAlarmVal );
  }
  while (menu == 7) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      HighTempAlarmVal  = HighTempAlarmVal  + rval;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 1);
      lcd.print(HighTempAlarmVal );
      lcd.print(F(" "));
      if (HighTempAlarmVal > MaxTemp)HighTempAlarmVal = MaxTemp;
      if (HighTempAlarmVal < MinTemp)HighTempAlarmVal = MinTemp;
    }

    if (!SetButton()) {                     //if !=not setbutton pressed
      menu = 8;
      lcd.clear();
      delay(250);
    }
  }

  EEPROM.get(30, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (HighTempAlarmVal != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(30, HighTempAlarmVal);               // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempInt);
    lcd.print(F(" new= "));
    lcd.print(HighTempAlarmVal);
    for (int i = 0; i < 100; i++)Serial.println(F("HighTempAlarmVal DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }

  //888888888888888888888888888888888888888888888888888888888888888888888888888888
  TempLong = millis();  //reset innactive time counter
  if (menu == 8) {
    lcd.setCursor(0, 0);
    lcd.print(F("8 Low Temp Alarm"));
    lcd.setCursor(0, 1);
    lcd.print(LowTempAlarmVal );
    lcd.print(F(" "));
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
      lcd.setCursor(0, 1);
      lcd.print(LowTempAlarmVal );
      lcd.print(F(" "));
      if (LowTempAlarmVal > MaxTemp)LowTempAlarmVal = MaxTemp;
      if (LowTempAlarmVal < MinTemp)LowTempAlarmVal = MinTemp;
    }

    if (!SetButton()) {
      menu = 9;
      lcd.clear();
      delay(250);
    }
  }


  EEPROM.get(35, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (LowTempAlarmVal != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(35, LowTempAlarmVal);               // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempInt);
    lcd.print(F(" new= "));
    lcd.print(LowTempAlarmVal);
    for (int i = 0; i < 100; i++)Serial.println(F("LowTempAlarmVal DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }




  //9999999999999999999999999999999999999999999999999999999999999999999999999999999999999
  TempLong = millis();  //reset innactive time counter
  if (menu == 9) {
    lcd.setCursor(0, 0);
    lcd.print(F("9 MaxTime2SetPoint"));
    lcd.setCursor(0, 1);
    lcd.print(MaxTime2SetPoint );
    lcd.print(F(" "));
  }
  while (menu == 9) {
    if ((millis() - TempLong) > 10000) {
      TimeOut();
      break;
    }

    float rval;
    if ( rval = read_rotary() ) {
      MaxTime2SetPoint  = MaxTime2SetPoint  + rval;
      TempLong = millis();  //reset innactive time counter
      lcd.setCursor(0, 1);
      lcd.print(MaxTime2SetPoint );
      lcd.print(F(" "));
      if (MaxTime2SetPoint > MaxTimeRelayMayBeonInSeconds)MaxTime2SetPoint = MaxTimeRelayMayBeonInSeconds;
      if (MaxTime2SetPoint < 0)MaxTime2SetPoint = 0;
    }

    if (!SetButton()) {
      menu = 10;
      lcd.clear();
      delay(250);
    }
  }


  EEPROM.get(40, TempInt);                         // limmited write to eeprom = read is unlimmited
  if (MaxTime2SetPoint != TempInt) {             // only write to eeprom if value is different
    EEPROM.put(40, MaxTime2SetPoint);               // i have no idea wat eeprom adress to use just jump to 10
    lcd.setCursor(0, 0);
    lcd.print(F("Saving to EEPROM"));
    lcd.setCursor(0, 2);
    lcd.print(TempInt);
    lcd.print(F(" new= "));
    lcd.print(MaxTime2SetPoint);
    for (int i = 0; i < 100; i++)Serial.println(F("MaxTime2SetPoint DATA WRITEN / SAVED TO EEPROM "));
    lcd.clear();
  }





  //10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10-10
  TempLong = millis();  //reset innactive time counter
  yesorno = 2;
  if (menu == 10) {
    lcd.setCursor(0, 0);
    lcd.print(F("10 Factory Reset"));
    lcd.setCursor(8, 2);
    if (yesorno == 1)lcd.print(F("YES"));
    if (yesorno == 2)lcd.print(F("NO "));
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
      lcd.setCursor(8, 2);
      if (yesorno == 1)lcd.print(F("YES"));
      if (yesorno == 2)lcd.print(F("NO "));
    }


    if (!SetButton() && yesorno == 1) {
      EEPROM.put(0, 30.00);         // setpoint
      EEPROM.put(5, 0.00);          // callibration offset
      EEPROM.put(10, -0.3);         // below on
      EEPROM.put(15, 0.3);          // above off
      EEPROM.put(20, 600);          // max time in seconds relay on
      EEPROM.put(25, 2);            // 1=cool 2=heat
      EEPROM.put(30, 90);           // high temp alarm
      EEPROM.put(35, 20);           // low temp alarm
      EEPROM.put(40, 300);
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("Saving to EEPROM"));
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
      lcd.clear();

    }

    if (!SetButton() && yesorno == 2) { //no + button
      menu = 11;
      lcd.clear();
      delay(250);
    }

  }





  //11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11-11
  TempLong = millis();  //reset innactive time counter
  while (menu == 11) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    //runtime since boot/start days hours minutes seconds
    Serial.println(F(" rumtime menu"));
    lcd.setCursor(0, 0);
    lcd.print(F("10 RunTime"));
    lcd.setCursor(3, 2);
    lcd.print((millis() / 86400000) % 365);
    lcd.print(" ");
    lcd.print((millis() / 3600000) % 24);
    lcd.print(":");
    lcd.print((millis() / 60000) % 60);
    lcd.print(":");
    lcd.print((millis() / 1000) % 60);
    lcd.print("  ");


    if (SetButton() == LOW) {
      menu = 0;
      lcd.clear();
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

  if (Tc < SwitchOnTemp + relayonpointbelowsetpoint) {
    RelaisState = 1;
  }
  if (Tc > SwitchOnTemp + relayoffabovesetpoint) {
    RelaisState = 0;
  }


  if (Tc < LowTempAlarmVal) {
    for (int i = 0; i < 10; i++)Serial.println(F(" Alarm Temperature LOW "));
    lcd.setCursor(2, 2);
    lcd.print(F("Alarm Temp. LOW"));

    digitalWrite(11, 1);
  } else {
    digitalWrite(11, 0);

  }

  if (Tc > HighTempAlarmVal) {
    for (int i = 0; i < 10; i++)Serial.println(F(" Alarm Temperature HIGH "));
    digitalWrite(12, 1);
    lcd.setCursor(2, 2);
    lcd.print(F("Alarm Temp. High"));

  } else {
    digitalWrite(12, 0);
  }




  Serial.print(F("D10 "));
  Serial.print(RelaisState);

  Serial.print(F(" A0 "));
  Serial.print(Vo);
  Serial.print(F(" Temp "));
  Serial.print(Tc, 1);  // 1 decimal

  lcd.setCursor(2, 1);
  lcd.print(Tc, 1);
  lcd.print("\337C");//Cdegree sign


  if (Vo <= 60) {
    for (int i = 0; i < 10; i++)Serial.println(F(" analogread < 60 ERROR LOW "));
    RelaisState = 0; //turn off relays
    lcd.setCursor(2, 2);
    lcd.print(F("Alarm sensor low?"));
  }
  if (Vo >= 960) {
    for (int i = 0; i < 10; i++)Serial.println(F(" analogread > 960 ERROR HIGH "));
    RelaisState = 0; //turn off relays
    lcd.setCursor(2, 2);
    lcd.print(F("Alarm sensor high?"));

  }



  //Serial.print("menu nr ");
  //Serial.print(menu);

  Serial.print(F(" Set "));
  Serial.print(SwitchOnTemp);
  lcd.setCursor(10, 1);
  lcd.print(SwitchOnTemp, 1);
  lcd.print("\337C");   //Cdegree sign


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
    lcd.setCursor(0, 4);
    lcd.print(timeon);
    lcd.print(F(" Sec. Relay ON "));
  } else {
    lcd.setCursor(0, 4);
    lcd.print(F("Relay OFF        "));
  }

  EEPROM.get(666, TempInt);
  Serial.print(F(" Devil "));
  Serial.println(TempInt);


  if (timeon > MaxTimeRelayMayBeonInSeconds) {            //if there is a sensor fail or heating fail
    for (int i = 0; i < 10; i++)Serial.println(F(" >>>>>>>> ERROR max time relay ON <<<<<<<<<"));
    //shutdown more??? relay should not be on so long
    lcd.setCursor(1, 2);
    lcd.print(F("Max Time Relay ON"));
    delay(1000);
    lcd.setCursor(1, 2);
    lcd.print(F("                 "));
  }

  if (timeon > MaxTime2SetPoint) {            //
    for (int i = 0; i < 10; i++)Serial.println(F(" >>>>>>>> time on 2 desired setpoint to long <<<<<<<<<"));
    //shutdown more??? relay should not be on so long
    lcd.setCursor(1, 2);
    lcd.print(F("Warning heat time >"));
    delay(1000);
    lcd.setCursor(1, 2);
    lcd.print(F("                   "));

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
  lcd.clear();  //exit menu if 20 seconds innactive
  lcd.setCursor(0, 1);
  lcd.print(F("TimeOut"));
  lcd.setCursor(0, 2);
  lcd.print(F("Return to Mainscreen"));
  delay(2000);
  lcd.clear();
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



// it is mostly air i live on
// but i could use your support
// a nickle or dime for every device you put this code on
// would be appreciated Http://paypal.me/LDijkman
// more is allowed
