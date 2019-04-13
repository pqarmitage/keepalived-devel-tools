/* See also forkstat  downloaded */

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/connector.h>
#include <linux/cn_proc.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

/*
 * connect to netlink
 * returns netlink socket, or -1 on error
 */
static int nl_connect()
{
	int rc;
	int nl_sock;
	struct sockaddr_nl sa_nl;

	nl_sock = socket(PF_NETLINK, SOCK_DGRAM, NETLINK_CONNECTOR);
	if (nl_sock == -1) {
		perror("socket");
		return -1;
	}

	sa_nl.nl_family = AF_NETLINK;
	sa_nl.nl_groups = CN_IDX_PROC;
	sa_nl.nl_pid = getpid();

	rc = bind(nl_sock, (struct sockaddr *)&sa_nl, sizeof(sa_nl));
	if (rc == -1) {
		perror("bind");
		close(nl_sock);
		return -1;
	}

	return nl_sock;
}

/*
 * subscribe on proc events (process notifications)
 */
static int set_proc_ev_listen(int nl_sock, bool enable)
{
	int rc;
	struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__)) {
			struct cn_msg cn_msg;
			enum proc_cn_mcast_op cn_mcast;
		};
	} nlcn_msg;

	memset(&nlcn_msg, 0, sizeof(nlcn_msg));
	nlcn_msg.nl_hdr.nlmsg_len = sizeof(nlcn_msg);
	nlcn_msg.nl_hdr.nlmsg_pid = getpid();
	nlcn_msg.nl_hdr.nlmsg_type = NLMSG_DONE;

	nlcn_msg.cn_msg.id.idx = CN_IDX_PROC;
	nlcn_msg.cn_msg.id.val = CN_VAL_PROC;
	nlcn_msg.cn_msg.len = sizeof(enum proc_cn_mcast_op);

	nlcn_msg.cn_mcast = enable ? PROC_CN_MCAST_LISTEN : PROC_CN_MCAST_IGNORE;

	rc = send(nl_sock, &nlcn_msg, sizeof(nlcn_msg), 0);
	if (rc == -1) {
		perror("netlink send");
		return -1;
	}

	return 0;
}

/*
 * handle a single process event
 */
static volatile bool need_exit = false;
static int handle_proc_ev(int nl_sock)
{
	struct nlmsghdr *nlmsghdr;
	ssize_t len;
	char __attribute__ ((aligned(NLMSG_ALIGNTO)))buf[4096];

	struct __attribute__ ((aligned(NLMSG_ALIGNTO))) {
		struct nlmsghdr nl_hdr;
		struct __attribute__ ((__packed__)) {
			struct cn_msg cn_msg;
			struct proc_event proc_ev;
		};
	} nlcn_msg;
	struct cn_msg *cn_msg;
	struct proc_event *proc_ev;
	time_t tv;
	struct tm *tm_time;
	char tbuf[9];

	while (!need_exit) {
		len = recv(nl_sock, &buf, sizeof(buf), 0);
		if (len == 0) {
			/* shutdown? */
			return 0;
		} else if (len == -1) {
			if (errno == EINTR) continue;
			perror("netlink recv");
			return -1;
		}
		for (nlmsghdr = (struct nlmsghdr *)buf;
			NLMSG_OK (nlmsghdr, len);
			nlmsghdr = NLMSG_NEXT (nlmsghdr, len)) {

			if (nlmsghdr->nlmsg_type == NLMSG_ERROR ||
			    nlmsghdr->nlmsg_type == NLMSG_NOOP)
				continue;

			cn_msg = NLMSG_DATA(nlmsghdr);
			if ((cn_msg->id.idx != CN_IDX_PROC) ||
			    (cn_msg->id.val != CN_VAL_PROC))
				continue;

			proc_ev = (struct proc_event *)cn_msg->data;

			tv = time(NULL);
			tm_time = localtime(&tv);
			strftime(tbuf, sizeof tbuf, "%T", tm_time);

			switch (proc_ev->what)
			{
			case PROC_EVENT_NONE:
				printf("%s set mcast listen ok\n", tbuf);
				break;
			case PROC_EVENT_FORK:
				printf("%s fork: parent tid=%d pid=%d -> child tid=%d pid=%d\n", tbuf,
						proc_ev->event_data.fork.parent_pid,
						proc_ev->event_data.fork.parent_tgid,
						proc_ev->event_data.fork.child_pid,
						proc_ev->event_data.fork.child_tgid);
				break;
			case PROC_EVENT_EXEC:
				printf("%s exec: tid=%d pid=%d\n", tbuf,
						proc_ev->event_data.exec.process_pid,
						proc_ev->event_data.exec.process_tgid);
				break;
			case PROC_EVENT_UID:
				printf("%s uid change: tid=%d pid=%d from %d to %d\n", tbuf,
						proc_ev->event_data.id.process_pid,
						proc_ev->event_data.id.process_tgid,
						proc_ev->event_data.id.r.ruid,
						proc_ev->event_data.id.e.euid);
				break;
			case PROC_EVENT_GID:
				printf("%s gid change: tid=%d pid=%d from %d to %d\n", tbuf,
						proc_ev->event_data.id.process_pid,
						proc_ev->event_data.id.process_tgid,
						proc_ev->event_data.id.r.rgid,
						proc_ev->event_data.id.e.egid);
				break;
			case PROC_EVENT_SID:
				printf("%s sid change: tid=%d pid=%d", tbuf,
						proc_ev->event_data.sid.process_pid,
						proc_ev->event_data.sid.process_tgid);
				break;
			case PROC_EVENT_PTRACE:
				printf("%s ptrace change: tid=%d pid=%d tracer tid=%d, pid=%d", tbuf,
						proc_ev->event_data.ptrace.process_pid,
						proc_ev->event_data.ptrace.process_tgid,
						proc_ev->event_data.ptrace.tracer_pid,
						proc_ev->event_data.ptrace.tracer_tgid);
				break;
			case PROC_EVENT_COMM:
				printf("%s comm: tid=%d pid=%d comm %s\n", tbuf,
						proc_ev->event_data.comm.process_pid,
						proc_ev->event_data.comm.process_tgid,
						proc_ev->event_data.comm.comm);
				break;
			case PROC_EVENT_COREDUMP:
				printf("%s coredump: tid=%d pid=%d", tbuf,
						proc_ev->event_data.coredump.process_pid,
						proc_ev->event_data.coredump.process_tgid);
				break;
			case PROC_EVENT_EXIT:
				/* Check /proc/pid/stat - if not exist or Z, then process gone. What is status if coredumping */
				printf("%s exit: tid=%d pid=%d exit_code=%d\n", tbuf,
						proc_ev->event_data.exit.process_pid,
						proc_ev->event_data.exit.process_tgid,
						proc_ev->event_data.exit.exit_code,
						proc_ev->event_data.exit.exit_signal);
				break;
			default:
				printf("%s unhandled proc event %d\n", tbuf, proc_ev->what);
				break;
			}
		}
	}

	return 0;
}

static void on_sigint(int unused)
{
	need_exit = true;
}

int main(int argc, const char *argv[])
{
	int nl_sock;
	int rc = EXIT_SUCCESS;

	signal(SIGINT, &on_sigint);
	siginterrupt(SIGINT, true);

	nl_sock = nl_connect();
	if (nl_sock == -1)
		exit(EXIT_FAILURE);

	rc = set_proc_ev_listen(nl_sock, true);
	if (rc == -1) {
		rc = EXIT_FAILURE;
		goto out;
	}

	rc = handle_proc_ev(nl_sock);
	if (rc == -1) {
		rc = EXIT_FAILURE;
		goto out;
	}

	set_proc_ev_listen(nl_sock, false);

out:
	close(nl_sock);
	exit(rc);
}

