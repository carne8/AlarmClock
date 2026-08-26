#include <Arduino.h>
#include <driver/rtc_io.h>
#include <RTClib.h>
#include "helpers.h"
#include "time_sync.h"
#include "music_player.h"
#include "musics.h"
#include "main_functions.h"

// --- CONFIG ---
DateTime alarmTime = DateTime(0, 0, 0, 9, 0, 0);
// ______________


const int randomSeedPin = 27;
const int internalLed = 2;
const int redLed =   14;
const int greenLed = 12;
const int blueLed =  13;

const int buzzer = 18;
const int timeSyncButton = 15;
const int stopAlarmButton = 4;
const int clockInterruptPin = 25;

static RTC_DS3231 rtc;

enum state { WAITING, PLAYING, READY_TO_SLEEP, SYNCING_TIME, SET_NEXT_ALARM };
volatile enum state state = SET_NEXT_ALARM;

DateTime nextAlarmDateTime;

TaskHandle_t mainTask;
TaskHandle_t musicTask;

void main_loop(void* _);
void music_loop(void* _);

void IRAM_ATTR onAlarm() {
  int musicIdx = random(0, music_count());
  start_playing(musicIdx);
}
void IRAM_ATTR onTimeSyncRequested() { state = SYNCING_TIME; }

volatile ulong stopAlarmButtonPressedTime;
volatile bool stopAlarmButtonPressed = false;
void IRAM_ATTR onStopAlarmChanged()
{
  bool down = digitalRead(stopAlarmButton) == LOW;
  if (down) stopAlarmButtonPressedTime = millis();
  stopAlarmButtonPressed = down;
}

// // Method to print the reason by which ESP32 has been awaken from sleep
// void print_wakeup_reason(){
//   esp_sleep_wakeup_cause_t wakeup_reason;

//   wakeup_reason = esp_sleep_get_wakeup_cause();

//   switch(wakeup_reason)
//   {
//     case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
//     case ESP_SLEEP_WAKEUP_EXT1 :
//       Serial.println("Wakeup caused by external signal using RTC_CNTL");
//       Serial.println(log(esp_sleep_get_ext1_wakeup_status())/log(2));
//       break;
//     case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
//     case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
//     case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
//     default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
//   }
// }


void setup() {
  pinMode(internalLed, OUTPUT);
  pinMode(redLed,      OUTPUT);
  pinMode(greenLed,    OUTPUT);
  pinMode(blueLed,     OUTPUT);
  pinMode(buzzer, OUTPUT);
  pinMode(timeSyncButton, INPUT_PULLUP);
  pinMode(stopAlarmButton, INPUT_PULLUP);
  randomSeed(analogRead(randomSeedPin));

  digitalWrite(internalLed, HIGH);
  digitalWrite(redLed,   LOW);
  digitalWrite(greenLed, LOW);
  digitalWrite(blueLed,  LOW);

  // Setup serial
  Serial.begin(115200);
  delay(1000);

  // Setup RTC
  if (rtc.begin()) {
    rtc.disable32K();
    rtc.clearAlarm(1);
    rtc.clearAlarm(2);
    rtc.disableAlarm(1);
    rtc.disableAlarm(2);
    rtc.writeSqwPinMode(DS3231_OFF);
    Serial.println("RTC connected");
  } else {
    Serial.println("RTC NOT connected");
    digitalWrite(redLed, HIGH);
    return;
  }

  // Check if RTC lost power
  if (rtc.lostPower()) {
    Serial.println("Lost power");
    digitalWrite(redLed,   HIGH);
    digitalWrite(greenLed, HIGH);
    state = WAITING;
  }

  // Setup interrupts
  // RTC
  pinMode(clockInterruptPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(clockInterruptPin), onAlarm, FALLING);
  // Time sync
  pinMode(timeSyncButton, INPUT_PULLDOWN);
  attachInterrupt(digitalPinToInterrupt(timeSyncButton), onTimeSyncRequested, RISING);
  // Stop alarm
  pinMode(stopAlarmButton, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(stopAlarmButton), onStopAlarmChanged, CHANGE);

  // Setup deep sleep wakeup
  esp_sleep_enable_ext1_wakeup_io(1ULL << timeSyncButton, ESP_EXT1_WAKEUP_ANY_HIGH);

  // Indicate everything works fine
  digitalWrite(internalLed, LOW);

  // Handle potential gpio wakeup source
  int gpio_reason = log(esp_sleep_get_ext1_wakeup_status())/log(2);
  if (gpio_reason == timeSyncButton) onTimeSyncRequested();

  xTaskCreatePinnedToCore(
    main_loop, // Function to implement the task
    "mainTask", // Name of the task
    10000, // Stack size in words
    NULL, // Task input parameter
    1, // Priority of the task
    NULL, // Task handle.
    0 // Core where the task should run
  );

  xTaskCreatePinnedToCore(music_loop, "musicTask", 10000, NULL, 1, NULL, 1);
}

void main_loop(void* _) {
  while (true) {
    // Stop music if requested
    if (state != READY_TO_SLEEP && stopAlarmButtonPressed && millis() - stopAlarmButtonPressedTime >= 2000) {
      stop_playing();
      state = SET_NEXT_ALARM;
    }

    // Handle state
    switch (state) {
      case WAITING: delay(50); break;

      case READY_TO_SLEEP: {
        TimeSpan sleepTime = nextAlarmDateTime - rtc.now() - TimeSpan(5);

        if (sleepTime.totalseconds() <= 0) {
          state = WAITING;
          break;
        }

        Serial.println("Going to sleep now");
        uint64_t sleepMicroSeconds = (uint64_t)sleepTime.totalseconds() * 1000000ULL;
        esp_deep_sleep(sleepMicroSeconds);
        break;
      }

      case SYNCING_TIME:
        syncTime(buzzer, internalLed, rtc);
        state = SET_NEXT_ALARM;
        break;

      case SET_NEXT_ALARM: {
        nextAlarmDateTime = getNextAlarmDate(&rtc, alarmTime);
        bool succeed = rtc.setAlarm1(nextAlarmDateTime, DS3231_A1_Date);

        // Set RTC alarm
        if (succeed) {
          Serial.print("Successfully set alarm to ");
          print_datetime(nextAlarmDateTime);
        } else
          Serial.println("Failed to set alarm");

        // Reset LED
        digitalWrite(redLed,   LOW);
        digitalWrite(greenLed, LOW);
        state = READY_TO_SLEEP;
        delay(50);
        break;
      }

      default:
        delay(50);
        break;
    }
  }
}

void music_loop(void* _) {
  while (true) { update_music(buzzer); }
}

void loop() {}