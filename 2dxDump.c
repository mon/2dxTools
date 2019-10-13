#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "2dx.h"
#include "shared.h"

void extract_2dx(char* path) {
    FILE* f = fopen(path, "rb");
    
    if(!f) {
        printf("Could not open %s, skipping\n", path);
        return;
    }
    
    fileHeader_t fileHeader;
    uint32_t *fileOffsets;
    dxHeader_t dxHeader;
    char outPath[256];
    FILE* outFile;
    
    fread(&fileHeader, sizeof(fileHeader), 1, f);
    //printf("2dx contains %d file(s)\n", fileHeader.fileCount);
    fileOffsets = malloc(sizeof(uint32_t) * fileHeader.fileCount);
    fread(fileOffsets, sizeof(uint32_t), fileHeader.fileCount, f);
    
    for(int i = 0; i < fileHeader.fileCount; i++) {
        fseek(f, fileOffsets[i], SEEK_SET);
        
        // TODO verify 2DX9
        fread(&dxHeader, sizeof(dxHeader), 1, f);
        snprintf(outPath, 256, "%d.wav", i);
        outFile = fopen(outPath, "wb");
        // seek to RIFF start
        fseek(f, fileOffsets[i]+dxHeader.headerSize, SEEK_SET);
        fprintf(stderr, "Extracting %s...\n", outPath);
        transfer_file(f, outFile, dxHeader.wavSize);
        fclose(outFile);
    }
    fclose(f);
    free(fileOffsets);
    //printf("Done!\n");
}

int main(int argc, char** argv) {
    if(argc < 2) {
        printf("Usage: 2dxdump file1 [file2 ...]\n");
        return 1;
    }
    
    for(int i = 1; i < argc; i++) {
        extract_2dx(argv[i]);
    }
    return 0;
}
