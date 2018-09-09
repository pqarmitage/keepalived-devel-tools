#define _GNU_SOURCE
#include <sys/types.h>
#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Example (all need REG_EXTENDED):
 *   regex:   "Date: ..\/..\/.... .{12} Build: rpm-.{3,50}"
 *   regex:   "^Date: ..\/..\/.... .{12} Build: rpm-.{3,50}$"
 *   regex:   "^Date: [0-9]{2}\/[0-9]{2}\/[0-9]{4} [0-9:\.]{12} Build: rpm-.{3,50}$"
 *   regex:   "^Date: [[:digit:]]{2}\/[[:digit:]]{2}\/[[:digit:]]{4} [[:digit:]\.:]{12} Build: rpm-.{3,50}$"
 *   matches: "Date: 06/15/2018 09:11:17.303 Build: rpm-dev-claimsdocs"
 */

/*

DESCRIPTION
	cflags may be the bitwise-or of zero or more of the following:

	REG_EXTENDED = 1
		Use POSIX Extended Regular Expression syntax when interpreting regex. If not set, POSIX Basic Regular Expression syntax is used.

	REG_ICASE = 2
		Do not differentiate case. Subsequent regexec() searches using this pattern buffer will be case insensitive.

	REG_NOSUB = 8
		Do not report position of matches. The nmatch and pmatch arguments to regexec() are ignored if the pattern buffer supplied was compiled with this
		flag set.

	REG_NEWLINE = 4
		If set, anchors do not match newline characters in the string.

GNU regex compiling
    call re_set_syntax(mask) with flags below:

If this bit is not set, then \ inside a bracket expression is literal.
If set, then such a \ quotes the following character.
	RE_BACKSLASH_ESCAPE_IN_LISTS  = 1

If this bit is not set, then + and ? are operators, and \+ and \? are literals.
If set, then \+ and \? are operators and + and ? are literals.
	RE_BK_PLUS_QM = 2

If this bit is set, then character classes are supported.  They are:
  [:alpha:], [:upper:], [:lower:],  [:digit:], [:alnum:], [:xdigit:],
  [:space:], [:print:], [:punct:], [:graph:], and [:cntrl:].
If not set, then character classes are not supported.  
	RE_CHAR_CLASSES = 4

If this bit is set, then ^ and $ are always anchors (outside bracket
  expressions, of course).
If this bit is not set, then it depends:
     ^  is an anchor if it is at the beginning of a regular
        expression or after an open-group or an alternation operator;
     $  is an anchor if it is at the end of a regular expression, or
        before a close-group or an alternation operator.

This bit could be (re)combined with RE_CONTEXT_INDEP_OPS, because
POSIX draft 11.2 says that * etc. in leading positions is undefined.
We already implemented a previous draft which made those constructs
invalid, though, so we haven't changed the code back.  
	RE_CONTEXT_INDEP_ANCHORS = 8

If this bit is set, then special characters are always special
  regardless of where they are in the pattern.
If this bit is not set, then special characters are special only in
  some contexts; otherwise they are ordinary.  Specifically,
  * + ? and intervals are only special when not after the beginning,
  open-group, or alternation operator.  
	RE_CONTEXT_INDEP_OPS = 16

If this bit is set, then *, +, ?, and { cannot be first in an re or
  immediately after an alternation or begin-group operator.  
	RE_CONTEXT_INVALID_OPS = 32

If this bit is set, then . matches newline.
If not set, then it doesn't.  
	RE_DOT_NEWLINE = 64
If this bit is set, then . doesn't match NUL.
If not set, then it does.  
	RE_DOT_NOT_NULL = 128

If this bit is set, nonmatching lists [^...] do not match newline.
If not set, they do.  
	RE_HAT_LISTS_NOT_NEWLINE = 256

If this bit is set, either \{...\} or {...} defines an
  interval, depending on RE_NO_BK_BRACES.
If not set, \{, \}, {, and } are literals.  
	RE_INTERVALS = 512

If this bit is set, +, ? and | aren't recognized as operators.
If not set, they are.  
	RE_LIMITED_OPS = 1024

If this bit is set, newline is an alternation operator.
If not set, newline is literal.  
	RE_NEWLINE_ALT = 2048

If this bit is set, then `{...}' defines an interval, and \{ and \}
  are literals.
  If not set, then `\{...\}' defines an interval.  
	RE_NO_BK_BRACES = 4096

If this bit is set, (...) defines a group, and \( and \) are literals.
If not set, \(...\) defines a group, and ( and ) are literals.  
	RE_NO_BK_PARENS  = 8192

If this bit is set, then \<digit> matches <digit>.
If not set, then \<digit> is a back-reference.  
	RE_NO_BK_REFS 16384

If this bit is set, then | is an alternation operator, and \| is literal.
If not set, then \| is an alternation operator, and | is literal.  
	RE_NO_BK_VBAR = 32768

If this bit is set, then an ending range point collating higher
  than the starting range point, as in [z-a], is invalid.
If not set, then when ending range point collates higher than the
  starting range point, the range is ignored.  
	RE_NO_EMPTY_RANGES = 65536

If this bit is set, then an unmatched ) is ordinary.
If not set, then an unmatched ) is invalid.  
	RE_UNMATCHED_RIGHT_PAREN_ORD  = 2^17

If this bit is set, succeed as soon as we match the whole pattern,
without further backtracking.  
	RE_NO_POSIX_BACKTRACKING = 2^18

If this bit is set, do not process the GNU regex operators.
If not set, then the GNU regex operators are recognized. 
	RE_NO_GNU_OPS = 2^19

If this bit is set, turn on internal regex debugging.
If not set, and debugging was on, turn it off.
This only works if regex.c is compiled -DDEBUG.
We define this bit always, so that all that's needed to turn on
debugging is to recompile regex.c; the calling code can always have
this bit set, and it won't affect anything in the normal case. 
	RE_DEBUG = 2^20

If this bit is set, a syntactically invalid interval is treated as
a string of ordinary characters.  For example, the ERE 'a{1' is
treated as 'a\{1'.  
	RE_INVALID_INTERVAL_ORD =2^21

If this bit is set, then ignore case when matching.
If not set, then case is significant.  
	RE_ICASE = 2^22

This bit is used internally like RE_CONTEXT_INDEP_ANCHORS but only
for ^, because it is difficult to scan the regex backwards to find
whether ^ should be special.  
	RE_CARET_ANCHORS_HERE = 2^23

If this bit is set, then \{ cannot be first in an bre or
immediately after an alternation or begin-group operator.  
	RE_CONTEXT_INVALID_DUP = 2^24

If this bit is set, then no_sub will be set to 1 during
re_compile_pattern.  
	RE_NO_SUB = 2^25

// Standard syntaxes
RE_SYNTAX_EMACS
RE_SYNTAX_AWK
RE_SYNTAX_GNU_AWK
RE_SYNTAX_POSIX_AWK
RE_SYNTAX_GREP
RE_SYNTAX_EGREP
RE_SYNTAX_POSIX_EGREP
RE_SYNTAX_ED
RE_SYNTAX_SED
_RE_SYNTAX_POSIX_COMMON
RE_SYNTAX_POSIX_BASIC
RE_SYNTAX_POSIX_MINIMAL_BASIC
RE_SYNTAX_POSIX_EXTENDED
RE_SYNTAX_POSIX_MINIMAL_EXTENDED

regexec flags:
       REG_NOTBOL = 1
       REG_NOTEOL = 2
       REG_STARTEND = 4
*/



int main(int argc, char **argv)
{
	int cflags = 0;
	int eflags = 0;
	regex_t preg;
	int ret;
	char buf[2048] = "";
	size_t buf_size = 0;
	regmatch_t matches[128];
	reg_syntax_t syntax_options = 0;
	int i;

	if (argc >= 3)
		cflags = strtoul(argv[2], NULL, 0);
	if (argc >= 4)
		eflags = strtoul(argv[3], NULL, 0);
	if (argc >= 5) {
		syntax_options = strtoul(argv[4], NULL, 0);
		re_syntax_options = syntax_options;
	}
	if (argc >= 3)
		printf("cflags 0x%x, eflags 0x%x, syntax_options 0x%x\n", cflags, eflags, syntax_options);

	if ((ret = regcomp(&preg, argv[1], cflags))) {
		regerror(ret, &preg, buf, sizeof(buf));
		fprintf(stderr, "regcomp() error %s\n", buf);
		exit(1);
	}
	printf("Pattern compiled with options 0x%x\n", preg.syntax);

	while (fgets(buf + buf_size, sizeof buf - buf_size, stdin))
		buf_size += strlen(buf + buf_size);

	printf("Read: %s\n", buf);

	ret = regexec(&preg, buf, sizeof(matches)/sizeof(matches[0]), matches, eflags);
	if (ret == REG_NOMATCH) {
		printf("Match failed\n");
		exit(0);
	}

	if (ret) {
		regerror(ret, &preg, buf, sizeof(buf));
		fprintf(stderr, "regexec() error %s\n", buf);
		exit(2);
	}

	for (i = 0; i < sizeof(matches) / sizeof(matches[0]); i++)
		if (matches[i].rm_so != -1)
			printf("%3d: so = %d, eo = %d\n", i, matches[i].rm_so, matches[i].rm_eo);

	regfree(&preg);
}
