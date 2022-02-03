#ifndef CameraCommands_h
#define CameraCommands_h

#include <Arduino.h>
#include <map>
#include <string>
#include "..\CameraServer\CameraServer.h"

#define NUM_BYTES_IN_CMD 6
#define MAX_SYNC_ATTEMPTS 60
#define MIN_SYNC_DELAY  5

class CameraCommands{
    private:
        static void attemptInitialisation(const byte* init_cmd);
    public:
        void attemptSync();
        void parseInitialisationParameters(const char* command);
};

#endif
