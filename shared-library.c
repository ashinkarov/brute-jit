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
