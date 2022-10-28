#ifndef CameraCommands_h
#define CameraCommands_h

#include <stdexcept>
#include <map>
#include <string>
#include "..\CameraServer\CameraServer.h"

#define NUM_BYTES_IN_CMD 6
#define MAX_SYNC_ATTEMPTS 60
#define MIN_SYNC_DELAY  5

class CameraCommands {
    private:
        static void receiveCameraResponse(byte* reply);
        static void attemptInitialisation(const byte* init_cmd, bool set_package_size);
        static bool setPackageSize();
        static void getSnapshot(int num_frames);
    public:
        void attemptSync();
        void parseInitialisationParameters(const char* command);
        void parseSnapshotParameters(const char* command);
};

#endif