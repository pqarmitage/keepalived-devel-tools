#include <stdio.h>
#include <stdarg.h>

int(*joe)(char *, char *, va_list);

int fred(char *string, char *format, va_list ap)
{
	printf ("String is %s\n", string);

	return vprintf(format, ap);
}

int bert(char *string, char *format, ...)
{
	va_list ap;

	va_start(ap, format);
	return (*joe)(string, format, ap);
}

int main(int argc, char **argv)
{
	joe = fred;

	bert("I am here", "This is the string num %d, string %s\n", 5, "five");
}
