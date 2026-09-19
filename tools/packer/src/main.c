/*
    Copyright Jim Marshall 2026.
    File: main.c
    Description: File packer, pack multiple files into one
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

/////////////////////////////////////////////////////////////////////////////////////////// Third Party Includes

#include "cJSON.h"

/////////////////////////////////////////////////////////////////////////////////////////// First Party Includes

#include "pak.h"
#include "DynArray.h"

/////////////////////////////////////////////////////////////////////////////////////////// Typedefs

typedef uint8_t u8;

/////////////////////////////////////////////////////////////////////////////////////////// Defines

#define FILE_ALIGNMENT_BYTES 4

/////////////////////////////////////////////////////////////////////////////////////////// Structs

/* used by this tool to build the pak file */
struct PakFileEntry
{
    char name[MAX_PAK_FILE_NAME_LEN];
    char* bytes;
    int numBytes;
    struct PakFileEntry* pNext;
};

/////////////////////////////////////////////////////////////////////////////////////////// Private Globals

static const char* gManifestPath = "./in.json";
static const char* gOutPath = "./out.pak";
static char gErrorMsgBuf[256];
struct PakFileEntry* gPakFileListHead = NULL;
struct PakFileEntry* gPakFileListTail = NULL;
u32 gPakFileListCount = 0;

/////////////////////////////////////////////////////////////////////////////////////////// Private Functions


const char* LoadFileToString(const char* path)
{
	FILE* fp = fopen(path, "r");
	if (!fp) return NULL;
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
    sz++; // add one for null character
    fseek(fp, 0L, SEEK_SET);
	char* pOut = malloc(sz);
	size_t r = fread(pOut, 1, sz - 1, fp);
    pOut[sz - 1] = '\0';
	fclose(fp);
	return pOut;
}

char* LoadFile(const char* path, int* outSize)
{
	FILE* fp = fopen(path, "r");
	if (!fp) return NULL;
	fseek(fp, 0L, SEEK_END);
	*outSize = ftell(fp);
	int sz = *outSize;
	fseek(fp, 0L, SEEK_SET);
	void* pOut = malloc(sz);
	size_t r = fread(pOut, 1, sz, fp);
	fclose(fp);
	return pOut;
}

static bool AddPakFileEntry(const char* name, const char* filePath)
{
    int lenName = strlen(name);
    if (lenName > MAX_PAK_FILE_NAME_LEN)
    {
        sprintf(gErrorMsgBuf, "AddPakFileEntry, name '%s' length (%i) exceeds the maximum allowable %i characters", name, lenName, MAX_PAK_FILE_NAME_LEN);
        return true;
    }
    struct PakFileEntry* pEntry = (struct PakFileEntry*)malloc(sizeof(struct PakFileEntry));
    int size;
    pEntry->bytes = LoadFile(filePath, &size);
    pEntry->numBytes = size;
    pEntry->pNext = NULL;
    strcpy(pEntry->name, name);
    if(gPakFileListHead == NULL)
    {
        gPakFileListHead = pEntry;
        gPakFileListTail = pEntry;
    }
    else
    {
        gPakFileListTail->pNext = pEntry;
        gPakFileListTail = pEntry;
    }
    gPakFileListCount++;
}

static VECTOR(u8) Addu16ToByteArray(VECTOR(u8) vec, u16 val)
{
    u8* ptr = (u8*)&val;
    vec = VectorPush(vec, ptr++);
    vec = VectorPush(vec, ptr);
    return vec;
}

static VECTOR(u8) Addu32ToByteArray(VECTOR(u8) vec, u32 val)
{
    u8* ptr = (u8*)&val;
    vec = VectorPush(vec, ptr++);
    vec = VectorPush(vec, ptr++);
    vec = VectorPush(vec, ptr++);
    vec = VectorPush(vec, ptr);
    return vec;
}

static VECTOR(u8) AddPakFileHeaderEntry(VECTOR(u8) vec, const struct PakFileHeaderEntry* pHeaderEntry)
{
    u8* pCast = (u8*)pHeaderEntry;
    for(int i=0; i<sizeof(struct PakFileHeaderEntry); i++)
    {
        vec = VectorPush(vec, &pCast[i]);
    }
    return vec;
}


static VECTOR(u8) GetPakFileHeader()
{
    VECTOR(u8) pHeader = NEW_VECTOR(u8);
    pHeader = Addu32ToByteArray(pHeader, gPakFileListCount);
    struct PakFileEntry* pEntry = gPakFileListHead;
    u32 totalOffset = 0;
    while(pEntry)
    {
        struct PakFileHeaderEntry headerEntry;
        strcpy(headerEntry.name, pEntry->name);
        headerEntry.size = pEntry->numBytes;
        headerEntry.offset = totalOffset;
        pHeader = AddPakFileHeaderEntry(pHeader, &headerEntry);

        totalOffset += pEntry->numBytes;
        while(totalOffset % FILE_ALIGNMENT_BYTES != 0)
        {
            totalOffset++;
        }
        pEntry = pEntry->pNext;

    }
    return pHeader;
} 

static VECTOR(u8) AddPakFileContents(VECTOR(u8) vec)
{
    struct PakFileEntry* pEntry = gPakFileListHead;
    u32 totalOffset = 0;
    while(pEntry)
    {
        for(int i=0; i<pEntry->numBytes; i++)
        {
            vec = VectorPush(vec, &pEntry->bytes[i]);
        }
        totalOffset += pEntry->numBytes;
        while(totalOffset % FILE_ALIGNMENT_BYTES != 0)
        {
            u8 val = 0;
            vec = VectorPush(vec, &val);
            totalOffset++;
        }
        pEntry = pEntry->pNext;
    }
}

static void PrintHelp()
{
    const char* helpMsg = 
    "Packer. Jim Marshall 2026.\n"
    "usage:\n"
    "Packer -m [manifest path] -o [output path]\n"
    "OR\n"
    "Packer --manifest [manifest path] --output [output path]\n"
    "example manifest:\n"
    "{\n"
    "  \"files\": [\n"
    "    { \"name\": \"file_name\", \"path\": \"./assets/texture.texture\"}\n"
    "  ]\n"
    "}\n"
    ;
    printf("%s", helpMsg);
}

/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

int main(int argc, const char** argv)
{
    for(int i=0; i < argc; i++)
    {
        if((strcmp(argv[i], "--manifest") == 0) || (strcmp(argv[i], "-m") == 0))
        {
            if(i + 1 < argc)
            {
                gManifestPath = argv[++i];
            }
            else
            {
                printf("No argument supplied for manifest");
            }
        }
        else if((strcmp(argv[i], "--outFile") == 0) || (strcmp(argv[i], "-o") == 0))
        {
            if(i + 1 < argc)
            {
                gOutPath = argv[++i];
            }
            else
            {
                printf("No argument supplied for output file");
            }
        }
        else if((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0))
        {
            PrintHelp();
            return 0;
        }
    }

    const char* manifestString = LoadFileToString(gManifestPath);
    cJSON* manifestJSON = cJSON_Parse(manifestString);
    if (manifestJSON == NULL)
    {
        const char* error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            fprintf(stderr, "Error before: %s\n", error_ptr);
        }
        sprintf(gErrorMsgBuf, "Parsing JSON");
        goto error;
    }

    const cJSON* files = cJSON_GetObjectItemCaseSensitive(manifestJSON, "files");
    if(!cJSON_IsArray(files))
    {
        sprintf(gErrorMsgBuf, "file JSON item is not an array");
        goto error;
    }

    int i = 0;
    cJSON* file = NULL;
    cJSON_ArrayForEach(file, files)
    {
        cJSON* name = cJSON_GetObjectItemCaseSensitive(file, "name");
        cJSON* path = cJSON_GetObjectItemCaseSensitive(file, "path");

        if (!cJSON_IsString(name))
        {
            sprintf(gErrorMsgBuf, "file array item %i name field is not a string", i);
            goto error;
        }
        if (!cJSON_IsString(path))
        {
            sprintf(gErrorMsgBuf, "file array item %i path field is not a string", i);
            goto error;
        }

        const char* pathStr = path->valuestring;
        const char* nameStr = path->valuestring;
        
        AddPakFileEntry(nameStr, pathStr);

        i++;
    }
    VECTOR(u8) pak = GetPakFileHeader();
    pak = AddPakFileContents(pak);
    FILE* pFile = fopen(gOutPath, "w");
    fwrite(pak, VectorSize(pak), 1, pFile);
    fclose(pFile);
    return 0;
error:
    static char sErrorFormatBuf[512];
    sprintf(sErrorFormatBuf, "ERROR: %s", gErrorMsgBuf);
    printf("%s", sErrorFormatBuf);
    return 1;
}