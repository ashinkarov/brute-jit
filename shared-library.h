#ifndef __SHARED_LIBRARY_H__
#define __SHARED_LIBRARY_H__

/* A simple interface to the dynamic libray loader, which can be 
   adjusted depending on the operating system.  */

__BEGIN_DECLS
void * load_library (const char *);
void * get_function (void *, const char *);
bool close_library (void *);
__END_DECLS

#endif

