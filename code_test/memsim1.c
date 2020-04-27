#define _GNU_SOURCE

#include <stdio.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

/*
 * With 6k pages:
 *   RLIMIT_DATA	6132 iterations
 *   RLIMIT_AS		5597 iterations
 *
 */

#define STATIC

extern char etext, edata, end;

STATIC pid_t pid;
STATIC const char const *extn[] = { "stat", "status", "statm", "maps", "smaps", "smaps_rollup" };
STATIC char *extn1[] = { "stat1", "status1", "statm1", "maps1", "smaps1", "smaps_rollup1" };
STATIC const char * const extn2[] = { "stat2", "status2", "statm2", "maps2", "smaps2", "smaps_rollup2" };
const int ci = 23;
int vi = 54;
char extn3[] = "abcd";
char *extn4 = "abcd";
char fred[5000];

STATIC void
save_proc_files(int v)
{
	int i;
	char cmd[64];

	sprintf(cmd, "/tmp/%d.%d", pid, v);
	mkdir(cmd, 0755);

	for (i = 0; i < sizeof extn / sizeof extn[0]; i++) {
		sprintf(cmd, "cp -p /proc/%d/%s /tmp/%d.%d", pid, extn[i], pid, v);
		system(cmd);
	}
}

STATIC void
print_rlim(void *orig_brk)
{
	struct rlimit rlim;

	printf("Memory split: etext %p, edata %p, end %p, break %p, orig_brk %p, stack_cur %p\n", &etext, &edata, &end, sbrk(0), orig_brk, &rlim + sizeof(rlim));
	getrlimit(RLIMIT_AS, &rlim);
	printf("RLIMIT_AS soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
	getrlimit(RLIMIT_DATA, &rlim);
	printf("RLIMIT_DATA soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
	getrlimit(RLIMIT_STACK, &rlim);
	printf("RLIMIT_STACK soft: %p hard: %p\n", rlim.rlim_cur, rlim.rlim_max);
}

int main(int argc, char **argv)
{
	int i;
	void *orig_brk = sbrk(0);
	struct rlimit rlim;
	void *start_brk;
	int rlimit_type = RLIMIT_DATA;
	size_t page_size;

	pid = getpid();

	save_proc_files(0);

	page_size = sysconf(_SC_PAGESIZE);

	print_rlim(orig_brk);

	if (argc >= 2 && !strcmp(argv[1], "as"))
		rlimit_type == RLIMIT_AS;

	if (getrlimit(rlimit_type, &rlim))
		printf("getrlimit failed - %d (%m)\n", errno);
	rlim.rlim_cur = 6 * 1024 * page_size;
	if (setrlimit(rlimit_type, &rlim))
		printf("setrlimit failed - %d (%m)\n", errno);

	start_brk = sbrk(0);
	for (i = 0; i < 1024 * 1024; i++) {
		if (sbrk(page_size) == (void *)-1)
			break;

//		printf("%d  alloc success\n", i);
	}

	printf("sbrk failed for %d after %d iterations, brk now %p from %p\n", pid, i, sbrk(0), orig_brk);

	rlim.rlim_cur += 16 * page_size;
	if (setrlimit(rlimit_type, &rlim))
		printf("setrlimit failed - %d (%m)\n", errno);

	save_proc_files(3);

	printf("&pid %p, &extn %p, extn[0] %p ci %p &vi %p\n", pid, &extn[0], extn[0], &ci, &vi);
	printf("&extn1 %p extn1[0] %p\n", &extn1[0], extn1[0]);
	printf("&extn2 %p extn2[0] %p\n", &extn2[0], extn2[0]);
	printf("extn3 %p extn3[0] %d\n", &extn3[0], extn3[0]);
	printf("extn4 %p extn4[0] %d\n", &extn4[0], extn4[0]);
	printf("extn[0][0] %d extn1[0][0] %d extn2[0][0] %d extn3[0] %d extn4[0] %d\n", extn[0][0], extn1[0][0], extn2[0][0], extn3[0], extn4[0]);

	printf("%s %s\n", extn1[1], extn2[2]);
//	printf("writing 0\n"); fflush(stdout);
//	extn[0][0] = '5';
  //	printf("writing 1\n"); fflush(stdout);
  //	extn1[0][0] = '5';
//	printf("writing 2\n"); fflush(stdout);
//	extn2[0][0] = '5';
	printf("writing 3\n"); fflush(stdout);
	extn3[0] = '5';
//	printf("writing 4\n"); fflush(stdout);
//	extn4[0] = '5';
	printf("don writing\n"); fflush(stdout);
#if 0
	kill(getpid(), SIGSEGV);

	printf("Returned from sigsegv\n");
#endif
}
 
