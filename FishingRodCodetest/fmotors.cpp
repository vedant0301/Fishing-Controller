#include "fmotors.h"
int casted;

void fmotors_setup() {
  // DC MOTOR
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(ENA, OUTPUT);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
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
      reelSpeed = 5;
    } else if (joystickValue < 169) {
        reelSpeed = 10;
    } else if (joystickValue < 282) {
        reelSpeed = 15;
    } else if (joystickValue < 395) {
        reelSpeed = 20;
    } else if (joystickValue < 508) {
        reelSpeed = 25;
    } else if (joystickValue < 621) {
        reelSpeed = 30;
    } else if (joystickValue < 734) {
        reelSpeed = 35;
    } else if (joystickValue < 847) {
        reelSpeed = 40;
    } else if (joystickValue < 960) {
        reelSpeed = 45;
    } else {
        reelSpeed = 50;
    }
    // Calculate number of LEDs
    numberofLEDs = (reelSpeed * 2) / 10 ;

    // Update LEDs and motor only if number of LEDs changes or oldLEDnum is 20
    if ((numberofLEDs != oldLEDnum) || (oldLEDnum == 20)) {
        showColor(0, 255, 0, numberofLEDs, pin);
        // digitalWrite(IN1, LOW);
        // digitalWrite(IN2, HIGH);
        analogWrite(ENA, 55 + 20 * (numberofLEDs - 1));
        oldLEDnum = numberofLEDs;
    }
}


void updateDistanceAndLEDs(int joystickValue, uint8_t& oldLEDnum, uint8_t pin) {
    int distance;
    uint8_t numberofLEDs;
    int exit = 0;

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
    numberofLEDs = (distance * 2) / 10 ;

    // Update LEDs only if number of LEDs changes or oldLEDnum is 20
    if ((numberofLEDs != oldLEDnum) || (oldLEDnum == 20)) {
        showColor(0, 0, 255, numberofLEDs, pin); // Set LEDs to blue
        if(numberofLEDs == 1){
          casted = 0;
        }
        if((oldLEDnum > numberofLEDs) && (oldLEDnum!=20) && (casted == 0)){
          xTimerStop(castTimer, 0);
          //taskENTER_CRITICAL(); // Disable task switching
          //taskDISABLE_INTERRUPTS(); // Disable interrupts to protect the critical section
          Casting(oldLEDnum);
          casted = 1;
          //pressButton();
          
          //taskENABLE_INTERRUPTS();
          //taskEXIT_CRITICAL(); // Enable task switching
          xTimerStart(castTimer, 0);
          vTaskDelay(pdMS_TO_TICKS(400));
          pressButton();
        }
        oldLEDnum = numberofLEDs;
    }
}

void Stepper_Motor_setup() {
  pinMode(Aplus, OUTPUT);
  pinMode(Aminus, OUTPUT);
  pinMode(Bplus, OUTPUT);
  pinMode(Bminus, OUTPUT);
}
void Casting(int Desi_Dist) {
  int Cur_Dist = 0;
  bool A = 0;
  bool B = 0;
  Desi_Dist = Desi_Dist*(1200/10);
  while(Cur_Dist <= Desi_Dist){
    Serial.println(Cur_Dist);
    if(A && B){
      B = 0;
      digitalWrite(Bplus, LOW);
      digitalWrite(Bminus, HIGH);
    }
    else if(A && ~B){
      A = 0;
      digitalWrite(Aplus, LOW);
      digitalWrite(Aminus, HIGH);
    }
    else if(~A && B){
      A = 1;
      digitalWrite(Aminus, LOW);
      digitalWrite(Aplus, HIGH);
    }
    else if(~A && ~B){
      B = 1;
      digitalWrite(Bminus, LOW);    
      digitalWrite(Bplus, HIGH);  
    }
    //delay(10);
    vTaskDelay(pdMS_TO_TICKS(10));
    Cur_Dist++;
  }
  digitalWrite(Aplus, LOW);
  digitalWrite(Aminus, LOW);
  digitalWrite(Bplus, LOW);
  digitalWrite(Bminus, LOW);

}
//==============================================
void pressButton() {
  digitalWrite(A0, HIGH);
  digitalWrite(A1, LOW);
}
//==============================================
void releaseButton() {
  digitalWrite(A0, LOW);
  digitalWrite(A1, HIGH);
}

void solenoidOff() {
  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
}

void motorsOFF() {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, LOW);
    analogWrite(ENA, 0);
}

