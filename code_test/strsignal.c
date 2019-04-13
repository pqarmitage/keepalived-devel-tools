#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	printf("%s\n", strsignal(atoi(argv[1])));
}
