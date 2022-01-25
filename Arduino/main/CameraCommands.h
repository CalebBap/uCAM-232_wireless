#ifndef CameraCommands_h
#define CameraCommands_h

#include <Arduino.h>

class CameraCommands{
    private:
    
    public:
        void attemptSync();
        void attemptInitialisation(const char* command);
};

#endif
