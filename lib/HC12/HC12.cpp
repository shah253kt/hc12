#include "HC12.h"

namespace
{
    constexpr auto MAX_CHANNEL = 127;
    constexpr auto COMMAND_MODE = LOW;
    constexpr auto TRANSPARENT_MODE = HIGH;
    constexpr auto COMMAND_LENGTH = 15;
    constexpr auto RESPONSE_LENGTH = 50;
    constexpr auto RESPONSE_TIMEOUT_MS = 3000;
    constexpr auto UPDATE_TIMEOUT_MS = 100;
    constexpr auto MODE_CHANGE_WAIT_MS = 100;
}

HC12::HC12(Stream &stream, uint8_t setPin)
    : m_stream{&stream},
      m_setPin{setPin},
      m_response{new char(RESPONSE_LENGTH)},
      m_currentResponseIndex{0},
      m_characterToCheck{'\n'}
{
    if (m_setPin != NO_SET_PIN)
    {
        pinMode(m_setPin, OUTPUT);
        digitalWrite(m_setPin, TRANSPARENT_MODE);
    }
}

HC12::~HC12()
{
    delete[] m_response;
}

Stream *HC12::getStream()
{
    return m_stream;
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
        char c = read();
        
        m_response[m_currentResponseIndex++] = c;
        m_response[m_currentResponseIndex] = '\0';

        if (m_onCharacterReceivedEventHandler != nullptr && c == m_characterToCheck)
        {
            m_onCharacterReceivedEventHandler();
        }

        if (isResponseFull())
        {
            resetResponse();
            return;
        }
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
    sprintf(command, "AT+B%ld\n", baudRate);
    return sendCommandExpectingOkResponse(command);
}

bool HC12::setChannel(uint8_t channel)
{
    if (channel > MAX_CHANNEL)
    {
        return false;
    }

    char command[COMMAND_LENGTH];
    sprintf(command, "AT+C%03d\n", channel);
    return sendCommandExpectingOkResponse(command);
}

void HC12::setCommandMode(bool isCommandMode)
{
    if (m_setPin != NO_SET_PIN)
    {
        digitalWrite(m_setPin, isCommandMode ? COMMAND_MODE : TRANSPARENT_MODE);
        delay(MODE_CHANGE_WAIT_MS);
    }
}

void HC12::onCharacterReceived(const char c, void (*funcPtr)())
{
    m_characterToCheck = c;
    m_onCharacterReceivedEventHandler = funcPtr;
}

char *HC12::getResponse()
{
    return m_response;
}

void HC12::resetResponse()
{
    m_currentResponseIndex = 0;
    m_response[0] = '\0';
}

bool HC12::sendCommandExpectingOkResponse(char *msg)
{
    setCommandMode(true);
    resetResponse();
    send(msg);

    const auto commandSentAt = millis();
    auto receivedOkResponse = false;

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

        m_response[m_currentResponseIndex++] = read();
        m_response[m_currentResponseIndex] = '\0';

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

    setCommandMode(false);
    return receivedOkResponse;
}

bool HC12::isResponseFull()
{
    return strlen(m_response) >= RESPONSE_LENGTH - 1;
}
