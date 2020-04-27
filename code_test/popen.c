#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv)
{
	FILE *fp = popen(argv[1], "r");
	char buf[40];
	size_t len;

	if (!fp) {
		fprintf(stderr, "open failed - errno %d - %m\n", errno);
		exit(1);
	}

	if (!fgets(buf, sizeof buf, fp)) {
		fprintf(stderr, "fgets failed - errno %d - %m\n", errno);
		exit(1);
	}

	pclose(fp);

	if ((len = strlen(buf)) && buf[len-1] == '\n')
		buf[--len] = '\0';

	printf("%s returned: %s\n", argv[1], buf);

	if (len > 10 && buf[len-1] == ')') {
		if (!strncmp(buf + len - 1 - 6, "legacy", 6))
			printf("Using iptables legacy\n");
		else if (!strncmp(buf + len - 1 - 9, "nf_tables", 9))
			printf("Using iptables nf_tables\n");
	}
}
