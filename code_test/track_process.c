/* See also forkstat  downloaded */

#define _GNU_SOURCE

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
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define ALL_MSGS

static pid_t proc_pid;
static const char *proc_name;
static size_t proc_name_len;

static void
check_process(pid_t pid)
{
	char cmdline[] = "/proc/xxxxxxx/cmdline";
	int fd;
	char buf[proc_name_len + 2];
	ssize_t len;

	sprintf(cmdline, "/proc/%d/cmdline", pid);

	if ((fd = open(cmdline, O_RDONLY)) == -1)
		return;

	len = read(fd, buf, sizeof(buf) - 1);
	close(fd);
	buf[len] = '\0';
	if (!strcmp(buf, proc_name)) {
		printf("%s: %d\n", proc_name, pid);
		proc_pid = pid;
	}
}

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

char buf[256]; unsigned char *p; char *bufp; size_t i;
for (i = 0, p = (void*)&nlcn_msg, bufp = buf; i < sizeof nlcn_msg; i++, p++) bufp += sprintf(bufp, "%2.2x ", *p); 
printf("nlcn_msg: %s\n", buf);
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

			switch (proc_ev->what)
			{
			case PROC_EVENT_NONE:
				printf("set mcast listen %d\n", proc_ev->event_data.ack.err);
				break;
			case PROC_EVENT_FORK:
#ifdef ALL_MSGS
				printf("fork: parent tid=%d pid=%d -> child tid=%d pid=%d\n",
						proc_ev->event_data.fork.parent_pid,
						proc_ev->event_data.fork.parent_tgid,
						proc_ev->event_data.fork.child_pid,
						proc_ev->event_data.fork.child_tgid);
#endif
				break;
			case PROC_EVENT_EXEC:
#ifdef ALL_MSGS
				printf("exec: tid=%d pid=%d\n",
						proc_ev->event_data.exec.process_pid,
						proc_ev->event_data.exec.process_tgid);
				check_process(proc_ev->event_data.exec.process_pid);
#endif
				break;
			case PROC_EVENT_UID:
#ifdef ALL_MSGS
				printf("uid change: tid=%d pid=%d from %d to %d\n",
						proc_ev->event_data.id.process_pid,
						proc_ev->event_data.id.process_tgid,
						proc_ev->event_data.id.r.ruid,
						proc_ev->event_data.id.e.euid);
#endif
				break;
			case PROC_EVENT_GID:
#ifdef ALL_MSGS
				printf("gid change: tid=%d pid=%d from %d to %d\n",
						proc_ev->event_data.id.process_pid,
						proc_ev->event_data.id.process_tgid,
						proc_ev->event_data.id.r.rgid,
						proc_ev->event_data.id.e.egid);
#endif
				break;
			/* Since Linux v3.2 */
			case PROC_EVENT_COMM:
#ifdef ALL_MSGS
				printf("comm: tid=%d pid=%d comm %s\n",
						proc_ev->event_data.comm.process_pid,
						proc_ev->event_data.comm.process_tgid,
						proc_ev->event_data.comm.comm);
#endif
				break;
			case PROC_EVENT_EXIT:
				printf("exit: tid=%d pid=%d exit_code=%d\n",
						proc_ev->event_data.exit.process_pid,
						proc_ev->event_data.exit.process_tgid,
						proc_ev->event_data.exit.exit_code);
				if (proc_ev->event_data.exit.process_pid == proc_pid) {
					printf("%s (%d) exitted\n", proc_name, proc_pid);
					proc_pid = 0;
				}
				break;
			/* Since Linux v2.6.32 */
			case PROC_EVENT_SID:
#ifdef ALL_MSGS
				printf("sid change: tid=%d pid=%d",
						proc_ev->event_data.sid.process_pid,
						proc_ev->event_data.sid.process_tgid);
#endif
				break;
			/* Since Linux v3.1 */
			case PROC_EVENT_PTRACE:
#ifdef ALL_MSGS
				printf("ptrace change: tid=%d pid=%d tracer tid=%d, pid=%d",
						proc_ev->event_data.ptrace.process_pid,
						proc_ev->event_data.ptrace.process_tgid,
						proc_ev->event_data.ptrace.tracer_pid,
						proc_ev->event_data.ptrace.tracer_tgid);
#endif
				break;
			/* Since Linux v3.10 */
			case PROC_EVENT_COREDUMP:
#ifdef ALL_MSGS
				printf("coredump: tid=%d pid=%d",
						proc_ev->event_data.coredump.process_pid,
						proc_ev->event_data.coredump.process_tgid);
#endif
				break;
			default:
				printf("unhandled proc event %d\n", proc_ev->what);
				break;
			}
		}
	}

	return 0;
}

// Namespaces are in /proc/PID/ns/* and proc/PID/task/*/ns/*
//   If a namespace file is bind mounted somewhere, it will exist
//   without any process in it - see /run/netns for network namespaces.
//   Try readlink /proc/*/task/*/ns/* | sort -u
//   Same if st_ino in stat buf is same */

#ifdef UNUSED_CODE
static int scandir_filter(const struct dirent *dirent)
{
	if (dirent->d_type != DT_DIR)
		return false;

	if (dirent->d_name[0] <= '0' || dirent->d_name[0] > '9')
		return false;

	return true;
}

static int scandir_sort(const struct dirent **a, const struct dirent **b)
{
	return 0;
}

static pid_t
read_procs(const char *name)
{
	struct dirent **namelist;
	struct dirent **ent_p;
	struct dirent *ent;
	int ret;

	ret = scandir("/proc", &namelist, scandir_filter, scandir_sort);
	printf("scandir returned %d\n", ret);

	for (ent_p = namelist, ent = *namelist; ret--; ent = *++ent_p) {
		printf("0x%p: %s\n", ent, ent->d_name);
		free(ent);
	}

	free(namelist);
}
#endif

static pid_t
read_procs(const char *name)
{
	DIR *proc_dir = opendir("/proc");
	struct dirent *ent;
	char cmdline[] = "/proc/xxxxxxx/cmdline";
	int fd;
	char buf[strlen(name)+2];
	ssize_t len;
	pid_t pid = 0;

	while (ent = readdir(proc_dir)) {
		if (ent->d_type != DT_DIR)
			continue;
		if (ent->d_name[0] <= '0' || ent->d_name[0] >= '9')
			continue;
//		printf("%s\n", ent->d_name);

		sprintf(cmdline, "/proc/%s/cmdline", ent->d_name);

		if ((fd = open(cmdline, O_RDONLY)) == -1)
			continue;

		len = read(fd, buf, sizeof(buf) - 1);
		close(fd);
		buf[len] = '\0';
		if (!strcmp(buf, name)) {
			printf("%s: %s\n", name, ent->d_name);
			pid = atoi(ent->d_name);
			break;
		}
	}

	closedir(proc_dir);

	return pid;
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

	proc_pid = read_procs(argv[1] ?: "bash");
	proc_name = argv[1] ?: "bash";
	proc_name_len = strlen(proc_name);

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

