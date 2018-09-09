#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int aint;
	unsigned long slong;
	char *endptr;

	aint = atoi(argv[1]);
	errno = 0;
	slong = strtoul(argv[1], &endptr, 10);
	
	printf("aint %d, slong %lu, (int)slong %u, endptr '%s'%s\n", aint, slong, (unsigned)slong, endptr, errno == ERANGE ? ", ERANGE" : "");
}
