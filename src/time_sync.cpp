#include <WiFiUdp.h>
#include <NTPClient.h>
#include <RTClib.h>
#include "connectivity.h"
#include "helpers.h"

bool syncTime(int buzzer, int internalLed, RTC_DS3231 rtc) {
  // Connect wifi
  if (!connect_wifi(buzzer, internalLed, 8000)) return false;

  // Retrieve time
  WiFiUDP ntpUDP;
  NTPClient timeClient(ntpUDP);
  bool ledState = digitalRead(internalLed) == HIGH;
  ulong startTime = millis();

  timeClient.setTimeOffset(0);
  timeClient.forceUpdate();

  while (millis() - startTime <= 3000UL)
  {
    timeClient.update();

    digitalWrite(internalLed, ledState ? LOW : HIGH);
    ledState = !ledState;
    Serial.print(".");
    delay(50);
  }
  digitalWrite(internalLed, LOW);
  Serial.println();
  timeClient.end();

  // If failed
  if (timeClient.getEpochTime() <= 6) {
    Serial.println("Failed to sync time");
    return false;
  }

  // If succeeded
  DateTime utcTime = DateTime(timeClient.getEpochTime());
  rtc.adjust(utcTime);

  // Print time
  Serial.print("Time synced: ");
  print_datetime(utcTime);

  return true;
}