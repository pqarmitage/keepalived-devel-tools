#include <stdio.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int pipes[2][2];
	int ep;
	struct epoll_event ev;
	int data;
	int res;

	pipe(pipes[0]);
	pipe(pipes[1]);

	ep = epoll_create1(0);
	ev.events = EPOLLIN;
	ev.data.u32 = 0;
	epoll_ctl(ep, EPOLL_CTL_ADD, pipes[0][0], &ev);
	ev.data.u32 = 1;
	epoll_ctl(ep, EPOLL_CTL_ADD, pipes[1][0], &ev);

	data = 0;
	write(pipes[0][1], &data, sizeof data);
	data = 1;
	write(pipes[1][1], &data, sizeof data);

	res = epoll_wait(ep, &ev, 1, 1400);
	printf("epoll_wait returned %d, errno %d\n", res, errno);
	if (res == 1) {
		read(pipes[ev.data.u32][0], &data, sizeof data);
		printf("epoll returned data %d, read %d\n", ev.data.u32, data);
	}

	res = epoll_wait(ep, &ev, 1, 1400);
	printf("epoll_wait returned %d, errno %d\n", res, errno);
	if (res == 1) {
		read(pipes[ev.data.u32][0], &data, sizeof data);
		printf("epoll returned data %d, read %d\n", ev.data.u32, data);
	}

	res = epoll_wait(ep, &ev, 1, 1400);
	printf("epoll_wait returned %d, errno %d\n", res, errno);
	if (res == 1) {
		read(pipes[ev.data.u32][0], &data, sizeof data);
		printf("epoll returned data %d, read %d\n", ev.data.u32, data);
	}

	epoll_ctl(ep, EPOLL_CTL_DEL, pipes[0][0], &ev);
	ev.data.u32 = 1;
	epoll_ctl(ep, EPOLL_CTL_DEL, pipes[1][0], &ev);
	close(ep);

	close(pipes[0][0]);
	close(pipes[0][1]);
	close(pipes[1][0]);
	close(pipes[1][1]);
}
