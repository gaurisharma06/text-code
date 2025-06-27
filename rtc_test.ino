#include "RTClib.h"
RTC_PCF8563 rtc;

void setup() {
  Serial.begin(115200);
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }

  if (rtc.lostPower()) {
    // Serial.println("RTC lost power, setting the time!");
    // Set the RTC to the date & time this sketch was compiled
    rtc.adjust(DateTime(2025,3,19,2,0,1));
  }

  if (! rtc.isrunning()) {
  Serial.println("RTC is NOT running, let's set the time!");
  // When time needs to be set on a new device, or after a power loss, the
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }
}

void loop() {
  DateTime time = rtc.now();

    //Full Timestamp
   Serial.println(String("DateTime::TIMESTAMP_FULL:\t")+time.timestamp(DateTime::TIMESTAMP_FULL));

  //Date Only
   Serial.println(String("DateTime::TIMESTAMP_DATE:\t")+time.timestamp(DateTime::TIMESTAMP_DATE));

  //Full Timestamp
   Serial.println(String("DateTime::TIMESTAMP_TIME:\t")+time.timestamp(DateTime::TIMESTAMP_TIME));

   Serial.println("\n");

   delay(5000);
}
