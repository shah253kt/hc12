#include <Arduino.h>

#include "HC12.h"

HC12 hc12(Serial2, 8);

void setup()
{
    Serial.begin(115200);
    Serial2.begin(9600);

    // hc12.setCommandMode(true);
    hc12.onCharacterReceived('\n', []
                             {
                                Serial.print("Response: ");
                                Serial.println(hc12.getResponse());
                                hc12.resetResponse(); });
}

void loop()
{
    hc12.update();

    if (Serial.available())
    {
        char c = Serial.read();

        switch (c)
        {
        case '1':
        {
            Serial.print("Setting baud rate: ");
            Serial.println(hc12.setBaudRate(9600) ? "OK" : "Failed");
            break;
        }
        case '2':
        {
            Serial.print("Setting channel: ");
            Serial.println(hc12.setChannel(50) ? "OK" : "Failed");
            break;
        }
        case '3':
        {
            Serial.print("Getting channel: ");
            const auto channel = hc12.getChannel();
            if (channel == INVALID_CHANNEL)
            {
                Serial.println("Invalid channel received.");
            }
            else
            {
                Serial.println(channel);
            }
            break;
        }
        }
    }

    // if (Serial.available())
    // {
    //     hc12.getStream()->write(Serial.read());
    // }

    // if (hc12.getStream()->available())
    // {
    //     Serial.write(hc12.getStream()->read());
    // }
}
