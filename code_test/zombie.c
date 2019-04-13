#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	pid_t pid;
	pid_t child_pid, wait_pid;
	int status;

	pid = getpid();

	child_pid = fork();
	if (child_pid == -1) {
		printf("fork() failed\n");
		exit(1);
	}

	if (!child_pid)
		exit(0);

	printf("parent %d sleeping for child %d\n", pid, child_pid);
	sleep(20);

	wait_pid = wait(&status);
	printf("parent(%d) exiting after child(%d) %d\n", pid, child_pid, wait_pid);
}
