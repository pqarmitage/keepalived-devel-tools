#define _GNU_SOURCE

#include <stdio.h>
#include <sys/resource.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/mman.h>

/* Memory layout
 *
 * _________________
 *
 *    kernel
 *
 * _________________
 *
 *    argv, environ
 * -----------------
 *
 *    Stack
 * _________________
 *
 *   Unallocated
 *
 * _________________  sbrk(0)
 *
 *      Heap
 * -----------------  &end
 *    Unitialized (RW) data (bss)
 * -----------------  &edata 
 *    Initialized RW data
 * _________________ 
 *
 *    RO data
 * _________________  &etext
 *
 *   Text (exec)
 * _________________
 * 
 *   Unallocated
 * _________________ 0
 *
 */ 

/* With 6k pages:
 *   RLIMIT_DATA	6132 iterations
 *   RLIMIT_AS		5597 iterations
 *
 */

#define STATIC

extern char etext, edata, end;

STATIC pid_t pid;
STATIC const char const *extn[] = { "stat", "status", "statm", "maps", "smaps", "smaps_rollup" };
STATIC void *stack_base, *stack_top;
STATIC size_t page_size;


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

STATIC void *
get_cur_stack_base(void)
{
	int i;
	void *ip = (void *)&i;

	return ip;
}

STATIC void *
get_heap_base(void)
{
	char file[128];
	FILE *fp;
	unsigned long vsize; 
	long rss;
	unsigned long rsslim; 
	void *startcode; 
	void *endcode; 
	void *startstack; 
	void *start_data; 
	void *end_data; 
	void *start_brk; 
	void *arg_start; 
	void *arg_end; 
	void *env_start; 
	void *env_end;
	int ret;

	sprintf(file, "/proc/%d/stat", pid);
	if (!(fp = fopen(file, "r")))
		printf("Open of %s failed\n", file);
	int mpid; char name[17]; char status;
//	ret = fscanf(fp, "%d (%s %c", &mpid, name, &status);
	ret = fscanf(fp, "%*d (%*s %*c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %*lu %*lu %*ld %*ld %*ld %*ld %*ld %*ld %*llu %lu %ld %lu %lu %lu %lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*lu %*d %*d %*u %*u %*llu %*lu %*ld %lu %lu %lu %lu %lu %lu %lu %*d", &vsize, &rss, &rsslim, &startcode, &endcode, &startstack, &start_data, &end_data, &start_brk, &arg_start, &arg_end, &env_start, &env_end);
	fclose(fp);
	printf("fscanf returned %d\n", ret);
//	printf("%d %s %c\n", mpid, name, status);

	printf("vsize %lu rss %ld rsslim %lu startcode %p endcode %p startstack %p start_data %p end_data %p start_brk %p arg_start %p arg_end %p env_start %p env_end %p\n", vsize, rss, rsslim, startcode, endcode, startstack, start_data, end_data, start_brk, arg_start, arg_end, env_start, env_end);
	printf("vsize 0x%lx rss 0x%lx rsslim 0x%lx\n", vsize, rss, rsslim);

	return start_brk;
}

STATIC void
increase_stack(size_t size)
{
	char stack[size];

	stack[0] = 23;
}

STATIC void *
set_stack_bounds(void)
{
	char cmd[128];
	char buf[128];
	FILE *fp;
	size_t incr;
	struct rlimit rlim;

	sprintf(cmd, "grep \"\\[stack\\]$\" /proc/%d/maps | sed -e \"s/ .*//\" -e \"s/-/ /\"", pid);
//	printf("cmd is: %s\n", cmd);
	fp = popen(cmd, "r");
	fgets(buf, sizeof(buf), fp);
	fclose(fp);
	stack_base = (void *)strtoul(buf, NULL, 16);
	stack_top = (void *)strtoul(strchr(buf, ' '), NULL, 16);
	printf("Ret: %.*s: %p - %p len 0x%x, cur_stack_base %p\n", strlen(buf) - 1, buf, stack_base, stack_top, stack_top - stack_base, get_cur_stack_base());

	if (getrlimit(RLIMIT_STACK, &rlim))
		printf("getrlimit stack failed - %d (%m)\n", errno);

	incr = rlim.rlim_cur - (stack_top - get_cur_stack_base());
	printf("Incr = 0x%x\n", incr); fflush(stdout);
	increase_stack(incr - 0x14);	/* 0x13 not OK */

	fp = popen(cmd, "r");
	fgets(buf, sizeof(buf), fp);
	fclose(fp);
	printf("\nAfter increase by 0x%x, stack_base = %p\n", incr, (void *)strtoul(buf, NULL, 16));

//	printf("Ret: %.*s: %p - %p len 0x%x, cur_stack_base %p\n", strlen(buf) - 1, buf, stack_base, stack_top, stack_top - stack_base, get_cur_stack_base());
}

STATIC void
reset_stack(void)
{
	void *cur_stack_top;
	struct rlimit rlim;
	int res;

	cur_stack_top = get_cur_stack_base();
	cur_stack_top -= (size_t)cur_stack_top % page_size;
	getrlimit(RLIMIT_STACK, &rlim);

	printf("munmap(%p, %zu: %p - %p\n", stack_top - rlim.rlim_cur, cur_stack_top - (stack_top - rlim.rlim_cur), stack_top - rlim.rlim_cur, cur_stack_top - 1);
	res = munmap(stack_top - rlim.rlim_cur, cur_stack_top - (stack_top - rlim.rlim_cur));
	printf("munmap returned %d - errno %d(%m)\n", res, errno);
}

int main(int argc, char **argv)
{
	unsigned i;
	void *orig_brk = sbrk(0);
	struct rlimit rlim;
	void *start_brk;
	int rlimit_type = RLIMIT_DATA;
	void *heap_base;
        char *envp_min = (char *)-1;
        char *envp_max = 0;
        char **envp;

	pid = getpid();
	printf("PID is %d\n", pid);

        for (envp = environ, i = 0; *envp; envp++, i++)
        {
                if (*envp < envp_min)
                        envp_min = *envp;
                if (*envp > envp_max)
                        envp_max = *envp + strlen(*envp) + 1;
//              printf("envp[%d] = 0x%p (%s)\n", i, *envp, *envp);
        }


	printf("SIGSTKSZ %lu, MINSIGSTKSZ %lu\n", SIGSTKSZ, MINSIGSTKSZ);
	printf("Top of standard stack for %d is near %10p\n", pid, (void *) &i);
	printf("argv %p, argv[0] %p, &argv[%d] %p\n", argv, argv[0], argc, &argv[argc]);
	printf("envp %p, envp[0] %p, &envp[%d] %p\n", environ, environ[0], i, &environ[i]);
	printf("argv %p - %p, envp %p - %p\n",
		argv[0], argv[argc - 1] + strlen(argv[argc - 1]),
		envp_min, envp_max);

	save_proc_files(0);

	page_size = sysconf(_SC_PAGESIZE);

	heap_base = get_heap_base();

	print_rlim(orig_brk);

	set_stack_bounds();

	save_proc_files(1);

	reset_stack();

	save_proc_files(2);

	set_stack_bounds();

	save_proc_files(3);

	if (argc >= 2 && !strcmp(argv[1], "as"))
		rlimit_type = RLIMIT_AS;

	printf("rlimit type %d\n", rlimit_type);

	if (getrlimit(rlimit_type, &rlim))
		printf("getrlimit failed - %d (%m)\n", errno);
	rlim.rlim_cur = 6 * 1024 * page_size;
	if (setrlimit(rlimit_type, &rlim))
		printf("setrlimit failed - %d (%m)\n", errno);

	start_brk = sbrk(0);
	for (i = 0; i < 6 * 1024; i++) {
		if (sbrk(page_size) == (void *)-1)
			break;

//		printf("%d  alloc success\n", i);
	}

//	printf("sbrk failed for %d after %d iterations, brk now %p from %p\n", pid, i, sbrk(0), orig_brk);
	printf("sbrk failed for %d after %u iterations, brk now %p from %p, at start %p, other data size 0x%lx, pages %u (0x%x)\n", pid, i, sbrk(0), start_brk, orig_brk, (unsigned long)(6U * 1024 * page_size - ((char *)sbrk(0) - (char *)heap_base)), 6U * 1024 - i, 6U * 1024 - i);

	rlim.rlim_cur += 16 * page_size;
	if (setrlimit(rlimit_type, &rlim))
		printf("setrlimit failed - %d (%m)\n", errno);

	save_proc_files(99);

#if 0
	kill(getpid(), SIGSEGV);

	printf("Returned from sigsegv\n");
#endif
}
