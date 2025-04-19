#include "fledstrip.h"

void LEDStrip_setup(){
  pinMode(dataPin1, OUTPUT);
  resetStrip(dataPin1);
  showColor(0, 0, 0, 0, dataPin1);
  pinMode(dataPin2, OUTPUT);
  resetStrip(dataPin2);
  showColor(0, 0, 0, 0, dataPin1);
}

// Reset signal to latch new data
void resetStrip(int pin) {
    digitalWrite(pin, LOW);
    delayMicroseconds(300);  // WS2812B reset requires >50µs LOW
}

// ** Send one bit with precise timing **
void sendBit(bool bitVal, int pin) {
  if(pin == dataPin1){
    if (bitVal) {
        asm volatile (
            "sbi %[port], %[bit] \n\t"  // HIGH
            "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.8µs
            "cbi %[port], %[bit] \n\t"  // LOW
            "nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.45µs
            :
            : [port] "I" (_SFR_IO_ADDR(PORTD)), [bit] "I" (dataPin1)
        );
    } else {
        asm volatile (
            "sbi %[port], %[bit] \n\t"  // HIGH
            "nop \n\t nop \n\t nop \n\t" // ~0.4µs
            "cbi %[port], %[bit] \n\t"  // LOW
            "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.85µs
            :
            : [port] "I" (_SFR_IO_ADDR(PORTD)), [bit] "I" (dataPin1)
        );
    }
  }
  else if(pin == dataPin2){
        if (bitVal) {
        asm volatile (
            "sbi %[port], %[bit] \n\t"  // HIGH
            "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.8µs
            "cbi %[port], %[bit] \n\t"  // LOW
            "nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.45µs
            :
            : [port] "I" (_SFR_IO_ADDR(PORTD)), [bit] "I" (dataPin2)
        );
    } else {
        asm volatile (
            "sbi %[port], %[bit] \n\t"  // HIGH
            "nop \n\t nop \n\t nop \n\t" // ~0.4µs
            "cbi %[port], %[bit] \n\t"  // LOW
            "nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t nop \n\t" // ~0.85µs
            :
            : [port] "I" (_SFR_IO_ADDR(PORTD)), [bit] "I" (dataPin2)
        );
    }

  }
}


// Send 8-bit data (MSB first)
void sendByte(uint8_t byte, int pin) {
    for (int i = 7; i >= 0; i--) {
        sendBit(byte & (1 << i), pin);  // MSB First
    }
}

// Send a single LED color (GRB order)
void sendColor(uint8_t r, uint8_t g, uint8_t b, int pin) {
    sendByte(g, pin);  // GRB order
    sendByte(r, pin);
    sendByte(b, pin);
}

// Send color to all numLEDs
void showColor(uint8_t r, uint8_t g, uint8_t b, int numLEDsToShow, int pin) {
//     for (int i = 0; i < numLEDsToShow; i++) {
//         sendColor(r, g, b, pin);
//     }
//     resetStrip(pin);
// }
    if (xSemaphoreTake(xLEDMutex, portMAX_DELAY) == pdTRUE) {
        taskENTER_CRITICAL(); // Disable task switching

        // Send color data for the specified number of LEDs
        for (int i = 0; i < numLEDsToShow; i++) {
        sendColor(r, g, b, pin);
        }

        // Send "off" (0, 0, 0) for the remaining LEDs on the strip to prevent white
        for (int i = numLEDsToShow; i < 10; i++) { // Assuming strip has max 10 LEDs
            sendColor(0, 0, 0, pin);
        }

        resetStrip(pin);
        taskEXIT_CRITICAL(); // Enable task switching

        xSemaphoreGive(xLEDMutex); // Release the mutex after updating LEDs
    }
}


// Set LED strip color
void LEDStripColor(uint8_t color, int pin) {
    if (color == LEDStripRED) {
        showColor(32, 0, 0, 10, pin); // RED in GRB order
    } 
    else if (color == LEDStripOFF) {
        showColor(0, 0, 0, 10, pin); // OFF
    }
    else if (color == LEDStripDIMYELLOW) {
        showColor(32, 32, 0, 10, pin); // Dim Yellow (Lower brightness)
    } 
    else if (color == LEDStripBRIGHTYELLOW) {
        showColor(255, 255, 0, 10, pin); // Bright Yellow
    }
}
