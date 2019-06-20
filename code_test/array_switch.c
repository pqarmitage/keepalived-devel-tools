#include <linux/rtnetlink.h>
#include <stdio.h>
#include <stdlib.h>

#if defined ARRAY || defined ARRAY_MIN
#ifdef ARRAY
#define strval(x) [x] = #x
#else
#define min_val  RTM_NEWLINK
#define strval(x) [x-min_val] = #x
#endif

const char *
get_nl_msg_type(unsigned type)
{
	static const char *vals[] = {
		strval(RTM_NEWLINK),
		strval(RTM_DELLINK),
		strval(RTM_NEWADDR),
		strval(RTM_DELADDR),
		strval(RTM_NEWROUTE),
		strval(RTM_DELROUTE),
		strval(RTM_NEWRULE),
		strval(RTM_DELRULE),
		strval(RTM_GETLINK),
		strval(RTM_GETADDR),
	};

//printf("Using %u, got %p\n", type, vals[type]); fflush(stdout);
#ifdef ARRAY
	if (type < sizeof(vals) / sizeof(*vals) && vals[type])
#else
	if (type >= min_val && type - min_val < sizeof(vals) / sizeof(*vals) && vals[type - min_val])
#endif
		return vals[type];

	return "";
}
#elif defined SWITCH_DEF
#define strval(x) case x: return #x
const char *
get_nl_msg_type(unsigned type)
{
	switch (type) {
		strval(RTM_NEWLINK);
		strval(RTM_DELLINK);
		strval(RTM_NEWADDR);
		strval(RTM_DELADDR);
		strval(RTM_NEWROUTE);
		strval(RTM_DELROUTE);
		strval(RTM_NEWRULE);
		strval(RTM_DELRULE);
		strval(RTM_GETLINK);
		strval(RTM_GETADDR);
	};

	return "";
}
#else
const char *
get_nl_msg_type(unsigned type)
{
	switch (type) {
	case RTM_NEWLINK:
		return "RTM_NEWLINK";
		break;
	case RTM_DELLINK:
		return "RTM_DELLINK";
		break;
	case RTM_NEWADDR:
		return "RTM_NEWADDR";
		break;
	case RTM_DELADDR:
		return "RTM_DELADDR";
		break;
	case RTM_NEWROUTE:
		return "RTM_NEWROUTE";
		break;
	case RTM_DELROUTE:
		return "RTM_DELROUTE";
		break;
	case RTM_NEWRULE:
		return "RTM_NEWRULE";
		break;
	case RTM_DELRULE:
		return "RTM_DELRULE";
		break;
	case RTM_GETLINK:
		return "RTM_GETLINK";
		break;
	case RTM_GETADDR:
		return "RTM_GETADDR";
		break;
	}

	return "";
}
#endif

int main(int argc, char **argv)
{
	unsigned val = atoi(argv[1]);
printf("val %d\n", val);fflush(stdout);
const char *p = get_nl_msg_type(val);
printf("%u is %s\n", val, p);
}
