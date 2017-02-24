#include <stdio.h>
#include <stdlib.h>

#define LINUX_NATIVE_APP	1

#include "../bms.h"
#include "../data.h"

int main(int argc, char *argv[])
{
	printf("Utility program\n");
	
	printf("size of datablock_t structure: %lu\n", sizeof( datablock_t ) );

  return 0;
}
