#include <linux/ip_vs.h>
#include <stdio.h>
#include <stdlib.h>

#if defined ARRAY || defined ARRAY_MIN
#ifdef ARRAY
#define strval(x) [x] = #x
#else
#define min_val  IP_VS_BASE_CTL
#define strval(x) [x-min_val] = #x
#endif
#endif

#ifdef IFS
const char * __attribute__((pure))
get_nl_msg_type(unsigned type)
{
//	if (type == IP_VS_SO_GET_DESTS)
//		return "IP_VS_SO_GET_DESTS";
	if (type == IP_VS_SO_GET_INFO)
		return "IP_VS_SO_GET_INFO";
//	if (type == IP_VS_SO_GET_SERVICE)
//		return "IP_VS_SO_GET_SERVICE";
	if (type == IP_VS_SO_SET_ADD)
		return "IP_VS_SO_SET_ADD";
	if (type == IP_VS_SO_SET_ADDDEST)
		return "IP_VS_SO_SET_ADDDEST";
	if (type == IP_VS_SO_SET_DEL)
		return "IP_VS_SO_SET_DEL";
	if (type == IP_VS_SO_SET_DELDEST)
		return "IP_VS_SO_SET_DELDEST";
	if (type == IP_VS_SO_SET_EDIT)
		return "IP_VS_SO_SET_EDIT";
	if (type == IP_VS_SO_SET_EDITDEST)
		return "IP_VS_SO_SET_EDITDEST";
	if (type == IP_VS_SO_SET_FLUSH)
		return "IP_VS_SO_SET_FLUSH";
	if (type == IP_VS_SO_SET_STARTDAEMON)
		return "IP_VS_SO_SET_STARTDAEMON";
	if (type == IP_VS_SO_SET_STOPDAEMON)
		return "IP_VS_SO_SET_STOPDAEMON";
	if (type == IP_VS_SO_SET_TIMEOUT)
		return "IP_VS_SO_SET_TIMEOUT";
	if (type == IP_VS_SO_SET_ZERO)
		return "IP_VS_SO_SET_ZERO";

	return "(unknown)";
}
#elif defined ARRAY || defined ARRAY_MIN
const char * __attribute__((pure))
get_nl_msg_type(unsigned type)
{
	static const char *vals[] = {
//		strval(IP_VS_SO_GET_DESTS),
		strval(IP_VS_SO_GET_INFO),
//		strval(IP_VS_SO_GET_SERVICE),
		strval(IP_VS_SO_SET_ADD),
		strval(IP_VS_SO_SET_ADDDEST),
		strval(IP_VS_SO_SET_DEL),
		strval(IP_VS_SO_SET_DELDEST),
		strval(IP_VS_SO_SET_EDIT),
		strval(IP_VS_SO_SET_EDITDEST),
		strval(IP_VS_SO_SET_FLUSH),
		strval(IP_VS_SO_SET_STARTDAEMON),
		strval(IP_VS_SO_SET_STOPDAEMON),
		strval(IP_VS_SO_SET_TIMEOUT),
		strval(IP_VS_SO_SET_ZERO),
	};

//printf("Using %u, got %p\n", type, vals[type]); fflush(stdout);
#ifdef ARRAY
	if (type < sizeof(vals) / sizeof(*vals) && vals[type])
#else
	if (type >= min_val && type - min_val < sizeof(vals) / sizeof(*vals) && vals[type - min_val])
#endif
		return vals[type];

	return "(unknown)";
}
#elif defined SWITCH_DEF
#define strval(x)  case(x): return #x; break

const char * __attribute__((pure))
get_nl_msg_type(unsigned type)
{
	switch (type) {
//		strval(IP_VS_SO_GET_DESTS);
		strval(IP_VS_SO_GET_INFO);
//		strval(IP_VS_SO_GET_SERVICE);
		strval(IP_VS_SO_SET_ADD);
		strval(IP_VS_SO_SET_ADDDEST);
		strval(IP_VS_SO_SET_DEL);
		strval(IP_VS_SO_SET_DELDEST);
		strval(IP_VS_SO_SET_EDIT);
		strval(IP_VS_SO_SET_EDITDEST);
		strval(IP_VS_SO_SET_FLUSH);
		strval(IP_VS_SO_SET_STARTDAEMON);
		strval(IP_VS_SO_SET_STOPDAEMON);
		strval(IP_VS_SO_SET_TIMEOUT);
		strval(IP_VS_SO_SET_ZERO);
	};

	return "(unknown)";
}
#else
const char * __attribute__((pure))
get_nl_msg_type(unsigned type)
{
	switch (type) {
//	case IP_VS_SO_GET_DESTS:
//		return "IP_VS_SO_GET_DESTS";
//		break;
	case IP_VS_SO_GET_INFO:
		return "IP_VS_SO_GET_INFO";
		break;
//	case IP_VS_SO_GET_SERVICE:
//		return "IP_VS_SO_GET_SERVICE";
//		break;
	case IP_VS_SO_SET_ADD:
		return "IP_VS_SO_SET_ADD";
		break;
	case IP_VS_SO_SET_ADDDEST:
		return "IP_VS_SO_SET_ADDDEST";
		break;
	case IP_VS_SO_SET_DEL:
		return "IP_VS_SO_SET_DEL";
		break;
	case IP_VS_SO_SET_DELDEST:
		return "IP_VS_SO_SET_DELDEST";
		break;
	case IP_VS_SO_SET_EDIT:
		return "IP_VS_SO_SET_EDIT";
		break;
	case IP_VS_SO_SET_EDITDEST:
		return "IP_VS_SO_SET_EDITDEST";
		break;
	case IP_VS_SO_SET_FLUSH:
		return "IP_VS_SO_SET_FLUSH";
		break;
	case IP_VS_SO_SET_STARTDAEMON:
		return "IP_VS_SO_SET_STARTDAEMON";
		break;
	case IP_VS_SO_SET_STOPDAEMON:
		return "IP_VS_SO_SET_STOPDAEMON";
		break;
	case IP_VS_SO_SET_TIMEOUT:
		return "IP_VS_SO_SET_TIMEOUT";
		break;
	case IP_VS_SO_SET_ZERO:
		return "IP_VS_SO_SET_ZERO";
		break;
	}

	return "(unknown)";
}
#endif

int main(int argc, char **argv)
{
	unsigned val = atoi(argv[1]);
	const char *p = get_nl_msg_type(val);

	printf("%u is %s\n", val, p);
}
