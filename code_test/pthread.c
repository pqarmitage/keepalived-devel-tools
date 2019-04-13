#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>   /* For SYS_xxx definitions */


static int sleep_time = 10;

static void *
child_thread(void *p)
{
	pid_t pid = getpid();
	pid_t tid = syscall(SYS_gettid);
	pthread_t self = pthread_self();

	printf("Child pid %d, tid %d, self 0x%x, p 0x%x\n", pid, tid, self, p);

	if (p == (void *)5)
		execl("/usr/bin/date", "date");
	sleep(sleep_time);
	printf("%d:%d exiting\n", pid, tid);
}

int main(int argc, char **argv)
{
	void *p = (char *)1;
	int ret;
	pthread_attr_t attr;
	int i;
	pid_t pid = getpid();
	pid_t tid = syscall(SYS_gettid);
	pthread_t self = pthread_self();
	pthread_t ptid;
	int num_threads = 10;
	int inter_thread_sleep_time = 2;

	if (argc >= 2)
		num_threads = atoi(argv[1]);
	if (argc >= 3)
		sleep_time = atoi(argv[2]);
	if (argc >= 4)
		inter_thread_sleep_time = atoi(argv[3]);

	pthread_attr_init(&attr);

	for (i = 0; i < num_threads; i++) {
		ret = pthread_create(&ptid, &attr, child_thread, p);
		printf("create thread ret %d, pid %d, tid %d, ptid 0x%x, self 0x%x, p 0x%x\n", ret, pid, tid, ptid, self, p);
		if (inter_thread_sleep_time)
			sleep(inter_thread_sleep_time);
		p++;
	}
}
