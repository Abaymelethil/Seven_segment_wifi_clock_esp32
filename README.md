Live WiFi Clock
A minimal yet polished ESP32-C3 desk clock that syncs time over WiFi and displays it on a 128×64 SSD1306 OLED with smooth per-digit rolling animations.
Inspired by retro flip clocks and designed to be simple, clean, and always accurate.
Features

Live NTP time sync over WiFi
Smooth per-digit roll animation

Each digit slides up independently on change
Fast boot-up roll from 00:00:00 to current time on startup


12-hour display with AM/PM indicator
Blinking colon separator
Auto-centered date display (e.g. Sat, 30 May 2026)
India Standard Time (IST, UTC+5:30) out of the box
Zero artificial delays — jumps straight to clock after sync

Hardware Required

ESP32-C3 SuperMini
0.96" OLED Display (128×64, SSD1306, I²C)
Jumper wires
USB cable

Wiring
OLED (SSD1306 — I²C)
OLEDESP32-C3GNDGNDVDD3.3VSCKGPIO 9SDAGPIO 8
See Circuit Diagram for a visual reference.
Software Requirements

Arduino IDE 2.x recommended
ESP32 Board Package

Open Arduino IDE
Go to Preferences
Add this to Additional Board URLs:
https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json


Libraries (install via Library Manager)

Adafruit SSD1306
Adafruit GFX Library



Important — Before Uploading
The ESP32 can sometimes store stale WiFi credentials in flash memory from previous sketches, which causes silent connection failures even with correct credentials. To prevent this:

In Arduino IDE go to Tools → Erase All Flash Before Sketch Upload → Enabled
Upload the sketch once
You can set it back to Disabled after the first successful upload

This is especially important if the clock is stuck on "Connecting to WiFi..." or fails silently after flashing.
