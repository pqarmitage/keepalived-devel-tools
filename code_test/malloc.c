#define _GNU_SOURCE

#include <memory.h>
#include <stdio.h>
#include <sys/resource.h>
#include <unistd.h>
#include <dlfcn.h>
#include <stdlib.h>
#include <malloc.h>	// for malloc_info

#define STATIC

/* Get strsignal() declaration from <string.h> */

extern char etext, edata, end; /* The symbols must have some type,
				  or "gcc -Wall" complains */

static void *(*malloc_lib)(size_t);
static void *(*calloc_lib)(size_t, size_t);
static void *(*realloc_lib)(void *, size_t);
static void *(*reallocarray_lib)(void *, size_t, size_t);
static void *(*free_lib)(void *);

void *
malloc(size_t size)
{
	void *ret;
	static int depth;

	if (!depth++)
		printf("malloc %zu\n", size);

	ret = (*malloc_lib)(size);

	depth--;

	return ret;
}

void *
calloc(size_t nmemb, size_t size)
{
	void *ret;
	static int depth;

	if (!depth++)
		printf("calloc %zu:%zu\n", nmemb, size);

	ret = (*calloc_lib)(nmemb, size);

	depth--;

	return ret;
}

void *
realloc(void *ptr, size_t size)
{
	void *ret;
	static unsigned depth;

	if (!depth++)
		printf("realloc %p %zu\n", ptr, size);

	ret = (*realloc_lib)(ptr, size);

	depth--;

	return ret;
}

void *
reallocarray(void *ptr, size_t nmemb, size_t size)
{
	void *ret;
	static unsigned depth;

	if (!depth++)
		printf("reallocarray %p %zu %zu\n", ptr, nmemb, size);

	ret = (*reallocarray_lib)(ptr, nmemb, size);

	depth--;

	return ret;
}

void
free(void *ptr)
{
	static unsigned free_depth;

	if (!free_depth++)
		printf("free %p\n", ptr);

	(*free_lib)(ptr);

	free_depth--;
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

int main(int argc, char **argv)
{
	malloc_lib = dlsym(RTLD_NEXT, "malloc");
	calloc_lib = dlsym(RTLD_NEXT, "calloc");
	realloc_lib = dlsym(RTLD_NEXT, "realloc");
	reallocarray_lib = dlsym(RTLD_NEXT, "reallocarray");
	free_lib = dlsym(RTLD_NEXT, "free");

	print_rlim();

	printf("malloc %p, malloc_lib %p\n", malloc, malloc_lib);

	char *fred = malloc(23);
	char *bert = calloc(4,5);
	char *joe = realloc(fred, 46);
	char *jim = reallocarray(bert, 10, 20);

	/* See fmemopen(3) and open_memstream(3) */
	malloc_info(0, stdout);

	free(joe);
	free(jim);
}
