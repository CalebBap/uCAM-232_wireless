#ifndef CAMERA_COMMANDS_H
#define CAMERA_COMMANDS_H

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <cstdint>
#include <map>
#include <string>
#include <string_view>

class CameraCommands {
    public:
        explicit CameraCommands(WebSocketsServer& wbs);

        void attemptSync();
        void unrecognisedCommand(const std::string& command);
        void attemptInitialisation(const std::string& command);
        void attemptSnapshot(const std::string& command);

    private:
        WebSocketsServer& mWebSocket;
        std::string current_colour_type;
        std::string current_resolution;

        void sendClientMessage(const std::string& message);
        void receiveCameraResponse(std::vector<byte>& reply, int size);
        bool sendCameraCommand(const std::vector<byte>& cmd, const byte id);
        void sendClientCommand(const std::vector<byte>& cmd);
        bool getCameraCommand(const byte id, uint8_t& nak_reason);

        int parseSnapshotParameters(std::string command);
        bool parseInitParameters(std::vector<byte>& init_cmd, std::string command);

        bool setPackageSize();
        void getPicture(byte data_type);
        void getData(byte data_type);
        void getJpegData(uint32_t img_size);
        void getRawData(uint32_t img_size);
};

#endif