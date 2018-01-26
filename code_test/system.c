#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	int res = system(argv[1]);

	printf("`%s` - ", argv[1]);
	if (res == -1)
		printf("could not create process or get exit code\n");
	else if (WIFEXITED(res) && WEXITSTATUS(res) == 127)
		printf("could not find file\n");
	else if (WIFEXITED(res) && WEXITSTATUS(res) == 126)
		printf("no permission for file\n");
	else if (WIFEXITED(res))
		printf("exited with exit code %d\n", WEXITSTATUS(res));
	else if (WIFSIGNALED(res))
		printf("terminated by signal %d\n", WTERMSIG(res));
	else if (WIFSTOPPED(res))
		printf("stopped by signal %d\n", WSTOPSIG(res));
	else if (WIFCONTINUED(res))
		printf("continued\n");
	else
		printf("returned 0x%x\n", res);
}
