#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

FILE *fopen_safe(const char *path, const char *mode)
{
	int fd;
	FILE *file;
	int flags = O_NOFOLLOW | O_CREAT;
	int sav_errno;

	if (mode[0] == 'r')
		return fopen(path, mode);

	if ((mode[0] != 'a' && mode[0] != 'w') ||
	    (mode[1] &&
	     (mode[1] != '+' || mode[2]))) {
		errno = EINVAL;
		return NULL;
	}

	if (mode[0] == 'w')
		flags |= O_TRUNC;
	else
		flags |= O_APPEND;

	if (mode[1])
		flags |= O_RDWR;
	else
		flags |= O_WRONLY;

	fd = open(path, flags, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (fd == -1)
		return NULL;

	if (fchown(fd, 0, 0)) {
		fprintf(stderr, "Unable to change file ownership - errno %d (%m)\n", errno);
		close(fd);
		return NULL;
	}

	if (fchmod(fd, S_IRUSR | S_IWUSR)) {
		fprintf(stderr, "Unable to change file permission - errno %d (%m)\n", errno);
		close(fd);
		return NULL;
	}

	file = fdopen (fd, "w");
	if (!file) {
		sav_errno = errno;
		fprintf(stderr, "fdopen failed - errno %d (%m)\n", errno);
		close(fd);
		errno = sav_errno;
		return NULL;
	}

	return file;
}

// If file is aleady opened by a non root used (e.g. with tail -f), can it still read it?
int
main(int argc, char **argv)
{
	FILE *file;

	if (argc != 3) {
		fprintf(stderr, "Need two parameters\n");
		exit(1);
	}

	file = fopen_safe(argv[1], argv[2]);
	if (!file) {
		fprintf(stderr, "fopen_safe returned error %d (%m)\n", errno);
		exit(1);
	}

	fputs("Another line\n", file);

	fclose(file);
	
	return 0;
}
