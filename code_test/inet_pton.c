#include <stdio.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <string.h>
#include <memory.h>

int main(int argc, char **argv)
{
	char buf[40];
	uint32_t *p4 = (uint32_t *)buf;
	uint64_t *p8 = (uint64_t *)buf;
	uint16_t *p2 = (uint16_t *)buf;
	uint8_t *p1 = (uint8_t *)buf;

	memset(buf, 0, sizeof buf);
	inet_pton(strchr(argv[1], ':') ? AF_INET6 : AF_INET, argv[1], &buf);

	printf("%" PRIx8 " %" PRIx16 " %"PRIx32 " %" PRIx64 "\n", *p1, *p2, *p4, *p8);
}
