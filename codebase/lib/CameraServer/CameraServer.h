#ifndef CameraServer_h
#define CameraServer_h

#include "..\CameraCommands\CameraCommands.h"
#include <ESP8266WebServer.h>
#include <WebSocketsServer.h>
#include <string_view>

class CameraServer {
    public:
        void initialise();
        void handleWifi();

    private:
        ESP8266WebServer server;
        WebSocketsServer webSocket = WebSocketsServer(81);
        CameraCommands cameraCommands = CameraCommands(webSocket);

        static constexpr std::string_view syncCmd { "#sync" };
        static constexpr std::string_view initialiseCmd { "#init" };
        static constexpr std::string_view getPictureCmd { "#getPicture" };
        static constexpr std::string_view snapshotCmd { "#snapshot" };
        static constexpr std::string_view setPackageSizeCmd { "#setPackageSize" };
        static constexpr std::string_view setBaudRateCmd { "#setBaudRate" };
        static constexpr std::string_view resetCmd { "#reset" };
        static constexpr std::string_view dataCmd { "#data" };
        static constexpr std::string_view ackCmd { "#ack" };
        static constexpr std::string_view nakCmd { "#nak" };
        static constexpr std::string_view lightCmd { "#light" };

        void webSocketEvent(WStype_t type, uint8_t* payload);
        void sendFile();
};

#endif
