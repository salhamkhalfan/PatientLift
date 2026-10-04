# PatientLift

> Lift. Transfer. Independence.
> An assistive wheelchair that lifts the user and moves them safely to the next surface.

![Project photo or demo GIF](docs/demo.gif)

**Hackathon:** Claude SuperHuman ImpactLab · **Date:** October 2026 · **Team:** Hugo

---

## Table of Contents
- [Overview](#overview)
- [The Problem](#the-problem)
- [Features](#features)
- [How It Works](#how-it-works)
- [Hardware](#hardware)
- [Wiring](#wiring)
- [Software Setup](#software-setup)
- [Usage](#usage)
- [Safety](#safety)
- [Roadmap](#roadmap)
- [Team](#team)
- [License](#license)

---

## Overview

This project is an assistive wheelchair whose seat platform can be raised and lowered to help a user transfer to another surface, such as a car seat. It is aimed at wheelchair users who want to transfer with less help, and at caregivers who want to avoid lifting by hand. The lift is controlled from a phone through a simple web page, and the ESP32 controller enforces angle and speed limits so that motion stays smooth and bounded.

Current status: **MVP**. The seat platform lifts and is controlled from a phone. The full lift, move and lower transfer is in development.

## The Problem

Transferring from a wheelchair to a car seat, bed or chair is one of the riskiest moments of daily life for wheelchair users and their caregivers.

- **Caregiver strain:** manual lifting is physically demanding and repeated daily
- **Fall risk:** gaps and height differences between surfaces
- **Lost independence:** many users cannot transfer without help

[Add a sourced statistic or a short user story here.]

## Features

- Seat platform lift controlled from any phone
- Web interface served directly by the ESP32, no internet required
- Angle limited to **0°-110°** on the controller
- Speed capped at **30°/s** with smooth ramped motion
- **+5° / -5°** step control
- **STOP** button that holds the current position
- Serial control for debugging and testing

## How It Works

```
Phone  -->  Wi-Fi (ESP32 hotspot)  -->  ESP32  -->  Servo  -->  Seat lift
(browser)                              (limits +             (moves seat)
                                        smooth motion)
```

The ESP32 creates its own Wi-Fi network and serves a control page. Each button press sends a request to the ESP32, which checks the limits and moves the servo gradually to the new target angle.

[Add an architecture diagram or CAD image: `docs/architecture.png`]

## Hardware

| Component | Details |
|---|---|
| Microcontroller | ESP32 ([exact board name]), Wi-Fi access-point mode |
| Actuator | [Servo model], 50 Hz, 500-2400 µs pulse range |
| Power supply | [Voltage / current rating] |
| Frame / lift mechanism | [Describe] |
| Other | [Emergency stop, harness, sensors, etc.] |

## Wiring

| Servo wire | Connects to |
|---|---|
| Signal | GPIO 18 |
| Power (+) | [External supply, not the ESP32 pin] |
| Ground | GND (shared with ESP32) |

> Power the servo from a supply rated for its stall current, and connect its ground to the ESP32 ground.

[Add wiring diagram: `docs/wiring.png`]

## Software Setup

**Requirements**
- Arduino IDE 2.x (or PlatformIO)
- ESP32 board package by Espressif
- Library: **ESP32Servo**

**Steps**
1. Clone the repository
   ```bash
   git clone [repo-url]
   cd [repo-folder]
   ```
2. Open `PatientLift_WebServo.ino` in Arduino IDE.
3. Select your ESP32 board and port.
4. **Change the Wi-Fi password** (`AP_PASS`) near the top of the file.
5. Upload the sketch.

**Configurable settings** (top of the sketch)

| Setting | Default | Meaning |
|---|---|---|
| `AP_SSID` | `PatientLift` | Wi-Fi network name |
| `AP_PASS` | `change-me-1234` | Wi-Fi password (change it) |
| `SERVO_PIN` | `18` | Servo signal pin |
| `MIN_ANGLE` / `MAX_ANGLE` | `0` / `110` | Allowed angle range |
| `START_ANGLE` | `90` | Position on power-up |
| `MAX_SPEED_DPS` | `30` | Maximum speed in degrees per second |

## Usage

1. Power on the device.
2. On your phone, connect to the Wi-Fi network **PatientLift**.
3. Open `http://192.168.4.1` in a browser.
4. Use **+5°** and **-5°** to move the seat. Press **STOP** to hold position.

**Serial commands** (115200 baud): a number from 0 to 110, `+`, or `-`.

## Safety

> **This is a prototype. Do not use it to lift a person until the items below are complete and it has been tested under supervision.**

**Built in**
- Angle limits enforced on the controller
- Speed cap and smooth motion
- Small step control, no sudden jumps
- STOP holds position
- Password-protected Wi-Fi

**Required before real use**
- [ ] Mechanical end stops
- [ ] Hardware emergency power cut-off
- [ ] Load-rated lift mechanism and harness
- [ ] Stop on connection loss
- [ ] Testing with caregivers and clinicians

## Roadmap

- [x] Seat platform lift
- [x] Phone web control
- [ ] Move: carry the user across to the next surface
- [ ] Lower: gentle set-down
- [ ] Safety hardware
- [ ] User trials

## Team

| Hugo |
|---|---|
| Anurag K | Team Lead |
| Angeline Anna Jaison | Team Member |
| Salha M Khalfan | Team Member |
| Ayisha Sidra | Team Member |

## License

[MIT / Apache-2.0 / other]. See [LICENSE](LICENSE).

## Acknowledgements

- [Mentors, organizers, libraries, resources]
