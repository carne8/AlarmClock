#include <AceTime.h>
#include <RTClib.h>

using namespace ace_time;

static ExtendedZoneProcessor parisProcessor;

TimeZone utcTz = TimeZone();
TimeZone parisTz = TimeZone::forZoneInfo(
  &zonedbx::kZoneEurope_Paris,
  &parisProcessor
);

ZonedDateTime dateTimeToZoned(DateTime dt, TimeZone tz) {
  return ZonedDateTime::forComponents(
    dt.year(),
    dt.month(),
    dt.day(),
    dt.hour(),
    dt.minute(),
    dt.second(),
    tz
  );
}

DateTime zonedToDateTime(ZonedDateTime dt) {
  return DateTime(
    dt.year(),
    dt.month(),
    dt.day(),
    dt.hour(),
    dt.minute(),
    dt.second()
  );
}

DateTime localToUtc(DateTime dt) {
  return zonedToDateTime(
    dateTimeToZoned(dt, parisTz)
      .convertToTimeZone(utcTz)
  );
}

DateTime utcToLocal(DateTime dt) {
  return zonedToDateTime(
    dateTimeToZoned(dt, utcTz)
      .convertToTimeZone(parisTz)
  );
}