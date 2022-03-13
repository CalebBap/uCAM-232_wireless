#include "../lib/CameraServer/CameraServer.h"

#define BAUD_RATE 115200
#define SERIAL_RX_BUFFER_SIZE 512

CameraServer cameraServer;

void setup(){
  Serial.setRxBufferSize(SERIAL_RX_BUFFER_SIZE); // increase RX buffer size to avoid overhead when transmitting JPEG image data
  Serial.begin(BAUD_RATE);
  delay(10);

  cameraServer.initialise();  
}

void loop(){
  cameraServer.handleWifi();
}
