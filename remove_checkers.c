#include <stdio.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char **argv)
{
	char buf[256];
	bool in_checker = false;
	unsigned depth = 0;

	while (fgets(buf, sizeof buf, stdin)) {
		if (!in_checker &&
		    (strstr(buf, "TCP_CHECK") || strstr(buf, "HTTP_GET")))
			in_checker = true;
		if (in_checker) {
			if (strchr(buf, '{'))
				depth++;
			if (strchr(buf, '}')) {
				if (!--depth)
					in_checker = false;
			}
		} 
		else
			printf("%s", buf);
	}
}
