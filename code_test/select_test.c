#define __FD_SETSIZE 2048
#define FD_SETSIZE 2048

#include <sys/select.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	fd_set q;

#ifdef __USE_FORTIFY_LEVEL
	printf("fortify_level = %d\n", __USE_FORTIFY_LEVEL);
#endif
	FD_ZERO(&q);
	FD_SET(1280, &q);
}
