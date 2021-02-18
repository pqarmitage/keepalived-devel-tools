#include <glob.h>
#include <stdio.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

/* NOTE: missing_files is declared static, since otherwise GCC (at least v9.3.0,
 * 9.3.1 and 10.2.1) -O1 optimisation assumes that it cannot be altered by the call to glob(), if
 * it has static scope. Declaring it static volatile also solves the problem, as does not
 * initialising the values in open_and_check_glob (which just wouldn't work).
 * This is an optimisation error of course, since the gl_opendir() and gl_lstat() functions can modify
 * the value, and pointers to these functions are passed to glob().
 * What makes this even more difficult is that if the values of missing_files
 * is printed after the return from glob(), then everything works OK.
 *
 * See https://gcc.gnu.org/bugzilla/show_bug.cgi?id=97783 for more details.
 */

static unsigned missing_files;

static int
gl_lstat(const char *pathname, struct stat *statbuf)
{
	int ret;

	ret = lstat(pathname, statbuf);

	if (ret)
		missing_files++;

// fprintf(stderr, "lstat(%s) returned %d, missing_files %u\n", pathname, ret, missing_files);
	return ret;
}

static int
open_and_check_glob(glob_t *globbuf, const char *conf_file)
{
	int	res;

	globbuf->gl_offs = 0;

	globbuf->gl_closedir = (void *)closedir;
	globbuf->gl_readdir = (void *)readdir;
	globbuf->gl_opendir = (void *)opendir;
	globbuf->gl_lstat = (void *)gl_lstat;
	globbuf->gl_stat = (void *)stat;

	missing_files = 0;

	res = glob(conf_file, GLOB_MARK | GLOB_ALTDIRFUNC | GLOB_BRACE , NULL, globbuf);
#ifdef TEST1
fprintf(stderr, "glob(%s) returned %d, missing files %u\n", conf_file, res, missing_files);
fprintf(stderr, "glob(%s) returned %d, missing files %u\n", conf_file, res, missing_files);

	if (res)
		return 0;
#endif

	if (missing_files) {
		fprintf(stderr, "%d files missing: \"%s\"\n", missing_files, conf_file);

		globfree(globbuf);
		return 0;
	}

	return 1;
}

int main(int argc, char **argv)
{
	glob_t globbuf;

	if (open_and_check_glob(&globbuf, argv[1]))
		printf("returned 1\n");
	else
		printf("returned 0\n");
}
