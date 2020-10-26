#include <glob.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdlib.h>
#include <libgen.h>
#include <stdbool.h>

unsigned missing_files;
unsigned missing_directories;
bool log_all;
bool log_errors;

static int
gl_closedir(DIR *dirp)
{
	int ret;

	ret = closedir(dirp);
	if (log_all)
		printf("closedir(%p) returning %d (errno %d)\n", dirp, ret, errno);

	return ret;
}
static struct dirent *
gl_readdir(DIR *dirp)
{
	struct dirent *de;

	de = readdir(dirp);
	if (log_all)
		printf("readdir(%p) returning %s, errno %d\n", dirp, !de ? "NULL" : de->d_name, errno);

	return de;
}
static DIR *
gl_opendir(const char *name)
{
	DIR *dirp;

	dirp = opendir(name);
	if (log_all)
		printf("opendir(%s) returning %p, errno %d\n", name, dirp, errno);

	if (!dirp) {
		if (log_errors)
			printf("Directory %s not found\n", name);
		missing_directories++;
	}

	return dirp;
}
static int
gl_lstat(const char *pathname, struct stat *statbuf)
{
	int ret;

	ret = lstat(pathname, statbuf);
	if (log_all)
		printf("lstat(%s) returning %d (errno %d)\n", pathname, ret, errno);

	if (ret) {
		if (log_errors)
			printf("File %s not found\n", pathname);
		missing_files++;
	}

	return ret;
}

static int
gl_stat(const char *pathname, struct stat *statbuf)
{
	int ret;

	ret = stat(pathname, statbuf);
	if (log_all)
		printf("stat(%s) returning %d (errno %d)\n", pathname, ret, errno);

	if (ret)
		missing_files++;

	return ret;
}

static int
errfunc(const char *epath, int eerrno) {
	printf("%s had error %d\n", epath, eerrno);
}

int main(int argc, char **argv)
{
	glob_t globbuf;
	size_t i;
	int	res;
	int flags = GLOB_MARK | GLOB_BRACE;
	int opt;

	while ((opt = getopt(argc, argv, ":Aefah")) != -1) {
		switch(opt) {
		case 'A':
			log_all = true;
			break;
		case 'e':
			flags |= GLOB_ERR;
			break;
		case 'f':
			flags |= GLOB_ALTDIRFUNC;
			globbuf.gl_closedir = (void *)closedir;
			globbuf.gl_readdir = (void *)readdir;
			globbuf.gl_opendir = (void *)gl_opendir;
			globbuf.gl_lstat = (void *)gl_lstat;
			globbuf.gl_stat = (void *)stat;
			log_errors = true;
			break;
		case 'a':
			flags |= GLOB_ALTDIRFUNC;
			globbuf.gl_closedir = (void *)gl_closedir;
			globbuf.gl_readdir = (void *)gl_readdir;
			globbuf.gl_opendir = (void *)gl_opendir;
			globbuf.gl_lstat = (void *)gl_lstat;
			globbuf.gl_stat = (void *)gl_stat;
			log_all = true;
			break;
		case 'h':
			printf ("%s -A -a -e -f -h this (-e sets GLOB_ERR, -f sets used alternate functions, -a sets all functions -A logs all status)\n", basename(argv[0]));
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

	globbuf.gl_offs = 0;

	res = glob(argv[optind], flags, flags & GLOB_ERR ? errfunc : NULL, &globbuf);

	errno = 0;
	if (res) {
		printf("Unable to find config file(s) '%s' - res %d (%s), errno %d (%m).\n", argv[optind], res, res == GLOB_NOSPACE ? "GLOB_NOSPACE" : res == GLOB_ABORTED ? "GLOB_ABORTED" : res == GLOB_NOMATCH ? "GLOB_NOMATCH" : "?", errno);
	}
	else if (errno)
		printf("Unable to find config file(s) '%s' - errno %d (%m).\n", argv[optind], errno);

	printf("glob returned %d matches", globbuf.gl_pathc);
	if (flags & GLOB_ALTDIRFUNC)
		printf(" (%d missing files, %d missing directories)", missing_files, missing_directories);
	printf("\n");

	for(i = 0; i < globbuf.gl_pathc; i++){
		printf("Opening file '%s'.\n", globbuf.gl_pathv[i]);
	}

	globfree(&globbuf);
}
