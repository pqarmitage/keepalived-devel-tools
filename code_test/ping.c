//#include "check_ping.h"
#include <error.h>
#include <stdio.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdbool.h>
//#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/icmp6.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdlib.h>

#define ICMP_BUFSIZE 256
#define SOCK_RECV_BUFF 512

enum connect_result {
        connect_error,
        connect_in_progress,
        connect_timeout,
        connect_fail,
        connect_success,
        connect_result_next
};

enum connect_result ping_it(int fd)
{
	struct icmphdr icmp_hdr;
	struct sockaddr_in dest = { .sin_family = AF_INET, .sin_addr.s_addr = htonl(0xac15350a)};
	int size = 0;
	int seq = 1;
	socklen_t addrlen;
	char send_buf[ICMP_BUFSIZE];

	memset(&icmp_hdr, 0, sizeof icmp_hdr);
	icmp_hdr.type = ICMP_ECHO;
	icmp_hdr.un.echo.id = fd;//arbitrary id
	icmp_hdr.un.echo.sequence = seq++;
	addrlen = sizeof(dest);
	memcpy(send_buf, &icmp_hdr, sizeof(icmp_hdr));
	size = sendto(fd, send_buf, sizeof(send_buf), 0, (struct sockaddr*)&dest, addrlen);

	if(size < 0)
	{
		fprintf(stderr, "send ICMP packet fail!\n");
		return connect_error;
	}
	return connect_success;
}

enum connect_result recv_it(int fd)
{
	char recv_buf[ICMP_BUFSIZE];
	int size = 0;
	int ret = connect_error;
	struct icmphdr rcv_hdr;

	memset(recv_buf, 0, sizeof(recv_buf));
	size = recv(fd, recv_buf, sizeof(recv_buf), 0);

	if(size < 0) {
		fprintf(stderr, "recv ICMP packet error!\n");
		return ret;
	} else if ((unsigned int)size < sizeof(rcv_hdr)) {
		fprintf(stderr, "Error, got short ICMP packet, %d bytes\n", size);
		return ret;
	}
	memcpy(&rcv_hdr, recv_buf, sizeof(rcv_hdr));
	if (rcv_hdr.type == ICMP_ECHOREPLY)
		ret = connect_success;
	else {
		fprintf(stderr, "Got ICMP packet with type 0x%x ?!?\n", rcv_hdr.type);
	}
	return ret;
}

enum connect_result ping6_it(int fd)
{
	struct icmp6_hdr* send_hdr;
	struct sockaddr_in6 dest_in6;
	int size = 0;
	int seq = 1;
	char send_buf[ICMP_BUFSIZE];
	send_hdr = (struct icmp6_hdr*)&send_buf;

	memset(send_hdr, 0, sizeof(struct icmp6_hdr));
	send_hdr->icmp6_type = ICMP6_ECHO_REQUEST;
	send_hdr->icmp6_id = fd;
	send_hdr->icmp6_seq = seq;

	size = sendto(fd, send_buf, ICMP_BUFSIZE, 0, (struct sockaddr *)&dest_in6, sizeof(dest_in6));
	if(size < 0)
	{
		fprintf(stderr, "send ICMPv6 packet fail!\n");
		return connect_error;
	}
	return connect_success;
}

enum connect_result recv6_it(int fd)
{
	char recv_buf[ICMP_BUFSIZE];
	int size = 0;
	int ret = connect_error;
	struct icmp6_hdr* rcv_hdr;

	size = recv(fd, recv_buf, sizeof(recv_buf), 0);

	if (size < 0) {
		fprintf(stderr, "recv ICMPv6 packet error!\n");
		return ret;
	} else if ((unsigned int)size < sizeof(rcv_hdr)) {
		fprintf(stderr, "Error, got short ICMPv6 packet, %d bytes\n", size);
		return ret;
	}
	rcv_hdr = (struct icmp6_hdr*)&recv_buf;

	if (rcv_hdr->icmp6_type == ICMP6_ECHO_REPLY)
		ret = connect_success;
	else
		fprintf(stderr, "Got ICMPv6 packet with type 0x%x ?!?\n", rcv_hdr->icmp6_type);

	return ret;
}


static void
one_run(int family)
{
	int fd;
	enum connect_result status;
	int size = SOCK_RECV_BUFF;

        if ((fd = socket(family, SOCK_DGRAM, (family == AF_INET)?IPPROTO_ICMP:IPPROTO_ICMPV6)) == -1) {
		fprintf(stderr, "socket failed errno %d (%m)", errno);
		exit (1);
        }

        setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &size, sizeof(size));
        /*
         * OK if setsockopt fails
         * Prevent users from pinging broadcast or multicast addresses
         */
        if (family == AF_INET) {
                status = ping_it(fd);
		recv_it(fd);
	}
        else {
                status = ping6_it(fd);
		recv6_it(fd);
	}

	close(fd);
}

int
main (int argc, char **argv)
{
	while (1)
	{
		one_run(AF_INET);
		printf("Done one\n");
		sleep(1);
	}
}
