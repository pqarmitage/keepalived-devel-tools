#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/prctl.h>
#include <linux/capability.h>

int main(int argc, char **argv, char **envp)
{
	printf("PID %d: %sCAP_SYS_RESOURCE, argv: 0x%x, envp: 0x%x\n", getpid(), prctl(PR_CAPBSET_READ, CAP_SYS_RESOURCE) ? "" : "no ", argv, envp);

	sleep(60);
}
