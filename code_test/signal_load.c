#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv)
{
	unsigned loop_count = 1000;
	useconds_t delay = 1000;
	pid_t pid;

	if (argc < 2) {
		fprintf(stderr, "Need pid [loop_ count [delay]]\n");
		return 1;
	}

	pid = atoi(argv[1]);
	if (pid <= 1) {
		fprintf(stderr, "pid must be more than 1\n");
		return 1;
	}

	if (argc >= 3) {
		if (strcmp(argv[2], "."))
			loop_count = atoi(argv[2]);
		if (argc >= 4) {
			if (strcmp(argv[3], "."))
				delay = atoi(argv[3]);
		}
	}

	while (loop_count--) {
		kill(pid, SIGUSR1);
		if (usleep(delay)) {
			fprintf(stderr, "usleep() returned errno %d - %m\n", errno);
			return 1;
		}
	}
}
