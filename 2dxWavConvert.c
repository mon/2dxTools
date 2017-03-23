#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "shared.h"
#include "2dxWav.h"

int main(int argc, char * argv[])
{
  if(argc != 3) {
      printf("2dxWavConvert infile outwav\n");
      return 1;
  }
  
  if(convert_wav(argv[1], argv[2], 0)) {
      printf("Conversion failed!\n");
      return 1;
  }
  return 0;
}
