#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int aint;
	long slong;
	char *endptr;

	aint = atoi(argv[1]);
	errno = 0;
	slong = strtol(argv[1], &endptr, 10);
	
	printf("aint %d, slong %ld, (int)slong %d, endptr '%s'%s\n", aint, slong, (int)slong, endptr, errno == ERANGE ? ", ERANGE" : "");
}
