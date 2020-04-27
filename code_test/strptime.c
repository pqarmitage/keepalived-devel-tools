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

	p = argv[1];
	p += strspn(argv[1], "0123456789 :-");
	if (*p) {
		fprintf(stderr, "Invalid character at %s\n", p);
		exit(1);
	}

	len = p - argv[1];
	if (len != 8 && len != 17 && len != 19) {
		fprintf(stderr, "Unsupported length %zu\n", len);
		exit(1);
	}
	
	if (len > 8) {
		/* Check date */
		p = argv[1];
		if ((p += strspn(argv[1], "0123456789")) - argv[1] != len - 15 ||
		    *p != '-') {
			fprintf(stderr, "Year len not valid\n");
			exit(1);
		}

		if ((p += strspn(++p, "0123456789")) - argv[1] != len - 12 ||
		    *p != '-') {
			fprintf(stderr, "Month len not valid\n");
			exit(1);
		}

		if ((p += strspn(++p, "0123456789")) - argv[1] != len - 9 ||
		    *p != ' ') {
			fprintf(stderr, "Day len not valid\n");
			exit(1);
		}
		p++;
	}
	else
		p = argv[1];

	/* Check time */
	ts = p;
	if ((p += strspn(ts, "0123456789")) - ts != 2 ||
	    *p != ':') {
		fprintf(stderr, "Hour len not valid\n");
		exit(1);
	}

	if ((p += strspn(++p, "0123456789")) - ts != 5 ||
	    *p != ':') {
		fprintf(stderr, "Minute len not valid\n");
		exit(1);
	}

	if ((p += strspn(++p, "0123456789")) - ts != 8 ||
	    *p) {
		fprintf(stderr, "Seconds len not valid\n");
		exit(1);
	}

	localtime_r(&now, &tm);

	end = strptime(argv[1], len == 8 ? "%H:%M:%S" : len == 17 ? "%y-%m-%d %H:%M:%S" : "%Y-%m-%d %H:%M:%S", &tm);
	if (end && *end) {
		printf("Remaining: %s\n", end);
		exit(1);
	}
	else if (!end) {
		printf("NULL returned\n");
		exit(1);
	}

	printf("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d %s\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst ? " DST" : "");

	/*
	strftime(buf, sizeof(buf), "%c", &tm);
	printf("%s\n", buf);
	now = mktime(&tm);
	localtime_r(&now, &tm1);
	strftime(buf, sizeof(buf), "%c", &tm1);
	printf("%s\n\n", buf);
	*/

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
		tm1.tm_mday++;
		t = mktime(&tm1);
		printf("Going to tomorrow\n");
	}

	localtime_r(&t, &tm1);
	strftime(buf, sizeof(buf), "%c", &tm1);
	printf("mktime - %s%s - %s\n\n", buf, tm1.tm_isdst ? " DST" : "", tzname);
}

#if 0
int main(int argc, char **argv)
{
	struct tm tm;
	char *end;

	end = strptime(argv[1], argv[2], &tm);
	if (end && *end)
		printf("Remaining: %s\n", end);
	else if (!end)
		printf("NULL returned\n");

	printf("%4.4d-%2.2d-%2.2d %2.2d:%2.2d:%2.2d %s\n", tm.tm_year, tm.tm_mon, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec, tm.tm_isdst ? " DST" : "");

	strftime(buf, sizeof(buf), "%c", &tm);
	printf("%s\n", buf);
}
#endif
