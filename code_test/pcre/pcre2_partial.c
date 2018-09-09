#define PCRE2_CODE_UNIT_WIDTH 8

#include <sys/time.h>
#include <pcre2.h>               /* PCRE lib        NONE  */
#include <stdio.h>              /* I/O lib         C89   */
#include <stdlib.h>             /* Standard Lib    C89   */
#include <string.h>             /* Strings         C89   */
#include <stdbool.h>

char *page =
"    rubbish rubbish rubbish"
"	BUILD: rpm-maintenance_147_147 :: DATE: 06/12/2018 18:29:57 :: HOST: sflgnvrpm06.thig.com :: SESSION: A4303ED70370A73B84C6188436AB671B\n"
"</html>\n";

char *pattern="BUILD: rpm-.{3,50} :: DATE: ..\\/..\\/.... .{12}";

int main(int argc, char *argv[]) {
	pcre2_code *reCompiled;
	int pcreExecRet;
	int subStrVec[30];
	const char *pcreErrorStr;
	int pcreErrorNumber;
	PCRE2_SIZE pcreErrorOffset;
	char *aStrRegex;
	const char *psubStrMatchStr;
	int j;
	unsigned loop_count = 100000;
	unsigned iter;
	pcre2_match_data *match_data;
	PCRE2_SIZE *ovector;
	int pcreRet;
#ifdef USE_JIT
	pcre2_match_context *mcontext;
	pcre2_jit_stack *jit_stack;
#endif
	uint32_t max_lookbehind;
	aStrRegex = pattern;
	printf("Regex to use: %s\n", aStrRegex);
	char *start;
	PCRE2_SIZE len;

	// First, the regex string must be compiled.
	reCompiled = pcre2_compile(aStrRegex, PCRE2_ZERO_TERMINATED, 0, &pcreErrorNumber, &pcreErrorOffset, NULL);

	// pcre_compile returns NULL on error, and sets pcreErrorOffset & pcreErrorStr
	if(reCompiled == NULL) {
		PCRE2_UCHAR buffer[256];
		pcre2_get_error_message(pcreErrorNumber, buffer, sizeof buffer);
		printf("ERROR: Could not compile '%s': %s\n", aStrRegex, buffer);
		exit(1);
	} /* end if */

#ifdef USE_JIT
	pcreRet = pcre2_jit_compile(reCompiled, PCRE2_JIT_PARTIAL_HARD /* | PCRE2_JIT_COMPLETE */);
	mcontext = pcre2_match_context_create(NULL);
	jit_stack = pcre2_jit_stack_create(32 * 1024, 512 * 1024, NULL);
	pcre2_jit_stack_assign(mcontext, NULL, jit_stack);
#endif
	match_data = pcre2_match_data_create_from_pattern(reCompiled, NULL);
	pcre2_pattern_info(reCompiled, PCRE2_INFO_MAXLOOKBEHIND, &max_lookbehind);
	printf("Max lookbehind is %u\n", max_lookbehind);

	struct timeval start_time, end_time;
	gettimeofday(&start_time, NULL);

	for (iter = 0; iter < loop_count; iter++) {
		start = page;
		len = 50;
		while (true) {
			/* Try to find the regex in aLineToMatch, and report results. */
			if (iter == loop_count - 1)
				printf("Searching from %d with length %d\n", start - page, len);
#ifdef USE_JIT
			pcreExecRet = pcre2_jit_match
#else
			pcreExecRet = pcre2_match
#endif
							(reCompiled,
							 start, 
							 len,  // length of string
							 0,                      // Start looking at this point
							 PCRE2_PARTIAL_HARD,     // OPTIONS
							 match_data,
#ifdef USE_JIT
							 mcontext
#else
							 NULL
#endif
							 );                    // context

			if (pcreExecRet == PCRE2_ERROR_PARTIAL) {
				ovector = pcre2_get_ovector_pointer(match_data);
				if (iter == loop_count - 1)
					printf("Partial returned, ovector %d, max_lookbehind %d\n", ovector[0], max_lookbehind);
				if (ovector[0] - max_lookbehind <= 0)
					len += 50;
				else
					len =  50;
				start += ovector[0] - max_lookbehind;
				continue;
			}
				
			// Report what happened in the pcre_exec call..
			if(pcreExecRet < 0) { // Something bad happened..
				switch(pcreExecRet)
				{
				case PCRE2_ERROR_NOMATCH      : printf("String did not match the pattern\n");        break;
				case PCRE2_ERROR_NULL         : printf("Something was null\n");                      break;
				case PCRE2_ERROR_BADOPTION    : printf("A bad option was passed\n");                 break;
				case PCRE2_ERROR_BADMAGIC     : printf("Magic number bad (compiled re corrupt?)\n"); break;
//				case PCRE2_ERROR_UNKNOWN_NODE : printf("Something kooky in the compiled re\n");      break;
				case PCRE2_ERROR_NOMEMORY     : printf("Ran out of memory\n");                       break;
				default                      : printf("Unknown error\n");                           break;
				} /* end switch */

				exit(1);
			}
			break;
		}

		if (iter == loop_count - 1)
		{
			printf("Result: We have a match!\n");

			ovector = pcre2_get_ovector_pointer(match_data);
			printf("Match succeeded at offset %d\n", ovector[0]);

			// At this point, rc contains the number of substring matches found...
			if(pcreExecRet == 0) {
				printf("But too many substrings were found to fit in subStrVec!\n");
				// Set rc to the max number of substring matches possible.
			} /* end if */

			// PCRE contains a handy function to get the matches
			for(j=0; j<pcreExecRet; j++) {
				PCRE2_SPTR substring_start = start + ovector[2*j];
				size_t substring_length = ovector[2 * j + 1] - ovector[2*j];
				printf("%2d: %.*s\n", j, substring_length, substring_start);
			} /* end for */

			printf("\n");
		}
	}

	gettimeofday(&end_time, NULL);
	unsigned durn = (end_time.tv_sec - start_time.tv_sec) * 1000000 + end_time.tv_usec - start_time.tv_usec;
	printf("pcre2 took %u usecs for %u loops\n", durn, loop_count);

	// Free up the regular expression.
	pcre2_code_free(reCompiled);
	pcre2_match_data_free(match_data);
#ifdef USE_JIT
	pcre2_match_context_free(mcontext);
	pcre2_jit_stack_free(jit_stack);
#endif
		  
	// We are all done..
	return 0;

} /* end func main */
