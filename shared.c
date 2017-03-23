#include "shared.h"
#include <stdlib.h>

// TODO: buffering for speed
void transfer_file(FILE* in, FILE* out, size_t bytes) {
    char byte;
    for(size_t i = 0; i < bytes; i++) {
        if(!fread(&byte, 1, 1, in)) {
            printf("Could not read all bytes from input!\n");
            exit(1);
        }
        fwrite(&byte, 1, 1, out);
    }
}

// returns 0 if file is empty or noexistant, filesize otherwise
size_t file_size(const char* path) {
    FILE* f = fopen(path, "rb");
    if(!f)
        return 0;
    fseek(f, 0L, SEEK_END);
    size_t size = ftell(f);
    fclose(f);
    return size;
}

// platform agnostic and proves we can read a file too
int file_exists(const char *path) {
    FILE *file;
    if ((file = fopen(path, "rb"))) {
        fclose(file);
        return 1;
    }
    return 0;
}