#define _GNU_SOURCE         /* See feature_test_macros(7) */
//#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#ifndef SYS_memfd_create
#define SYS_memfd_create __NR_memfd_create
#endif

static const char * const str = "I am here\n";

static int
memfd_create(const char *name, int flags)
{
	int ret;

	ret = syscall(SYS_memfd_create, name, flags);

	return ret;
}

int main(int argc, char **argv)
{
	int fd = memfd_create("/keepalived.memfd/bert", 0);

	write(fd, str, strlen(str));

	printf("%d has written memfd, waiting 20 seconds\n", getpid());
	sleep(20);
}
