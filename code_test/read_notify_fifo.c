#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

int
main(int argc, char **argv)
{
	int fd;
	char *buf[8192];
	ssize_t len;
	char *fifo_name = "/tmp/notify_fifo.low";

	if (argc > 1)
		fifo_name = argv[1];

	mkfifo(fifo_name, 0660);

	fd = open(fifo_name, O_RDONLY);
	printf("FIFO opened\n");

	while ((len = read(fd, buf, sizeof(buf))) > 0) {
		write(1, buf, len);
	}
}
