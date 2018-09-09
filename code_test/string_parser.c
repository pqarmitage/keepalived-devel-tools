#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define MAXBUF 4096
#define WHITE_SPACE " \t"
#define COMMENT_START_CHRS	"#!"

void
alloc_strvec_quoted_escaped(char *src)
{
	char cur_quote = 0;
	char *ofs_op;
	char *op_buf;
	char *ofs, *ofs1;
	char op_char;

printf("alloc_strvec_quoted_escaped(\"%s\")\n", src);
	/* Create a vector and alloc each command piece */
	op_buf = malloc(MAXBUF);

	ofs = src;
	while (*ofs) {
		/* Find the next 'word' */
		ofs += strspn(ofs, WHITE_SPACE);
		if (!*ofs || strchr(COMMENT_START_CHRS, *ofs))
			break;

		ofs_op = op_buf;

		while (*ofs) {
			ofs1 = strpbrk(ofs, cur_quote == '"' ? "\"\\" : cur_quote == '\'' ? "'\\" : WHITE_SPACE COMMENT_START_CHRS "'\"\\");

			if (!ofs1) {
				size_t len;
				if (cur_quote) {
					printf("String '%s': missing terminating %c\n", src, cur_quote);
					goto err_exit;
				}
				strcpy(ofs_op, ofs);
				len =  strlen(ofs);
				ofs += len;
				ofs_op += len;
				break;
			}

			/* Save the wanted text */
			strncpy(ofs_op, ofs, ofs1 - ofs);
			ofs_op += ofs1 - ofs;
			ofs = ofs1;

			if (*ofs == '\\') {
				/* It is a '\' */
				ofs++;

				if (!*ofs) {
					printf("Missing escape char at end: '%s'\n", src);
					goto err_exit;
				}

				if (*ofs == 'x' && isxdigit(ofs[1])) {
					op_char = 0;
					ofs++;
					while (isxdigit(*ofs)) {
						op_char <<= 4;
						op_char |= isdigit(*ofs) ? *ofs - '0' : (10 + *ofs - (isupper(*ofs)  ? 'A' : 'a'));
						ofs++;
					}
				}
				else if (*ofs >= '0' && *ofs <= '7') {
					op_char = *ofs++ - '0';
					if (*ofs >= '0' && *ofs <= '7') {
						op_char <<= 3;
						op_char += *ofs++ - '0';
					}
					if (*ofs >= '0' && *ofs <= '7') {
						op_char <<= 3;
						op_char += *ofs++ - '0';
					}
				}
				else {
					switch (*ofs) {
					case 'a':
						op_char = '\a';
						break;
					case 'b':
						op_char = '\b';
						break;
					case 'f':
						op_char = '\f';
						break;
					case 'n':
						op_char = '\n';
						break;
					case 'r':
						op_char = '\r';
						break;
					case 't':
						op_char = '\t';
						break;
					case 'v':
						op_char = '\v';
						break;
					default: /* \"'  */
						op_char = *ofs;
						break;
					}
					ofs++;
				}

				*ofs_op++ = op_char;
				continue;
			}

			if (cur_quote) {
				/* It's the close quote */
				ofs++;
				cur_quote = 0;
				continue;
			}

			if (*ofs == '"' || *ofs == '\'') {
				cur_quote = *ofs++;
				continue;
			}

			break;
		}

		*ofs_op = '\0';
printf("Word is: %*s\n", ofs_op - op_buf, op_buf);
	}

	free(op_buf);

	return;

err_exit:
	free(op_buf);
}

int main(int argc, char **argv)
{
	alloc_strvec_quoted_escaped(argv[1]);
}
