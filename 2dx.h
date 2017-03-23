#include <stdint.h>

typedef struct {
    char name[16];
    uint32_t headerSize; // this + offsets table
    uint32_t fileCount;
    char unknown[48]; // contains rest of title, some random flags
} fileHeader_t;

typedef struct{
    char dx[4]; // should be "2DX9";
    uint32_t headerSize; // always 24, includes dx chars
    uint32_t wavSize;
    int16_t unk1; // always 0x3231
    int16_t trackId; // always -1 for previews, 0-7 for song + effected versions, 9 to 11 used for a few effects
    int16_t unk2; // all 64, except song selection change 'click' is 40
    int16_t attenuation; // 0-127 for varying quietness
    int32_t loopPoint; // sample to loop at * 4
} dxHeader_t;