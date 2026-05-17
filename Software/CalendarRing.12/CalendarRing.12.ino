// CalendarRing
// 365 LED ring as a visual indicator of progression through the year.
// Shows first days of every month, solstices/equinoxes, and current day.
// Range finder senses proximity and controls ring brightness.
//
// Matt Pfeiffer - 2026
//
// Architecture: all logic lives in Logic.h / Logic.cpp.
// Hardware is wrapped in: RTC, SonarSensor, OledDisplay, mkp_dotStar.
// Calendar math lives in CalendarData.
// Set DEBUG = true in Logic.h to bypass sonar and run at full brightness.

// This is the Claud code version

#include "Logic.h"

Logic logic; //

void setup() { 
  logic.setup(); 
}
void loop()  { 
  logic.loop();  
}
