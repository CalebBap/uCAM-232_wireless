#ifndef CameraCommands_h
#define CameraCommands_h

#include <ESP8266WiFi.h>
#include <WebSocketsServer.h>

class CameraCommands {
    public:
        CameraCommands(WebSocketsServer& wbs) : webSocket(wbs) {}

        void attemptSync();
        void parseInitialisationParameters(const char* command);
        void parseSnapshotParameters(const char* command);

    private:
        WebSocketsServer& webSocket;
        static constexpr int CMD_CLIENT_MESSAGE_SIZE { 37 };
        static constexpr int NUM_BYTES_IN_CMD { 6 };
        static constexpr int MAX_SYNC_ATTEMPTS { 60 };
        static constexpr int MIN_SYNC_DELAY { 5 };

        void receiveCameraResponse(byte* reply);
        void sendClientMessage(const char* message);
        void sendClientCommand(const byte cmd[]);
        void attemptInitialisation(const byte* init_cmd, bool set_package_size);
        bool setPackageSize();
        void getSnapshot(int num_frames);    
};

#endif