# Live WiFi Clock

A minimal yet polished ESP32-C3 desk clock that syncs time over WiFi and displays it on a 128×64 SSD1306 OLED with smooth per-digit rolling animations.
Inspired by retro flip clocks and designed to be simple, clean, and always accurate.

![Live WiFi Clock](IMG_20260530_022332383.jpg)

---

## Features

- Live NTP time sync over WiFi
- Smooth per-digit roll animation
  - Each digit slides up independently on change
  - Fast boot-up roll from 00:00:00 to current time on startup
- 12-hour display with AM/PM indicator
- Blinking colon separator
- Auto-centered date display (e.g. Sat, 30 May 2026)
- India Standard Time (IST, UTC+5:30) out of the box
- Zero artificial delays — jumps straight to clock after sync

---

## Hardware Required

- ESP32-C3 SuperMini
- 0.96" OLED Display (128×64, SSD1306, I²C)
- Jumper wires
- USB cable

---

## Wiring

![Circuit Diagram](Circuit Diagram.png)

OLED (SSD1306 — I²C)

| OLED | ESP32-C3 | Wire Color |
|------|----------|------------|
| GND | GND | Black |
| VDD | 3.3V | Red |
| SCK | GPIO 9 (SCL) | Yellow |
| SDA | GPIO 8 (SDA) | Blue |

---

## Software Requirements

- Arduino IDE 2.x recommended
- ESP32 Board Package
  - Open Arduino IDE
  - Go to Preferences
  - Add this to Additional Board URLs:
    ```
    https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
    ```
- Libraries (install via Library Manager)
  - Adafruit SSD1306
  - Adafruit GFX Library

---
## Important — Before Uploading

The ESP32 can sometimes store stale WiFi credentials in flash memory from previous sketches, which causes silent connection failures even with correct credentials. To prevent this:

- In Arduino IDE go to **Tools → Erase All Flash Before Sketch Upload → Enabled**
- Upload the sketch once
- You can set it back to **Disabled** after the first successful upload

This is especially important if the clock is stuck on "Connecting to WiFi..." or fails silently after flashing.

---

## Setup

### 1. Install Libraries (Arduino IDE)

- **Adafruit SSD1306** — `Adafruit_SSD1306`
- **Adafruit GFX Library** — `Adafruit_GFX`

Install both via **Sketch → Include Library → Manage Libraries**.

### 2. Configure WiFi Credentials

Open `7_segment_wifi_clock_esp32.ino` and edit lines 8–9:

```cpp
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

### 3. Board Settings

In Arduino IDE, select:
- **Board:** `ESP32C3 Dev Module` (or your specific SuperMini variant)
- **Upload Speed:** 115200
- **USB CDC On Boot:** Enabled (for Serial monitor)

### 4. Flash & Run

Upload the sketch. On boot the display shows:
1. `Live WiFi Clock`
2. `Connecting to WiFi...`
3. `WiFi Connected!` (or `Connection Failed` if unreachable)
4. Clock starts immediately — all digits roll up from zero to the synced time

---

## Timezone

The sketch is hardcoded to **IST (UTC+5:30)**. To change it, replace the POSIX string on line 12:

```cpp
const char* timezoneIST = "IST-5:30";
```

Examples for other zones:
```cpp
"UTC0"             // UTC
"EST5EDT"          // US Eastern
"CET-1CEST"        // Central Europe
"GMT0BST,M3.5.0,M10.5.0"  // UK
```

A full list of POSIX timezone strings is available at [this reference](https://github.com/nayarsystems/posix_tz_db/blob/master/zones.csv).

---

## How the Animation Works

Each of the six digit positions (HH:MM:SS) is tracked independently. When a digit changes:

1. The **old digit slides upward** off-screen
2. The **new digit slides up** from below into place
3. Animation runs over **180 ms** (normal) or **350 ms** (boot-up roll)

Progress is linear, calculated per-frame in `loop()` at ~10 ms intervals.
