#define _XOPEN_SOURCE
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>

static char buf[128];

int main(int argc, char **argv)
{
	time_t now = time(NULL);
	time_t t;
	struct tm tm, tm1;
	char *p, *ts;
	size_t len;
	char *end;

	len = strlen(argv[1]);
	if (len != 8 && len != 17 && len != 19) {
		fprintf(stderr, "Unsupported length %zu\n", len);
		exit(1);
	}
	
	localtime_r(&now, &tm);

	end = strptime(argv[1], len == 8 ? "%H:%M:%S" : len == 17 ? "%y-%m-%d %H:%M:%S" : "%Y-%m-%d %H:%M:%S", &tm);
	if (!end || *end) {
		printf("Date/time invalid\n");
		exit(1);
	}

	printf("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst ? " DST" : "");

	tm.tm_isdst = -1;
	strftime(buf, sizeof(buf), "%c", &tm);
	printf("strf - %s\n", buf);

	tm1 = tm;
	t = mktime(&tm1);
	if (tm.tm_year != tm1.tm_year ||
	    tm.tm_mon != tm1.tm_mon ||
	    tm.tm_mday != tm1.tm_mday ||
	    tm.tm_hour != tm1.tm_hour ||
	    tm.tm_min != tm1.tm_min ||
	    tm.tm_sec != tm1.tm_sec) {
		printf("*** mismatch ***\n");
	}
	if (t <= now) {
		if (len == 8) {
			tm1.tm_mday++;
			t = mktime(&tm1);
			printf("Going to tomorrow\n");
		} else {
			printf("Date/time is in the past\n");
		}
	}

	localtime_r(&t, &tm1);
	strftime(buf, sizeof(buf), "%c", &tm1);
	printf("mktime - %s%s - %s\n\n", buf, tm1.tm_isdst ? " DST" : "", tzname);
}
