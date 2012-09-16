#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include "compile.h"
#include "shared-library.h"


static const char * const  libname = "/tmp/libfoo.so";
static const char * const  funcname = "foo";

int main (int argc, char *argv[])
{
	void * lib = NULL;
	int (*fun) ();
	struct compiler * compiler;
	int rc = EXIT_FAILURE;

	char prog[] = "int foo () { return 42; }";
	compiler = compiler_get_gcc ();

	if (comp_failed == compile_from_string (compiler, prog, libname))
		goto out;

	if (! (lib = load_library (libname)))
		goto out;

	fun = (int (*) ()) get_function (lib, funcname);
	if (!fun)
		goto out;

	printf ("function '%s' from '%s' returned '%d'\n", funcname, libname, fun ());

out:
	if (lib)
		if (close_library (lib))
			rc = EXIT_SUCCESS;
	return rc;
	
}
