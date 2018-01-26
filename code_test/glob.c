#include <glob.h>
#include <stdio.h>
#include <errno.h>

int main(int argc, char **argv)
{
	glob_t globbuf;
	size_t i;
	int	res;

	globbuf.gl_offs = 0;
	res = glob(argv[1], GLOB_MARK | GLOB_BRACE, NULL, &globbuf);

	errno = 0;
	if (res) {
		printf("Unable to find config file(s) '%s' - res %d (%s), errno %d (%m).\n", argv[1], res, res == GLOB_NOSPACE ? "GLOB_NOSPACE" : res == GLOB_ABORTED ? "GLOB_ABORTED" : res == GLOB_NOMATCH ? "GLOB_NOMATCH" : "?", errno);
	}
	else if (errno)
		printf("Unable to find config file(s) '%s' - errno %d (%m).\n", argv[1], errno);

	printf("glob returned %d matches\n", globbuf.gl_pathc);
	for(i = 0; i < globbuf.gl_pathc; i++){
		printf("Opening file '%s'.\n", globbuf.gl_pathv[i]);
	}

	globfree(&globbuf);
}
