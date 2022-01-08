/* Define the signal names glibc uses for the NPTL signals */
#define SIGCANCEL	32
#define SIGSETXID	33

extern int our_sigaddset(sigset_t *, int);
extern int our_sigdelset(sigset_t *, int);
extern void restore_nptl_intercept(void);
