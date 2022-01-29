#ifndef CameraServer_h
#define CameraServer_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "..\..\src\Credentials.h"
#include "..\FileOperations\FileOperations.h"
#include "..\CameraCommands\CameraCommands.h"

class CameraServer{
    public:
        void initialise();
        void handleWifi();
        static void sendClientMessage(const char* message);
    private:
        static void sendFile();
};

#endif
