/* Copyright (c) 2012 Artem Shinkarov <artyom.shinkaroff@gmail.com>

   Permission to use, copy, modify, and distribute this software for any
   purpose with or without fee is hereby granted, provided that the above
   copyright notice and this permission notice appear in all copies.
   THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
   WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
   MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
   ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
   WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
   ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
   OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE. */

#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <dlfcn.h>
#include <err.h>


void *
load_library (const char * name)
{
	void *  handle = dlopen (name, RTLD_NOW | RTLD_GLOBAL);
	if (!handle) {
		errx (EXIT_FAILURE, "dlopen failed: %s", dlerror ());
		return NULL;
	}
	return handle;
}

void *
get_function (void *  library, const char *  fname)
{
	void *  func;

	if (!library)
		return NULL;

	if (NULL == (func = dlsym (library, fname))) {
		warnx ("dlsym failed: %s", dlerror ());
		return NULL;
	}
	
	return func;
}

bool
close_library (void * library)
{
	assert (library);

	if (dlclose (library)) {
		warnx ("dlclose failed: %s", dlerror ());
		return false;
	}

	return true;
}
