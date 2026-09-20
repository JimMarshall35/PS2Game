/*
    Copyright Jim Marshall 2026.
    File: main.c
    Description: Texture maker, convert pngs into ps2 format textures suitable for upload to the GS
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

/////////////////////////////////////////////////////////////////////////////////////////// Third Party Includes

#include "IDTEX8_CSM1_CLUTSwizzleTable.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

#include "libimagequant.h"

/////////////////////////////////////////////////////////////////////////////////////////// Typedefs

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;

/////////////////////////////////////////////////////////////////////////////////////////// Defines

#define PSMT8_PAGE_WIDTH  128
#define PSMT8_PAGE_HEIGHT 64

#define PSMT8_BLOCK_WIDTH  16
#define PSMT8_BLOCK_HEIGHT 16


/////////////////////////////////////////////////////////////////////////////////////////// Enums

enum OutputType {
    PSMT8,
    PSMT4
};

enum CLUTStorageFormat {
    PSMCT32,
    PSMCT16,
    PSMCT16S,
};

/////////////////////////////////////////////////////////////////////////////////////////// Structs

struct Args
{
    const char* outPath;
    const char* inPNGPath;
    const char* outColourLUT;
    enum OutputType outputType;
    enum CLUTStorageFormat clutStorageFormat;
    float ditherLevel;
};

/////////////////////////////////////////////////////////////////////////////////////////// Private Globals

static struct Args gArgs = {
    .outPath = "out.texture",
    .inPNGPath = "in.png",
    .outColourLUT = "out.colourLUT",
    .outputType = PSMT8,
    .clutStorageFormat = PSMCT32,
    .ditherLevel = 1.0
};

static const i32 PSMT8_BlockLayout[32] =
{
    0,  1,  4,  5, 16, 17, 20, 21,
    2,  3,  6,  7, 18, 19, 22, 23,
    8,  9, 12, 13, 24, 25, 28, 29,
    10, 11, 14, 15, 26, 27, 30, 31
};

static const u8 PSMT8_ColumnWord[2][64] =
{
    {
        0, 1, 4, 5, 8, 9,12,13,
        0, 1, 4, 5, 8, 9,12,13,
        2, 3, 6, 7,10,11,14,15,
        2, 3, 6, 7,10,11,14,15,

        8, 9,12,13, 0, 1, 4, 5,
        8, 9,12,13, 0, 1, 4, 5,
        10,11,14,15, 2, 3, 6, 7,
        10,11,14,15, 2, 3, 6, 7
    },
    {
        8, 9,12,13, 0, 1, 4, 5,
        8, 9,12,13, 0, 1, 4, 5,
        10,11,14,15, 2, 3, 6, 7,
        10,11,14,15, 2, 3, 6, 7,

        0, 1, 4, 5, 8, 9,12,13,
        0, 1, 4, 5, 8, 9,12,13,
        2, 3, 6, 7,10,11,14,15,
        2, 3, 6, 7,10,11,14,15
    }
};

static const u8 PSMT8_ColumnByte[64] =
{
    0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,

    0,0,0,0,0,0,0,0,
    2,2,2,2,2,2,2,2,

    1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3,

    1,1,1,1,1,1,1,1,
    3,3,3,3,3,3,3,3
};

/////////////////////////////////////////////////////////////////////////////////////////// Private Functions

static void PrintHelp()
{
    const char* helpMsg = 
    "TextureMaker. Jim Marshall 2026.\n"
    "Convert png files into 8 bit indexed ps2 textures and colourLUT\n"
    "usage:\n"
    "TextureMaker -p [input png] -o [output texture] -l [output colour lookup table] [optional --4bit]\n"
    "OR\n"
    "TextureMaker --png [input png] --outFile [output texture] --outColourLUT [output colour lookup table] [optional --4bit]\n"
    "Options:"
    "--4bit : quantize image to 16 colours and generate a 4bpp texture (8bpp is default).\n"
    "--PSMCT16 : output PSMCT16 format CLUT (PSMCT32 is default).\n"
    "--PSMCT16S : output PSMCT16S format CLUT (PSMCT32 is default).\n"
    "Note: texures need to be powers of 2 in size, this tool will exit with a non zero exit code if they're not.\n"
    ;
    printf("%s", helpMsg);
}

bool IsPowerOfTwo(u32 n)
{
    return n != 0 && (n & (n - 1)) == 0;
}

static size_t SwizzlePSMT8(int x, int y, int w, int h)
{
    u32 widthPages = w / PSMT8_PAGE_WIDTH;
    u32 heightPages = h / PSMT8_PAGE_HEIGHT;
    
    // find the page
    int pageX = x / PSMT8_PAGE_WIDTH;
    int pageY = y / PSMT8_PAGE_HEIGHT;
    int page = pageX + pageY * widthPages;

    // x and y coordinate within the page
    int px = x % PSMT8_PAGE_WIDTH;
    int py = y % PSMT8_PAGE_HEIGHT;
    
    // find the block
    int blockX = px / PSMT8_BLOCK_WIDTH;
    int blockY = py / PSMT8_BLOCK_HEIGHT;
    int block = PSMT8_BlockLayout[blockX + blockY * 8];

    // Find the position inside the block
    int bx = px % 16;
    int by = py % 16;

    int column = by / 4;
    int cx = bx;
    int cy = by % 4;

    int index = cx + cy * 16;
    int cw = PSMT8_ColumnWord[column & 1][index];
    int cb = PSMT8_ColumnByte[index];

    return 
        page   * 8192
        + block  * 256
        + column * 64
        + cw     * 4
        + cb;
}

static void WriteTextureFile(u8* pIndices, u32 width, u32 height)
{
    //size_t fwrite( const void* restrict buffer, size_t size, size_t count,FILE* restrict stream );
    switch(gArgs.outputType)
    {
    case PSMT8:
        {
            u8* pSwizzled = malloc(width * height);
            memset(pSwizzled, 0, width * height);
            /*
                Width and height in PS2 vram PSMT8 pages.
                These are 128x64 pixels, 8192 bytes
            */
            
            for (int y = 0; y < height; y++)
            {
                for (int x = 0; x < width; x++)
                {
                    u8 pixel = pIndices[y * width + x];
                    size_t address = SwizzlePSMT8(x, y, width, height);
                    pSwizzled[address] = pixel;
                }
            }

            FILE* pF = fopen(gArgs.outPath, "w");
            fwrite(pSwizzled, width * height, 1, pF);
            fclose(pF);
        }
        break;
    case PSMT4:
        {
            // TODO: implement
        }
        break;
    }
}

void WritePaletteFile(const liq_palette* pPalette)
{
    switch(gArgs.outputType)
    {
    case PSMT8:
        {
            switch(gArgs.clutStorageFormat)
            {
            case PSMCT32:
                {
                    uint32_t* pCLUT = malloc(sizeof(uint32_t) * 256);
                    memset(pCLUT, 0, sizeof(uint32_t) * 256);
                    for(int i=0; i<pPalette->count; i++)
                    {
                        size_t swizzledIndex = (size_t)CLUT_IDTEX8_CSM1_SwizzleTable[i];
                        pCLUT[swizzledIndex] = (pPalette->entries[i].r) | 
                            (pPalette->entries[i].g << 8) |
                            (pPalette->entries[i].b << 16) |
                            (pPalette->entries[i].a << 16);
                    }
                    FILE* pF = fopen(gArgs.outColourLUT, "w");
                    fwrite(pCLUT, sizeof(u32) * 256, 1, pF);
                    fclose(pF);
                    free(pCLUT);
                }
                break;
            case PSMCT16:
                // TODO: implement
                break;
            case PSMCT16S:
                // TODO: implement
                break;
            }
        }
        break;
    case PSMT4:
        {
            // TODO: implement
        }
        break;
    }
}

/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

int main(int argc, const char** argv)
{
    for(int i=0; i < argc; i++)
    {
        if((strcmp(argv[i], "--png") == 0) || (strcmp(argv[i], "-p") == 0))
        {
            if(i + 1 < argc)
            {
                gArgs.inPNGPath = argv[++i];
            }
            else
            {
                printf("No argument supplied for input png");
            }
        }
        else if((strcmp(argv[i], "--outFile") == 0) || (strcmp(argv[i], "-o") == 0))
        {
            if(i + 1 < argc)
            {
                gArgs.outPath = argv[++i];
            }
            else
            {
                printf("No argument supplied for output file");
            }
        }
        else if((strcmp(argv[i], "--outColourLUT") == 0) || (strcmp(argv[i], "-l") == 0))
        {
            if(i + 1 < argc)
            {
                gArgs.outColourLUT = argv[++i];
            }
            else
            {
                printf("No argument supplied for output CLUT");
            }
        }
        else if((strcmp(argv[i], "--help") == 0) || (strcmp(argv[i], "-h") == 0))
        {
            PrintHelp();
            return 0;
        }
        else if((strcmp(argv[i], "--PSMCT16") == 0))
        {
            gArgs.clutStorageFormat = PSMCT16;
        }
        else if((strcmp(argv[i], "--PSMCT16S") == 0))
        {
            gArgs.clutStorageFormat = PSMCT16S;
        }
        else if((strcmp(argv[i], "--4bit") == 0))
        {
            gArgs.clutStorageFormat = PSMCT16S;
        }
        else if((strcmp(argv[i], "--ditherLevel") == 0) || (strcmp(argv[i], "-d") == 0))
        {
            if(i + 1 < argc)
            {
                atof(argv[++i]);
            }
            else
            {
                printf("No argument supplied for output file");
            }
        }
    }

    u32 width, height, channels;
    u8* rawData = stbi_load(gArgs.inPNGPath, (int*)&width, (int*)&height, (int*)&channels, 4);

    if (rawData == NULL)
    {
        fprintf(stderr, "Failed to load image '%s': %s\n", argv[1], stbi_failure_reason());
        return 1;
    }

    if (channels != 4)
    {
        fprintf(stderr, "Expected 4 channels but got %i\n", channels);
        return 1;
    }

    if(!IsPowerOfTwo(width) || !IsPowerOfTwo(height))
    {
        fprintf(stderr, "Texture width and height need to be powers of two. Actual W: %i, Actual H: %i\n", width, height);
        return 1;
    }

    liq_attr* pHandle = liq_attr_create();

    switch(gArgs.outputType)
    {
    case PSMT8:
        liq_set_max_colors(pHandle, 256);
        break;
    case PSMT4:
        liq_set_max_colors(pHandle, 16);
        break;
    }

    
    liq_image* pInputImage = liq_image_create_rgba(pHandle, rawData, width, height, 0);
    
    liq_result* pQuantRes;
    if (liq_image_quantize(pInputImage, pHandle, &pQuantRes) != LIQ_OK)
    {
        fprintf(stderr, "Quantization failed\n");
        return 1;
    }

    size_t pixelsSize = width * height;
    unsigned char* pRaw8bitPixels = malloc(pixelsSize);
    liq_set_dithering_level(pQuantRes, 1.0);

    liq_write_remapped_image(pQuantRes, pInputImage, pRaw8bitPixels, pixelsSize);
    const liq_palette* pPalette = liq_get_palette(pQuantRes);

    WritePaletteFile(pPalette);

    WriteTextureFile(pRaw8bitPixels, width, height);

    stbi_image_free(rawData);


    liq_result_destroy(pQuantRes);
    liq_image_destroy(pInputImage);
    liq_attr_destroy(pHandle);

    return 0;
}
