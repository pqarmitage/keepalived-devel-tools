#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	pid_t pid;
	int signum;
	int sleep_time = 0;

	if (argc < 3) {
		fprintf(stderr, "kill signum pid [sleep]\n");
		exit(1);
	}

	if (argc >= 4)
		sleep_time = atoi(argv[3]);

	signum = atoi(argv[1]);
	pid = atoi(argv[2]);

	printf("Pid %d sending signal %d to pid %d\n", getpid(), signum, pid);

	kill(pid, signum);

	if (sleep_time)
		sleep(sleep_time);
}
