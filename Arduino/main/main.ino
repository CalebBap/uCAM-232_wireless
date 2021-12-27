#include "CameraServer.h"

CameraServer cameraServer;

void setup(){
  Serial.begin(57600);
  delay(10);

  cameraServer.initialise();  
}

void loop(){
  cameraServer.handleWifi();
}
