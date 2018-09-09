#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

int main(int argc, char **argv)
{
	double adouble;
	float sfloat;
	char *endptr;

	adouble = atof(argv[1]);
	errno = 0;
	sfloat = strtof(argv[1], &endptr);
	
	printf("adouble %g, sfloat %g, endptr '%s'%s, isnan %d, isinf %d, sfloat %c= HUGE_VAL\n", adouble, sfloat, endptr, errno == ERANGE ? ", ERANGE" : "", isnan(sfloat), isinf(sfloat), sfloat == HUGE_VAL ? '=' : '!');
}
