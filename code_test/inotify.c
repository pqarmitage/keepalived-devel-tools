#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int inotify_fd = inotify_init1(IN_CLOEXEC);
	char buf[256];
	char *buf_ptr;
	struct inotify_event* event;
	ssize_t len;
	int i;

	// IN_ALL_EVENTS is all below wxcept IN_EXCL_UNLINK. IN_MOVED = IN_MOVED_FROM | IN_MOVED_TO. IN_CLOSE = IN_CLOSE_WRITE | IN_CLOSE_NOWRITE
	for (i = 1; i < argc; i++) {
		if (inotify_add_watch(inotify_fd, argv[i],
			IN_ACCESS |
			IN_ATTRIB |
			IN_CLOSE_WRITE |
			IN_CLOSE_NOWRITE |
			IN_CREATE |
			IN_DELETE |
			IN_DELETE_SELF |
			IN_MODIFY |
			IN_MOVE_SELF |
			IN_MOVED_FROM |
			IN_MOVED_TO |
			IN_OPEN |
			IN_EXCL_UNLINK) == -1) {
			printf("inotify_add_watch(%s) failed - errno %d - %m\n", argv[i], errno);
			exit(1);
		}
	}

	while (1) {
		if ((len = read(inotify_fd, buf, sizeof(buf))) < sizeof(struct inotify_event)) {
			if (len == -1) {
				printf("read() returned error %d - %m\n", errno);
				continue;
			}
			printf("read returned short length %d\n", len);
			exit(1);
		}

		printf("read returned %d bytes\n", len);
		for (buf_ptr = buf; buf_ptr < buf + len; buf_ptr += event->len + sizeof(struct inotify_event)) {
			event = (struct inotify_event*)buf_ptr;

			printf("File %s, cookie %d,", event->name, event->cookie);
			if (event->mask & IN_ACCESS) printf(" IN_ACCESS");
			if (event->mask & IN_ATTRIB) printf(" IN_ATTRIB");
			if (event->mask & IN_CLOSE_WRITE) printf(" IN_CLOSE_WRITE");
			if (event->mask & IN_CLOSE_NOWRITE) printf(" IN_CLOSE_NOWRITE");
			if (event->mask & IN_CREATE) printf(" IN_CREATE");
			if (event->mask & IN_DELETE) printf(" IN_DELETE");
			if (event->mask & IN_DELETE_SELF) printf(" IN_DELETE_SELF");
			if (event->mask & IN_MODIFY) printf(" IN_MODIFY");
			if (event->mask & IN_MOVE_SELF) printf(" IN_MOVE_SELF");
			if (event->mask & IN_MOVED_FROM) printf(" IN_MOVED_FROM");
			if (event->mask & IN_MOVED_TO) printf(" IN_MOVED_TO");
			if (event->mask & IN_OPEN) printf(" IN_OPEN");
			if (event->mask & IN_IGNORED) printf(" IN_IGNORED");
			if (event->mask & IN_ISDIR) printf(" IN_ISDIR");
			if (event->mask & IN_Q_OVERFLOW) printf(" IN_Q_OVERFLOW");
			if (event->mask & IN_UNMOUNT) printf(" IN_UNMOUNT");
			printf("\n");
		}
	}
}
