# Wireless Camera using uCAM-232 + ESP8266
This project aims to build a wireless camera that can operate over a WLAN and be controlled from a web browser. Its main components are a uCAM-232 serial camera module and a ESP8266 WiFi MCU.

## How to use
### Prerequisites
1) The hardware circuit mentioned in step 1 of the installation instructions below must be built.
2) The project is being built in the PlatformIO development platform. It is recommended that the [PlatformIO extension](https://platformio.org/install/ide?install=vscode) is installed in [Visual Studio Code](https://code.visualstudio.com/).
3) A separate circuit and 3.3V power supply is required to program the ESP8266. Instructions can be found in [this resource.](https://tttapa.github.io/ESP8266/Chap01%20-%20ESP8266.html)
4) You should have a device running a modern web browser from which you can control the wiress camera. The device and ESP8266 need to be connected to a WLAN that you known the SSID and password of.

### Installation
1) [The schematic](https://github.com/CalebBap/uCAM-232_wireless/blob/master/schematic.png) for this project is available under the repository's root directory.
2) After cloning or downloading the repository, the "codebase" folder should be opened using the PlatformIO extension for Visual Studio Code.
3) A header file named "Credentials" should then be created with the following file path: `\uCAM-232_wireless\codebase\src\Credentials.h`
4) Populate Credentials.h as follows:<br>
    `#define WAN_NAME "[SSID]"`<br>
    `#define PASSWORD "[Password]"`<br>
    Where [SSID] and [Password] are your WiFi network's name and password, respectively.
5) The code can now be uploaded to the ESP8266.
6) Once the code has been uploaded and the ESP8266 is placed in the project's circuit, it can then be controlled by accessing the IP address `10.100.0.200` in a web browser (please note that the browser must be running on a device connected to the same WLAN as the ESP8266).
