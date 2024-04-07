# inkplate10-poc

This is a proof of concept to get a selected set of the features of the [Inkplate
10](https://soldered.com/product/inkplate-10-9-7-e-paper-board-copy/) working.
The aim was for me to focus on getting the features of the hardware and software
stack working before worrying about a real project.

This is my first time writing C/C++ and my first time working with hardware like this so don't assume any of this code is best practice or optimal.

## Features
The working features are:
* Wi-Fi
* Custom Fonts
* Partial Display Updates
* Lower Power Mode
  * Waking Up Using Button
* Real Time Clock
* Writing to Files
* Logging
* HTTP Requests
* JSON Parsing

The features still to be implemented:
* Touchscreen
* Tests

The excluded features are:
* HTTPS Requests
  * My follow on project will use a local server to reduce the number of network
    requests the Inkplate has to made, so HTTPS won't be needed.
* Bluetooth
* Serial Communication
* Debugging
  * As far as I can tell debugging for the Inkplate is limited to serial messages

## Build Flags

The project has the following build flags.

| Variable | Description | Required |
| ------ | ------ | ------ |
| `WIFI_SSID` | The SSID of a Wi-Fi network to connect to. | |
| `WIFI_PASSWORD` | The Password of a Wi-Fi network to connect to. | |
| `BATTERY_LOG_FILE` | A CSV file to record battery voltages to. | |
