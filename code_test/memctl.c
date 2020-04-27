#define _GNU_SOURCE

#include <memory.h>
#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <execinfo.h>
#include <sys/resource.h>

#define STATIC

/* Get strsignal() declaration from <string.h> */

extern char etext, edata, end; /* The symbols must have some type,
				  or "gcc -Wall" complains */

#define BT_BUF_SIZE	200

STATIC char *extn[] = { "stat", "status", "statm", "maps", "smaps" };
STATIC char cmd[128];
STATIC void *stack_buf[BT_BUF_SIZE];
STATIC int fd;
STATIC pid_t pid;

STATIC void
save_proc_files(int v)
{
	int i;

	sprintf(cmd, "/tmp/%d.%d", pid, v);
	mkdir(cmd, 0755);

	for (i = 0; i < sizeof extn / sizeof extn[0]; i++) {
		sprintf(cmd, "cp -p /proc/%d/%s /tmp/%d.%d", pid, extn[i], pid, v);
		system(cmd);
	}
}

STATIC void
sigsegvHandler(int sig)
{
	int x;

	/* UNSAFE: This handler uses non-async-signal-safe functions
	(printf(), strsignal(), fflush(); see Section 21.1.2) */

	printf("Caught signal %d (%s)\n", sig, strsignal(sig));
	printf("Top of handler stack near %10p\n", (void *) &x);
	fflush(NULL);

	save_proc_files(-1);

//	sprintf(cmd, "/tmp/%d/stacktrace", pid);
//	fd = open(cmd, O_WRONLY | O_APPEND);
	write(fd, "\n\nsigsegv\n\n", 11);
	backtrace_symbols_fd(stack_buf, BT_BUF_SIZE, fd);
//	close(fd);

	_exit(1 /*EXIT_FAILURE*/);			/* Can't return after SIGSEGV */
}

STATIC void			/* A recursive function that overflows the stack */
overflowStack(int callNum)
{
	char a[100000];		/* Make this stack frame large */

	if (callNum > 80 || callNum <= 1)
		printf("Call %4d - top of stack near %10p/%10p\n", callNum, &a[0], &a[99999]);

//	save_proc_files(callNum);

//	sprintf(cmd, "/tmp/%d/stacktrace", pid);
//	fd = open(cmd, O_WRONLY | O_APPEND);
	write(fd, "\n\noverflow\n\n", 12);
	backtrace_symbols_fd(stack_buf, BT_BUF_SIZE, fd);
//	close(fd);

	overflowStack(callNum+1);
}

STATIC void
print_rlim(void)
{
	struct rlimit rlim;

	printf("Memory split: etext %p, edata %p, end %p, break %p, stack_cur %p\n", &etext, &edata, &end, sbrk(0), &rlim);
	getrlimit(RLIMIT_AS, &rlim);
	printf("RLIMIT_AS soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
	getrlimit(RLIMIT_DATA, &rlim);
	printf("RLIMIT_DATA soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
	getrlimit(RLIMIT_STACK, &rlim);
	printf("RLIMIT_STACK soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
}

STATIC void
allocate_stack(void)
{
	stack_t sigstack;
	struct sigaction sa;

	/* Allocate alternate stack and inform kernel of its existence */
	sigstack.ss_sp = malloc(SIGSTKSZ);
	if (sigstack.ss_sp == NULL)
		fprintf(stderr, "malloc\n");

	sigstack.ss_size = SIGSTKSZ;
	sigstack.ss_flags = 0;

	if (sigaltstack(&sigstack, NULL) == -1)
		fprintf(stderr, "sigaltstack\n");

	printf("Alternate stack is at %10p-%p\n", sigstack.ss_sp, (char *) sbrk(0) - 1);

	sa.sa_handler = sigsegvHandler;		/* Establish handler for SIGSEGV */
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_ONSTACK;

	/* Handler uses alternate stack */
	if (sigaction(SIGSEGV, &sa, NULL) == -1)
		fprintf(stderr, "sigaction\n");
}

int
main(int argc, char *argv[])
{
	int i;
	char *envp_min = (char *)-1;
	char *envp_max = 0;
	char **envp;

	pid = getpid();

	for (envp = environ, i = 0; *envp; envp++, i++)
	{
		if (*envp < envp_min)
			envp_min = *envp;
		if (*envp > envp_max)
			envp_max = *envp + strlen(*envp + 1);;
//		printf("envp[%d] = 0x%p (%s)\n", i, *envp, *envp);
	}

	printf("SIGSTKSZ %lu, MINSIGSTKSZ %lu\n", SIGSTKSZ, MINSIGSTKSZ);
	printf("Top of standard stack for %d is near %10p\n", pid, (void *) &i);
	printf("argv %p, argv[0] %p, &argv[%d] %p\n", argv, argv[0], argc, &argv[argc]);
	printf("envp %p, envp[0] %p, &envp[%d] %p\n", environ, environ[0], i, &environ[i]);
	printf("argv %p - %p, envp %p - %p\n",
		argv[0], argv[argc - 1] + strlen(argv[argc - 1]),
		envp_min, envp_max);

	printf("First address past:\n");
	printf("    program text (etext)      %10p\n", &etext);
	printf("    initialized data (edata)  %10p\n", &edata);
	printf("    uninitialized data (end)  %10p\n", &end);
	printf("    brk                       %10p\n", sbrk(0));

	print_rlim();

	allocate_stack();

	save_proc_files(0);

	sprintf(cmd, "/tmp/%d.0/stacktrace", pid);
	fd = open(cmd, O_WRONLY | O_APPEND | O_CREAT);
	write(fd, "\n\n", 2);
	backtrace_symbols_fd(stack_buf, BT_BUF_SIZE, fd);
//	close(fd);

	overflowStack(1);
}
