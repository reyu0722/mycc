#ifdef __STDC__
#include <stdarg.h>
#include <stdio.h>
#else
typedef __builtin_va_list va_list;
#endif

void my_snprintf(char *buf, int len, char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	vsnprintf(buf, len, fmt, &(ap[0]));
}

int main()
{
	char *buf = calloc(1, 100);
	my_snprintf(buf, 100, "hello %s", "world");
	assert(0, strcmp(buf, "hello world"));

	my_snprintf(buf, 100, "%s %s %s", "foo", "bar", "baz");
	assert(0, strcmp(buf, "foo bar baz"));
	return 0;
}