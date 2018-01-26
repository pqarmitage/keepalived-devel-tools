#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	struct passwd *pwd;
	struct group *grp;
	uid_t uid;
	gid_t gid;
	int ret;

	pwd = getpwnam(argv[2]);
	uid = pwd->pw_uid;
	gid = pwd->pw_gid;

	if (argc >= 4) {
		grp = getgrnam(argv[3]);
		gid = grp->gr_gid;
	}

	setegid(gid);
	seteuid(uid);
	printf ( "Running %s as real %d:%d eff %d:%d\n", argv[1], getuid(), getgid(), geteuid(), getegid());

	ret = system(argv[1]);

	printf("system() returned 0x%x\n", ret);
}
