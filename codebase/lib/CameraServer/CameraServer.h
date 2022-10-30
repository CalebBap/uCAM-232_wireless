#ifndef CameraServer_h
#define CameraServer_h

#include "..\CameraCommands\CameraCommands.h"
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>

class CameraServer {
    public:
        void initialise();
        void handleWifi();
        static void sendClientMessage(const char* message);
        static void sendClientCommand(const byte cmd[]);

    private:
        ESP8266WebServer server;
        WebSocketsServer webSocket = WebSocketsServer(81);
        CameraCommands cameraCommands = CameraCommands(webSocket);

        const char* syncCmd { "#sync" };
        const char* initialiseCmd { "#initial" };
        const char* getPictureCmd { "#getPicture" };
        const char* snapshotCmd { "#snapshot" };
        const char* setPackageSizeCmd { "#setPackageSize" };
        const char* setBaudRateCmd { "#setBaudRate" };
        const char* resetCmd { "#reset" };
        const char* dataCmd { "#data" };
        const char* ackCmd { "#ack" };
        const char* nakCmd { "#nak" };
        const char* lightCmd { "#light" };

        void webSocketEvent(WStype_t type, uint8_t* payload);
        void sendFile();
};

#endif
