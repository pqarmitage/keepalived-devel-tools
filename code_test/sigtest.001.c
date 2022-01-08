#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <sys/syscall.h>


/* To ensure that signals 32 and 33 are caught, link with at least one of -lssl, -lnl-3 or -lcrypto
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

static struct kernel_sigaction act_32, act_33;

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

static int
save_signal(int signum, struct kernel_sigaction *oact)
{
	int ret;

#if 0
	ret = sigaction(signum, NULL, oact);
	if (ret)
		log_message(LOG_INFO, "sigaction returned %d - errno %d (%m)" EOL, ret, errno);
#endif

	ret = our_sigaction(signum, NULL, oact);
	if (ret)
		log_message(LOG_INFO, "save_signal %d error %d - %m" EOL, signum, errno);

	log_message(LOG_INFO, "Signal %d, flags 0x%lx, handler %p sa_restorer %p mask 0x%lx" EOL, signum, oact->sa_flags, oact->k_sa_handler, oact->sa_restorer, oact->sa_mask.__val[0]);

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

static void sig_handler(int signum, siginfo_t *info, void *p)
{
	char buf[1024];
	FILE *fp;
	size_t len;
	size_t i;

	log_message(LOG_INFO, "In sig_handler, sig %d" EOL, signum);

	log_message(LOG_INFO, "signal %d code %d pid %d uid %u" EOL, info->si_signo, info->si_code, info->si_pid, info->si_uid);

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
			log_message(LOG_INFO, "Command was %s" EOL, buf);
		}
		fclose(fp);
	}

	if (signum == 32)
		signal_chain(signum, &act_32, info, p);
	else if (signum == 33)
		signal_chain(signum, &act_33, info, p);

	log_message(LOG_INFO, "Ending sig_handler" EOL);
}

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

static void
init_nptl_signal_intercept(void)
{
	save_signal(32, &act_32);
	save_signal(33, &act_33);

	set_action(32, sig_handler, &act_32);
	set_action(33, sig_handler, &act_33);

}

#ifdef STANDALONE
int main(int argc, char **argv)
{
	struct kernel_sigaction koact;
	struct sigaction oact;
	int ret;
	int signum = 32;

	if (argc >=2)
		signum = atoi(argv[1]);

	log_message(LOG_INFO, "SIGRTMIN=%d" EOL, SIGRTMIN);

	log_message(LOG_INFO, "PID is %d" EOL, getpid());

	init_nptl_signal_intercept();

	ret = sleep(20);

	log_message(LOG_INFO, "sleep returned %d" EOL, ret);
}
#endif
