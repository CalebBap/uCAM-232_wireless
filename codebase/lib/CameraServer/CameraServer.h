#ifndef CAMERA_SERVER_H
#define CAMERA_SERVER_H

#include "CameraCommands.h"
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <string_view>

class CameraServer {
    public:
        explicit CameraServer(uint16_t port);
        void initialise();
        void handleWifi();

    private:
        ESP8266WebServer mServer;
        WebSocketsServer mWebSocket;
        CameraCommands mCameraCommands;

        void webSocketEvent(WStype_t type, uint8_t* payload);
        void issueCommand(const std::string& payload);
        void sendFile();
};

#endif
