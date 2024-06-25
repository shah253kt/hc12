#include <Arduino.h>

#include "HC12.h"

HC12 hc12(Serial2, 8);

void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600);

    hc12.setBaudRate(9600);
    hc12.setChannel(50);

    // In this case, you can add Regex for example to check if the response match anything of interest to you.
    hc12.onResponseAvailable = [](char *response)
    {
        if (false) // Check if response is ready to be processed.
        {
            return;
        }

        hc12.resetResponse(); // Reset the response once it is processed.
    };
}

void loop()
{
    hc12.send("My text");
    hc12.send(123);
    hc12.send(123.45, 5, 2);
    hc12.update();
}
