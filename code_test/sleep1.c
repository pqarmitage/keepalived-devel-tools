#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	char *ptr[4];

	ptr[0] = malloc(1);
	*ptr[0] = 1;
	ptr[1] = malloc(0x30000);
//	ptr[1][0] = 2;
//	ptr[1][0xffff] = 2;
//	ptr[2] = malloc(0x10000);
//	ptr[2][0xffff] = 3;
//	ptr[3] = malloc(0x10000);
//	ptr[3][0xffff] = 4;
	sleep(100);
}
