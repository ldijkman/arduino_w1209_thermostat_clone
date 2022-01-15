

//https://github.com/ldijkman/arduino_w1209_thermostat_clone
// Do Not be afraid Arduino is fun to play with
// 
// and arduino clones are cheap with loads of cheap components



// Feature request? IF desired settemp not reached within ??? seconds THEN alarm



// version 22-june-2019 12:30
//Sketch uses 15680 bytes (51%) of program storage space. Maximum is 30720 bytes.
//Global variables use 516 bytes (25%) of dynamic memory, leaving 1532 bytes for local variables. Maximum is 2048 bytes.








// version 23-june-2019
// should become a safe arduino version of w1209 thermostat
// but far from finished
// parts of code used from
// http://www.circuitbasics.com/arduino-thermistor-temperature-sensor-tutorial/
// https://github.com/tehniq3/DS18B20_thermostat_4digit_7segment_led_display/blob/master/4dig7segm_ac_18b20_thermostat_ver4m7.ino
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
  pinMode(2, INPUT_PULLUP);       // set button
  pinMode(3, INPUT_PULLUP);       // + button
  pinMode(4, INPUT_PULLUP);       // - button
  pinMode(10, OUTPUT);            // relais
  pinMode(11, OUTPUT);            // lowtemp alarm relais
  pinMode(12, OUTPUT);            // hightemp alarm relais
  pinMode(13, OUTPUT);            // another relais
}




void loop() {

  // int SetButton() = SetButton();
  // int PlusButton = PlusButton();
  // int MinButton = MinButton();

  if (SetButton() == LOW) {
    menu = 1;
    while (SetButton() == LOW) {
      // loop until button released
      // maybe a timer here
      // alarm if button never released
    }
    lcd.clear();
  }



  //1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 
  //setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint setpoint 
  TempLong = millis();  //reset innactive time counter
  while (menu == 1) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    delayval = 150;
    while (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      SwitchOnTemp = SwitchOnTemp + 0.1;
      if (SwitchOnTemp > MaxTemp) {
        SwitchOnTemp = MaxTemp;
        Serial.println(F("Reached MaxTemp"));
      }
      delay(delayval);
      delayval = delayval - 2;
      if (  delayval < 1)  delayval = 1;
      //Serial.println("menu 1 ");
      Serial.print(F("SwitchOnTemp "));
      Serial.println(SwitchOnTemp);
      lcd.setCursor(0, 1);
      lcd.print(SwitchOnTemp);
      lcd.print(F(" "));
    }
    while (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      SwitchOnTemp = SwitchOnTemp - 0.1;
      if (SwitchOnTemp < MinTemp) {
        SwitchOnTemp = MinTemp;
        Serial.println(F("Reached MinTemp "));
      }
      delay(delayval);
      delayval = delayval - 2;
      if (  delayval < 1)  delayval = 1;
      //Serial.println("menu 1 ");
      Serial.print(F("SwitchOnTemp "));
      Serial.println(SwitchOnTemp);
      lcd.setCursor(0, 1);
      lcd.print(SwitchOnTemp);
      lcd.print(F(" "));
    }
    //Serial.println("menu 1 ");
    Serial.print(F("SwitchOnTemp "));
    Serial.println(SwitchOnTemp);
    lcd.setCursor(0, 0);
    lcd.print(F("1 SetTemp"));
    lcd.setCursor(0, 1);
    lcd.print(SwitchOnTemp);
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 2) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      CalibrationOffset = CalibrationOffset + 0.1;
      if (CalibrationOffset > 7)CalibrationOffset = 7;
      delay(200);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      CalibrationOffset = CalibrationOffset - 0.1;
      if (CalibrationOffset < -7)CalibrationOffset = -7;
      delay(200);
    }
    //Serial.println("menu 2 ");
    Serial.print(F("CalibrationOffset "));
    Serial.println(CalibrationOffset);
    lcd.setCursor(0, 0);
    lcd.print(F("2 Cal. Offset"));
    lcd.setCursor(0, 1);
    lcd.print(CalibrationOffset);
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 3) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      relayonpointbelowsetpoint = relayonpointbelowsetpoint + 0.1;
      if (relayonpointbelowsetpoint > -0.1)relayonpointbelowsetpoint = -0.1;
      delay(50);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      relayonpointbelowsetpoint = relayonpointbelowsetpoint - 0.1;
      if (relayonpointbelowsetpoint < -2)relayonpointbelowsetpoint = -2;
      delay(50);
    }

    //Serial.println("menu 3 ");
    Serial.print(F("relayonpointbelowsetpoint "));
    Serial.println(relayonpointbelowsetpoint);
    lcd.setCursor(0, 0);
    lcd.print(F("3 Below Set"));
    lcd.setCursor(0, 1);
    lcd.print(relayonpointbelowsetpoint);
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 4) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      relayoffabovesetpoint = relayoffabovesetpoint + 0.1;
      if (relayoffabovesetpoint > 2)relayoffabovesetpoint = 2;
      delay(50);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      relayoffabovesetpoint = relayoffabovesetpoint - 0.1;
      if (relayoffabovesetpoint < 0)relayoffabovesetpoint = 0;
      delay(50);
    }

    //Serial.println("menu 3 ");
    Serial.print(F("relayoffabovesetpoint "));
    Serial.println(relayoffabovesetpoint);
    lcd.setCursor(0, 0);
    lcd.print(F("4 Above Set"));
    lcd.setCursor(0, 1);
    lcd.print(relayoffabovesetpoint);
    lcd.print(F(" "));

    if (SetButton() == LOW) {
      menu = 5;
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
  while (menu == 5) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      MaxTimeRelayMayBeonInSeconds = MaxTimeRelayMayBeonInSeconds + 1;
      if (MaxTimeRelayMayBeonInSeconds > 3600)MaxTimeRelayMayBeonInSeconds = 3600;
      delay(50);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      MaxTimeRelayMayBeonInSeconds = MaxTimeRelayMayBeonInSeconds - 1;
      if (MaxTimeRelayMayBeonInSeconds < 30)MaxTimeRelayMayBeonInSeconds = 30;
      delay(50);
    }

    Serial.print(F("maximum time the relay may be on in seconds "));
    Serial.println(MaxTimeRelayMayBeonInSeconds);
    lcd.setCursor(0, 0);
    lcd.print(F("5 Max T Relay on Sec"));
    lcd.setCursor(0, 1);
    lcd.print(MaxTimeRelayMayBeonInSeconds);
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 6) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      CoolorHeat = CoolorHeat + 1;
      if (CoolorHeat > 2)CoolorHeat = 1;
      delay(250);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      CoolorHeat = CoolorHeat - 1;
      if (CoolorHeat < 1)CoolorHeat = 2;
      delay(250);
    }

    Serial.println(F("cool / heat / ldr on dark / ldr on light "));
    //Serial.println(SwitchOnTemp);
    lcd.setCursor(0, 0);
    lcd.print(F("6 Cool / Heat"));
    lcd.setCursor(0, 1);
    if (CoolorHeat == 1)lcd.print(F("Not yet! = COOL"));
    if (CoolorHeat == 2)lcd.print(F("Not yet! = HEAT"));

    if (SetButton() == LOW) {
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
  while (menu == 7) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      HighTempAlarmVal = HighTempAlarmVal + 1;
      if (HighTempAlarmVal > MaxTemp)HighTempAlarmVal = MaxTemp;
      delay(50);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      HighTempAlarmVal = HighTempAlarmVal - 1;
      if (HighTempAlarmVal < MinTemp)HighTempAlarmVal = MinTemp;
      delay(50);
    }

    Serial.print(F("High Temp alarm  "));
    Serial.println(HighTempAlarmVal);
    lcd.setCursor(0, 0);
    lcd.print(F("7 High Temp Alarm"));
    lcd.setCursor(0, 1);
    lcd.print(HighTempAlarmVal );
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 8) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      LowTempAlarmVal = LowTempAlarmVal + 1;
      if (LowTempAlarmVal > MaxTemp)LowTempAlarmVal = MaxTemp;
      delay(250);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      LowTempAlarmVal = LowTempAlarmVal - 1;
      if (LowTempAlarmVal < MinTemp)LowTempAlarmVal = MinTemp;
      delay(250);
    }

    Serial.print(F("Low Temp alarm  "));
    Serial.println(LowTempAlarmVal);
    lcd.setCursor(0, 0);
    lcd.print(F("8 Low Temp Alarm"));
    lcd.setCursor(0, 1);
    lcd.print(LowTempAlarmVal );
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 9) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      MaxTime2SetPoint = MaxTime2SetPoint + 1;
      if (MaxTime2SetPoint > MaxTimeRelayMayBeonInSeconds)MaxTime2SetPoint = MaxTimeRelayMayBeonInSeconds;
      delay(200);
    }
    if (MinButton() == LOW) {
      TempLong = millis();  //reset innactive time counter
      MaxTime2SetPoint = MaxTime2SetPoint - 1;
      if (MaxTime2SetPoint < 0)MaxTime2SetPoint = 0;
      delay(200);
    }

    Serial.print(F("MaxTime2SetPoint  "));
    Serial.println(MaxTime2SetPoint);
    lcd.setCursor(0, 0);
    lcd.print(F("9 MaxTime2SetPoint"));
    lcd.setCursor(0, 1);
    lcd.print(MaxTime2SetPoint );
    lcd.print(F(" "));

    if (SetButton() == LOW) {
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
  while (menu == 10) {
    if ((millis() - TempLong) / 1000 > 20) {
      TimeOut();
      break;
    }
    if (PlusButton() == LOW && MinButton() == LOW) {
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
      delay(250);
      menu = 11;
    }

    Serial.println(F(" factory reset press + and -   or SetButton() to exit menu"));
    lcd.setCursor(0, 0);
    lcd.print(F("10 Factory Reset"));
    lcd.setCursor(0, 1);
    lcd.print(F("+ & - to Reset"));
    lcd.setCursor(0, 2);
    lcd.print(F("SET = Exit Menu"));

    if (SetButton() == LOW) {
      menu = 11;
      lcd.clear();
      delay(250);
    }
    if (menu == 11) {
      lcd.clear();
      break;
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
    delay(500);
    lcd.setCursor(1, 2);
    lcd.print(F("                 "));
  }

  if (timeon > MaxTime2SetPoint) {            //
    for (int i = 0; i < 10; i++)Serial.println(F(" >>>>>>>> time on 2 desired setpoint to long <<<<<<<<<"));
    //shutdown more??? relay should not be on so long
    lcd.setCursor(1, 2);
    lcd.print(F("Warning heat time >"));
    delay(500);
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
  Serial.print(F("SetButton="));
  Serial.println(sval);
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



// it is mostly air i live on
// but i could use your support
// a nickle or dime for every device you put this code on
// would be appreciated Http://paypal.me/LDijkman
// more is allowed
// https://github.com/ldijkman/arduino_w1209_thermostat_clone  
  
  
