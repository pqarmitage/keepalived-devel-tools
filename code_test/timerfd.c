#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/timerfd.h>
#include <inttypes.h>
#include <unistd.h>

/* Two consecutive calls to clock_gettime give approx 3.5usec difference
 * A wait of 1 nsec gives about 24 - 29 usec difference
 */ 
int main(int argc, char **argv)
{
	struct itimerspec its = { .it_value.tv_nsec = 1 };
	int timer_fd;
	struct timespec before, after, res;
	struct timespec between = {};
	unsigned long wait_time = 1;
	uint64_t exp_count;

	clock_getres(CLOCK_MONOTONIC, &res);

	if (argc >= 2) {
		wait_time = strtoul(argv[1], NULL, 10);
		its.it_value.tv_sec = wait_time / 1000000000UL;
		its.it_value.tv_nsec = wait_time % 1000000000UL;
	}

	timer_fd = timerfd_create(CLOCK_MONOTONIC, 0);

	clock_gettime(CLOCK_MONOTONIC, &before);
	if (wait_time) {
		timerfd_settime(timer_fd, 0,  &its, NULL);
		clock_gettime(CLOCK_MONOTONIC, &between);
		read(timer_fd, &exp_count, sizeof exp_count);
	}
	clock_gettime(CLOCK_MONOTONIC, &after);

	wait_time = (after.tv_sec - between.tv_sec) * 1000000000UL + after.tv_nsec - between.tv_nsec;
	printf("%lu.%9.9ld -> %lu.%9.9ld -> %lu.%9.9ld ->%lu.%9.9ld (delay %lu). Resolution %ld, count %lu\n", before.tv_sec, before.tv_nsec, its.it_value.tv_sec, its.it_value.tv_nsec, between.tv_sec, between.tv_nsec, after.tv_sec, after.tv_nsec, wait_time, res.tv_nsec, exp_count);
}
