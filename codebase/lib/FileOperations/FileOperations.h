#ifndef FILE_OPERATIONS_H
#define FILE_OPERATIONS_H

#include <FS.h>

class FileOperations {
    public:
        static String getMimeType(const String filename);
        static String getFilePath(String uri);
    
    private:
};

#endif
