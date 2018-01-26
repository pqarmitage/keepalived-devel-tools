#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv)
{
	char *p;

	p = realpath(argv[1], NULL);

	if (!p)
		printf("Error %d - %m\n", errno);
	else
		printf("Realpath: %s\n", p);
}
