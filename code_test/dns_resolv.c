/*
Copyright (C) 2013 SourceXR

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
*/

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <errno.h>
#include <netdb.h>

// ParseRecord
void parse_record (unsigned char *buffer, size_t r,
		   const char *section, ns_sect s,
		   int idx, ns_msg *m) {

    ns_rr rr;
    int k = ns_parserr (m, s, idx, &rr);
    if (k == -1) {
	fprintf(stderr, "%d %s\n", errno, strerror (errno));
	return;
    }

    printf("%s %s %d %d %d ", section, ns_rr_name (rr), ns_rr_type (rr), ns_rr_class (rr), ns_rr_ttl (rr));

    const size_t size = NS_MAXDNAME;
    unsigned char name[size];
    int t = ns_rr_type (rr);

    const u_char *data = ns_rr_rdata (rr);
    if (t == T_MX) {
	int pref = ns_get16 (data);
	ns_name_unpack (buffer, buffer + r, data + sizeof (u_int16_t),
			name, size);
	char name2[size];
	ns_name_ntop (name, name2, size);
	printf("%d %s", pref, name2);
    }
    else if (t == T_A) {
	unsigned int addr = ns_get32 (data);
	struct in_addr in;
	in.s_addr = ntohl (addr);
	char *a = inet_ntoa (in);
	printf("%s", a);
    }
    else if (t == T_AAAA) {
	unsigned int addr = ns_get32 (data);
	struct in6_addr in6;
	char buf[64];
	inet_ntop (AF_INET6, data, buf, sizeof(buf));
	printf("%s", buf);
    }
    else if (t == T_NS) {
	ns_name_unpack (buffer, buffer + r, data, name, size);
	char name2[size];
	ns_name_ntop (name, name2, size);
	printf("%s", name2);
    }
    else {
	printf("%s", "unhandled record");
    }

    printf("\n");
}
// ParseRecord

int main (int argc, char **argv)
{
    struct __res_state rs = {.options = 0};

    if (argc < 2) {
	fprintf(stderr, "Use %s host\n", argv[0]);
	return 1;
    }

    // MainBegin
    const size_t size = 1024;
    unsigned char buffer[size];

    const char *host = argv[1];

    res_ninit(&rs);
    int r = res_nquery(&rs, host, C_IN, T_MX, buffer, size);
    if (r == -1) {
	fprintf(stderr, "%d %s\n", h_errno, hstrerror (h_errno));
	return 1;
    }
    else {
	if (r == (int) (size)) {
	    fprintf(stderr, "%s", "Buffer too small reply truncated\n");
	    return 1;
	}
    }
    // MainBegin

    // ErrorHandling
    HEADER *hdr = (HEADER*) (buffer);

    if (hdr->rcode != NOERROR) {

	fprintf(stderr, "Error: ");
	switch (hdr->rcode) {
	case FORMERR:
	    fprintf(stderr, "Format error");
	    break;
	case SERVFAIL:
	    fprintf(stderr, "Server failure");
	    break;
	case NXDOMAIN:
	    fprintf(stderr, "Name error");
	    break;
	case NOTIMP:
	    fprintf(stderr, "Not implemented");
	    break;
	case REFUSED:
	    fprintf(stderr, "Refused");
	    break;
	default:
	    fprintf(stderr, "Unknown error");
	}
	return 1;
    }
    // ErrorHandling

    // ReadHeader
    int question = ntohs (hdr->qdcount);
    int answers = ntohs (hdr->ancount);
    int nameservers = ntohs (hdr->nscount);
    int addrrecords = ntohs (hdr->arcount);

    printf("Reply: question: %d, answers: %d, nameservers: %d, address records: %d\n",
		    question, answers, nameservers, addrrecords);
    // ReadHeader

    // InitParse
    ns_msg m;
    int k = ns_initparse (buffer, r, &m);
    if (k == -1) {
	fprintf(stderr, "%d %m\n", errno, strerror (errno));
	return 1;
    }
    // InitParse

    // ReadRecords
    for (int i = 0; i < question; ++i) {
	ns_rr rr;
	int k = ns_parserr (&m, ns_s_qd, i, &rr);
	if (k == -1) {
	    fprintf(stderr, "%d %s\n", errno, strerror (errno));
	    return 1;
	}
	printf("question %s %d %d\n", ns_rr_name (rr), ns_rr_type (rr), ns_rr_class (rr));
    }
    for (int i = 0; i < answers; ++i) {
	parse_record (buffer, r, "answers", ns_s_an, i, &m);
    }

    for (int i = 0; i < nameservers; ++i) {
	parse_record (buffer, r, "nameservers", ns_s_ns, i, &m);
    }

    for (int i = 0; i < addrrecords; ++i) {
	parse_record (buffer, r, "addrrecords", ns_s_ar, i, &m);
    }
    // ReadRecords

    return 0;
}
