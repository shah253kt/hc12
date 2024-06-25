#include <Arduino.h>

#include "HC12.h"

HC12 hc12(Serial2, 8);

void matchCallback(const char *match,         // matching string (not null-terminated)
                    const unsigned int length, // length of matching string
                    const MatchState &ms)      // MatchState in use (to get captures)
{
  char cap[10]; // must be large enough to hold captures

  Serial.print("Matched: ");
  Serial.write((byte *)match, length);
  Serial.println();

  for (byte i = 0; i < ms.level; i++)
  {
    Serial.print("Capture ");
    Serial.print(i, DEC);
    Serial.print(" = ");
    ms.GetCapture(cap, i);
    Serial.println(cap);
  } // end of for each capture
}

void setup()
{
  Serial.begin(115200);
  Serial2.begin(9600);

  hc12.setOnResponseMatchCallback("(%a+)( )", matchCallback);
}

void loop()
{
  hc12.update();
}
