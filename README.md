# S3XY Button - M5Stack AtomS3 Lite / Arduino IDE

This version is adapted for an **M5Stack AtomS3 Lite** and Arduino IDE.

## Hardware

- Board: **M5Stack AtomS3 Lite / ESP32-S3**
- Physical button: **GPIO 41**
- Built-in RGB LED: **GPIO 35**

### Button wiring

Connect the external push button between:

```text
AtomS3 Lite GPIO 41 ---- BUTTON ---- GND
```

The firmware uses `INPUT_PULLUP`, so no external resistor is required.

## Button actions

- **1 short press** -> `s3xy_send_single()`
- **2 short presses** -> `s3xy_send_double()`
- **Hold >= 800 ms** -> `s3xy_send_long()`

Timing constants are at the top of `s3xy_virtual_button_atomS3.ino`:

```cpp
DEBOUNCE_MS = 30
LONG_PRESS_MS = 800
DOUBLE_CLICK_MS = 350
```

## RGB LED

The built-in AtomS3 Lite RGB LED is driven directly with `neopixelWrite()`:

- **Red** = waiting for S3XY connection
- **Green** = connected / ready
- **Blue** = command sent
- **Amber** = button command ignored because S3XY is not ready

No Adafruit NeoPixel library is required when using a recent Arduino-ESP32 core that provides `neopixelWrite()`.

## Arduino IDE setup

1. Install Arduino IDE.
2. Install the **ESP32 by Espressif Systems** board package using Boards Manager.
3. Select the AtomS3 Lite / ESP32-S3 board supported by your installed ESP32 package. If `M5Stack-AtomS3` is available, use it.
4. Open `s3xy_virtual_button_atomS3.ino`.
5. Keep these three files in the same sketch folder:
   - `s3xy_virtual_button_atomS3.ino`
   - `S3XYButton.cpp`
   - `S3XYButton.h`
6. Select the correct USB port.
7. Compile and upload.

The BLE implementation is kept in `S3XYButton.cpp` and uses the Arduino-ESP32 BLE API.

## Serial monitor

115200 baud.

The serial output reports connection state and every physical button action.


## Source

https://github.com/Beat-YT/s3xy-virtual-button
