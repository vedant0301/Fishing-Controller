#include "fmotors.h"
int casted;

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
    // Calculate number of LEDs
    //numberofLEDs = (reelSpeed * 2) / 10 ;

    // Update LEDs and motor only if number of LEDs changes or oldLEDnum is 20
    if ((numberofLEDs != oldLEDnum) || (oldLEDnum == 20)) {
        showColor(0, 32, 0, numberofLEDs, pin);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, HIGH);
        analogWrite(ENA, 28*reelSpeed);
        oldLEDnum = numberofLEDs;
    }
}

void updateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin) {
    int distance;
    uint8_t numberofLEDs;

    // Determine distance based on joystickValue
    if (joystickValue < 113) {
        distance = 5;
    } else if (joystickValue < 169) {
        distance = 10;
    } else if (joystickValue < 282) {
        distance = 15;
    } else if (joystickValue < 395) {
        distance = 20;
    } else if (joystickValue < 508) {
        distance = 25;
    } else if (joystickValue < 621) {
        distance = 30;
    } else if (joystickValue < 734) {
        distance = 35;
    } else if (joystickValue < 847) {
        distance = 40;
    } else if (joystickValue < 960) {
        distance = 45;
    } else {
        distance = 50;
    }

    // Calculate number of LEDs
    numberofLEDs = (distance * 2) / 10;

    // Update LEDs only if numberofLEDs changes
    if (numberofLEDs != oldLEDnum) {
        showColor(0, 0, 32, numberofLEDs, pin); // Set LEDs to blue
    }

    // **Track highest reached LED count**
    static uint8_t maxLEDnum = 1;
    if (numberofLEDs > maxLEDnum) {
        maxLEDnum = numberofLEDs;  // Store highest LED count
    }

    // **Ensure user moves joystick before first cast**
    static bool mustStartAtOne = true;
    static bool movedAwayFromOne = false;

    if (numberofLEDs == 1) {
        if (!movedAwayFromOne) {
            mustStartAtOne = true; // Still need to move up first
        }
    } else {
        movedAwayFromOne = true; // User has moved joystick up
    }

    if (oldLEDnum == 1 && movedAwayFromOne) {
        mustStartAtOne = false;
    }

    // **Casting happens based on the highest LED count reached**
    if (!mustStartAtOne && numberofLEDs == 1 && casted == 0) {
        casted = 1;  // Prevent multiple triggers
        xTimerStop(castTimer, 0);
        Casting(maxLEDnum);  // Cast using highest LED count reached
        //vTaskDelay(pdMS_TO_TICKS(100));  // Small delay to avoid retriggering
        xTimerStart(castTimer, 0);
        vTaskDelay(pdMS_TO_TICKS(190));
        releaseButton();

        // **Reset maxLEDnum after casting**
        maxLEDnum = 1;
    }

    // Reset casted flag if joystick moves away from casting zone (more than 1 LED)
    if (numberofLEDs > 1) {
        casted = 0;
    }

    // Update old LED number for next iteration
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
    if(digitalRead(redButton) == LOW){//((emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
      xTimerStop(castTimer, 0);
      digitalWrite(Aplus, LOW);
      digitalWrite(Aminus, LOW);
            Serial.println("1");
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
        Serial.println("1");
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
  vTaskDelay(pdMS_TO_TICKS(1));
  digitalWrite(Bplus, LOW);
  digitalWrite(Bminus, LOW);
}

