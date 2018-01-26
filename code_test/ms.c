#include <sys/types.h>
#include <sys/ipc.h>
#define _GNU_SOURCE
#include <sys/msg.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>

struct my_msgbuf
{
	long mtype;
	uid_t uid;
	gid_t gid;
	char mtext[200];
};

static int msqid;

// Use POSIX message queues

void
init_msq(void)
{
	key_t key;

// We mustn't exit on error - if fail must exec in usual way
// A fixed key means only 1 instance of keepalived can run
	if ((key = ftok("/tmp/msq_key.txt", 'B')) == -1) {
		printf("msqw ftok error (%s)\n", strerror(errno));
		exit(1);
	}

	if ((msqid = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)) == -1) { /* connect to the queue */
		printf("msqw msgget error (%s)\n", strerror(errno));
		exit(1);
	}
}

static void
clean_msq(void)
{
	printf("Removing msqid:%d\n", msqid);
	msgctl(msqid, IPC_RMID, NULL);
}

/*
 * Write on msq
 *
 */
static void
read_msq(void)
{
	struct my_msgbuf buf;

// What causes this child to terminate?
	while (true) {
		if (msgrcv(msqid, &buf, sizeof(buf.uid) + sizeof(buf.gid) + sizeof(buf.mtext), 0, 0) == -1) {
			int err_num = errno;

			printf("msgrcv error(%s)\n", strerror(errno));

// Why remove message queue if interrupted?
			if(err_num == EINTR)
				clean_msq();

// Why terminate after any error
			exit(1);	/*TBF*/
		}

// Need to close the message q after the fork
//		system_call(buf.mtext, buf.uid, buf.gid);
	}
}

/*
 *  Initialize reader message queue
 *
 */
int start_msqr_child(void)
{
#ifndef _DEBUG_
	pid_t pid;

	/*
	 * Initialize child process to read 
	 * and execute cmds in message queue
	 */
	pid = fork();

	if (pid < 0) {
		printf("msqr child process fork error(%s)\n", strerror(errno));
		return -1;
	} else if (pid) {
		//vrrp_child = pid;
		printf("Starting msqr child process, pid=%d\n", pid);

		return 0;
	}
#endif

	printf("Started msqr child process\n");

	/*Init msqr*/
	init_msq();
	read_msq();

	return 0;
}

/*
 * Write on msq
 *
 * */
void write_msq(const char *cmd, uid_t uid, gid_t gid)
{
	struct my_msgbuf buf;
	size_t len;

	buf.uid = uid;
	buf.gid = gid;

	buf.mtype = 1; /* we don't really care in this case */

// Check length - why limit to 200? Problem with long notify name and its parameters
	strcpy(buf.mtext, cmd); /*Max 200 bytes*/
	len = sizeof(buf.uid) + sizeof(buf.gid) + strlen(buf.mtext) + 1;

// What it write error - 
	if (msgsnd(msqid, &buf, len, IPC_NOWAIT) == -1)
		printf("msgsnd error(%s)\n", strerror(errno));
}

int main(int argc, char**argv)
{
	struct msqid_ds ds;
	struct msginfo mi;

	init_msq();

	if (msgctl(msqid, IPC_STAT, &ds))
		printf("msgctl IPC_STAT returned errno %d\n", errno);
	else
		printf("Max bytes in queue %d\n", ds.msg_qbytes);

	if (msgctl(msqid, MSG_INFO, (struct msqid_ds*)&mi))
		printf("msgctl MSG_INFO returned errno %d\n", errno);
	else
		printf("Max queues %d, max bytes in msg %d, max bytes written to queue %d, max tql %d\n", mi.msgmni, mi.msgmax, mi.msgmnb, mi.msgtql );

	clean_msq();
}
