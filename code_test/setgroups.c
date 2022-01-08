#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <grp.h>
#include <errno.h>

int main(int argc, char **argv)
{
	gid_t *sgid_list = NULL;
	int sgid_num;
	uid_t our_uid;
	gid_t our_gid;
	int i;

	our_uid = geteuid();
	our_gid = getegid();

	printf("I am %u:%u\n", our_uid, our_gid);

	sgid_num = getgroups(0, NULL);
	sgid_list = malloc(sgid_num * sizeof(gid_t));
	getgroups(sgid_num, sgid_list);

	for (i = 0; i < sgid_num; i++)
		printf("%s%d", i ? " " : "", sgid_list[i]);
	printf("\n");

	if (setgroups(0, NULL))
		printf("setgroups errno %d - %m\n", errno);

	sgid_num = getgroups(sgid_num, sgid_list);

	for (i = 0; i < sgid_num; i++)
		printf("%s%d", i ? " " : "", sgid_list[i]);
	printf("\n");

	free(sgid_list);
}
