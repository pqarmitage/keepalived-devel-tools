#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>

#include "sigtest.h"

/* To ensure that signals SIGCANCEL and SIGSETXID are caught, link with at least one of -lssl, -lnl-3 or -lcrypto
 * although many other libraries will cause it too.
 */

#ifndef LOG_INFO
#define STANDALONE
#define log_message fprintf
#define LOG_INFO stdout
#define EOL	"\n"
#else
#define EOL
#endif

#ifndef STANDALONE
#define USE_SIGNAL_FD
#endif

typedef void (*__sighandler_act_t)(int, siginfo_t *, void *);

/* This is the sigaction structure from the Linux 3.2 kernel.  */
#define HAS_SA_RESTORER
struct kernel_sigaction
{
	union {
		__sighandler_t k_sa_handler;
		__sighandler_act_t k_sa_action;
	};
	unsigned long sa_flags;
#ifdef HAS_SA_RESTORER
	void (*sa_restorer) (void);
#endif
  /* glibc sigset is larger than kernel expected one, however sigaction
     passes the kernel expected size on rt_sigaction syscall.  */
  sigset_t sa_mask;
};

/* nptl has flag SA_SIGINFO for SIGCANCEL (32) and SA_SIGINFO | SA_RESTART for SIGSETXID (33).
 *
 * sigcancel_handler and sighandler_setxid use only si->si_pid and si->si_code. It does not use ctx.
 * si_code must be SI_TKILL. si->si_pid must be the process's own pid.
 */

static struct kernel_sigaction act_cancel, act_setxid;

static int
our_sigaction(int signum, const struct kernel_sigaction *act, struct kernel_sigaction *oact)
{
	long retl;

	retl = syscall(SYS_rt_sigaction, signum, act, oact, (SIGRTMAX + 1) / 8);
	if (retl) {
		log_message(LOG_INFO, "Signal %d sigaction syscall returned %ld - errno %d (%m)" EOL, signum, retl, errno);
		exit(1);
	}

	return (int)retl;
}

#ifdef USE_SIGNAL_FD
static int
our_sigprocmask(int how, const sigset_t *set, sigset_t *oldset)
{
	long retl;

	retl = syscall(SYS_rt_sigprocmask, how, set, oldset, (SIGRTMAX + 1) / 8);
	if (retl) {
		log_message(LOG_INFO, "sigprocmask syscall returned %ld - errno %d (%m)" EOL, retl, errno);
		exit(1);
	}

	return (int)retl;
}

int
our_sigaddset(sigset_t *set, int signum)
{
	if (signum == SIGCANCEL || signum == SIGSETXID) {
		set->__val[0] |= 1UL << (signum - 1);
		return 0;
	}

	return sigaddset(set, signum);
}

int
our_sigdelset(sigset_t *set, int signum)
{
	if (signum == SIGCANCEL || signum == SIGSETXID) {
		set->__val[0] &= ~(1UL << (signum - 1));
		return 0;
	}

	return sigdelset(set, signum);
}

#ifdef INCLUDE_UNUSED_CODE
void
block_nptl_signals(void)
{
	sigset_t set;

	sigemptyset(&set);
	our_sigaddset(&set, SIGCANCEL);
	our_sigaddset(&set, SIGSETXID);
log_message(LOG_INFO, "SIGCANCEL: %d, SIGSETXID: %d" EOL, sigismember(&set, SIGCANCEL), sigismember(&set,SIGSETXID));
	our_sigprocmask(SIG_BLOCK, &set, NULL);
}
#endif
#endif

static int
save_signal(int signum, struct kernel_sigaction *oact)
{
	int ret;

	ret = our_sigaction(signum, NULL, oact);
	if (ret)
		log_message(LOG_INFO, "save_signal %d error %d - %m" EOL, signum, errno);

	return ret;
}

static void
signal_chain(int signum, const struct kernel_sigaction *act, siginfo_t *info, void *p)
{
	if (!act->k_sa_handler) {
		log_message(LOG_INFO, "No chained function for signal %d" EOL, signum);
		return;
	}

	if (act->sa_flags & SA_SIGINFO)
		(act->k_sa_action)(signum, info, p);
	else
		(act->k_sa_handler)(signum);
}

static void
sig_handler(int signum, siginfo_t *info, void *p)
{
	char buf[1024];
	FILE *fp;
	size_t len;
	size_t i;

	log_message(LOG_INFO, "Signal %d received, code %d pid %d uid %u, %sfrom NPTL" EOL, info->si_signo, info->si_code, info->si_pid, info->si_uid, info->si_pid != getpid() || info->si_code != SI_TKILL ? "NOT " : "");

	sprintf(buf, "/proc/%d/cmdline", info->si_pid);
	fp = fopen(buf, "r");
	if (!fp)
		log_message(LOG_INFO, "Unable to open %s" EOL, buf);
	else {
		buf[sizeof(buf)-1] = '\0';
		len = fread(buf, 1, sizeof(buf), fp);
		if (!len)
			log_message(LOG_INFO, "fread eof %d, error %d" EOL, feof(fp), ferror(fp));
		else {
			for (i = 0; i < len - 1; i++)
				if (!buf[i] && buf[i+1])
					buf[i++] = ' ';
			log_message(LOG_INFO, "  program was %s" EOL, buf);
		}
		fclose(fp);
	}

	if (signum == SIGCANCEL)
		signal_chain(signum, &act_cancel, info, p);
	else if (signum == SIGSETXID)
		signal_chain(signum, &act_setxid, info, p);
}

#if !defined USE_SIGNAL_FD
static void
set_action(int signum, void (*new_action)(int, siginfo_t *, void *), struct kernel_sigaction *act)
{
	void (*sav_action)(int, siginfo_t *, void*);
	struct kernel_sigaction old_act;

	sav_action = act->k_sa_action;
	act->k_sa_action = new_action;

	our_sigaction(signum, act, &old_act);

	if (old_act.k_sa_action != sav_action)
		log_message(LOG_INFO, "old action %p does not match returned old action %p" EOL, sav_action, old_act.k_sa_handler);

	act->k_sa_action = sav_action;
}

#else

#include <sys/signalfd.h>

static void
nptl_signal(void *siginfo, int signum)
{
	struct signalfd_siginfo *ss = siginfo;
	siginfo_t si = { .si_pid = 0 };

	/* Copy the relevant info from the struct signalfd_siginto to the siginto_t */
	si.si_signo = ss->ssi_signo;
	si.si_errno = ss->ssi_errno;
	si.si_code = ss->ssi_code;
	if (ss->ssi_code == SI_USER ||
	    ss->ssi_code == SI_MESGQ ||
	    ss->ssi_code == SI_TKILL ||		/* sigaction(3) does not say what fields */
	    ss->ssi_code == SI_KERNEL ||	/* sigaction(3) does not say what fields */
	    ss->ssi_code == SI_QUEUE) {
		si.si_pid = ss->ssi_pid;
		si.si_uid = ss->ssi_uid;
		si.si_int = ss->ssi_int;
		si.si_ptr = (void *)ss->ssi_ptr;
	} else if (ss->ssi_code == SI_TIMER) {
		si.si_overrun = ss->ssi_overrun;
		si.si_timerid = ss->ssi_tid;
	}

	sig_handler(signum, &si, NULL);
}
#endif

#ifdef USE_SIGNAL_FD
void
restore_nptl_intercept(void)
{
	sigset_t set;
	sigemptyset(&set);

	if (act_cancel.k_sa_handler != SIG_IGN &&
	    act_cancel.k_sa_handler != SIG_DFL) {
		signal_set(SIGCANCEL, nptl_signal, V_SIGINFO);
		our_sigaddset(&set, SIGCANCEL);
	}

	if (act_setxid.k_sa_handler != SIG_IGN &&
	    act_setxid.k_sa_handler != SIG_DFL) {
		signal_set(SIGSETXID, nptl_signal, V_SIGINFO);
		our_sigaddset(&set, SIGSETXID);
	}

	our_sigprocmask(SIG_BLOCK, &set, NULL);

	struct kernel_sigaction ks = { .sa_flags = 0 };
	ks.k_sa_handler = SIG_IGN;
	our_sigaction(SIGCANCEL, &ks, NULL);
	our_sigaction(SIGSETXID, &ks, NULL);
}
#endif

static void
init_nptl_signal_intercept(void)
{
	save_signal(SIGCANCEL, &act_cancel);
	save_signal(SIGSETXID, &act_setxid);

#ifndef USE_SIGNAL_FD
	set_action(SIGCANCEL, sig_handler, &act_cancel);
	set_action(SIGSETXID, sig_handler, &act_setxid);
#else
	restore_nptl_intercept();
#endif
}

#ifdef STANDALONE
int main(int argc, char **argv)
{
	struct kernel_sigaction koact;
	struct sigaction oact;
	int ret;
	int signum = SIGCANCEL;

	if (argc >=2)
		signum = atoi(argv[1]);

	log_message(LOG_INFO, "SIGRTMIN=%d" EOL, SIGRTMIN);

	log_message(LOG_INFO, "PID is %d" EOL, getpid());

	init_nptl_signal_intercept();

	ret = sleep(20);

	log_message(LOG_INFO, "sleep returned %d" EOL, ret);
}
#endif
