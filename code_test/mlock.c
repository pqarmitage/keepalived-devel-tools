#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <alloca.h>

#define NUM_PAGES 256

static char buf[512];

static void
get_stack(pid_t pid, void **start, void **end)
{
	FILE *maps;

	sprintf(buf, "grep stack /proc/%d/maps", pid);
	maps = popen(buf, "r");
	fgets(buf, sizeof(buf), maps);
	fclose(maps);
	sscanf(buf, "%p-%p", start, end);
	printf("Read: %s -- start %p, end %p\n", buf, *start, *end);
}

int main(int argc, char **argv)
{
	pid_t pid = getpid();
	int num = 0;
	char *stack;
	int i;
	void *start, *end;

	sprintf(buf, "cp /proc/%d/smaps /tmp/smaps.%d.%d", pid, pid, num++);
	system(buf);

	mlockall(MCL_CURRENT | MCL_FUTURE);

	sprintf(buf, "cp /proc/%d/smaps /tmp/smaps.%d.%d", pid, pid, num++);
	system(buf);

	stack = alloca(4096 * NUM_PAGES);

	printf("pid %p num %p stack %p i %p stack_base %p\n", &pid, &num, &stack, &i, stack);

	sprintf(buf, "cp /proc/%d/smaps /tmp/smaps.%d.%d", pid, pid, num++);
	system(buf);

	stack[0] = 45;

	sprintf(buf, "cp /proc/%d/smaps /tmp/smaps.%d.%d", pid, pid, num++);
	system(buf);

	for (i = 1; i < NUM_PAGES; i++)
		stack[i * 4096] = i;
	printf("Written %p to %p, top %p\n", &stack[0], &stack[(NUM_PAGES - 1) * 4096], &stack[NUM_PAGES * 4096 - 1]);

	stack[NUM_PAGES * 4096 - 1] = 45;

	sprintf(buf, "cp /proc/%d/smaps /tmp/smaps.%d.%d", pid, pid, num++);
	system(buf);

	get_stack(pid, &start, &end);

	printf("Stack is %d pages long\n", (end - start) / 4096);

	mincore(start, end - start, (unsigned char *)buf);
	for (i = 0; i < (end - start) / 4096; i++)
		printf("%c", buf[i] & 1 ? '*' : '-');
	printf("\n");

}
