#include <Arduino.h>
#include <WiFi.h>

const char* ssid = "";
const char* password = "";

bool connect_wifi(int buzzer, int internalLed, ulong timeout)
{
  WiFi.mode(WIFI_STA);

  // Disconnect
  WiFi.disconnect();
  delay(100);

  // Connect
  ulong start = millis();
  bool ledState = digitalRead(internalLed) == HIGH;

  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");

  while (WiFi.status() != WL_CONNECTED && millis() - start < timeout)
  {
    digitalWrite(internalLed, ledState ? LOW : HIGH);
    ledState = !ledState;
    Serial.print('.');
    delay(100);
  }

  // Return
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("Connected: ");
    Serial.println(WiFi.localIP());
  } else
    Serial.println("WIFI connection failed.");

  digitalWrite(internalLed, LOW);

  return WiFi.status() == WL_CONNECTED;
}