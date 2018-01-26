#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

char file_name[] = "/tmp/test_system.sh";

void write_script(void)
{
	FILE *fp = fopen(file_name, "w");
	char *script =
"#!/bin/bash\n"
"\n"
"[[ $1 != NO ]] && kill -${1:-TERM} $$ 2>/dev/null\n"
"\n"
"exit ${2:-12}\n"
;

	fwrite(script, strlen(script), 1, fp);
	fclose(fp);
}

static void
run_command(const char *cmd, const char *feature)
{
	int ret;
	char command_buf[sizeof(file_name) + 30];	// Give ourselves lots of space

	sprintf(command_buf, "%s %s 2>/dev/null", file_name, cmd);
	ret = system(command_buf);
	printf("system() on %s returned exited with ", feature);
	if (WIFSIGNALED(ret))
		printf("signal %d", WTERMSIG(ret));
	else if (WIFEXITED(ret))
		printf("exit code %d", WEXITSTATUS(ret));
	else
		printf("return value 0x%x\n", ret);
	printf("\n");
}

int main(int argc, char **argv)
{
	struct stat buf;

	if (stat(file_name, &buf) == 0) {
		fprintf(stderr, "%s exists, not overwriting\n", file_name);
		exit(1);
	}

	run_command("11", "on non existant script");

	write_script();
	chmod(file_name, S_IWUSR);

	run_command("12", "non readable script");

	chmod(file_name, S_IRUSR | S_IWUSR);

	run_command("13", "non executable script");

	chmod(file_name, S_IRUSR | S_IWUSR | S_IXUSR);

	run_command("TERM", "script terminating with SIGTERM");

	run_command("NO 10", "script exiting with exit status 10");

	run_command("65 15", "script exiting after invalid signal and exit status 15");

	unlink(file_name);
}
