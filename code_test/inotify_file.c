#include <sys/inotify.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

#if 0
           IN_ACCESS (+)
                  File was accessed (e.g., read(2), execve(2)).
           IN_ATTRIB (*)
                  Metadata changed—for example, permissions (e.g., chmod(2)), timestamps (e.g., utimensat(2)), extended attributes (setxattr(2)), link count (since Linux 2.6.25; e.g.,  for
                  the target of link(2) and for unlink(2)), and user/group ID (e.g., chown(2)).
           IN_CLOSE_WRITE (+)
                  File opened for writing was closed.
           IN_CLOSE_NOWRITE (*)
                  File or directory not opened for writing was closed.
           IN_CREATE (+)
                  File/directory created in watched directory (e.g., open(2) O_CREAT, mkdir(2), link(2), symlink(2), bind(2) on a UNIX domain socket).
           IN_DELETE (+)
                  File/directory deleted from watched directory.
           IN_DELETE_SELF
                  Watched  file/directory  was  itself  deleted.   (This  event  also occurs if an object is moved to another filesystem, since mv(1) in effect copies the file to the other
                  filesystem and then deletes it from the original filesystem.)  In addition, an IN_IGNORED event will subsequently be generated for the watch descriptor.
           IN_MODIFY (+)
                  File was modified (e.g., write(2), truncate(2)).
           IN_MOVE_SELF
                  Watched file/directory was itself moved.
           IN_MOVED_FROM (+)
                  Generated for the directory containing the old filename when a file is renamed.
           IN_MOVED_TO (+)
                  Generated for the directory containing the new filename when a file is renamed.
           IN_OPEN (*)
                  File or directory was opened.
#endif

char new_line[128];
char old_line[128];

static char *watch_file_name;

static void
read_file(void)
{
	FILE *fp = fopen(watch_file_name, "r");
	size_t len;

	if (fp) {
		if (fgets(new_line, sizeof(new_line), fp)) {
			if ((len = strlen(new_line)) && new_line[len - 1] == '\n')
				new_line[--len] = '\0';

			if (strcmp(old_line, new_line)) {
				printf("%s: %s\n", watch_file_name, new_line);
				strcpy(old_line, new_line);
			}
			else
				printf("Line not changed\n");
		} else {
			printf("fgets returned NULL\n");
			old_line[0] = '\0';
		}
	}

	fclose(fp);
}

static int
watch_file(int fd)
{
	int wd;

printf("add watch for %s\n", watch_file_name);
	if ((wd = inotify_add_watch(fd, watch_file_name,
//		IN_ACCESS |
//		IN_ATTRIB |
		IN_CLOSE_WRITE | /*
//		IN_CLOSE_NOWRITE |
//		IN_CREATE |
//		IN_DELETE |
//		IN_DELETE_SELF |
//		IN_MODIFY |
//		IN_MOVE_SELF |
//		IN_MOVED_FROM |
//		IN_MOVED_TO |
		IN_OPEN*/)) == -1) {
		printf("inotify_add_watch(%s) failed - errno %d - %m\n", watch_file_name, errno);
		return -1;
	}

	return wd;
}

int main(int argc, char **argv)
{
	int inotify_fd = inotify_init1(IN_CLOEXEC);
	char buf[256];
	char *buf_ptr;
	struct inotify_event* event;
	ssize_t len;
	int dir_wd, file_wd;

	watch_file_name = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
	strcpy(watch_file_name, argv[1]);
	strcat(watch_file_name, "/");
	strcat(watch_file_name, argv[2]);

// Need close_write, delete, MOVED_FROM/TO ? UNLINK ... If file created, need to watch it as well DELETE_SELF, MODIFY, MOVE_SELF
	// IN_ALL_EVENTS is all below wxcept IN_EXCL_UNLINK. IN_MOVED = IN_MOVED_FROM | IN_MOVED_TO. IN_CLOSE = IN_CLOSE_WRITE | IN_CLOSE_NOWRITE
	if ((dir_wd = inotify_add_watch(inotify_fd, argv[1],
//		IN_ACCESS |
//		IN_ATTRIB |
//		IN_CLOSE_WRITE |
//		IN_CLOSE_NOWRITE |
		IN_CREATE |
		IN_DELETE |
		IN_DELETE_SELF |
//		IN_MODIFY |
		IN_MOVE_SELF |
		IN_MOVED_FROM |
		IN_MOVED_TO /* |
		IN_OPEN*/)) == -1) {
		printf("inotify_add_watch(%s) failed - errno %d - %m\n", argv[1], errno);
		exit(1);
	}

	file_wd = watch_file(inotify_fd);
	if (file_wd != -1)
		read_file();

	printf("dir_wd = %d, file_wd = %d\n", dir_wd, file_wd);

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

			printf("File %s, wd %d, cookie %d,", event->len ? event->name : "[NONE]", event->wd, event->cookie);
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

			if (file_wd != -1 && event->wd == file_wd) {
			       	if (event->mask & (IN_DELETE_SELF | IN_MOVE_SELF)) {
					inotify_rm_watch(inotify_fd, file_wd);
					printf("Removed watch %d\n", file_wd);
					file_wd = -1;
					old_line[0] = '\0';
				}
				if (event->mask & IN_CLOSE_WRITE)
					read_file();
			}

			if (event->wd == dir_wd) {
				if (event->mask & (IN_DELETE_SELF | IN_MOVE_SELF)) {
					/* The directory has gone */
					break;
				} else if (event->len && !strcmp(event->name, argv[2])) {
					if (event->mask & (IN_CREATE | IN_MOVED_TO)) {
						file_wd = watch_file(inotify_fd);
						printf("file_wd = %d\n", file_wd);
						read_file();
					}
					else if (event->mask & (IN_DELETE | IN_MOVED_FROM)) {
						inotify_rm_watch(inotify_fd, file_wd);
						printf("Removed watch %d\n", file_wd);
						file_wd = -1;
						old_line[0] = '\0';
					}
				}
			}
		}
	}

	close(inotify_fd);
	free(watch_file_name);
}
