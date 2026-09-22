#ifndef PAK_H
#define PAK_H
#include "IntTypes.h"
#include <stdbool.h>

#define MAX_PAK_FILE_NAME_LEN 31

struct PakFileHeaderEntry
{
    char name[MAX_PAK_FILE_NAME_LEN + 1];

    /*
                    u32               sizeof(struct PakFileHeaderEntry)    sizeof(struct PakFileHeaderEntry)
        | number of header entries |             header entry           |             header entry           | .. etc | file data | file data | file data | etc ...
                                                                                                             |
                                                                                                             |
                                                                                                             V
                                                                   end of the header entries array, offset is from here in bytes to the start of the file

    
    */
    u32 offset;
    /*
        size in bytes
    */
    u32 size;
};

void Pk_CloseCurrentPakFile();

bool Pk_OpenPakFile(const char* path);

u32 Pk_GetPakFileEntrySize(const char* packedFileName);

bool Pk_LoadPakFileEntry(const char* packedFileName, char* outputBuffer, u32 outputBufferSize);

#endif