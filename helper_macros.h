#include <string.h>
#include <errno.h>

#ifndef HELPER_MACROS
#define HELPER_MACROS

#ifdef DEBUG
	/* debug_n - print a number @num with text @text to stdout */
	#define debug_n(text, num)					       \
		printf("%s: %d\n", (text), (num))

	/* debug_a - print a address @addr with text @text to stdout */
	#define debug_a(text, addr)					       \
		printf("%s: %x\n", (text), (addr))

	/* debug_s - print a string @str with text @text to stdout */
	#define debug_s(text, str)					       \
		printf("%s: %s\n", (text), (str))

#else
	/* Empty declarations if no debugging support */
	#define debug_n(text, num)
	#define debug_a(text, addr)
	#define debug_s(text, str)

#endif /* DEBUG */

/* Makro for testing not equal zero */
#define test(var)							       \
	if ((var) != 0) {						       \
		debug_s("Not equal 0", #var);				       \
		debug_s("errno", strerror(errno));			       \
		return var;						       \
	}

#endif /* HELPER_MACROS */

