#include <Arduino.h>

void sinelon();
void reconnectWifi();
void handleLeds();
void handleServer();
void parseColor(uint32_t color);
void setAlarmTime(String time);
void setAlarmOffsetTime(uint32_t time);
void setAlarmWeekdays(uint8_t weekdays);
void setupWifi();
void getPreferences();
void getTime();
void handleFakeSun();
void intervalCounter();