#include <magic.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	magic_t magic = magic_open(MAGIC_PRESERVE_ATIME | MAGIC_ERROR | MAGIC_NO_CHECK_CDF | MAGIC_NO_CHECK_COMPRESS | MAGIC_NO_CHECK_ENCODING); //  | MAGIC_NO_CHECK_TEXT);
	const char *desc;

	magic_load(magic, NULL);

	desc = magic_file(magic, argv[1]);

	if (!desc)
		printf("magic error %s\n", magic_error(magic));
	else
		printf("%s\n", desc);
}
