#include <stdio.h>

char buf[64];

int main(int argc, char **argv)
{
	int i = 0;

	while (fgets(buf, 64, stdin))
		printf("\n\n\n\nLine %d:\n@%s@\n", ++i, buf);

	printf("There were %d lines\n", i);
}
