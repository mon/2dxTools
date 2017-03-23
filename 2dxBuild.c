#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "2dx.h"
#include "shared.h"

int count_wav(void) {
    int ret = 0;
    char filename[256];
    while(snprintf(filename, 256, "%d.wav", ret), file_exists(filename))
        ret++;
    return ret;
}

void build_2dx(char* path) {
    int wavCount = count_wav();
    printf("Writing %d wavs to %s\n",wavCount, path);
    
    FILE* outFile = fopen(path, "wb");
    
    if(!outFile) {
        printf("Could not open %s\n", path);
        return;
    }
    
    fileHeader_t fileHeader;
    dxHeader_t dxHeader;
    char wavPath[256];
    
    // +1 to ignore null terminator, clobbers headerSize but we write that next
    strncpy(fileHeader.name, path, sizeof(fileHeader.name)+1);
    fileHeader.headerSize = sizeof(fileHeader) + sizeof(uint32_t)*wavCount;
    fileHeader.fileCount = wavCount;
    memset(fileHeader.unknown, 0, sizeof(fileHeader.unknown));
    fwrite(&fileHeader, sizeof(fileHeader), 1, outFile);
    
    // file offsets
    uint32_t offset = fileHeader.headerSize;
    for(int i = 0; i < wavCount; i++) {
        snprintf(wavPath, 256, "%d.wav", i);
        FILE* wav = fopen(wavPath, "rb");
        
        fseek(wav, 0L, SEEK_END);
        size_t wavSize = ftell(wav);
        rewind(wav);
        
        fwrite(&offset, sizeof(uint32_t), 1, outFile);
        offset += wavSize + sizeof(dxHeader_t);
        fclose(wav);
    }
    // the actual wavs
    for(int i = 0; i < wavCount; i++) {
        snprintf(wavPath, 256, "%d.wav", i);
        FILE* wav = fopen(wavPath, "rb");
        
        fseek(wav, 0L, SEEK_END);
        size_t wavSize = ftell(wav);
        rewind(wav);
        
        memcpy(dxHeader.dx, "2DX9", 4);
        dxHeader.headerSize = 24;
        dxHeader.wavSize = wavSize;
        dxHeader.unk1 = 0x3231;
        // these match preview files for convenience
        dxHeader.trackId = -1; // I may regret this
        dxHeader.unk2 = 64;
        dxHeader.attenuation = 1;
        dxHeader.loopPoint = 0;
        fwrite(&dxHeader, sizeof(dxHeader), 1, outFile);
        transfer_file(wav, outFile, wavSize);
        
        fclose(wav);
    }
    
    fclose(outFile);
    printf("Done!\n");
}

int main(int argc, char** argv) {
    if(argc != 2) {
        printf("Usage: 2dxbuild output\n");
        return 1;
    }
    
    build_2dx(argv[1]);
    return 0;
}