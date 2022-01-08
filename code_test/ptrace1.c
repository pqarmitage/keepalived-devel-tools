#include <stdio.h>
#include <sys/ptrace.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	long ret;

	if (fork()) {
		/* Parent */
		sleep(30);
		printf("parent exiting\n");
		exit(0);
	}

	ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	printf("PID %d is sleeping\n", getpid());

	sleep(60);

	printf("Now exiting\n");
}
