#include <unistd.h>
#define LIBIPSET_NFPROTO_H
#define LIBIPSET_NF_INET_ADDR_H
#include <libipset/session.h>
#include <libipset/types.h>
#include <netinet/in.h>
#include <linux/types.h>        /* For __beXX types in userland */
#include <linux/netfilter.h>    /* For nf_inet_addr */
#include <stdint.h>
#include <stdio.h>

static bool
ipset_create(struct ipset_session* session, const char *setname, const char *typename, uint8_t family)
{
	const struct ipset_type *type;
	int r;

	ipset_session_data_set(session, IPSET_SETNAME, setname);

	ipset_session_data_set(session, IPSET_OPT_TYPENAME, typename);

	type = ipset_type_get(session, IPSET_CMD_CREATE);
	if (type == NULL)
		return false;

	ipset_session_data_set(session, IPSET_OPT_TYPE, type);
	ipset_session_data_set(session, IPSET_OPT_FAMILY, &family);

	r = ipset_cmd(session, IPSET_CMD_CREATE, 0);
	return r == 0;
}

bool
has_ipset_setname(struct ipset_session* session, const char *setname)
{
	ipset_session_data_set(session, IPSET_SETNAME, setname);

	return ipset_cmd(session, IPSET_CMD_HEADER, 0) == 0;
}

static int create_sets(const char* addr4, const char* addr6, const char* addr_if6)
{
	struct ipset_session *session;

	session = ipset_session_init(printf);
	if (!session) {
		printf("Cannot initialize ipset session.");
		return false;
	}

	/* don't worry if sets already exist */
	ipset_envopt_parse(session, IPSET_ENV_EXIST, NULL);

if (!has_ipset_setname(session, addr4))
	ipset_create(session, addr4, "hash:ip", NFPROTO_IPV4);
else
	printf ("Set %s exists", addr4);
if (!has_ipset_setname(session, addr6))
	ipset_create(session, addr6, "hash:ip", NFPROTO_IPV6);
else
	printf ("Set %s exists", addr6);
if (!has_ipset_setname(session, addr_if6))
#ifdef HAVE_IPSET_ATTR_IFACE
	/* hash:net,iface was introduced in Linux 3.1 */
	ipset_create(session, addr_if6, "hash:net,iface", NFPROTO_IPV6);
#else
	ipset_create(session, addr_if6, "hash:ip", NFPROTO_IPV6);
#endif
else
	printf ("Set %s exists", addr_if6);

	ipset_session_fini(session);

	return true;
}

bool ipset_init(void)
{
	ipset_load_types();

/*
	if (!load_mod_xt_set()) {
		printf("Unable to load xt_set module");
		return false;
	}
*/

	return true;
}

struct ipset_session* ipset_session_start(void)
{
	return ipset_session_init(NULL);
}

void ipset_session_end(struct ipset_session* session)
{
	ipset_session_fini(session);
}

int main(int argc, char **argv)
{
	ipset_init();
	create_sets("teepalived", "teepalived6", "teepalived_if6");
}

