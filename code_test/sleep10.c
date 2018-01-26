#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <stdlib.h>

static void
do_sigterm(int signo)
{
	printf("Child %d received signal %d\n", getpid(), signo);
	exit(0);
}

int main(int argc, char **argv)
{
	pid_t pid = getpid();

	printf("Child pid is %d, parent is %d\n", pid, getppid());

	struct sigaction sa;
	sa.sa_handler = do_sigterm;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = 0;
	if (sigaction(SIGTERM, &sa, NULL)) {
		printf("sigaction failed errno %d - %m", errno);
		exit(1);
	}

	sleep(10);
	printf("Child has woken up\n");
}
