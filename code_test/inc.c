#include "config.h"

#include "../keepalived/include/snmp.h"

extern int printf(const char *, ...);

#ifdef TEST_MEMORY_H
void *zalloc(unsigned long fred)
{
	return NULL;
}
#endif

int main(int argc, char **argv)
{
	printf("I am here\n");
#ifdef TEST_MEMORY_H
	char *fred = MALLOC(5);
	FREE(fred);
	fred = REALLOC(fred,5);

	FREE_PTR(fred);
#endif
}
