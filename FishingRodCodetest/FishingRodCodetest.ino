#include <Arduino_FreeRTOS.h> //hello
#include <timers.h>
#include <event_groups.h>
#include "fledstrip.h"
#include "fmotors.h"
/************************************************************************************************
SETTINGs
************************************************************************************************/
#define casttimerPeriod pdMS_TO_TICKS(200)// milli secs

//pins
const uint8_t powerButton = 4;
const uint8_t blueButton = 5;
const uint8_t greenButton = 6;
const uint8_t redButton = 7;

const uint8_t joyStick = A2;

/************************************************************************************************

************************************************************************************************/

// //DC MOTOR
// #define ENA 11

#define castFlag ( 1 << 0 )
#define powerFlag ( 1 << 1 )
#define emergencyFlag ( 1 << 2 )
#define reelFlag ( 1 << 4 )
#define allFlags (reelFlag | emergencyFlag | powerFlag | castFlag)

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200; // Set debounce time (in milliseconds)

volatile uint16_t joystickValue = 0;

volatile uint16_t pushButtoncount = 0;

extern int casted;

extern uint8_t maxLEDnum;
extern bool hasBeenAtOneBefore;
extern bool wentAboveOne;

/* Declare a variable to hold the created event group. */
EventGroupHandle_t fishingrodEvents;

TimerHandle_t castTimer;

TaskHandle_t const castTask;
TaskHandle_t const reelTask;
TaskHandle_t const emergencyTask;

// define=
void TaskReelMode( void *pvParameters );
void TaskCastMode( void *pvParameters );
void TaskEmergencyMode( void *pvParameters );

// the setup function runs once when you press reset or power the board

void setup() {
  LEDStrip_setup();

  fmotors_setup();

  Serial.begin(115200);
  Serial.println("serial");
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }
  Serial.println("serial done");
  pinMode(powerButton, INPUT_PULLUP);
  pinMode(blueButton, INPUT_PULLUP);
  pinMode(greenButton, INPUT_PULLUP);
  pinMode(redButton, INPUT_PULLUP);

  pinMode(joyStick, INPUT);

  // Enable pin change interrupt for green button (pin 4 is in PCINT2 group)
  PCICR |= (1 << PCIE2);    // Enable pin change interrupt for PCINT[23:16]
  PCMSK2 |= (1 << PCINT20); // Enable interrupt on pin 4 (PCINT20)
  PCMSK2 |= (1 << PCINT21); // Enable interrupt on pin 5 (PCINT21)
  PCMSK2 |= (1 << PCINT22); // Enable interrupt on pin 6 (PCINT22)
  PCMSK2 |= (1 << PCINT23); // Enable interrupt on pin 7 (PCINT23)

  /* Attempt to create the event group. */
  fishingrodEvents = xEventGroupCreate();
  /* Was the event group created successfully? */
  if( fishingrodEvents == NULL )
  {
    /* The event group was not created because there was insufficient FreeRTOS heap available. */
    while(1);
  }
  xTaskCreate(
    TaskReelMode
    ,  "ReelMode"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  1  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &reelTask ); //Task Handle
  xTaskCreate(
    TaskCastMode
    ,  "CastMode" // A name just for humans
    ,  128  // Stack size
    ,  NULL //Parameters for the task
    ,  1  // Priority
    ,  &castTask ); //Task Handle
  // // Now set up two Tasks to run independently.
  xTaskCreate(
    TaskEmergencyMode
    ,  "EmergencyMode"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  3  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &emergencyTask ); //Task Handle

  castTimer = xTimerCreate(
    "Cast timer"
    , casttimerPeriod
    , pdTRUE
    , 0
    , casttimerCallback);
  xTimerStart(castTimer, 0);
  Serial.println("setup done");
}

void loop() {
  // put your main code here, to run repeatedly:

}

ISR(PCINT2_vect) {
  //unsigned long currentTime = millis();
  //if ((currentTime - lastDebounceTime) > debounceDelay) { // Only trigger if debounce delay has passed
  //  lastDebounceTime = currentTime; // Update last debounce time
    // This ISR will be triggered when any pin change occurs on PCINT[23:16] group
    if (digitalRead(greenButton) == LOW) {
      // Handle green button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, reelFlag);/* The bits being set. */
      Serial.println("green");
    } else if (digitalRead(blueButton) == LOW) {
      // Handle blue button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, castFlag);/* The bits being set. */
      Serial.println("blue");
    } else if (digitalRead(redButton) == LOW) {
      // Handle red button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, emergencyFlag);/* The bits being set. */
      Serial.println("red");
    } else if (digitalRead(powerButton) == LOW) {
      // Handle power button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, powerFlag);/* The bits being set. */
    }
  //}
}
void casttimerCallback(TimerHandle_t castTimer){
  //Serial.println("READ CAST TIMER");
  joystickValue = analogRead(joyStick);
  //Serial.println(joystickValue);
  if(pushButtoncount <= 8) {
    pushButtoncount++;
  }
}
void TaskReelMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
      LEDStripColor(LEDStripDIMYELLOW, dataPin1);
      xEventGroupClearBits(fishingrodEvents, allFlags);

      taskENTER_CRITICAL(); // Disable task switching
      taskDISABLE_INTERRUPTS(); // Disable interrupts to protect the critical section
      releaseButton();
      vTaskDelay(pdMS_TO_TICKS(500));
      solenoidOff();
      taskEXIT_CRITICAL(); // Disable task switching
      taskENABLE_INTERRUPTS(); // Disable interrupts to protect the critical section
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (reelFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & reelFlag) == reelFlag){
       //Serial.println("Reel flag set");
     }
      releaseButton();
      //control motors
      xTimerStart(castTimer, 0);
      uint8_t reelSpeed = 0;
      uint8_t oldLEDnum = 20;
      while(1){
        if((castFlag | emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
          xTimerStop(castTimer, 0);
          //casted = 0;
          break;
        }
        updateReelAndLEDs(joystickValue, oldLEDnum, dataPin1);
      }
      //reel motor off
      analogWrite(ENA, 0);
      solenoidOff();

      xEventGroupClearBits(fishingrodEvents, reelFlag);
      //vTaskDelay(pdMS_TO_TICKS(1500));  // Let the flag clear settle
  }
}
void TaskCastMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (castFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & castFlag) == castFlag){
      Serial.println("Blue Button");
    }

    // Extra cleanup before pressing the button
    releaseButton();
    solenoidOff();  // if it's safe to turn this off here

    vTaskDelay(pdMS_TO_TICKS(100)); // Let hardware settle

    Serial.println("About to press button from Cast Mode");
    pressButton();
    pushButtoncount = 0;


    //pressButton();

    xTimerStart(castTimer, 0);
    uint8_t distance = 0;
    uint8_t oldLEDnum = 20;
    casted = 1;
    uint8_t buttonDone = 1;
    while(1){
      if((reelFlag | emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
        xTimerStop(castTimer, 0);
        casted = 1;
        maxLEDnum = 1;
        hasBeenAtOneBefore = false;
        wentAboveOne = false;
        break;
      }
      if((pushButtoncount >= 8) && buttonDone){
        solenoidOff();
        buttonDone = 0;
      }
      updateDistanceAndLEDs(joystickValue, oldLEDnum, dataPin1);
    }
    
    xEventGroupClearBits(fishingrodEvents, castFlag);
    pushButtoncount = 0;
    buttonDone = 1;
  }
}
void TaskEmergencyMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (emergencyFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & emergencyFlag) == emergencyFlag){
      Serial.println("Red Button");
    }

    LEDStripColor(LEDStripRED, dataPin1);
    taskENTER_CRITICAL(); // Disable task switching
    taskDISABLE_INTERRUPTS(); // Disable interrupts to protect the critical section
    motorsOFF();
    releaseButton();
    vTaskDelay(pdMS_TO_TICKS(1500));
    solenoidOff();
    taskENABLE_INTERRUPTS();
    taskEXIT_CRITICAL(); // Enable task switching


    xEventGroupClearBits(fishingrodEvents, emergencyFlag);
  }
}
