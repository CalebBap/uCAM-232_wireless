#ifndef CameraServer_h
#define CameraServer_h

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include "Credentials.h"

class CameraServer{
    public:
        void initialise();
        static void sendClientMessage(const char* message);
        void handleWifi();
    private:      
        static String getContentType(String filename);
        static bool handleFileRead(String path);
};

#endif
