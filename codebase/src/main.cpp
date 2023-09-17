#include "CameraServer.h"

constexpr uint16_t port { 81 };
static CameraServer cameraServer(port);

void setup() {
  constexpr int bufferSize { 512 };
  Serial.setRxBufferSize(bufferSize); // increase RX buffer size to avoid overhead when transmitting JPEG image data
  
  constexpr int baudRate { 115200 };
  Serial.begin(baudRate);
  
  delay(10);

  cameraServer.initialise();  
}

void loop() {
  cameraServer.handleWifi();
}
