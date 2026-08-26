#include <RTClib.h>

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void print_datetime(DateTime dt) {
  String yearStr = String(dt.year(), DEC);
  String monthStr = (dt.month() < 10 ? "0" : "") + String(dt.month(), DEC);
  String dayStr = (dt.day() < 10 ? "0" : "") + String(dt.day(), DEC);
  String hourStr = (dt.hour() < 10 ? "0" : "") + String(dt.hour(), DEC);
  String minuteStr = (dt.minute() < 10 ? "0" : "") + String(dt.minute(), DEC);
  String secondStr = (dt.second() < 10 ? "0" : "") + String(dt.second(), DEC);
  String dayOfWeek = daysOfTheWeek[dt.dayOfTheWeek()];
  String formattedTime = dayOfWeek + ", " + yearStr + "-" + monthStr + "-" + dayStr + " " + hourStr + ":" + minuteStr + ":" + secondStr;
  Serial.println(formattedTime);
}