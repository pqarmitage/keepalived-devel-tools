#include <stdio.h>
#include <signal.h>

int main(int argc, char **argv)
{
	sigset_t fred;
	int i;

	for (i = 1; i <= 64; i++) {
		sigemptyset(&fred);
		sigaddset(&fred, i);
		printf("%2.2d: 0x%16.16lx\n", i, fred.__val[0]);
	}
}
