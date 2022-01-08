#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <shadow.h>
#include <crypt.h>

/// @return 0 - password is correct, otherwise no
static int
check_password(const char* user, const char* password)
{
	struct passwd* passwdEntry = getpwnam(user);

	if (!passwdEntry)
	{
		printf("User '%s' doesn't exist\n", user);
		return 1;
	}

	if (0 != strcmp(passwdEntry->pw_passwd, "x"))
		return strcmp(passwdEntry->pw_passwd, crypt(password, passwdEntry->pw_passwd));
	else {
		// password is in shadow file
		struct spwd* shadowEntry = getspnam(user);
		if (!shadowEntry)
		{
			printf("Failed to read shadow entry for user '%s'\n", user);
			return 1;
		}

printf("Shadow password: %s\n", shadowEntry->sp_pwdp);
		return strcmp(shadowEntry->sp_pwdp, crypt(password, shadowEntry->sp_pwdp));
	}
}

int main(int argc, char **argv)
{
	int ret;

	ret = check_password(argv[1], argv[2]);

	printf("check password returned %d\n", ret);
}
