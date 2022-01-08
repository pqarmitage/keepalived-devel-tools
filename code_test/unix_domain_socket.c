#define _GNU_SOURCE

#include <stdio.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

static void
run_server(int sock)
{
	int fd;
	FILE *fp;
	struct iovec msg_iov[1];
	struct cmsghdr* cmsg;
	union {
		struct cmsghdr cm;
		char	control[CMSG_SPACE(sizeof(int))];
	} control_un = { .control[0] = 0 };
	struct msghdr msg = { .msg_namelen = 0, .msg_iov = msg_iov, .msg_iovlen = 1, .msg_control = control_un.control, .msg_controllen = sizeof(control_un.control) };
	int ret;
	char rx_buf[128];
	char rx_cmsg[128];

	/* Open some extra file descriptors so numbers should be different in client */
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_RDONLY);

	fd = memfd_create("I am a silly boy\n", MFD_CLOEXEC);
	fp = fdopen(fd, "r+");

	fprintf(fp, "This is my file message\n");
	fflush(fp);

	msg_iov[0].iov_base = "This is my sent message\n";
	msg_iov[0].iov_len = strlen(msg_iov[0].iov_base) + 1;
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	*(int *)CMSG_DATA(cmsg) = fd;
	cmsg->cmsg_len = CMSG_LEN(sizeof(int));
	msg.msg_controllen = CMSG_SPACE(cmsg->cmsg_len);
	msg.msg_controllen = cmsg->cmsg_len;

	printf("Server sending message with fd %d\n", fd);
	ret = sendmsg(sock, &msg, 0);
	if (ret == -1)
		printf("Server sendmsg returned errno %d (%m)\n", errno);
	else
		printf("Server sendmsg returned %d\n", ret);

	msg_iov[0].iov_base = rx_buf;
	msg_iov[0].iov_len = sizeof(rx_buf);
	msg.msg_iov = msg_iov;
	msg.msg_iovlen = 1;
	msg.msg_control = rx_cmsg;
	msg.msg_controllen = sizeof(rx_cmsg);
	ret = recvmsg(sock, &msg, 0);
	if (ret < 0)
		printf("Server recvmsg returned errno %d (%m)\n", errno);
	else {
		printf("Server recvmsg returned %d (%s)\n", ret, rx_buf);

		rewind(fp);

		printf("Server memfile read:\n");
		while (fgets(rx_buf, sizeof(rx_buf), fp))
			printf("\t%s\n", rx_buf);
	}

	ret = close(fd);
	if (ret == -1)
		printf("Server close(fd) errno %d (%m)\n", errno);

	ret = fclose(fp);
	if (ret == EOF)
		printf("Server fclose(fp) errno %d expected (%m)\n", errno);

	ret = close(sock);
	if (ret == -1)
		printf("Server close(sock) errno %d (%m)\n", errno);

	exit(1);
}

static void
run_client(int sock)
{
	int fd;
	FILE *fp;
	struct iovec msg_iov[1];
	struct cmsghdr* cmsg;
	union {
		struct cmsghdr cm;
		char	control[CMSG_SPACE(sizeof(int))];
	} control_un = { .control[0] = 0 };
	struct msghdr msg = { .msg_namelen = 0, .msg_iov = msg_iov, .msg_iovlen = 1, .msg_control = control_un.control, .msg_controllen = sizeof(control_un.control) };
	int ret;
	char rx_buf[128];
	char rx_cmsg[128];

	msg_iov[0].iov_base = rx_buf;
	msg_iov[0].iov_len = sizeof(rx_buf);
	msg.msg_iov = msg_iov;
	msg.msg_iovlen = 1;
	msg.msg_control = rx_cmsg;
	msg.msg_controllen = sizeof(rx_cmsg);
	ret = recvmsg(sock, &msg, 0);
	if (ret < 0)
		printf("Client recvmsg error %d (%m)\n", errno);
	else
		printf("Client recvmsg returned %d (%s)\n", ret, rx_buf);

	for (cmsg = CMSG_FIRSTHDR(&msg); cmsg; cmsg = CMSG_NXTHDR(&msg, cmsg)) {
		if (cmsg->cmsg_level == SOL_SOCKET &&
		    cmsg->cmsg_type == SCM_RIGHTS) {
			fd = *(int *)CMSG_DATA(cmsg);
			printf("Client received fd %d\n", fd);
		} else
			printf("Client received control message level %d type %d\n", cmsg->cmsg_level, cmsg->cmsg_type);
	}

	printf("Client msg recv: %s\n", rx_buf);

	fp = fdopen(fd, "r+");
	rewind(fp);

	printf("Client mem file read:\n");
	while (fgets(rx_buf, sizeof(rx_buf), fp))
		printf("\t%s\n", rx_buf);

	ftruncate(fd, 8);
	lseek(fd, 0, SEEK_END);
	write(fd, "my reply\n", 9);
	fdatasync(fd);

	ret = fclose(fp);
	if (ret == EOF)
		printf("Client fclose(fp) errno %d (%m)\n", errno);

	ret = close(fd);
	if (ret == -1)
		printf("Client close(fd) errno %d expected (%m)\n", errno);

	msg_iov[0].iov_base = "This is my reply message\n";
	msg_iov[0].iov_len = strlen(msg_iov[0].iov_base) + 1;
	msg.msg_control = NULL;
	msg.msg_controllen = 0;
	ret = sendmsg(sock, &msg, 0);
	if (ret == -1)
		printf("Client sendmsg returned errno %d (%m)\n", errno);
	else
		printf("Client sendmsg returned \n", ret);

	ret = close(sock);
	if (ret == -1)
		printf("Server close(sock) errno %d (%m)\n", errno);


	exit(0);
}

int main(int argc, char **argv)
{
	int sv[2];
	int ret;
	pid_t server_pid;
	pid_t client_pid;
	int num_terminated = 0;
	int i;
	pid_t pid;
	int wstatus;
	const char *name;

	ret = socketpair(AF_LOCAL, SOCK_STREAM | SOCK_CLOEXEC, 0, sv);
	if (ret) {
		fprintf(stderr, "socketpair() failed, errno %d (%m)\n");
		exit(1);
	} else
		printf("socketpair returned %d and %d\n", sv[0], sv[1]);

	if (!(server_pid = fork())) {
		run_server(sv[0]);
		exit(0);
	}
	printf("Server is pid %d\n", server_pid);

	if (!(client_pid = fork())) {
		run_client(sv[1]);
		exit(0);
	}
	printf("Client is pid %d\n", client_pid);

	close(sv[0]);
	close(sv[1]);

	for (i = 0; i < 2;) {
		pid = waitpid(-1, &wstatus, 0);

		if (pid == server_pid) {
			server_pid = 0;
			name = "Server";
			i++;
		} else if (pid == client_pid) {
			client_pid = 0;
			name = "Client";
			i++;
		} else
			name = "Unknown";

		printf("%s (%d) exited, status 0x%x\n", name, pid, wstatus);
	}
}
