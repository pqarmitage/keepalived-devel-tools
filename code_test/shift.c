#include <stdio.h>
#include <limits.h>
#include <strings.h>

int main(__attribute__((unused)) int argc, __attribute__((unused)) char **argv)
{
	int val = INT_MAX;
	unsigned uval;

	val++;
	uval = val;
	printf("val: 0x%x, uval: 0x%x, ffs(val) = %d, ffs(uval) = %d\n", val, uval, ffs(val), ffs(uval));

	val >>=1;
	uval >>= 1;
	printf("val: 0x%x, uval: 0x%x, ffs(val) = %d, ffs(uval) = %d\n", val, uval, ffs(val), ffs(uval));
}
