#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/prctl.h>
#include <errno.h>
#include <stdlib.h>

static void
do_sigterm(int signo)
{
	printf("Received signal %d\n", signo);
	exit(0);
}

int main(int argc, char **argv)
{
	pid_t pid = getpid();

	printf("Parent pid is %d\n", pid);

	pid = fork();
	if (pid < 0) {
		printf("fork() failed - errno %d - %m\n", errno);
		exit(1);
	}

	if (pid) {
		sleep (1);
		exit(0);
	}

	prctl(PR_SET_PDEATHSIG, SIGTERM);

	execl("tmp/sleep10", "tmp/sleep10", NULL);
	printf("Exec failed - %m\n");
}
