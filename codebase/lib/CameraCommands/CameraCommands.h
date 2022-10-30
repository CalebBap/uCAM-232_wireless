#ifndef CameraCommands_h
#define CameraCommands_h

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>
#include <string>
#include <string_view>

class CameraCommands {
    public:
        explicit CameraCommands(WebSocketsServer& wbs) : webSocket(wbs) {}

        void attemptSync();
        void unrecognisedCommand(std::string command);
        void parseInitialisationParameters(std::string command);
        void parseSnapshotParameters(std::string command);

    private:
        WebSocketsServer& webSocket;

        static constexpr int CMD_CLIENT_MESSAGE_SIZE { 37 };
        static constexpr int NUM_BYTES_IN_CMD { 6 };
        static constexpr int MAX_SYNC_ATTEMPTS { 60 };
        static constexpr int MIN_SYNC_DELAY { 5 };

        static constexpr std::string_view cmd_delimiter = ":";
        static constexpr std::string_view value_delimiter = ",";

        void receiveCameraResponse(byte* reply);
        void sendClientMessage(std::string message);
        void sendClientCommand(const byte cmd[]);
        void attemptInitialisation(const byte* init_cmd, bool set_package_size);
        bool setPackageSize();
        void getSnapshot(int num_frames);    
};

#endif