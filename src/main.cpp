
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "wifi_secrets.h"

// ===== Hardware =====

constexpr uint8_t DATA_PIN  = 14;
constexpr uint8_t CLOCK_PIN = 13;
constexpr uint8_t LATCH_PIN = 12;
constexpr uint8_t OE_PIN    = 5;

constexpr uint8_t RELAY_COUNT = 16;

uint16_t relayState = 0;
WebServer server(80);
String serialBuffer;

// ===== Relay driver =====

void updateRelays()
{
    digitalWrite(LATCH_PIN, LOW);

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST,
             highByte(relayState));

    shiftOut(DATA_PIN, CLOCK_PIN, MSBFIRST,
             lowByte(relayState));

    digitalWrite(LATCH_PIN, HIGH);
}

bool setRelay(uint8_t channel, bool on)
{
    if (channel < 1 || channel > 16)
        return false;

    uint16_t mask = (uint16_t)1 << (channel - 1);

    if (on)
        relayState |= mask;
    else
        relayState &= ~mask;

    updateRelays();

    Serial.printf("Relay %02u: %s\n",
                  channel, on ? "ON" : "OFF");

    return true;
}

void setAllRelays(bool on)
{
    relayState = on ? 0xFFFF : 0x0000;
    updateRelays();

    Serial.println(on ? "All relays ON" : "All relays OFF");
}

// ===== JSON status =====

String statusJson()
{
    String json = "{\"online\":true,\"relays\":[";

    for (uint8_t i = 0; i < 16; i++)
    {
        if (i > 0) json += ",";

        json += (relayState & ((uint16_t)1 << i))
                    ? "true" : "false";
    }

    json += "]}";
    return json;
}

void sendJson(int code, const String& json)
{
    server.sendHeader("Cache-Control", "no-store");
    server.send(code, "application/json", json);
}

// ===== Web dashboard =====

const char DASHBOARD[] PROGMEM = R"HTML(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport"
      content="width=device-width, initial-scale=1">

<title>Konceptux Relay16</title>

<style>
:root {
    color-scheme: dark;
    font-family: Arial, sans-serif;
}

* {
    box-sizing: border-box;
}

body {
    margin: 0;
    background: #101827;
    color: #f1f5f9;
}

.container {
    max-width: 900px;
    margin: auto;
    padding: 24px 16px;
}

header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    gap: 12px;
    margin-bottom: 25px;
}

h1 {
    margin: 0;
    font-size: 25px;
}

.subtitle {
    color: #94a3b8;
    margin-top: 6px;
}

.status {
    font-size: 13px;
    color: #fbbf24;
}

.stats {
    background: #1e293b;
    border-radius: 14px;
    padding: 20px;
    margin-bottom: 20px;
}

.count {
    font-size: 32px;
    font-weight: bold;
}

.grid {
    display: grid;
    grid-template-columns:
        repeat(4, minmax(0, 1fr));
    gap: 12px;
}

.relay {
    background: #1e293b;
    border: 1px solid #334155;
    border-radius: 12px;
    padding: 18px 10px;
    text-align: center;
}

.relay.active {
    border-color: #22c55e;
    background: #173c35;
}

.relay h3 {
    font-size: 15px;
    margin: 0 0 12px;
}

.relay button {
    border: none;
    border-radius: 8px;
    padding: 12px 16px;
    font-weight: bold;
    cursor: pointer;
    background: #475569;
    color: white;
    width: 100%;
}

.relay.active button {
    background: #22c55e;
    color: #052e16;
}

button:disabled {
    opacity: 0.5;
    cursor: wait;
}

.actions {
    display: flex;
    gap: 12px;
    margin-top: 20px;
}

.actions button {
    flex: 1;
    padding: 15px;
    border: none;
    border-radius: 10px;
    cursor: pointer;
    font-weight: bold;
}

.alloff {
    background: #475569;
    color: white;
}

.allon {
    background: #2563eb;
    color: white;
}

.message {
    min-height: 22px;
    color: #fbbf24;
    margin-top: 15px;
}

footer {
    text-align: center;
    margin-top: 30px;
    color: #64748b;
    font-size: 12px;
}

@media(max-width:600px) {
    .grid {
        grid-template-columns:
            repeat(2, minmax(0, 1fr));
    }

    h1 {
        font-size: 21px;
    }
}
</style>
</head>

<body>
<div class="container">

<header>
    <div>
        <h1>KONCEPTUX</h1>
        <div class="subtitle">
            ESP32 Smart Relay Controller
        </div>
    </div>
    <div class="status" id="connection">
        Connecting...
    </div>
</header>

<div class="stats">
    <div>Active relays</div>
    <div class="count" id="count">0 / 16</div>
</div>

<div class="grid" id="relayGrid"></div>

<div class="actions">
    <button class="alloff"
            onclick="setAll(false)">
        ALL OFF
    </button>

    <button class="allon"
            onclick="setAll(true)">
        ALL ON
    </button>
</div>

<div class="message" id="message"></div>

<footer>
    Konceptux Technologies | ESP32 Relay16
</footer>

</div>

<script>
let states = Array(16).fill(false);
let busy = false;

const grid = document.getElementById("relayGrid");
const message = document.getElementById("message");
const connection = document.getElementById("connection");

function render() {
    grid.replaceChildren();

    states.forEach((on, index) => {
        const card = document.createElement("div");
        card.className = "relay" + (on ? " active" : "");

        const heading = document.createElement("h3");
        heading.textContent =
            "Relay " + String(index + 1).padStart(2, "0");

        const button = document.createElement("button");
        button.textContent = on ? "ON" : "OFF";
        button.disabled = busy;
        button.addEventListener("click", () => {
            setRelay(index + 1, !on);
        });

        card.append(heading, button);
        grid.appendChild(card);
    });

    const active = states.filter(Boolean).length;
    document.getElementById("count").textContent =
        active + " / 16";

    document.querySelectorAll(".actions button")
        .forEach(button => button.disabled = busy);
}

async function request(url, options) {
    const response = await fetch(url, {
        cache: "no-store",
        ...options
    });

    if (!response.ok) {
        throw new Error("HTTP " + response.status);
    }

    return response.json();
}

async function refresh() {
    if (busy) return;

    try {
        const data = await request("/api/status");

        states = data.relays;
        connection.textContent = "● Connected";
        connection.style.color = "#22c55e";
        message.textContent = "";
        render();
    } catch (error) {
        connection.textContent = "● Disconnected";
        connection.style.color = "#f87171";
        message.textContent =
            "Unable to communicate with ESP32.";
    }
}

async function setRelay(channel, on) {
    if (busy) return;
    busy = true;
    render();

    try {
        const data = await request(
            "/api/relay?channel=" + channel +
            "&state=" + (on ? 1 : 0),
            { method: "POST" }
        );

        states = data.relays;
        message.textContent = "";
    } catch (error) {
        message.textContent =
            "Relay command failed: " + error.message;
    } finally {
        busy = false;
        render();
        refresh();
    }
}

async function setAll(on) {
    if (busy) return;
    busy = true;
    render();

    try {
        const data = await request(
            "/api/all?state=" + (on ? 1 : 0),
            { method: "POST" }
        );

        states = data.relays;
        message.textContent = "";
    } catch (error) {
        message.textContent =
            "Command failed: " + error.message;
    } finally {
        busy = false;
        render();
        refresh();
    }
}

render();
refresh();
setInterval(refresh, 1500);
</script>
</body>
</html>
)HTML";

// ===== HTTP handlers =====

void handleRelay()
{
    if (!server.hasArg("channel") ||
        !server.hasArg("state"))
    {
        sendJson(400, "{\"error\":\"Missing parameters\"}");
        return;
    }

    String channelText = server.arg("channel");
    String stateText = server.arg("state");

    if (channelText.isEmpty() ||
        channelText.length() > 2 ||
        (stateText != "0" && stateText != "1"))
    {
        sendJson(400, "{\"error\":\"Invalid parameters\"}");
        return;
    }

    for (char c : channelText)
    {
        if (c < '0' || c > '9')
        {
            sendJson(400, "{\"error\":\"Invalid channel\"}");
            return;
        }
    }

    int channel = channelText.toInt();

    if (!setRelay(channel, stateText == "1"))
    {
        sendJson(400, "{\"error\":\"Channel must be 1-16\"}");
        return;
    }

    sendJson(200, statusJson());
}

void handleAll()
{
    if (!server.hasArg("state") ||
        (server.arg("state") != "0" &&
         server.arg("state") != "1"))
    {
        sendJson(400, "{\"error\":\"Invalid state\"}");
        return;
    }

    setAllRelays(server.arg("state") == "1");
    sendJson(200, statusJson());
}

void setupWebServer()
{
    server.on("/", HTTP_GET, []() {
        server.send_P(200, "text/html", DASHBOARD);
    });

    server.on("/api/status", HTTP_GET, []() {
        sendJson(200, statusJson());
    });

    server.on("/api/relay", HTTP_POST, handleRelay);
    server.on("/api/all", HTTP_POST, handleAll);

    server.onNotFound([]() {
        sendJson(404, "{\"error\":\"Not found\"}");
    });

    server.begin();
    Serial.println("HTTP server started");
}

// ===== Serial commands =====

void processSerial(String command)
{
    command.trim();
    command.toUpperCase();

    if (command == "STATUS")
    {
        Serial.println(statusJson());
    }
    else if (command == "ON ALL")
    {
        setAllRelays(true);
    }
    else if (command == "OFF ALL")
    {
        setAllRelays(false);
    }
    else if (command == "HELP")
    {
        Serial.println(
            "ON 1-16 | OFF 1-16 | ON ALL | OFF ALL | STATUS"
        );
    }
    else if (command.startsWith("ON ") ||
             command.startsWith("OFF "))
    {
        bool on = command.startsWith("ON ");
        String number = command.substring(on ? 3 : 4);
        number.trim();

        bool valid = !number.isEmpty() &&
                     number.length() <= 2;

        for (char c : number)
            if (c < '0' || c > '9')
                valid = false;

        if (!valid || !setRelay(number.toInt(), on))
            Serial.println("Invalid relay number");
    }
    else
    {
        Serial.println("Unknown command. Type HELP.");
    }
}

// ===== Setup =====

void setup()
{
    Serial.begin(115200);

    // Disable outputs during initialization
    digitalWrite(OE_PIN, HIGH);
    pinMode(OE_PIN, OUTPUT);

    pinMode(DATA_PIN, OUTPUT);
    pinMode(CLOCK_PIN, OUTPUT);
    pinMode(LATCH_PIN, OUTPUT);

    digitalWrite(DATA_PIN, LOW);
    digitalWrite(CLOCK_PIN, LOW);
    digitalWrite(LATCH_PIN, LOW);

    relayState = 0;
    updateRelays();

    digitalWrite(OE_PIN, LOW);

    Serial.println("\nKonceptux Relay16");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");

    unsigned long start = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - start < 20000)
    {
        delay(500);
        Serial.print(".");
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.println("\nWi-Fi connected");
        Serial.print("IP address: ");
        Serial.println(WiFi.localIP());

        setupWebServer();
    }
    else
    {
        Serial.println("\nWi-Fi connection failed");
        Serial.println("Serial control remains available");
    }

    Serial.println("Type HELP for serial commands");
}

// ===== Main loop =====

void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        server.handleClient();
    }

    while (Serial.available())
    {
        char c = (char)Serial.read();

        if (c == '\n' || c == '\r')
        {
            if (!serialBuffer.isEmpty())
            {
                processSerial(serialBuffer);
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
            Serial.println("Command too long");
        }
    }
}
