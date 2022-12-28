#ifndef FileOperations_h
#define FileOperations_h

#include <FS.h>

class FileOperations {
    public:
        static String getMimeType(const String filename);
        static String getFilePath(String uri);
    
    private:
};

#endif
