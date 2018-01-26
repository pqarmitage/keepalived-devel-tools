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

	struct sigaction sa;
	sa.sa_handler = do_sigterm;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL)) {
		printf("sigaction failed errno %d - %m", errno);
		exit(1);
	}

	prctl(PR_SET_PDEATHSIG, SIGTERM);

	sleep(5);
	printf("Child has woken up\n");
}
