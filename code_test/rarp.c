/*
 * rarpd.c	RARP daemon.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *		Jakub Jelinek, <jakub@redhat.com>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/errno.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <netinet/ether.h>
#include <linux/if_packet.h>
#include <sys/random.h>


struct arp_pkt {
	struct arphdr a;
	unsigned char ar_sha[ETH_ALEN];   /* Sender hardware address.  */
	unsigned char ar_sip[4];          /* Sender IP address.  */
	unsigned char ar_tha[ETH_ALEN];   /* Target hardware address.  */
	unsigned char ar_tip[4];          /* Target IP address.  */
};

static int send_arp;


__attribute__((noreturn)) static void
usage()
{
	fprintf(stderr, "Usage: rarp [ -A ] [ interface]\n");
	exit(1);
}

static void
send_request(int fd, int ifidx)
{
	unsigned char buf[1024];
	struct arp_pkt *arp = (struct arp_pkt*)buf;
	struct sockaddr_ll sll = { .sll_ifindex = ifidx };

	memset(arp, 0, sizeof(*arp));
	arp->a.ar_hrd = htons(ARPHRD_ETHER);
	arp->a.ar_pro = htons(ETH_P_IP);
	arp->a.ar_hln = 6;
	arp->a.ar_pln = 4;
	arp->a.ar_op = htons(ARPOP_RREQUEST);
	getrandom(arp->ar_sha, 6, 0);
	memcpy(arp->ar_tha, arp->ar_sha, 6);

	sll.sll_pkttype = PACKET_BROADCAST;
	sll.sll_hatype = htons(ARPHRD_ETHER);
	sll.sll_protocol = htons(send_arp ? ETH_P_ARP : ETH_P_RARP);
	sll.sll_halen = 6;
	sll.sll_addr[0] = sll.sll_addr[1] = sll.sll_addr[2] = sll.sll_addr[3] = sll.sll_addr[4] = sll.sll_addr[5] =  0xff;

	if (sendto(fd, buf, sizeof(*arp), 0, (struct sockaddr*)&sll, sizeof(sll)) != sizeof(*arp))
		printf("sendto failed - %d - %m\n", errno);
}

int main(int argc, char **argv)
{
	int fd;
	int opt;
	char *ifname = NULL;
	int ifidx = -1;

	opterr = 0;
	while ((opt = getopt(argc, argv, "A")) != EOF) {
		switch (opt) {
		case 'A':
			++send_arp;
			break;

		default:
			usage();
		}
	}
	if (argc > optind) {
		if (argc > optind+1)
			usage();
		ifname = argv[optind];
        }

	fd = socket(PF_PACKET, SOCK_DGRAM, 0);

	if (fd == -1) {
		fprintf(stderr, "failed to bind any socket. Aborting.\n");
		exit(1);
	}

	if (ifname) {
                struct ifreq ifr;

                memset(&ifr, 0, sizeof(ifr));
                strncpy(ifr.ifr_name, ifname, IFNAMSIZ-1);
                if (ioctl(fd, SIOCGIFINDEX, &ifr)) {
                        perror("ioctl(SIOCGIFINDEX)");
                        usage();
                }
                ifidx = ifr.ifr_ifindex;
	} else {
		fprintf(stderr, "An interface must be specified\n");
		exit(1);
	}

	if (fd >= 0) {
		struct sockaddr_ll sll;
		memset(&sll, 0, sizeof(sll));
		sll.sll_family = AF_PACKET;
		sll.sll_protocol = send_arp ? htons(ETH_P_ARP) : htons(ETH_P_RARP);
		sll.sll_ifindex = ifidx;
		if (bind(fd, (struct sockaddr*)&sll, sizeof(sll)) < 0) {
			close(fd);
			fd = -1;
		}
	}

	send_request(fd, ifidx);
}
