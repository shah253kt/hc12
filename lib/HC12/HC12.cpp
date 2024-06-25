#include "HC12.h"

namespace
{
    constexpr auto MAX_CHANNEL = 127;
    constexpr auto COMMAND_MODE = LOW;
    constexpr auto TRANSPARENT_MODE = HIGH;
    constexpr auto COMMAND_LENGTH = 15;
    constexpr auto RESPONSE_LENGTH = 50;
    constexpr auto RESPONSE_TIMEOUT_MS = 1000;
    constexpr auto UPDATE_TIMEOUT_MS = 100;
}

HC12::HC12(Stream &stream, uint8_t setPin)
    : m_stream{&stream},
      m_setPin{setPin},
      m_response{new char(RESPONSE_LENGTH)},
      m_currentResponseIndex{0},
      m_responseMatchCallbackIsSet{false}
{
    if (m_setPin != NO_SET_PIN)
    {
        pinMode(m_setPin, OUTPUT);
    }
}

HC12::~HC12()
{
    delete[] m_response;
}

int HC12::available()
{
    return m_stream->available();
}

int HC12::read()
{
    return m_stream->read();
}

void HC12::update()
{
    const auto updateStartedAt = millis();

    while (millis() - updateStartedAt < UPDATE_TIMEOUT_MS && available())
    {
        m_response[m_currentResponseIndex] = read();
        m_response[m_currentResponseIndex + 1] = '\0';

        if (isResponseFull())
        {
            m_currentResponseIndex = 0;
            return;
        }
    }

    if (onResponseAvailable)
    {
        onResponseAvailable(m_response);
    }

    if (!m_responseMatchCallbackIsSet)
    {
        return;
    }

    MatchState ms(m_response);

    if (ms.GlobalMatch(m_regex, m_matchCallback) > 0)
    {
        m_currentResponseIndex = 0;
        m_response[0] = '\0';
    }
}

void HC12::send(char *msg)
{
    m_stream->print(msg);
}

void HC12::send(const int value)
{
    char message[COMMAND_LENGTH];
    itoa(value, message, DEC);
    send(message);
}

void HC12::send(const float value, uint8_t width, uint8_t precision)
{
    char message[COMMAND_LENGTH];
    dtostrf(value, width, precision, message);
    send(message);
}

bool HC12::setBaudRate(uint32_t baudRate)
{
    char command[COMMAND_LENGTH];
    sprintf(command, "AT+B%ld", baudRate);
    return sendCommandExpectingOkResponse(command);
}

bool HC12::setChannel(uint8_t channel)
{
    if (channel > MAX_CHANNEL)
    {
        return false;
    }

    char command[COMMAND_LENGTH];
    sprintf(command, "AT+C%03d", channel);
    return sendCommandExpectingOkResponse(command);
}

void HC12::setOnResponseMatchCallback(const char *regex, GlobalMatchCallback callback)
{
    m_responseMatchCallbackIsSet = true;
    strcpy(m_regex, regex);
    m_matchCallback = callback;
}

bool HC12::sendCommandExpectingOkResponse(char *msg)
{
    if (m_setPin != NO_SET_PIN)
    {
        digitalWrite(m_setPin, COMMAND_MODE);
    }

    Serial.print("Sending command: ");
    Serial.println(msg);
    send(msg);
    const auto commandSentAt = millis();
    auto receivedOkResponse = false;
    m_currentResponseIndex = 0;

    while (millis() - commandSentAt < RESPONSE_TIMEOUT_MS)
    {
        if (!available())
        {
            continue;
        }

        if (isResponseFull())
        {
            m_currentResponseIndex = 0;
            break;
        }

        m_response[m_currentResponseIndex] = read();
        m_response[m_currentResponseIndex + 1] = '\0';

        if (strstr(m_response, "OK") != nullptr)
        {
            receivedOkResponse = true;
            break;
        }
    }

    // Clear buffer
    while (available())
    {
        read();
    }

    if (m_setPin != NO_SET_PIN)
    {
        digitalWrite(m_setPin, TRANSPARENT_MODE);
    }

    return receivedOkResponse;
}

bool HC12::isResponseFull()
{
    return strlen(m_response) >= RESPONSE_LENGTH - 1;
}