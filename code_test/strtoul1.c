#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>

char str[] = "  \n\r\f\v\t -1234";
int main(int argc, char **argv)
{
	int aint;
	unsigned long slong;
	char *endptr;

	aint = atoi(str);
	errno = 0;
	slong = strtoul(str, &endptr, 10);
	
	printf("aint %d, slong %lu, (int)slong %u, endptr '%s'%s\n", aint, slong, (unsigned)slong, endptr, errno == ERANGE ? ", ERANGE" : "");
}
