#include <errno.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <linux/in.h>
//#include <linux/in6.h>

enum connect_result {
	connect_error,
	connect_in_progress,
	connect_timeout,
	connect_success
};

/* IP string to sockaddr_storage */
int
inet_stosockaddr(char *ip, const char *port, struct sockaddr_storage *addr)
{
	void *addr_ip;
	unsigned port_num = 25;
	int res;

	addr->ss_family = (strchr(ip, ':')) ? AF_INET6 : AF_INET;

	if (port)
		port_num = atoi(port);

	if (addr->ss_family == AF_INET6) {
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *) addr;
		addr6->sin6_port = htons(port_num);
		addr_ip = &addr6->sin6_addr;
	} else {
		struct sockaddr_in *addr4 = (struct sockaddr_in *) addr;
		addr4->sin_port = htons(port_num);
		addr_ip = &addr4->sin_addr;
	}

	res = inet_pton(addr->ss_family, ip, addr_ip);

	if (!res) {
		addr->ss_family = AF_UNSPEC;
		return -1;
	}

	return 0;
}

static
enum connect_result
socket_bind_connect(char *addr_str, char *port, char *iface)
{
	int opt;
	socklen_t optlen;
	struct linger li;
	int ret;
	struct sockaddr_storage addr;
	int fd;
	union {
		struct sockaddr_in *in;
		struct sockaddr_in6 *in6;
	} addr_p = { .in = (struct sockaddr_in *)&addr };

	inet_stosockaddr(addr_str, port, &addr);

	/* Create the socket, failing here should be an oddity */
	if ((fd = socket(addr.ss_family, SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK, IPPROTO_TCP)) == -1) {
		fprintf(stderr, "SMTP_CHECK connection failed to create socket.\n");
		return connect_error;
	}

	/* free the tcp port after closing the socket descriptor */
	li.l_onoff = 1;
	li.l_linger = 5;
	setsockopt(fd, SOL_SOCKET, SO_LINGER, (char *) &li, sizeof (struct linger));

#ifdef _WITH_SO_MARK_
	if (co->fwmark) {
		if (setsockopt (fd, SOL_SOCKET, SO_MARK, &co->fwmark, sizeof (co->fwmark)) < 0) {
			fprintf(stderr, "Error setting fwmark %d to socket: %s\n", co->fwmark, strerror(errno));
			return connect_error;
		}
	}
#endif

	if (iface) {
		if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, iface, (unsigned)strlen(iface) + 1) < 0) {
			fprintf(stderr, "Checker can't bind to device %s: %s\n", iface, strerror(errno));
			return connect_error;
		}
	}

	/* Bind socket */
/*
	if (((struct sockaddr *) bind_addr)->sa_family != AF_UNSPEC) {
		addrlen = sizeof(*bind_addr);
		if (bind(fd, (struct sockaddr *) bind_addr, addrlen) != 0) {
			fprintf(stderr, "Checker bind failed: %s\n", strerror(errno));
			return connect_error;
		}
	}
*/

	/* Set remote IP and connect */
	ret = connect(fd, (struct sockaddr *)&addr, addr.ss_family == AF_INET ? sizeof(struct sockaddr_in) : sizeof(struct sockaddr_in6));

	/* Immediate success */
	if (ret == 0)
		return connect_success;

	/* If connect is in progress then return 1 else it's real error. */
	if (ret < 0) {
		fprintf(stderr, "connect returned %d - %m\n", errno);
		if (errno != EINPROGRESS)
			return connect_error;
	}

	return connect_in_progress;
}

int main(int argc, char **argv)
{
	char *addr = "1.2.3.4";
	char *port = "25";
	char *iface = NULL;

	if (argc > 1 && strcmp(argv[1], "."))
		addr = argv[1];
	if (argc > 2 && strcmp(argv[2], "."))
		port = argv[2];
	if (argc > 3 && strcmp(argv[3], "."))
		iface = argv[3];

	printf("socket_bind_connect() returned %d\n", socket_bind_connect(addr, port, iface));
}
