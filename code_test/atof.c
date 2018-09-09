#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <errno.h>
#include <math.h>

int main(int argc, char **argv)
{
	double adouble;
	double sdouble;
	char *endptr;

	adouble = atof(argv[1]);
	errno = 0;
	sdouble = strtod(argv[1], &endptr);
	
	printf("adouble %g, sdouble %g, endptr '%s'%s, isnan %d, isinf %d, sdouble %c= HUGE_VAL\n", adouble, sdouble, endptr, errno == ERANGE ? ", ERANGE" : "", isnan(sdouble), isinf(sdouble), sdouble == HUGE_VAL ? '=' : '!');
	printf("sdouble <= 0 %d, sdouble >= 0 %d\n", sdouble <= 0, sdouble >= 0);
}
