#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	printf("Num processors configured %ld, num online %ld\n", sysconf(_SC_NPROCESSORS_CONF), sysconf(_SC_NPROCESSORS_ONLN));
}
