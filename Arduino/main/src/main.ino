#include "CameraServer.h"

CameraServer cameraServer;

void setup(){
  Serial.begin(115200);
  delay(10);

  cameraServer.initialise();  
}

void loop(){
  cameraServer.handleWifi();
}
