#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	printf ("GR_SIZE_MAX %ld\n", sysconf(_SC_GETGR_R_SIZE_MAX));
}
