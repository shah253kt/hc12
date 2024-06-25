#pragma once

#include "Arduino.h"

#ifndef NO_SET_PIN
#define NO_SET_PIN -1
#endif

class HC12
{
public:
    HC12(Stream &stream, uint8_t setPin = NO_SET_PIN);
    ~HC12();

    Stream *getStream();
    int available();
    int read();
    void update();
    void send(char *msg);
    void send(int value);
    void send(float value, uint8_t width = 6, uint8_t precision = 2);

    bool setBaudRate(uint32_t baudRate);
    bool setChannel(uint8_t channel);

    void setCommandMode(bool isCommandMode);
    void resetResponse();

    void (*onResponseAvailable)(char *response);

private:
    Stream *m_stream;
    uint8_t m_setPin;
    char *m_response;
    uint8_t m_currentResponseIndex;

    bool sendCommandExpectingOkResponse(char *msg);
    bool isResponseFull();
};
