#include <sys/statfs.h>
#include <magic.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	struct statfs buf;
	int ret;

	ret = statfs(argv[1], &buf);
	printf("statfs returned %d\n", ret); fflush(stdout);

	if (ret) {
		fprintf(stderr, "statfs(%s) returned errno %d (%m)\n", argv[1], errno);
		exit(1);
	}

	printf("fs type: 0x%x\n", buf.f_type);
}
