/* To change cmdline, and environment, see
 *
 * http://stupefydeveloper.blogspot.com/2008/10/linux-change-process-name.html (bad)
 * https://github.com/systemd/systemd/blob/master/src/basic/process-util.c (good)
 */
#include <stdio.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv)
{
	int i = 1;
	unsigned sleep_time = 5;
	unsigned start_sleep = 10;
	unsigned child_sleep = 7;
	unsigned num_forks = 1;
	int prog_index = 0;
	int stat_loc;
	int j, k;

	printf("PID %d\n", getpid());

	for (i = 1; i < argc; i++) {
		if (isdigit(argv[i][0])) {
			if (i == 1)
				sleep_time = atoi(argv[1]);
			else if (i == 2)
				start_sleep = atoi(argv[2]);
			else if (i == 3)
				child_sleep = atoi(argv[3]);
			else if (i == 4)
				num_forks = atoi(argv[4]);
			if (i <= 4)
				continue;
		}

		prog_index = i;
		break;
	}

	if (!prog_index) {
		fprintf(stderr, "Need process name\n");
		exit(1);
	}

	sleep(start_sleep);

	i = prog_index;
	while (1) {

		if (prctl(PR_SET_NAME, argv[i])) {
			printf("prctl error %d - %m\n", errno);
			exit(errno);
		}
		printf("%s\n", argv[i]);

		if (i == argc - 1) {
			for (j = 1; j <= child_sleep; j++) {
				for (k = 0; k < num_forks; k++) {
					if (!fork()) {
						sleep(j);
						printf("Exiting %d\n", j);
						exit(0);
					}
				}
			}
			printf("Done %d forks\n", child_sleep * num_forks);
		}

		for (j = 0; j < sleep_time; j++) {
			sleep(1);
			while (waitpid(0, &stat_loc, WNOHANG) > 0);
		}

		if (++i >= argc)
			i = prog_index;
	}
}
