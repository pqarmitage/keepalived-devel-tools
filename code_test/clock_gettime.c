#include <stdio.h>
#include <time.h>
#include <errno.h>

#define ten2nine	1000000000LU

static void
check_timer(int type, const char *type_name)
{
	struct timespec start_time, cur_time, res_time;
	int i;
	unsigned long diff;

	if (clock_gettime(type, &start_time)) {
		printf("clock_gettime type %s failed - errno %d - %m\n", type_name, errno);
		return;
	}

	for (i = 0; i < 50000; i++)
		clock_gettime(type, &cur_time);

	diff = (cur_time.tv_sec - start_time.tv_sec) * ten2nine + cur_time.tv_nsec - start_time.tv_nsec;
	clock_getres(type, &res_time);
	printf("%s - duration %lu.%9.9lu, resolution %luns\n", type_name, diff / ten2nine, diff % ten2nine, res_time.tv_sec * ten2nine + res_time.tv_nsec);
}

int main(int argc, char **argv)
{
	check_timer(CLOCK_REALTIME, "CLOCK_REALTIME");
	check_timer(CLOCK_REALTIME_COARSE, "CLOCK_REALTIME_COARSE");
	check_timer(CLOCK_MONOTONIC, "CLOCK_MONOTONIC");
	check_timer(CLOCK_MONOTONIC_COARSE, "CLOCK_MONOTONIC_COARSE");
	check_timer(CLOCK_MONOTONIC_RAW, "CLOCK_MONOTONIC_RAW");
}
