#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

/* Compile with -S --fverbose-asm -o eagain_optimise.s to see assembler code.
 *
 * With gcc -O0 the (check errno == EAGAIN || errno == EWOULDBLOCK) is
 * not optimised, but if check_EAGAIN is used, it is optimised even with
 * -O0.
 *
 * -O2 optimises both, and if check_EINTR is defined as false, any relevant
 *  tests are optimised out, and loops depending on it are not compiled
 *  as loops. */

#if defined FULL
#define check_EAGAIN(xx)	((xx) == EAGAIN || (xx) == EWOULDBLOCK)
#define check_EINTR(xx)		((xx) == EINTR)
#else
#define check_EAGAIN(xx)	((xx) == EAGAIN)
#define check_EINTR(xx)		(false)
#endif

static void __attribute((noinline))
doit_long(void)
{
	if (errno == EAGAIN || errno == EWOULDBLOCK)
		printf("again, again I say\n");
	if (errno == EINTR)
		printf("Someone interferred with us\n");
}

static void __attribute((noinline))
doit_short(void)
{
	if (check_EAGAIN(errno) || check_EINTR(errno))
		printf("Got one or the other\n");
}

static void __attribute((noinline))
doit_eintr(void)
{
	if (check_EINTR(errno))
		printf("Got EINTR\n");
}

static int __attribute((noinline))
pretend_slow_call(void)
{
	errno = rand();
	return -(rand() & 1);
}

static void __attribute__((noinline))
loop_eintr(void)
{
	while (pretend_slow_call() == -1 && check_EINTR(errno));
}

int main(int argc, char **argv)
{
	doit_long();
	doit_short();
	doit_eintr();

	loop_eintr();

	return 0;
}
