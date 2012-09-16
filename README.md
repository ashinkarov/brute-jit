brute-jit
=========

Compile a source code and link against it in runtime.

Sometimes it might be convenient to create a C function out of a 
certain specefincation and call it during the runtime.  Basically
to avoid interpreting the stuff all over again.

This is a very simple routine to create a library from the source-code,
link this library and execute the function.

Usage
-----

Currently the make file would produce a library called `libdynaic.a`,
whichc would contain everything you need.  Here is a sample usage example:
```C
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

	/* This is a text of the program we will compile.  */
  char prog[] = "int foo () { return 42; }";
	
  /* Get a pre-defined set-up of the GCC compiler.  */
  compiler = compiler_get_gcc ();

	/* Try to compile the program.  */
  if (comp_failed == compile_from_string (compiler, prog, libname))
		goto out;

  /* Load library.  */
	if (! (lib = load_library (libname)))
		goto out;

  /* Load function.  */
	fun = (int (*) ()) get_function (lib, funcname);
	if (!fun)
		goto out;

  /* Use the function loaded.  */
	printf ("function '%s' from '%s' returned '%d'\n", funcname, libname, fun ());

out:
	if (lib)
		if (close_library (lib))
			rc = EXIT_SUCCESS;
	return rc;
	
}
```

Comments
--------

The design of the library is very generic.  It doesn't really matter what language
is being used, unless the compiler can produce a shared library with visible symbosd.

Currently shared-library code is just a wrapper around dlfcn, however, it can be replaced
with a different mechanism, assuming that the interface is being preserved.

Suggestions and patches are always appreciated.
