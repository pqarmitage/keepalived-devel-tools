#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#define INADDRSZ 4

int
inet_ston(const char *addr, uint32_t *dst)
{
        static char digits[] = "0123456789";
        int saw_digit, octets, ch;
        u_char tmp[INADDRSZ], *tp;

        saw_digit = 0;
        octets = 0;
        *(tp = tmp) = 0;

        while ((ch = *addr++) != '\0' && ch != '/' && ch != '-') {
                const char *pch;
                if ((pch = strchr(digits, ch)) != NULL) {
                        u_int new = *tp * 10 + (pch - digits);
                        if (new > 255)
                                return 0;
                        *tp = new;
                        if (!saw_digit) {
                                if (++octets > 4)
                                        return 0;
                                saw_digit = 1;
                        }
                } else if (ch == '.' && saw_digit) {
                        if (octets == 4)
                                return 0;
                        *++tp = 0;
                        saw_digit = 0;
                } else
                        return 0;
        }

        if (octets < 4)
                return 0;

        memcpy(dst, tmp, INADDRSZ);
        return 1;
}

int main(int argc, char **argv)
{
	struct in_addr inp, ina;
	uint32_t ins;
	struct addrinfo hints = { .ai_flags = AI_NUMERICHOST, .ai_family = AF_INET };
	struct addrinfo *res;

	if (!inet_aton(argv[1], &ina))
		printf("inet_aton error\n");
	else
		printf("0x%x\n", ina.s_addr);

	if (inet_pton(AF_INET, argv[1], &inp) <= 0)
		printf("inet_aton error\n");
	else
		printf("0x%x\n", inp.s_addr);

	if (!inet_ston(argv[1], &ins))
		printf("inet_ston error\n");
	else
		printf("0x%x\n", ins);

	if (getaddrinfo(argv[1], NULL, &hints, &res))
		printf("getaddrinfo error\n");
	else {
		printf("0x%x\n", ((struct sockaddr_in *)res->ai_addr)->sin_addr);
		freeaddrinfo(res);
	}
}
