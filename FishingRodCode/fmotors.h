#ifndef FMOTORS_H
#define FMOTORS_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
#include <timers.h>
#include "fledstrip.h"
/************************************************************************************************
SETTINGs
************************************************************************************************/
// DC MOTOR pins
#define IN1 12
#define IN2 13
#define ENA 11 // 0-255

// Push button motor pins
const int IN3 = A0;
const int IN4 = A1;
//0,3,4,8,10,11,12,13
#define Aplus 10
#define Aminus 11
#define Bplus 12
#define Bminus 13
//4,532 steps 360
//1010 max steps motor
/************************************************************************************************

************************************************************************************************/

extern TimerHandle_t castTimer;

void fmotors_setup();

void motorsOFF();

void updateReelAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin);//, int IN1, int IN2, int ENA);
void updateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin);

void Casting(int Desi_Dist);
void Stepper_Motor_setup();

void pressButton();
void releaseButton();
void solenoidOff();

#endif // FMOTORS_H