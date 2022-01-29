#ifndef CameraCommands_h
#define CameraCommands_h

#include <Arduino.h>
#include "..\CameraServer\CameraServer.h"

#define NUM_BYTES_IN_CMD 6
#define MAX_SYNC_ATTEMPTS 60
#define MIN_SYNC_DELAY  5

class CameraCommands{
    private:
    
    public:
        void attemptSync();
        void attemptInitialisation(const char* command);
};

#endif
