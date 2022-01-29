#ifndef FileOperations_h
#define FileOperations_h

#include <Arduino.h>

class FileOperations{
    private:

    public:
        static String getMimeType(String filename);
        static String getFilePath(String uri);
};

#endif
