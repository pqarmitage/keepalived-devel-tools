#include <crypt.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <stdlib.h>


int
main(int argc, char **argv)
{
	struct crypt_data data;
	const char *enc;
	char *s;

	memset(&data, 0, sizeof(data));

	if (argc <= 2) {
		s = crypt_gensalt("$6$", 0, NULL, 0);
		if (!s) {
			fprintf(stderr, "crypt_gensalt returned %d (%m)\n", errno);
			exit(1);
		}
		strcpy(data.setting, s);
	} else
		strcpy(data.setting, argv[2]);

	printf("Salt is: %s\n", data.setting);
	strcpy(data.input, argv[1]);

	errno = 0;
	enc = crypt_r(data.input, data.setting, &data);
	if (errno)
		printf("crypt_r returned error %d (%m)\n", errno);

	printf("enc: %s\n", enc);

	memset(&data, 0, sizeof(data));
}
