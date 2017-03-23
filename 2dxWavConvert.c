#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "shared.h"
#include "2dxWav.h"

int main(int argc, char * argv[])
{
  if(argc != 3 && argc != 4) {
      printf("2dxWavConvert infile outwav [preview]\n");
      return 1;
  }
  int preview = 0;
  if(argc == 4 && !strcmp("preview", argv[3]))
      preview = 1;
  
  if(convert_wav(argv[1], argv[2], preview)) {
      printf("Conversion failed!\n");
      return 1;
  }
  return 0;
}
