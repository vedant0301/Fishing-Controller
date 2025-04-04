#include <Arduino_FreeRTOS.h>//hello whats up 
#include <semphr.h>  // add the FreeRTOS functions for Semaphores (or Flags).
#include <timers.h>
#include <event_groups.h>
#include "fledstrip.h"
#include "fmotors.h"
/************************************************************************************************
SETTINGs
************************************************************************************************/
#define casttimerPeriod pdMS_TO_TICKS(200)// milli secs // joystick 
#define reeltimerPeriod pdMS_TO_TICKS(200)// milli secs // joystick 

//pins
const uint8_t powerButton = 4;
const uint8_t blueButton = 5;
const uint8_t greenButton = 6;
const uint8_t redButton = 7;

const uint8_t joyStick = A2;

/************************************************************************************************

************************************************************************************************/

// //DC MOTOR
// #define IN1 12
// #define IN2 13
// #define ENA 11

//leds strip
SemaphoreHandle_t xLEDMutex;

#define castFlag ( 1 << 0 )
#define powerFlag ( 1 << 1 )
#define emergencyFlag ( 1 << 2 )
#define reelFlag ( 1 << 4 )
#define startdoneFlag ( 1 << 5 )
#define allFlags (startdoneFlag | reelFlag | emergencyFlag | powerFlag | castFlag)

volatile unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 200; // Set debounce time (in milliseconds)

volatile uint16_t joystickValue = 0;

/* Declare a variable to hold the created event group. */
EventGroupHandle_t fishingrodEvents;

// Declare a mutex Semaphore Handle which we will use to manage the Serial Port.
// It will be used to ensure only one Task is accessing this resource at any time.
SemaphoreHandle_t xcastmotorMutex;
SemaphoreHandle_t xreelmotorMutex;
SemaphoreHandle_t xplungermotorMutex;

TimerHandle_t castTimer;
TimerHandle_t reelTimer;

TaskHandle_t const castTask;
TaskHandle_t const reelTask;
TaskHandle_t const startTask;
TaskHandle_t const emergencyTask;

// define=
void TaskStart( void *pvParameters );
void TaskReelMode( void *pvParameters );
void TaskCastMode( void *pvParameters );
void TaskEmergencyMode( void *pvParameters );

// the setup function runs once when you press reset or power the board

void setup() {
  LEDStrip_setup();

  //fmotors_setup();
  // // DC MOTOR
  // pinMode(IN1, OUTPUT);
  // pinMode(IN2, OUTPUT);
  // pinMode(ENA, OUTPUT);
  // digitalWrite(IN1, LOW);
  // digitalWrite(IN2, LOW);
  // analogWrite(ENA, 0);

  // // Push Button Motor
  // pinMode(IN3, OUTPUT);
  // pinMode(IN4, OUTPUT);
  // digitalWrite(IN3, LOW);
  // digitalWrite(IN4, LOW);

  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }
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
  xLEDMutex = xSemaphoreCreateMutex();

  if ( xcastmotorMutex == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xcastmotorMutex = xSemaphoreCreateMutex();  // Create a mutex semaphore
    if ( ( xcastmotorMutex ) != NULL )
      xSemaphoreGive( ( xcastmotorMutex ) );  // "Giving" the Semaphore.
  }
  if ( xreelmotorMutex == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xreelmotorMutex = xSemaphoreCreateMutex();  // Create a mutex semaphore
    if ( ( xreelmotorMutex ) != NULL )
      xSemaphoreGive( ( xreelmotorMutex ) );  // "Giving" the Semaphore.
  }
  if ( xplungermotorMutex == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
    xplungermotorMutex = xSemaphoreCreateMutex();  // Create a mutex semaphore
    if ( ( xplungermotorMutex ) != NULL )
      xSemaphoreGive( ( xplungermotorMutex ) );  // "Giving" the Semaphore.
  }


  //ledEventGroup = xEventGroupCreate();
  // Create a dedicated task for LED updates
  //Serial.println(xTaskCreate(TaskLEDUpdate, "LED Update", 128, NULL, 1, NULL) + 1);

  // Now set up two Tasks to run independently.
    Serial.println(xTaskCreate(
    TaskStart
    ,  "Start"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL //Parameters for the task
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  &startTask )); //Task Handle
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
  //xTimerStart(castTimer, 0);
  reelTimer = xTimerCreate(
    "Reel timer"
    , reeltimerPeriod
    , pdTRUE
    , 0
    , reeltimerCallback);
  //xTimerStart(reelTimer, 0);
}

void loop() {
  // put your main code here, to run repeatedly:
}

ISR(PCINT2_vect) {
  // unsigned long currentTime = millis();
  // if ((currentTime - lastDebounceTime) > debounceDelay) { // Only trigger if debounce delay has passed
  //   lastDebounceTime = currentTime; // Update last debounce time
    // This ISR will be triggered when any pin change occurs on PCINT[23:16] group
    if (digitalRead(greenButton) == LOW) {
      // Handle green button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, reelFlag);/* The bits being set. */
      Serial.println("green");
    } else if (digitalRead(blueButton) == LOW) {
      // Handle blue button press
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, castFlag);/* The bits being set. */
      //Serial.println("green");
    } else if (digitalRead(redButton) == LOW) {
      // Handle red button press
      //EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, emergencyFlag);/* The bits being set. */
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, powerFlag);/* The bits being set. */
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
}
void reeltimerCallback(TimerHandle_t reelTimer){
  //Serial.println("READ REEL TIMER");
  joystickValue = analogRead(joyStick);
  //Serial.println(joystickValue);
}

void TaskStart( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  uint8_t oNofF = 0;
  LEDStripColor(LEDStripOFF, dataPin1);
  LEDStripColor(LEDStripOFF, dataPin2);
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits = xEventGroupWaitBits(fishingrodEvents, powerFlag, pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits & powerFlag) == powerFlag){
      //Serial.println("Power Button task");
    }
    oNofF ^= 1;
    // See if we can obtain or "Take" the Serial Semaphore. // If the semaphore is not available, wait Scheduler to see if it becomes free.
    if ( ((xSemaphoreTake( xcastmotorMutex, portMAX_DELAY) && xSemaphoreTake( xreelmotorMutex, portMAX_DELAY)&& xSemaphoreTake( xplungermotorMutex, portMAX_DELAY)) == pdTRUE) && oNofF )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource. so we don't want it getting stolen during the middle of a conversion.
      Serial.println("ON");
      xEventGroupClearBits(fishingrodEvents, allFlags);

      LEDStripColor(LEDStripDIMYELLOW, dataPin1);

      vTaskDelay(pdMS_TO_TICKS(50));

      xEventGroupClearBits(fishingrodEvents, powerFlag);
      xSemaphoreGive( xcastmotorMutex ); // Now free or "Give" the Serial Port for others.
      xSemaphoreGive( xreelmotorMutex );
      xSemaphoreGive( xplungermotorMutex );
      EventBits_t eventBits = xEventGroupSetBits(fishingrodEvents, startdoneFlag);/* The bits being set. */

    }
    else{
      xEventGroupClearBits(fishingrodEvents, allFlags);

      LEDStripColor(LEDStripOFF, dataPin1);

      releaseButton();
      vTaskDelay(pdMS_TO_TICKS(500));

      motorsOFF();

      vTaskDelay(pdMS_TO_TICKS(50));

      xSemaphoreGive( xcastmotorMutex ); // Now free or "Give" the Serial Port for others.
      xSemaphoreGive( xreelmotorMutex );
      xSemaphoreGive( xplungermotorMutex );
    }
  }
}
void TaskReelMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (startdoneFlag|reelFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & startdoneFlag) == startdoneFlag){
      //Serial.println("Start done flag");
    }
    if((eventBits1 & reelFlag) == reelFlag){
       //Serial.println("Reel flag set");
     }
    // See if we can obtain or "Take" the Serial Semaphore.
    if ( (xSemaphoreTake( xcastmotorMutex, portMAX_DELAY) && xSemaphoreTake( xreelmotorMutex, portMAX_DELAY)&& xSemaphoreTake( xplungermotorMutex, portMAX_DELAY)) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource. so we don't want it getting stolen during the middle of a conversion.

      //control motors
      xTimerStart(reelTimer, 0);
      uint8_t reelSpeed = 0;
      uint8_t oldLEDnum = 20;
      while(1){
        if((castFlag | emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
          xTimerStop(reelTimer, 0);
          break;
        }
        updateReelAndLEDs(joystickValue, oldLEDnum, dataPin1);//, IN1, IN2, ENA);

      }
      //reel motor off
      analogWrite(ENA, 0);

      xEventGroupClearBits(fishingrodEvents, reelFlag);
      xSemaphoreGive( xcastmotorMutex ); // Now free or "Give" the Serial Port for others.
      xSemaphoreGive( xreelmotorMutex );
      xSemaphoreGive( xplungermotorMutex );
    }
  }
}
void TaskCastMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (startdoneFlag|castFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & startdoneFlag) == startdoneFlag){
      //Serial.println("start done cast");
    }
    if((eventBits1 & castFlag) == castFlag){
      //Serial.println("Blue Button");
    }
    // See if we can obtain or "Take" the Serial Semaphore.
    if ( (xSemaphoreTake( xcastmotorMutex, portMAX_DELAY) && xSemaphoreTake( xreelmotorMutex, portMAX_DELAY)&& xSemaphoreTake( xplungermotorMutex, portMAX_DELAY)) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource. so we don't want it getting stolen during the middle of a conversion.

      pressButton();

      xTimerStart(castTimer, 0);
      uint8_t distance = 0;
      uint8_t oldLEDnum = 20;
      while(1){
        if((reelFlag | emergencyFlag | powerFlag) & xEventGroupGetBits(fishingrodEvents)){
          xTimerStop(castTimer, 0);
          break;
        }
        updateDistanceAndLEDs(joystickValue, oldLEDnum, dataPin1);
      }
      releaseButton();
      vTaskDelay(pdMS_TO_TICKS(500));
      
      xEventGroupClearBits(fishingrodEvents, castFlag);
      xSemaphoreGive( xcastmotorMutex ); // Now free or "Give" the Serial Port for others.
      xSemaphoreGive( xreelmotorMutex );
      xSemaphoreGive( xplungermotorMutex );
    }
  }
}
void TaskEmergencyMode( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  /*
    TaskStart
  */
  for (;;) // A Task shall never return or exit.
  {
    EventBits_t eventBits1 = xEventGroupWaitBits(fishingrodEvents, (startdoneFlag|emergencyFlag), pdFALSE, pdTRUE, portMAX_DELAY);
    if((eventBits1 & startdoneFlag) == startdoneFlag){
      //Serial.println("start done cast");
    }
    if((eventBits1 & emergencyFlag) == emergencyFlag){
      Serial.println("Red Button");
    }
    // See if we can obtain or "Take" the Serial Semaphore.
    if ( (xSemaphoreTake( xcastmotorMutex, portMAX_DELAY) && xSemaphoreTake( xreelmotorMutex, portMAX_DELAY)&& xSemaphoreTake( xplungermotorMutex, portMAX_DELAY)) == pdTRUE )
    {
      // We were able to obtain or "Take" the semaphore and can now access the shared resource. so we don't want it getting stolen during the middle of a conversion.

      LEDStripColor(LEDStripRED, dataPin1);

      //motorsOFF();
      // digitalWrite(IN1, LOW);
      // digitalWrite(IN2, LOW);
      // analogWrite(ENA, 0);
      //releaseButton();

      vTaskDelay(pdMS_TO_TICKS(50));

      xEventGroupClearBits(fishingrodEvents, emergencyFlag);

      xSemaphoreGive( xcastmotorMutex ); // Now free or "Give" the Serial Port for others.
      xSemaphoreGive( xreelmotorMutex );
      xSemaphoreGive( xplungermotorMutex );
    }
  }
}
