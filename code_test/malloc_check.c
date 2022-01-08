#include <malloc.h>
#include <memory.h>
#include <stdio.h>
#include <mcheck.h>

static void
memstate(void)
{
	fprintf(stderr, "\n\nmalloc_info\n===========\n\n");
	malloc_info(0, stderr);


	fprintf(stderr, "\n\nmallinfo\n===========\n\n");

	struct mallinfo mi;
	mi = mallinfo();
	fprintf(stderr, "Total non-mmapped bytes (arena):       %d\n", mi.arena);
	fprintf(stderr, "# of free chunks (ordblks):            %d\n", mi.ordblks);
	fprintf(stderr, "# of free fastbin blocks (smblks):     %d\n", mi.smblks);
	fprintf(stderr, "# of mapped regions (hblks):           %d\n", mi.hblks);
	fprintf(stderr, "Bytes in mapped regions (hblkhd):      %d\n", mi.hblkhd);
	fprintf(stderr, "Max. total allocated space (usmblks):  %d\n", mi.usmblks);
	fprintf(stderr, "Free bytes held in fastbins (fsmblks): %d\n", mi.fsmblks);
	fprintf(stderr, "Total allocated space (uordblks):      %d\n", mi.uordblks);
	fprintf(stderr, "Total free space (fordblks):           %d\n", mi.fordblks);
	fprintf(stderr, "Topmost releasable block (keepcost):   %d\n", mi.keepcost);
	fprintf(stderr, "\n\n");

	fprintf(stderr, "\n\nmalloc_stats\n===========\n\n");
	malloc_stats();
}

int main(int argc, char **argv)
{
	void *mem[5];
	int i;

	mtrace();

	fprintf(stderr, "\n\nStart\n\n");
	memstate();

	char *fred = malloc(23);

	fprintf(stderr, "\n\nmalloc 23\n\n");
	memstate();

	free(fred);
	fred = NULL;
	fprintf(stderr, "\n\nfree 23\n\n");
	memstate();

	mem[0] = malloc(1024);
	mem[1] = malloc(24);
	mem[2] = malloc(124);
	mem[3] = malloc(1);
	mem[4] = malloc(10240);

	fprintf(stderr, "\n\nmalloc 1024 24 124 1 10240\n\n");
	memstate();

	if (fred)
		free(fred);
	for (i = 0; i < 5; i++)
		free(mem[i]);

	fprintf(stderr, "\n\nall freed\n\n");
	memstate();
}
