#define __USE_GNU

#include <sys/utsname.h>
#include <stdio.h>

int main(int argc, char** argv)
{
	struct utsname u;

	uname(&u);
	printf("Sysname %s\n", u.sysname);
	printf("Nodename %s\n", u.nodename);

	printf("Release %s\n", u.release);
	printf("Version %s\n", u.version);

	printf("Machine %s\n", u.machine);

	printf("Domain name %s\n", u.__domainname);
};
