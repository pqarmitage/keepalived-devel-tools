#define _GNU_SOURCE

#include <stdio.h>
#include <sched.h>
#include <sys/types.h>
#include <stdlib.h>
#include <errno.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <inttypes.h>

   struct sched_attr {
       uint32_t size;              /* Size of this structure */
       uint32_t sched_policy;      /* Policy (SCHED_*) */
       uint64_t sched_flags;       /* Flags */
       int32_t sched_nice;        /* Nice value (SCHED_OTHER,
				 SCHED_BATCH) */
       uint32_t sched_priority;    /* Static priority (SCHED_FIFO,
				 SCHED_RR) */
       /* Remaining fields are for SCHED_DEADLINE */
       uint64_t sched_runtime;
       uint64_t sched_deadline;
       uint64_t sched_period;
   };

int main(int argc, char **argv)
{
	pid_t pid = atoi(argv[1]);
	int sched;
	bool reset_on_fork;
	struct sched_param param;
	int rc;
	struct sched_attr attr;

	if ((sched = sched_getscheduler(pid)) < 0) {
		fprintf(stderr, "Failed to get scheduler for %d - errno %d(%m)\n", pid, errno);
		exit(1);
	}

	reset_on_fork = !!(sched & SCHED_RESET_ON_FORK);
	sched &= ~SCHED_RESET_ON_FORK;
	sched_getparam(pid, &param);
	printf("Scheduler = 0x%x (SCHED_%s)%s, priority %d\n", sched,
			sched == SCHED_OTHER ? "OTHER" :
			sched == SCHED_FIFO ? "FIFO" :
			sched == SCHED_RR ? "RR" :
			sched == SCHED_BATCH ? "BATCH" :
			sched == SCHED_IDLE ? "IDLE" :
			sched == SCHED_ISO ? "ISO" :
			sched == SCHED_DEADLINE ? "DEADLINE" :
			"unknown",
			reset_on_fork ? " reset on fork" : "",
			param.sched_priority);

	/* rc = sched_getattr(pid, &attr, sizeof(attr), 0); */
	rc = syscall(SYS_sched_getattr, pid, &attr, sizeof(attr), 0);
	if (rc) {
		fprintf(stderr, "sched_getattr failed - %d(%m)\n", errno);
		exit(1);
	}

	printf("policy %d, flags 0x%x, priority %u\n", attr.sched_policy, attr.sched_flags, attr.sched_priority);
}
