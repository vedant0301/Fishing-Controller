#ifndef FMOTORS_H
#define FMOTORS_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <event_groups.h>
#include <semphr.h>
#include <timers.h>
#include "fledstrip.h"
/************************************************************************************************
SETTINGs
************************************************************************************************/
// DC MOTOR pins
#define ENA 9 // 0-255

// Push button motor pins
const int IN3 = A0;
const int IN4 = A1;
//0,3,4,8,10,11,12,13
// #define Aplus 10
// #define Aminus 11
// #define Bplus 12
// #define Bminus 13


#define Aplus 10
#define Aminus 12
#define Bplus 11
#define Bminus 13

//4,532 steps 360
//1010 max steps motor
/************************************************************************************************

************************************************************************************************/

extern TimerHandle_t castTimer;
extern EventGroupHandle_t fishingrodEvents;
extern const uint8_t redButton;

#define powerFlag ( 1 << 1 )
#define emergencyFlag ( 1 << 2 )

void fmotors_setup();

void motorsOFF();

void updateReelAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin);
void updateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin);

void Casting(int Desi_Dist);
void Stepper_Motor_setup();

void pressButton();
void releaseButton();
void solenoidOff();

void step();
void NEWupdateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin);

#endif // FMOTORS_H