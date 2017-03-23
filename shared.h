#ifndef _SHARED_H
#define _SHARED_H

#include <stdio.h>

void transfer_file(FILE* in, FILE* out, size_t bytes);
size_t file_size(const char* path);
int file_exists(const char *path);

#endif