# Konceptux ESP32 16-Channel Relay Controller

**An embedded systems and IoT development series by [Konceptux Technologies](https://konceptux.com).**

A hands-on project documenting the development of a programmable 16-channel relay controller using an ESP32-WROOM-E, two 74HC595 shift registers, and custom firmware.

This repository accompanies the **Konceptux ESP32 Development Series**, where we explore hardware integration, firmware development, debugging, and connected-device applications.

The project evolves episode by episode, from flashing the first firmware to building a Wi-Fi-enabled automation controller.

## Project Overview

The development board features:

- ESP32-WROOM-E microcontroller module
- 16 electromechanical relays
- Two daisy-chained 74HC595D 8-bit shift registers
- 12 V DC board power supply
- Relay terminals with COM, NO, and NC contacts
- Relay contact rating advertised as up to 10 A per channel, subject to load type and board specifications

The firmware is developed using C++ and the Arduino framework, with Visual Studio Code and PlatformIO used throughout the series.

## Development Roadmap

| Episode | Topic | Status |
|---|---|---|
| 01 | Board preparation, firmware flashing, and debugging | Completed |
| 02 | Individual control of 16 relays using shift registers | Completed |
| 03 | Wi-Fi web dashboard and remote relay control | Completed |
| 04 | Physical push button and dashboard synchronization | Completed |
| 05 | SHT41 temperature and humidity monitoring | Completed |
| 06 | Automatic temperature-based relay control | In development |

Each episode builds on the previous implementation.

## Episode 1 — Getting Started with ESP32 Firmware

The first episode focuses on understanding the development board and successfully running custom firmware.

### Objectives

1. Inspect the ESP32 16-channel relay board.
2. Prepare the hardware and solder the required headers.
3. Power the board and observe its preinstalled relay demonstration.
4. Configure the ESP32 development environment.
5. Troubleshoot firmware upload failures.
6. Successfully flash and execute custom firmware.

### What We Learned

The initial firmware upload did not succeed immediately. Troubleshooting involved checking the programming process and using the board's BOOT and RESET buttons.

After resolving the upload issue, the ESP32 successfully executed the custom firmware and displayed diagnostic messages in the Serial Monitor.

This episode demonstrates a fundamental embedded engineering workflow:

**Hardware inspection → Firmware development → Debugging → Flashing → Verification**

### Source Code

The Episode 1 implementation focuses on basic firmware execution and serial debugging.

Later episodes introduce relay control, Wi-Fi connectivity, and sensor integration.

## Hardware Architecture

The ESP32 communicates with two cascaded 74HC595 shift registers, providing 16 output-control bits.

This reduces the number of microcontroller GPIO signals needed to control the relay channels.

### Shift Register Connections

| Signal | ESP32 GPIO |
|---|---|
| Serial Data | GPIO14 |
| Shift Clock | GPIO13 |
| Storage Latch | GPIO12 |
| Output Enable | GPIO5 |

The shift registers are controlled through GPIO-driven serial shifting and latching.

### Additional Connections Introduced Later

| Peripheral | ESP32 GPIO |
|---|---|
| Physical push button | GPIO27 |
| SHT41 SDA | GPIO22 |
| SHT41 SCL | GPIO25 |

The SHT41 communicates over I²C.

## Development Environment

**Software:**

- Visual Studio Code
- PlatformIO
- Arduino framework for ESP32
- C/C++

**Hardware:**

- ESP32-WROOM-E 16-channel relay board
- 12 V DC power supply appropriate for the board
- USB cable for programming
- Computer for firmware development

Additional sensors and input devices are introduced in subsequent episodes.

## Getting Started

### 1. Clone the repository

```bash
git clone https://github.com/konceptux-sketch/konceptux-environment-controller.git

cd konceptux-environment-controller
```

### 2. Open the project

Open the repository in Visual Studio Code with the PlatformIO extension installed.

Select the relevant episode branch or tag if available.

### 3. Build the firmware

Using the PlatformIO terminal:

```bash
pio run
```

### 4. Upload to the ESP32

```bash
pio run -t upload
```

Depending on the board's programming circuitry, you may need to use the BOOT and RESET buttons to enter the flashing mode.

### 5. Open the Serial Monitor

```bash
pio device monitor -b 115200
```

The expected output depends on the selected episode's firmware.

## Later Features

The development series progressively introduces:

**Individual relay control**

Control each relay independently using commands such as `ON 1`, `OFF 1`, and `STATUS`.

**Wi-Fi dashboard**

Control all 16 relays through a browser on the same local network.

**Physical inputs**

Toggle a relay using a push button while keeping the web dashboard synchronized.

**Environmental monitoring**

Read temperature and relative humidity from an SHT41 sensor and display the measurements on the dashboard.

**Automatic control — planned**

Use temperature setpoints and hysteresis to demonstrate automatic relay control.

## Safety and Limitations

This project is intended for educational development and prototyping.

- Disconnect power before changing wiring.
- Do not connect mains-powered equipment during initial testing.
- Verify relay ratings, PCB limitations, supply requirements, and load characteristics before connecting external equipment.
- GPIO signals operate at 3.3 V logic levels.
- Avoid using ESP32 GPIO6–GPIO11 because they are associated with the module's flash interface.
- The demonstration web server is intended for a trusted local network and is not a production-secure control interface.
- Automatic control requires appropriate fault handling and safety measures before use with real equipment.

The firmware and hardware demonstrations are not certified industrial or safety-critical control systems.

## About Konceptux Technologies

[Konceptux Technologies](https://konceptux.com) is a Canadian technology company focused on embedded systems, electronics, IoT, and custom software development.

Our engineering interests and capabilities include:

- Embedded C/C++ firmware development
- ESP32 and STM32 microcontrollers
- Hardware and sensor integration
- IoT connectivity and local automation
- Electronics prototyping
- Custom control systems
- Firmware debugging and system integration

This repository demonstrates practical engineering workflows and the evolution of a connected-device prototype.

**Have an embedded firmware, electronics, or IoT project?**

Visit [konceptux.com](https://konceptux.com) to learn more or discuss your requirements.

## Follow the Development Series

Watch the Konceptux ESP32 series to follow the hardware, firmware, debugging, and feature development.

New episodes introduce practical embedded systems concepts through real demonstrations.

## License

Copyright © 2026 Konceptux Technologies Inc.

All rights reserved unless a separate license is added to this repository.

Public access to the source code does not automatically grant permission to redistribute or reuse it.
