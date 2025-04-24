#include "fmotors.h"
int casted;

uint8_t maxLEDnum = 1;
bool hasBeenAtOneBefore = false;
bool wentAboveOne = false;

void fmotors_setup() {
  // DC MOTOR
  pinMode(ENA, OUTPUT);
  analogWrite(ENA, 0);
  // Push Button Motor
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);

  Stepper_Motor_setup();

  casted = 0;
}

void updateReelAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin) {
    int reelSpeed;
    uint8_t numberofLEDs;

    if (joystickValue < 113) {
      //reelSpeed = 5;
        numberofLEDs = 1;
        reelSpeed = 0;
    } else if (joystickValue < 169) {
        //reelSpeed = 10;
        numberofLEDs = 2;
        reelSpeed = 1;
    } else if (joystickValue < 282) {
        //reelSpeed = 15;
        numberofLEDs = 3;
        reelSpeed = 2;
    } else if (joystickValue < 395) {
        //reelSpeed = 20;
        numberofLEDs = 4;
        reelSpeed = 3;
    } else if (joystickValue < 508) {
        //reelSpeed = 25;
        numberofLEDs = 5;
        reelSpeed = 4;
    } else if (joystickValue < 621) {
        //reelSpeed = 30;
        numberofLEDs = 6;
        reelSpeed = 5;
    } else if (joystickValue < 734) {
        //reelSpeed = 35;
        numberofLEDs = 7;
        reelSpeed = 6;
    } else if (joystickValue < 847) {
        //reelSpeed = 40;
        numberofLEDs = 8;
        reelSpeed = 7;
    } else if (joystickValue < 960) {
        //reelSpeed = 45;
        numberofLEDs = 9;
        reelSpeed = 8;
    } else {
        //reelSpeed = 50;
        numberofLEDs = 10;
        reelSpeed = 9;
    }
    // Update LEDs and motor only if number of LEDs changes or oldLEDnum is 20
    if ((numberofLEDs != oldLEDnum) || (oldLEDnum == 20)) {
        showColor(0, 32, 0, numberofLEDs, pin);
        analogWrite(ENA, 28*reelSpeed);
        oldLEDnum = numberofLEDs;
    }
}
void updateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin) {
    int distance;
    uint8_t numberofLEDs;

    // Determine distance from joystick
    if (joystickValue < 113) {
      distance = 5;
    }
    else if (joystickValue < 169){
       distance = 10;
    }
    else if (joystickValue < 282) {
      distance = 15;
    }
    else if (joystickValue < 395) {
      distance = 20;
    }
    else if (joystickValue < 508) {
      distance = 25;
      }
    else if (joystickValue < 621) {
      distance = 30;
    }
    else if (joystickValue < 734) {
      distance = 35;
    }
    else if (joystickValue < 847) {
      distance = 40;
      }
    else if (joystickValue < 960) {
      distance = 45;
    }
    else {
      distance = 50;
    }

    // Calculate LEDs
    numberofLEDs = (distance * 2) / 10;

    // Only update visual LEDs when needed
    if (numberofLEDs != oldLEDnum) {
        showColor(0, 0, 32, numberofLEDs, pin);
    }

    // Track max reached for casting power
    //static uint8_t maxLEDnum = 1;
    if (numberofLEDs > maxLEDnum) {
        maxLEDnum = numberofLEDs;
    }

    // Core sequence state
    // static bool hasBeenAtOneBefore = false;
    // static bool wentAboveOne = false;

    if (numberofLEDs == 1) {
        if (!hasBeenAtOneBefore) {
            hasBeenAtOneBefore = true;  // First hit of 1
            wentAboveOne = false;       // Reset upward flag
        } else if (wentAboveOne && casted == 0) {
            // Valid full sequence: 1 -> >1 -> 1
            casted = 1;
            xTimerStop(castTimer, 0);
            Casting(maxLEDnum);
            xTimerStart(castTimer, 0);
            if(maxLEDnum == 10){
              vTaskDelay(pdMS_TO_TICKS(230));
            }
            else if(maxLEDnum == 9){
              vTaskDelay(pdMS_TO_TICKS(226));
            }
            else if(maxLEDnum == 8){
              vTaskDelay(pdMS_TO_TICKS(224));//touch early
            }
            else if(maxLEDnum == 7){
              vTaskDelay(pdMS_TO_TICKS(216));//touch late
            }
            else if(maxLEDnum == 6){
              vTaskDelay(pdMS_TO_TICKS(211));//
            }
            else if(maxLEDnum == 5){
              vTaskDelay(pdMS_TO_TICKS(210));//
            }
            else if(maxLEDnum == 4){
              vTaskDelay(pdMS_TO_TICKS(206));
            }
            else if(maxLEDnum == 3){
              vTaskDelay(pdMS_TO_TICKS(202));
            }
            else if(maxLEDnum == 2){
              vTaskDelay(pdMS_TO_TICKS(198));
            }
            else if(maxLEDnum == 1){
              vTaskDelay(pdMS_TO_TICKS(194));
            }
            else{
              vTaskDelay(pdMS_TO_TICKS(200));
            }
            //vTaskDelay(pdMS_TO_TICKS(230));
            releaseButton();
            maxLEDnum = 1;
            hasBeenAtOneBefore = false;
            wentAboveOne = false;
        }
    } else if (numberofLEDs > 1 && hasBeenAtOneBefore) {
        wentAboveOne = true;
    }

    // Reset casted flag if joystick moves up again
    if (numberofLEDs > 1) {
        casted = 0;
    }

    oldLEDnum = numberofLEDs;
}


// void NEWupdateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin){
//     // Determine distance based on joystickValue
//     bool castON = false;
//     int steps = 0;
//     int numberofLEDs = 0;
//     if (joystickValue < 500) {
//         castON = false;
//     } 
//     else {
//         castON = true;
//     }

//     showColor(0, 0, 255, numberofLEDs, pin); // Set LEDs to blue

//     // **Casting happens based on the highest LED count reached**
//     while (castON) {
//         xTimerStop(castTimer, 0);
//         step();
//         steps++;
//         if (steps % 100 == 0 && numberofLEDs < 10) {
//             numberofLEDs++;
//             showColor(0, 0, 255, numberofLEDs, pin); // Update LED color
//         }
//         if ((emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
//             digitalWrite(Aplus, LOW);
//             digitalWrite(Aminus, LOW);
//             vTaskDelay(pdMS_TO_TICKS(1));
//             digitalWrite(Bplus, LOW);
//             digitalWrite(Bminus, LOW);
//             releaseButton();
//             break;
//         }
//         if((steps > 1200) | (analogRead(A2) <= 500)){
//             digitalWrite(Aplus, LOW);
//             digitalWrite(Aminus, LOW);
//             vTaskDelay(pdMS_TO_TICKS(1));
//             digitalWrite(Bplus, LOW);
//             digitalWrite(Bminus, LOW);
//             vTaskDelay(pdMS_TO_TICKS(190));
//             releaseButton();
//             break;
//         }
//     }
//     xTimerStart(castTimer, 0);
// }
// void step(){
//     static bool A = 0;
//     static bool B = 0;
//     if (A && B) {
//       B = 0;
//       digitalWrite(Bplus, LOW);
//       digitalWrite(Bminus, HIGH);
//     } else if (A && !B) {
//       A = 0;
//       digitalWrite(Aplus, LOW);
//       digitalWrite(Aminus, HIGH);
//     } else if (!A && B) {
//       A = 1;
//       digitalWrite(Aminus, LOW);
//       digitalWrite(Aplus, HIGH);
//     } else if (!A && !B) {
//       B = 1;
//       digitalWrite(Bminus, LOW);
//       digitalWrite(Bplus, HIGH);
//     }
//     vTaskDelay(pdMS_TO_TICKS(10));
//   }
// //taskENTER_CRITICAL(); // Disable task switching
// //taskDISABLE_INTERRUPTS(); // Disable interrupts to protect the critical section
// Casting(oldLEDnum);
// casted = 1;
// //pressButton();

// //taskENABLE_INTERRUPTS();
// //taskEXIT_CRITICAL(); // Enable task switching

void Stepper_Motor_setup() {
  pinMode(Aplus, OUTPUT);
  pinMode(Aminus, OUTPUT);
  pinMode(Bplus, OUTPUT);
  pinMode(Bminus, OUTPUT);
  digitalWrite(Aplus, LOW);
  digitalWrite(Aminus, LOW);
  digitalWrite(Bplus, LOW);
  digitalWrite(Bminus, LOW);
}
void Casting(int Desi_Dist) {
  int Cur_Dist = 0;
  bool A = 0;
  bool B = 0;
  Desi_Dist = Desi_Dist*(1200/10);
  taskDISABLE_INTERRUPTS();
  while(Cur_Dist <= Desi_Dist){
    Serial.println(Cur_Dist);
    if(A && B){
      B = 0;
      digitalWrite(Bplus, LOW);
      Serial.println("1");//0.270ms
      vTaskDelay(pdMS_TO_TICKS(1));
      //Serial.println("2");
      digitalWrite(Bminus, HIGH);
    }
    else if(A && ~B){
      A = 0;
      digitalWrite(Aplus, LOW);
      Serial.println("1");//0.270ms
      vTaskDelay(pdMS_TO_TICKS(1));
      //Serial.println("2");
      digitalWrite(Aminus, HIGH);
    }
    else if(~A && B){
      A = 1;
      digitalWrite(Aminus, LOW);
      Serial.println("1");//0.270ms
      vTaskDelay(pdMS_TO_TICKS(1));
      //Serial.println("2");
      digitalWrite(Aplus, HIGH);
    }
    else if(~A && ~B){
      B = 1;
      digitalWrite(Bminus, LOW);
      Serial.println("1");//0.270ms
      vTaskDelay(pdMS_TO_TICKS(1));
      //Serial.println("2");
      digitalWrite(Bplus, HIGH);  
    }
    //delay(10);
    Serial.println("abcdefghijklmnop");//~4ms
    Serial.println("abc");//~1ms

    vTaskDelay(pdMS_TO_TICKS(10));
    Cur_Dist++;
    if(digitalRead(redButton) == LOW){
      xTimerStop(castTimer, 0);
      digitalWrite(Aplus, LOW);
      digitalWrite(Aminus, LOW);

      Serial.println("abc");//~1ms
      Serial.println("abcdefghijklmnop");//~4ms
      Serial.println("abcdefghijklmnop");//~4ms
      Serial.println("abcdefghijklmnop");//~4ms
      Serial.println("abcdefghijklmnop");//~4ms
      Serial.println("abcdefghijklmnop");//~4ms
      
      vTaskDelay(pdMS_TO_TICKS(1));
      digitalWrite(Bplus, LOW);
      digitalWrite(Bminus, LOW);
      Serial.println("out");
      taskENABLE_INTERRUPTS();
      return;
    }
  }
  vTaskDelay(pdMS_TO_TICKS(2000));
  digitalWrite(Aplus, LOW);
  digitalWrite(Aminus, LOW);
  
  Serial.println("abc");//~1ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms

  vTaskDelay(pdMS_TO_TICKS(1));
  digitalWrite(Bplus, LOW);
  digitalWrite(Bminus, LOW);
  taskENABLE_INTERRUPTS();
}
//==============================================
void pressButton() {
  digitalWrite(A0, LOW);
  digitalWrite(A1, HIGH);
}
//==============================================
void releaseButton() {
  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
}

void solenoidOff() {
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
}

void motorsOFF() {
  //reeling motor off
  analogWrite(ENA, 0);
  //thumb button motor off
  solenoidOff();
  //stepper motor off
  digitalWrite(Aplus, LOW);
  digitalWrite(Aminus, LOW);

    Serial.println("abc");//~1ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  Serial.println("abcdefghijklmnop");//~4ms
  vTaskDelay(pdMS_TO_TICKS(1));
  
  digitalWrite(Bplus, LOW);
  digitalWrite(Bminus, LOW);
}

