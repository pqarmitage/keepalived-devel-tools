#include <stdio.h>
#include <time.h>
#include <sys/time.h>
#include <signal.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

struct itimerval end_time;
bool stop = false;

static void signal_handler(int signum)
{
	stop = true;
	getitimer(ITIMER_PROF, &end_time);
	printf("Got signal\n");
}

static void report_times()
{
	struct timeval tv;
	struct timespec ts;

	gettimeofday(&tv, NULL);
	printf("time of day           = %lu.%6.6d\n", tv.tv_sec, tv.tv_usec);

	clock_gettime(CLOCK_REALTIME, &ts);
	printf("Real time             = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);

	clock_gettime(CLOCK_REALTIME_COARSE, &ts);
	printf("Real time coarse      = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);

	clock_gettime(CLOCK_MONOTONIC, &ts);
	printf("Monotonic time        = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);

	clock_gettime(CLOCK_MONOTONIC_COARSE, &ts);
	printf("Monotonic time coarse = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);

	clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
	printf("Process time          = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &ts);
	printf("Thread time           = %lu.%9.9d\n", ts.tv_sec, ts.tv_nsec);
}

int main(int argc, char **argv)
{
//	unsigned loops = 1000;
	struct timeval tv;
	struct itimerval run_time = { .it_value.tv_sec = 1 };
	struct itimerval prof_time = { .it_value.tv_sec = 100 };
	unsigned long loop_count = 0;
	struct sigaction act = { .sa_handler = signal_handler };
	struct timespec ts;

//	if (argc >= 2)
//		loops = strtoul(argv[1], NULL, 10);

	report_times();

	sigaction(SIGALRM, &act, NULL);

	setitimer(ITIMER_REAL, &run_time, NULL);
	setitimer(ITIMER_PROF, &prof_time, NULL);

	if (argc >= 2) {
		while (!stop) {
			gettimeofday(&tv, NULL);
			loop_count++;
		}
	} else {
		while (!stop) {
			clock_gettime(CLOCK_MONOTONIC, &ts);
			loop_count++;
		}
	}

	if (prof_time.it_value.tv_usec < end_time.it_value.tv_usec) {
		prof_time.it_value.tv_sec--;
		prof_time.it_value.tv_usec += 1000000;
	}
	prof_time.it_value.tv_sec -= end_time.it_value.tv_sec;
	prof_time.it_value.tv_usec -= end_time.it_value.tv_usec;

	printf("%lu loops of %s() executed using %lu.%6.6ld seconds\n", loop_count, argc >= 2 ? "gettimeofday" : "clock_gettime", prof_time.it_value.tv_sec, prof_time.it_value.tv_usec);

	report_times();
}
