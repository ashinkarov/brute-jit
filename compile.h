#ifndef __COMPILE_H__
#define __COMPILE_H__

/* Status of the compilation.  */
enum compile_status 
{
	comp_failed,
	comp_warning,
	comp_success
};


/* Set up the compiler you are going to use.  */
struct compiler
{
	char*  binary;
	char**  default_flags;
	char**  cflags;
	char**  dynamic_flags;

	const char *  output_option_name;
	const char *  stdin_option;
};

__BEGIN_DECLS
struct compiler * compiler_get_gcc ();
enum compile_status compile_from_string (struct compiler *, char *, const char *);
__END_DECLS

#endif /* __COMPILE_H__  */
