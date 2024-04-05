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

The features still to be implemented:
* Partial Updates
* HTTP Requests
* JSON Parsing
* Touchscreen
* Lower Power Mode
* Logging
  * Logging to SD card
  * Logging to Network
* Debugging

The excluded features are:
* HTTPS Requests
  * My follow on project will use a local server to reduce the number of network
    requests the Inkplate has to made, so HTTPS won't be needed.

## Build Flags

The project has the following build flags.

| Variable | Description | Required |
| ------ | ------ | ------ |
| `WIFI_SSID` | The SSID of a Wi-if network to connect to. | |
| `WIFI_PASSWORD` | The Password of a Wi-if network to connect to. | |
| `BATTERY_LOG_FILE` | A CSV file to record battery voltages to. | |
