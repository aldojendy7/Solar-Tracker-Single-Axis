#include <DS3231.h>
DS3231  rtc(SDA, SCL);
Time  t;

// setup the RTC to SDA SCL communication
void rtc_setup(){
  rtc.begin();
}

// Get RTC hour data from 
byte get_rtc(){
  // Get data from the DS3231
  t = rtc.getTime();
  byte data = t.hour;
  return data;
}
