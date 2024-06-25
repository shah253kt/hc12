#include <Arduino.h>

#include "HC12.h"

HC12 hc12(Serial2, 8);

void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600);
    
    hc12.setBaudRate(9600);
    hc12.setChannel(50);
}

void loop()
{
    hc12.send("My text");
    hc12.send(123);
    hc12.send(123.45, 5, 2);

    if (hc12.available())
    {
        char c = hc12.read();
        // Process data here
    }
}
