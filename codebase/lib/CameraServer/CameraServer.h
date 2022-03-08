#ifndef CameraServer_h
#define CameraServer_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "..\..\src\Credentials.h"
#include "..\FileOperations\FileOperations.h"
#include "..\CameraCommands\CameraCommands.h"

#define CMD_CLIENT_MESSAGE_SIZE 37

class CameraServer{
    public:
        void initialise();
        void handleWifi();
        static void sendClientMessage(const char* message);
        static void sendClientCommand(const byte cmd[]);
    private:
        static void sendFile();
};

#endif
