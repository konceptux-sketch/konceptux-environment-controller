
#include <Arduino.h>

// Konceptux ESP32 Relay Board X16
// Two daisy-chained 74HC595 shift registers

constexpr uint8_t DATA_PIN  = 14;
constexpr uint8_t CLOCK_PIN = 13;
constexpr uint8_t LATCH_PIN = 12;
constexpr uint8_t OE_PIN    = 5;

constexpr uint8_t RELAY_COUNT = 16;

uint16_t relayState = 0;
String serialBuffer;

// Update both 74HC595 shift registers
void updateRelays()
{
    digitalWrite(LATCH_PIN, LOW);

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST,
             highByte(relayState));

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST,
             lowByte(relayState));

    digitalWrite(LATCH_PIN, HIGH);
}

// Set individual relay (1-16)
void setRelay(uint8_t relay, bool on)
{
    if (relay < 1 || relay > RELAY_COUNT)
    {
        Serial.println("ERROR: Relay must be 1-16");
        return;
    }

    uint16_t mask = (uint16_t)1 << (relay - 1);

    if (on)
        relayState |= mask;
    else
        relayState &= ~mask;

    updateRelays();

    Serial.printf("Relay %02u: %s\n",
                  relay, on ? "ON" : "OFF");
}

// Set all relays
void setAllRelays(bool on)
{
    relayState = on ? 0xFFFF : 0x0000;

    updateRelays();

    Serial.println(on ?
        "All relays ON" :
        "All relays OFF");
}

// Display relay states
void printStatus()
{
    Serial.println("\n--- RELAY STATUS ---");

    for (uint8_t i = 1; i <= RELAY_COUNT; i++)
    {
        bool on = relayState & ((uint16_t)1 << (i - 1));

        Serial.printf("Relay %02u: %s\n",
                      i, on ? "ON" : "OFF");
    }

    Serial.println("--------------------");
}

// Sequential test
void testRelays()
{
    Serial.println("Starting relay test...");

    setAllRelays(false);

    for (uint8_t i = 1; i <= RELAY_COUNT; i++)
    {
        setRelay(i, true);
        delay(400);
        setRelay(i, false);
        delay(200);
    }

    Serial.println("Relay test completed.");
}

// Help menu
void printHelp()
{
    Serial.println("\n=== KONCEPTUX RELAY16 ===");
    Serial.println("ON 1       - Turn ON relay 1");
    Serial.println("OFF 1      - Turn OFF relay 1");
    Serial.println("ON 16      - Turn ON relay 16");
    Serial.println("OFF 16     - Turn OFF relay 16");
    Serial.println("ON ALL     - Turn ON all relays");
    Serial.println("OFF ALL    - Turn OFF all relays");
    Serial.println("STATUS     - Show relay states");
    Serial.println("TEST       - Sequential test");
    Serial.println("HELP       - Show commands");
    Serial.println("=========================");
}

// Process command from Serial Monitor
void processCommand(String command)
{
    command.trim();
    command.toUpperCase();

    if (command == "HELP")
    {
        printHelp();
        return;
    }

    if (command == "STATUS")
    {
        printStatus();
        return;
    }

    if (command == "TEST")
    {
        testRelays();
        return;
    }

    if (command == "ON ALL")
    {
        setAllRelays(true);
        return;
    }

    if (command == "OFF ALL")
    {
        setAllRelays(false);
        return;
    }

    int spaceIndex = command.indexOf(' ');

    if (spaceIndex > 0)
    {
        String action = command.substring(0, spaceIndex);
        String relayStr = command.substring(spaceIndex + 1);
        relayStr.trim();

        if (action == "ON" || action == "OFF")
        {
            // Require a numeric channel, not partial text
            if (relayStr.length() == 0)
            {
                Serial.println("ERROR: Missing relay number");
                return;
            }

            for (size_t i = 0; i < relayStr.length(); i++)
            {
                if (!isDigit(relayStr[i]))
                {
                    Serial.println("ERROR: Invalid relay number");
                    return;
                }
            }

            int relay = relayStr.toInt();

            if (relay < 1 || relay > RELAY_COUNT)
            {
                Serial.println("ERROR: Relay must be 1-16");
                return;
            }

            setRelay((uint8_t)relay, action == "ON");
            return;
        }
    }

    Serial.println("Unknown command. Type HELP.");
}

void setup()
{
    Serial.begin(115200);
    Serial.setTimeout(50);
    serialBuffer.reserve(64);

    // Disable shift-register outputs during setup
    digitalWrite(OE_PIN, HIGH);
    pinMode(OE_PIN, OUTPUT);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    digitalWrite(DATA_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(LATCH_PIN, LOW);

    relayState = 0x0000;
    updateRelays();

    // Enable outputs after initializing relay state
    digitalWrite(OE_PIN, LOW);

    delay(500);

    Serial.println("\nKonceptux Relay16 Ready!");
    printHelp();
}

void loop()
{
    while (Serial.available())
    {
        char c = (char)Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (serialBuffer.length() > 0)
            {
                processCommand(serialBuffer);
                serialBuffer = "";
            }
        }
        else if (serialBuffer.length() < 63)
        {
            serialBuffer += c;
        }
        else
        {
            serialBuffer = "";
            Serial.println("ERROR: Command too long");
        }
    }
}
