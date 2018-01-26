#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

int main(int argc, char** argv)
{
	char *slash;
	char *next = argv[1];
	char sav;
	int ret;
	struct stat buf;
	char perms[] = "rwxrwxrwx";

//	while ((slash = strchr(next, '/'))) {
	while (next) {
		slash = strchr(next, '/');
		if (slash) {
			next = slash + 1;
			if (slash == argv[1])
				slash++;
			sav = *slash;
			*slash = 0;
		}
		else
			next = NULL;

		ret = stat(argv[1], &buf);
		if (ret) {
			fprintf(stderr, "stat returned errno %d - %s\n", errno, strerror(errno));
			exit(1);
		}

		printf("\tstat - uid %d gid %d st_mode 0%o\n", buf.st_uid, buf.st_gid, buf.st_mode);
		if (buf.st_uid ||				/* Owner is not root */
		    (buf.st_gid && (buf.st_mode & S_IWGRP)) ||	/* group is not root and group write permission */
		    (buf.st_mode & S_IWOTH)) {			/* world has write permission */
			perms[0] = buf.st_mode & S_IRUSR ? 'r' : '-';
			perms[1] = buf.st_mode & S_IWUSR ? 'w' : '-';
			perms[2] = buf.st_mode & S_IXUSR ? 'x' : '-';
			perms[3] = buf.st_mode & S_IRGRP ? 'r' : '-';
			perms[4] = buf.st_mode & S_IWGRP ? 'w' : '-';
			perms[5] = buf.st_mode & S_IXGRP ? 'x' : '-';
			perms[6] = buf.st_mode & S_IROTH ? 'r' : '-';
			perms[7] = buf.st_mode & S_IWOTH ? 'w' : '-';
			perms[8] = buf.st_mode & S_IXOTH ? 'x' : '-';
			printf("Unsafe permissions on %s - %s for %d:%d\n", argv[1], perms, buf.st_uid, buf.st_gid);
		}

		if (slash)
			*slash = sav;
	}
}
