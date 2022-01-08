#include <stdio.h>
#include <sys/ptrace.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

static void
sigusr1(int signum)
{
	long retl;

	retl = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
}

int main(int argc, char **argv)
{
	int ret;
	long retl;
	struct sigaction new_act = { .sa_flags = 0 };

	if (fork()) {
		/* Parent */
		sleep(30);
		printf("parent exiting\n");
		exit(0);
	}

	new_act.sa_handler = sigusr1;

	ret = sigaction(SIGUSR1, &new_act, NULL);

	prctl(PR_SET_PDEATHSIG, SIGUSR1);

	retl = ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	printf("PID %d is sleeping 60 seconds, parent %d\n", getpid(), getppid());

	ret = sleep(60);
	if (ret) {
		printf("Sleep returned with %d seconds remaining, parent now %d\n", ret, getppid());
		sleep(ret);
	}

	printf("Now exiting\n");
}
