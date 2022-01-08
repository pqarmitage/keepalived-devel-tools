#include <stdio.h>
#include <sys/ptrace.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	long ret;
	
	ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);

	printf("PID %d is sleeping\n", getpid());

	sleep(60);

	printf("Now exiting\n");
}
