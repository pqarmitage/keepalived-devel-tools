#define _GNU_SOURCE

#include <stdio.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <dlfcn.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>


/*
 * RLIMIT_AS	 - virtual memory (address spaec) - brk(), mmap(), mremap()
 * RLIMIT_DATA	 - data segment limit - 
 * RLIMIT_STACK
 *
 * RLIMIT_CPU	 - limit in seconds of CPU time
 * RLIMIT_RTTIME
 */

/*
 * RESULTS
 *
 * 1. No heap is allocated initially (run sleep here and look at maps)
 *
 */

/*
 * QUERIES
 *
 * 1. In smaps VmFlags, what is accountable. ? does it count towards RLIMIT_DATA/RLIMIT_AS?
 * 2. Can we use smaps_rollup?
 *
 */

extern char etext, edata, end; /* The symbols must have some type,
				  or "gcc -Wall" complains */

static char *extn[] = { "stat", "status", "statm", "maps", "smaps", "smaps_rollup" };
static char cmd[128];
static void *(*malloc_lib)(size_t);
static int malloc_depth;
static pid_t pid;

void *
malloc(size_t len)
{
	void *ret;

	if (!malloc_depth++) {
		printf("malloc %zu\n", len);
	}	

	ret = (*malloc_lib)(len);

	if (!--malloc_depth)
		printf("malloc'd %zu at %p\n", len, ret);

	return ret;
}

static void
sigxcpu(int signum)
{
	printf("In sigxcpu\n");
}

static void
sigsegv(int signum)
{
	printf("In sigsegv\n");
//	exit(signum);
}

static void
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

static void
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

static void
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

        sa.sa_handler = sigsegv;         /* Establish handler for SIGSEGV */
        sigemptyset(&sa.sa_mask);
        sa.sa_flags = SA_ONSTACK;

        /* Handler uses alternate stack */
        if (sigaction(SIGSEGV, &sa, NULL) == -1)
                fprintf(stderr, "sigaction\n");
}

int main(int argc, char **argv)
{
	int i;
	char *envp_min = (char *)-1;
        char *envp_max = 0;
        char **envp;
	struct sigaction act = { .sa_handler = sigxcpu };
	void *orig_brk = sbrk(0);
	struct rlimit rlim;

	malloc_lib = dlsym(RTLD_NEXT, "malloc");

        pid = getpid();

//	save_proc_files(-1);

	free(malloc(1));
	void *new_brk = sbrk(0);

	save_proc_files(0);

        for (envp = environ, i = 0; *envp; envp++, i++)
        {
                if (*envp < envp_min)
                        envp_min = *envp;
                if (*envp > envp_max)
                        envp_max = *envp + strlen(*envp + 1);;
//              printf("envp[%d] = 0x%p (%s)\n", i, *envp, *envp);
        }

	printf("SIGSTKSZ %lu, MINSIGSTKSZ %lu\n", SIGSTKSZ, MINSIGSTKSZ);
	printf("Top of standard stack for %d is near %10p\n", pid, (void *) &argc < (void *)&argv ? (void *) &argv : (void *) &argc);
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
	printf("    orig brk                  %10p\n", orig_brk);
	printf("    new brk                   %10p\n", new_brk);

	print_rlim();

	save_proc_files(1);

	allocate_stack();

	save_proc_files(2);

	sigaction(SIGXCPU, &act, NULL);

	if (getrlimit(RLIMIT_DATA, &rlim))
		printf("getrlimit failed - %d (%m)\n", errno);
	rlim.rlim_cur = 24 * 1024 * 1024;
	if (setrlimit(RLIMIT_DATA, &rlim))
		printf("setrlimit failed - %d (%m)\n", errno);

	for (i = 0; i < 1024; i++) {
		if (sbrk(65536) == (void *)-1)
			break;

		printf("%d  alloc success\n", i);
	}

	printf("sbrk failed for %d after %d iterations, brk now %p\n", pid, i, sbrk(0));

	save_proc_files(3);
#if 0
	kill(getpid(), SIGSEGV);

	printf("Returned from sigsegv\n");
#endif
}

