#include <errno.h>
#include <malloc.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <termio.h>
#include <stdbool.h>


static int ctl_c;    /* was the conversation interrupted? */

static void
interrupt(int x)
{
	ctl_c = 1;
}

/* getinput -- read user input from stdin abort on ^C
 *    Entry    noecho == TRUE, don't echo input.
 *    Exit    User's input.
 *        If interrupted, send SIGINT to caller for processing.
 */
static char *
getinput(int noecho)
{
	struct termio tty;
	unsigned short tty_flags;
	char input[4096];
	int c;
	int i = 0;
	void (*sig)(int);

	ctl_c = 0;
	sig = signal(SIGINT, interrupt);
	if (noecho) {
		ioctl(fileno(stdin), TCGETA, &tty);
		tty_flags = tty.c_lflag;
		tty.c_lflag &= ~(ECHO | ECHOE | ECHOK | ECHONL);
		ioctl(fileno(stdin), TCSETAF, &tty);
	}

	/* go to end, but don't overflow PAM_MAX_RESP_SIZE */
	flockfile(stdin);
	while (ctl_c == 0 &&
		(c = getchar_unlocked()) != '\n' &&
		c != '\r' &&
		c != EOF) {
		if (i < sizeof(input)) {
			input[i++] = (char)c;
		}
	}
	funlockfile(stdin);
	input[i] = '\0';
	if (noecho) {
		tty.c_lflag = tty_flags;
		(void) ioctl(fileno(stdin), TCSETAW, &tty);
		(void) fputc('\n', stdout);
	}
	(void) signal(SIGINT, sig);
	if (ctl_c == 1)
		(void) kill(getpid(), SIGINT);

	return (strdup(input));
}

int
main(int argc, char **argv)
{
	printf("Entered: %s\n", getinput(true));
}
