#ifndef FLEDSTRIP_H
#define FLEDSTRIP_H

#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>

#define LEDStripRED 1
#define LEDStripOFF 2
#define LEDStripDIMYELLOW  3
#define LEDStripBRIGHTYELLOW  4
/************************************************************************************************
SETTINGs
************************************************************************************************/
// Constants
const int numLEDs = 10; // Number of LEDs in the strip

//LED strip pins
#define dataPin1  2 // led strip 1
#define dataPin2  0 // led strip 2
/************************************************************************************************

************************************************************************************************/
// Semaphore for LED strip access
extern SemaphoreHandle_t xLEDMutex;

void LEDStrip_setup();

// Function Declarations
void resetStrip(int pin);
void sendBit(bool bitVal, int pin);
void sendByte(uint8_t byte, int pin);
void sendColor(uint8_t r, uint8_t g, uint8_t b, int pin);
void showColor(uint8_t r, uint8_t g, uint8_t b, int numLEDsToShow, int pin);
void LEDStripColor(uint8_t color, int pin);


#endif // FLEDSTRIP_H