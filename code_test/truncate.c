#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int len = atoi(argv[2]);

	truncate(argv[1], len);
}
