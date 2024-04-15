# inkplate10-poc

This is a proof of concept to get a selected set of the features of the
[Inkplate 10][1] working. The aim was for me to focus on getting the features of
the hardware and software stack working before worrying about a real project.

This is my first time writing C/C++ and my first time working with hardware like
this so don't assume any of this code is best practice or optimal. Feedback is
welcome!

## Features
The working features are:
* Wi-Fi
  * Static IP Address (to keep wake time to a minimum)
* Custom Fonts
* Partial Display Updates
* Lower Power Mode
  * Waking Up Using Button
* Real Time Clock
* Writing to Files
* Logging
* HTTP Requests
* JSON Parsing
* Development and Production Builds

The excluded features are:
* Touchscreen
* Bluetooth
* Two-Way Serial Communication
* HTTPS Requests
  * My follow on project will use a local server to reduce the number of network
    requests the Inkplate has to made, so HTTPS won't be needed.
* Debugging
  * As far as I can tell debugging for the Inkplate is limited to serial
    messages due to the version of the ESP32 used. The ESP32-S2 and ESP32-S3
    have a built in JTAG interface but the ESP32-D0WD-V3 used by my Inkplate 10
    does not.
* Detect when charging
  * [This is not possible with the Inkplate hardware.][2]
* Updated Partition Map
  * I have not needed this yet.
* Tests
  * They seem unnecessary when the Inkplate is mostly just displaying things
* Storing WiFi credentials in flash
  * The `esp_wifi_set_storage` API call looks interesting but not worth writing
    a whole WiFi client for.

## Build Flags

The project has the following build flags.

| Flag | Description |
| ------ | ------ |
| `BATTERY_LOG_FILE` | A CSV file to record battery voltages to. |

## Wireless Network Configuration

The `WirelessConfig.h` should be updated to set your WiFi configuration.

| Macro | Description |
| ------ | ------ |
| `WIFI_SSID` | The SSID of a Wi-Fi network to connect to. |
| `WIFI_PASSWORD` | The Password of a Wi-Fi network to connect to. |
| `WIFI_IP_ADDRESS` | An IP address to use if not using DHCP |
| `WIFI_GATEWAY` | The gateway to use if not using DHCP. |
| `WIFI_SUBNET` | The subnet mask to use if not using DHCP. |
| `WIFI_DNS1` | The first DNS server to use if not using DHCP. |
| `WIFI_DNS1` | The second DNS server to use if not using DHCP. |

## Building `fontconvert`

The build requires the `fontconvert` binary to be on the `PATH`.

Here are instructions on how to build it on macOS:

1. Install `gcc` and `freetype` library using Homebrew.

    ```
    brew install gcc freetype
    ```

2. Put the Homebrew `gcc` on the `PATH` ahead of the Apple one:

    ```
    sudo ln -s $(which gcc-13) /usr/local/bin/gcc
    ```

3. Checkout the Adafruit GFX Library.

    ```
    git clone https://github.com/adafruit/Adafruit-GFX-Library.git
    ```

4. Edit `fontconvert/Makefile` and change:

    ```
    CFLAGS = -Wall -I/usr/local/include/freetype2 -I/usr/include/freetype2 -I/usr/include
    LIBS   = -lfreetype
    ```

    to:

    ```
    CFLAGS = -Wall -I/opt/homebrew/include/freetype2
    LIBS   = /opt/homebrew/lib/libfreetype.dylib
    ```

5. Build the binary.

    ```
    cd Adafruit-GFX-Library/fontconvert
    make
    ```

## Downloading fonts

If you have certificate errors when `convert_fonts.py` tries to download the
source font files either set `platformio-ide.useBuiltinPython` to `false` or run
the script outside of the PlatformIO build chain once to download the files.

## References

* [Inkplate Documentation][3]
* [ESP-IDF Programming Guide][4]
* [`SolderedElectronics/Inkplate-Arduino-library` on GitHub][5]
* [Platform IO Documentation][6]

[1]: https://soldered.com/product/inkplate-10-9-7-e-paper-board-copy/
[2]: https://github.com/SolderedElectronics/Inkplate-Arduino-library/issues/242
[3]: https://inkplate.readthedocs.io/
[4]: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/
[5]: https://github.com/SolderedElectronics/Inkplate-Arduino-library/
[6]: https://docs.platformio.org/
