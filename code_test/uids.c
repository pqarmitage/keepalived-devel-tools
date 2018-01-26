#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	printf ( "real %d:%d eff %d:%d\n", getuid(), getgid(), geteuid(), getegid());

	exit(4);
}
