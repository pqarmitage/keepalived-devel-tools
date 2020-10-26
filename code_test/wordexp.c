#include <stdio.h>
#include <stdlib.h>
#include <wordexp.h>
#include <errno.h>

int
main(int argc, char **argv)
{
	wordexp_t p;
	char **w;
	int i;
	char *str = "[a-c]*.c";
	int ret;

	if (argc > 1)
		str = argv[1];

	ret = wordexp(str, &p, 0);
	if (ret) {
		switch(ret) {
		case WRDE_BADCHAR:
			printf("%s\n", "WRDE_BADCHAR");
			break;
		case WRDE_BADVAL:
			printf("%s\n", "WRDE_BADVAL");
			break;
		case WRDE_CMDSUB:
			printf("%s\n", "WRDE_CMDSUB");
			break;
		case WRDE_NOSPACE:
			printf("%s\n", "WRDE_NOSPACE");
			break;
		case WRDE_SYNTAX:
			printf("%s\n", "WRDE_SYNTAX");
			break;
		}
		exit(1);
	}
	printf("wordexp returned %d (errno %d), count %d\n", ret, errno, p.we_wordc);

	w = p.we_wordv;
	for (i = 0; i < p.we_wordc; i++)
		printf("%s\n", w[i]);
	wordfree(&p);
	exit(EXIT_SUCCESS);
}

