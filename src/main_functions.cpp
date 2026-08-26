#include <RTClib.h>
#include <timezone.h>

DateTime getNextAlarmDate(RTC_DS3231* rtc, DateTime alarmTime) {
  DateTime utcNow = rtc->now();
  DateTime localNow = utcToLocal(utcNow);
  DateTime localNextAlarm = DateTime(
    localNow.year(),
    localNow.month(),
    localNow.day(),
    alarmTime.hour(),
    alarmTime.minute(),
    alarmTime.second()
  );

  if (localNow >= localNextAlarm)
    localNextAlarm = localNextAlarm + TimeSpan(1, 0, 0, 0); // Add a day

  DateTime utcNextAlarm = localToUtc(localNextAlarm);
  return utcNextAlarm;
}