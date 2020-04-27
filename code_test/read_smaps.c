#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	char buf[128];
	bool start_next = true;
	int size;
	int as_size = 0;

	while (fgets(buf, sizeof buf, stdin)) {
		if (start_next) {
			printf("Start: %s", buf);
			start_next = false;
			continue;
		}
		if (!strncmp(buf, "Size:", strlen("Size:"))) {
			printf("Size: %s", strchr(buf, ' '), ' ');
			size = atoi(strchr(buf, ' ')); //, NULL, 10);
			printf("size is %d\n", size);
			continue;
		}
		if (!strncmp(buf, "VmFlags:", strlen("VmFlags:"))) {
			printf("VmFlags: %s", strchr(buf, ' '), ' ');
			start_next = true;
			if (strstr(buf, " ac")) {
				as_size += size;
				printf("- %d\n", size);
			}
			continue;
		}
	}

	printf("Total as %d, pages %d\n", as_size, as_size / 4);
}
