#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "2dx.h"
#include "shared.h"

void update_2dx(char* inPath, char* outPath) {
    size_t wavSize;
    fileHeader_t fileHeader;
    uint32_t *fileOffsets;
    uint32_t *newOffsets;
    dxHeader_t dxHeader;
    char wavPath[256];
    
    FILE* inFile = fopen(inPath, "rb");
    if(!inFile) {
        printf("Could not open %s for reading\n", inPath);
        return;
    }
    
    FILE* outFile = fopen(outPath, "wb");
    if(!outFile) {
        printf("Could not open %s for writing\n", outPath);
        return;
    }
    
    fread(&fileHeader, sizeof(fileHeader), 1, inFile);
    fileOffsets = malloc(sizeof(uint32_t) * fileHeader.fileCount);
    newOffsets = malloc(sizeof(uint32_t) * fileHeader.fileCount);
    fread(fileOffsets, sizeof(uint32_t), fileHeader.fileCount, inFile);
    memcpy(newOffsets, fileOffsets, sizeof(uint32_t) * fileHeader.fileCount);

    // update offsets for new wavs, start at 1 cause it has same offset
    for(int i = 1; i < fileHeader.fileCount; i++) {
        snprintf(wavPath, 256, "%d.wav", i-1);
        // we have a new wav!
        if((wavSize = file_size(wavPath)) == 0) {
            // calculate size from old offsets
            wavSize = fileOffsets[i] - fileOffsets[i-1] - sizeof(dxHeader);
        }
        newOffsets[i] = newOffsets[i-1] + sizeof(dxHeader) + wavSize;
    }
    
    rewind(inFile);
    // unchanged
    fwrite(&fileHeader, sizeof(fileHeader), 1, outFile);
    // new offsets
    fwrite(newOffsets, sizeof(uint32_t), fileHeader.fileCount, outFile);
    // new wavs
    for(int i = 0; i < fileHeader.fileCount; i++) {
        // load existing header
        fseek(inFile, fileOffsets[i], SEEK_SET);
        fread(&dxHeader, sizeof(dxHeader), 1, inFile);
        
        snprintf(wavPath, 256, "%d.wav", i);
        // new wav to insert
        if((wavSize = file_size(wavPath))) {
            printf("Updating %s\n", wavPath);
            dxHeader.wavSize = wavSize;
            FILE* newWav = fopen(wavPath, "rb");
            fwrite(&dxHeader, sizeof(dxHeader), 1, outFile);
            transfer_file(newWav, outFile, wavSize);
            fclose(newWav);
        } else {
            fwrite(&dxHeader, sizeof(dxHeader), 1, outFile);
            transfer_file(inFile, outFile, dxHeader.wavSize);
        }
    }
    
    fclose(outFile);
    fclose(inFile);
    printf("Done!\n");
}

int main(int argc, char** argv) {
    if(argc != 3) {
        printf("Usage: 2dxupdate input output\n");
        return 1;
    }
    
    update_2dx(argv[1], argv[2]);
    return 0;
}