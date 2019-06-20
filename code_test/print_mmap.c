#define _GNU_SOURCE

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

int main(int argc, char **argv, char **envp)
{
	char cmd[] = "cat /proc/XXXXXXX/maps";

	printf("argv %p, argv[0] %p, envp %p, envp[0] %p, envp[1] %p, prog invoc name (%s) %p pin short (%s) %p\n", argv, argv[0], envp, envp[0], envp[1], program_invocation_name, program_invocation_name, program_invocation_short_name, program_invocation_short_name);

	snprintf(cmd, sizeof cmd, "cat /proc/%d/maps", getpid());
	system(cmd);

	program_invocation_name = "/a/b/c/../d";
	program_invocation_short_name = program_invocation_name + 10;
	printf("Names: %s %s\n", program_invocation_name, program_invocation_short_name);
}
