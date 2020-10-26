/*
 * For testing, there are various options to try.
 *
 * Use the following IPVS setup:
 *
 *	ipvsadm -A -u 10.0.7.3:53 -s rr
 *	ipvsadm -a -u 10.0.7.3:53 -r 172.21.53.10 -m
 *	ipvsadm -a -u 10.0.7.3:53 -r 172.21.53.11 -m
 *
 *	ipvsadm -A -u 10.0.6.3:53 -s rr -o
 *	ipvsadm -a -u 10.0.6.3:53 -r 172.21.53.10 -m
 *	ipvsadm -a -u 10.0.6.3:53 -r 172.21.53.11 -m
 *
 *	ipvsadm -A -u 10.0.5.3:53 -s rr
 *	ipvsadm -a -u 10.0.5.3:53 -r 172.21.53.10 -g
 *	ipvsadm -a -u 10.0.5.3:53 -r 172.21.53.11 -g
 *
 *	ipvsadm -A -u 10.0.4.3:53 -s rr -o
 *	ipvsadm -a -u 10.0.4.3:53 -r 172.21.53.10 -g
 *	ipvsadm -a -u 10.0.4.3:53 -r 172.21.53.11 -g
 *
 * For all tests, needs to remove the real server of the socket that is reused.
 *   To see this, use ipvsadm -Lnc to see which port stays, and get the IP address
 *   of the real server.
 *
 * Initially ipvsadm -L --timeout gives 900 120 300
 *
 * 1. Just remove the real server that the first connection uses.
 *
 * 2.  ipvsadm --set 0 0 10 # this makes the testing more manageable
 * 	and do 1. After 10 seconds ish the first conection recovers
 *
 * 3. Set One-Packet-Scheduling on the virtual server, and then as 1.
 * 	Do 1, and nothing should be lost since no conntrack entries are made.
 *
 * 4. ipvsadm --set 0 0 20
 *    clear One-Packet-Scheduling
 *      Do 1, and first server should recover after about 20 seconds
 *
 * 5. echo 1 >/proc/sys/net/expire_nodest_conn
 * 	Do 1, and now first server should recover after 1 failure (pre Linux 5.9)
 *
 * 6. Do 5 on a 5.9 or later kernel, and shouldn't get any failure
 */

#include <errno.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdbool.h>
#include <arpa/inet.h>
#include <errno.h>

#define DNS_SET_RD(flags, val) (flags |= ((val & 0x0001) <<  8))

#define DNS_TYPE_A       1
#define DNS_TYPE_NS      2
#define DNS_TYPE_CNAME   5
#define DNS_TYPE_SOA     6
#define DNS_TYPE_MX     15
#define DNS_TYPE_TXT    16
#define DNS_TYPE_AAAA   28
#define DNS_TYPE_RRSIG  46
#define DNS_TYPE_DNSKEY 48

typedef struct _dns_header {
        uint16_t id;
        uint16_t flags;
        uint16_t qdcount;
        uint16_t ancount;
        uint16_t nscount;
        uint16_t arcount;
} dns_header_t;


#define APPEND16(x, y) do { \
		*(uint16_t *)(x) = htons(y); \
		(x) = (uint8_t *) (x) + 2; \
	} while(0)

static size_t
dns_make_query(char *buf, const char *name, int type)
{
	uint16_t flags = 0;
	uint8_t *p;
	const char *s, *e;
	size_t n;
	dns_header_t *header = (dns_header_t *)buf;

	DNS_SET_RD(flags, 1);	/* Recursion Desired */

	header->id = random();
	header->flags = htons(flags);
	header->qdcount = htons(1);
	header->ancount = htons(0);
	header->nscount = htons(0);
	header->arcount = htons(0);

	p = (uint8_t *)(header + 1);

	/* QNAME */
	for (s = name; *s; s = *e ? ++e : e) {
		if (!(e = strchr(s, '.')))
			e = s + strlen(s);
		n = (size_t)(e - s);
		*(p++) = (uint8_t)n;
		memcpy(p, s, n);
		p += n;
	}
	
	if (name[0] != '.' || name[1] != '\0')
		*(p++) = 0;

	APPEND16(p, type);
	APPEND16(p, 1);		/* IN */

	return (size_t)(p - (uint8_t *) header);
}

static bool
dns_send(int fd, char *buf, size_t len, struct in_addr *server)
{
	ssize_t ret;
	struct sockaddr_in addr = { .sin_family = AF_INET, .sin_port = htons(53), .sin_addr = *server };

	ret = sendto(fd, buf, len, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		fprintf(stderr, "failed to write socket %d - %m.\n", fd);
		return false;
	}

	if (ret != (ssize_t) len)
		fprintf(stderr, "failed to write all of the datagram %zd instead of %zu.\n", ret, len);
	else
		printf("Wrote %zd bytes\n", ret);

	return true;
}

static int
do_query(int fd, const char *name, struct in_addr *server)
{
	size_t len;
	ssize_t rlen;
	char buf[128];
	struct sockaddr_in addr = { .sin_family = AF_UNSPEC };
	struct sockaddr_in bind_addr = { .sin_family = AF_INET };
	socklen_t addrlen = sizeof(addr);
	int port;

	if (fd <= -1) {
		bind_addr.sin_port = fd < -1 ? htons(-fd) : 0;
		if ((fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC /* | SOCK_NONBLOCK */, IPPROTO_UDP)) == -1) {
			fprintf(stderr, "failed to create socket. Rescheduling.\n");
			return -1;
		}

		if (bind_addr.sin_port) {
			printf("Binding to port %d\n", bind_addr.sin_port);
			if (bind(fd, (struct sockaddr *)&bind_addr, sizeof(bind_addr)))
				fprintf(stderr, "bind failed - %d - %m", errno);
		}
	}

	len = dns_make_query(buf, name, DNS_TYPE_A);
	if (!dns_send(fd, buf, len, server))
		return fd;

	rlen = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *)&addr, &addrlen);

	if (rlen == -1)
		printf("Receive on fd %d returned errno %d - m\n", fd, errno);
	else
		printf("Received %zd bytes from %s on fd %d\n", rlen, inet_ntoa(addr.sin_addr), fd);

	return fd;
}

int
main(int argc, char **argv)
{
	int fd = -1;
	int opt;
	char *name = ".";
	int sleep_time = 1;
	int queries_per_loop = 5;
	int iter = 2;
	int i;
	struct in_addr server_addr = { .s_addr = htonl(0x0a000703) };
	int local_port = 50000;

	while ((opt = getopt(argc, argv, ":n:d:s:c:i:p:H")) != -1) {
		switch(opt) {
		case 'n':
			name = optarg;
			break;
		case 'd':
			sleep_time = atoi(optarg);
			break;
		case 's':
			if (!inet_pton(AF_INET, optarg, &server_addr)) {
				fprintf(stderr, "Failed to parse %s\n", optarg);
				exit(1);
			}
		case 'c':
			queries_per_loop = atoi(optarg);
			break;
		case 'i':
			iter = atoi(optarg);
			break;
		case 'p':
			local_port = atoi(optarg);
			break;
		case 'H':
			printf ("-n query_name -s DNS server -d sleep time -c queries_per_loop -i iterations (0 = forever) -p local port -H this\n");
			exit(0);
			break;
		case '?':
			printf ("Unknown option -%c\n", optopt);
			exit(1);
			break;
		case ':':
			printf("Missing option argument for -%c\n", optopt);
			exit(1);
			break;
		}
	}

	while (true) {
		fd = do_query(fd == -1 ? -local_port : fd, name, &server_addr);
		for (i = 1; i < queries_per_loop; i++) {
			sleep(sleep_time);
			close(do_query(-1, name, &server_addr));
		}

		if (iter && !--iter)
			break;
		sleep(sleep_time);
	}

	close(fd);
}
