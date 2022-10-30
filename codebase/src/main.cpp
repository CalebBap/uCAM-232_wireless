#include "../lib/CameraServer/CameraServer.h"

CameraServer cameraServer;

void setup() {
  constexpr int serialRxBufferSize { 512 };
  Serial.setRxBufferSize(serialRxBufferSize); // increase RX buffer size to avoid overhead when transmitting JPEG image data
  
  constexpr int baudRate { 115200 };
  Serial.begin(baudRate);
  
  delay(10);

  cameraServer.initialise();  
}

void loop() {
  cameraServer.handleWifi();
}
