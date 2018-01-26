#include <stdlib.h>
#include <stdio.h>

int main(int argc, char ** argv)
{
	int ret;

	ret = system(argv[1]);
	printf ("system returned %d (0x%x)\n", ret, ret);

	exit(ret);
}
