#include <Wire.h>
#include "RTClib.h"

enum EPartOfTime
{
  EPartOfTime_hour,
  EPartOfTime_minute,
  EPartOfTime_second
};

enum EProgrammingState
{
  EProgrammingState_hour,
  EProgrammingState_minute,
  EProgrammingState_none
};

void hadleButtons();
void handleIncrementingButton();
void incrementHours();
void incrementMinutes();
void stopSeconds();
void writeByte(byte data, EPartOfTime partOfTime);
void printTimeToSerialMonitor(DateTime *time);
byte convertHourToBitwiseFormat(int decimal);
byte convertSecondsOrMinutesToBitwiseFormat(int decimal);
int getDataPin(EPartOfTime partOfTime);
int getClockPinPin(EPartOfTime partOfTime);
int getLatchPin(EPartOfTime partOfTime);

RTC_DS1307 RTC;

int hoursDataPin            = 11;
int hoursClockPin           = 12;
int hoursLatchPin           = 13;
int minutesDataPin          = 8;
int minutesClockPin         = 9;
int minutesLatchPin         = 10;
int secondsDataPin          = 2;
int secondsClockPin         = 3;
int secondsLatchPin         = 4;
int toggleProgrammingButton = 5;
int incrementingButton      = 6;

int lastProgrammingButtonState  = LOW;
int lastIncrementingButtonState = LOW;
EProgrammingState programmingState = EProgrammingState_none;

void setup()
{
  Serial.begin(9600);
  Wire.begin();
  RTC.begin();

  if (! RTC.isrunning())
  {
    Serial.println("RTC is NOT running!");  
  }
 // RTC.adjust(DateTime(__DATE__, __TIME__));
  
  pinMode(hoursDataPin,    OUTPUT);
  pinMode(hoursClockPin,   OUTPUT);
  pinMode(hoursLatchPin,   OUTPUT);
  pinMode(minutesDataPin,  OUTPUT);
  pinMode(minutesClockPin, OUTPUT);
  pinMode(minutesLatchPin, OUTPUT);
  pinMode(secondsDataPin,  OUTPUT);
  pinMode(secondsClockPin, OUTPUT);
  pinMode(secondsLatchPin, OUTPUT);

  pinMode(toggleProgrammingButton, INPUT);
  pinMode(incrementingButton,      INPUT);
}

void loop()
{
  DateTime now = RTC.now();
  printTimeToSerialMonitor(&now);

  hadleButtons();
  handleIncrementingButton();
  
  byte hours   = convertHourToBitwiseFormat(now.hour());
  byte minutes = convertSecondsOrMinutesToBitwiseFormat(now.minute());
  byte seconds = convertSecondsOrMinutesToBitwiseFormat(now.second());

  writeByte(hours,   EPartOfTime_hour);
  writeByte(minutes, EPartOfTime_minute);
  writeByte(seconds, EPartOfTime_second);

  //delay(200);
}

void hadleButtons()
{
  if (programmingState != EProgrammingState_none)
  {
    stopSeconds();
  }
  
  int buttonState = digitalRead(toggleProgrammingButton);
  if (buttonState != lastProgrammingButtonState)
  {
    lastProgrammingButtonState = buttonState;

    if (buttonState == HIGH)
    {
      if (programmingState == EProgrammingState_none)
      {
        programmingState = EProgrammingState_hour;
      }
      else if (programmingState == EProgrammingState_hour)
      {
        programmingState = EProgrammingState_minute;
      }
      else
      {
        programmingState = EProgrammingState_none;
      }
    }
  }
}

void handleIncrementingButton()
{
  if (programmingState == EProgrammingState_none)
  {
    return;
  }

  int buttonState = digitalRead(incrementingButton);
  if (buttonState != lastIncrementingButtonState)
  {
    lastIncrementingButtonState = buttonState;
    if (buttonState == HIGH)
    {
      if (programmingState == EProgrammingState_hour)
      {
        incrementHours();
      }
      else if (programmingState == EProgrammingState_minute)
      {
        incrementMinutes();
      }
    }
  }
}

void stopSeconds()
{
  DateTime currentTime = RTC.now();
  TimeSpan seconds(currentTime.second());
  currentTime = currentTime - seconds;
  RTC.adjust(currentTime);
}

void incrementHours()
{
  DateTime currentTime = RTC.now();
  TimeSpan oneHour(0, 1, 0, 0);
  currentTime = currentTime + oneHour;
  RTC.adjust(currentTime);
}

void incrementMinutes()
{
  DateTime currentTime = RTC.now();
  TimeSpan oneMinute(0, 0, 1, 0);
  currentTime = currentTime + oneMinute;
  RTC.adjust(currentTime);
}

void writeByte(byte data, EPartOfTime partOfTime)
{
  //data = 255;
  shiftOut(getDataPin(partOfTime), getClockPinPin(partOfTime), MSBFIRST, data);
  digitalWrite(getLatchPin(partOfTime), HIGH);
  digitalWrite(getLatchPin(partOfTime), LOW);
}

void printTimeToSerialMonitor(DateTime *time)
{
  static int lastSecond = 0;

  if (time->second() == lastSecond && programmingState == EProgrammingState_none) return;

  lastSecond = time->second();
  
  Serial.print(time->day(), DEC);
  Serial.print('/');
  Serial.print(time->month(), DEC);
  Serial.print('/');
  Serial.print(time->year(), DEC);
  Serial.print(' ');
  Serial.print(time->hour(), DEC);
  Serial.print(':');
  Serial.print(time->minute(), DEC);
  Serial.print(':');
  Serial.print(time->second(), DEC);
  Serial.println();
}

byte convertHourToBitwiseFormat(int decimal)
{
  byte bitwiseFormat;

  int firstPosition = decimal / 10;

  for (int i = 0 ; i < 2; ++i)
  {
    if (bitRead(firstPosition, i) == 1)
    {
      bitWrite(bitwiseFormat, i, bitRead(firstPosition, i));
    }
    else
    {
      bitWrite(bitwiseFormat, i, 0);
    }
  }

  int secondPosition = decimal % 10;

  for (int i = 0 ; i < 4; ++i)
  {
    if (bitRead(secondPosition, i) == 1)
    {
      bitWrite(bitwiseFormat, i + 2, 1);
    }
    else
    {
      bitWrite(bitwiseFormat, i + 2, 0);
    }
  }
  return bitwiseFormat;
}

byte convertSecondsOrMinutesToBitwiseFormat(int decimal)
{
  byte bitwiseFormat;

  int firstPosition = decimal / 10;

  for (int i = 0 ; i < 3; ++i)
  {
    if (bitRead(firstPosition, i) == 1)
    {
      bitWrite(bitwiseFormat, i, 1);
    }
    else
    {
      bitWrite(bitwiseFormat, i, 0);
    }
  }

  int secondPosition = decimal % 10;

  for (int i = 0 ; i < 4; ++i)
  {
    if (bitRead(secondPosition, i) == 1)
    {
      bitWrite(bitwiseFormat, i + 3, 1);
    }
    else
    {
      bitWrite(bitwiseFormat, i + 3, 0);
    }
  }
  return bitwiseFormat;
}

int getDataPin(EPartOfTime partOfTime)
{
  if (partOfTime == EPartOfTime_hour)
  {
    return hoursDataPin;
  }
  else if (partOfTime == EPartOfTime_minute)
  {
    return minutesDataPin;
  }
  else
  {
    return secondsDataPin;
  }
}

int getClockPinPin(EPartOfTime partOfTime)
{
  if (partOfTime == EPartOfTime_hour)
  {
    return hoursClockPin;
  }
  else if (partOfTime == EPartOfTime_minute)
  {
    return minutesClockPin;
  }
  else
  {
    return secondsClockPin;
  }
}

int getLatchPin(EPartOfTime partOfTime)
{
  if (partOfTime == EPartOfTime_hour)
  {
    return hoursLatchPin;
  }
  else if (partOfTime == EPartOfTime_minute)
  {
    return minutesLatchPin;
  }
  else
  {
    return secondsLatchPin;
  }
}

