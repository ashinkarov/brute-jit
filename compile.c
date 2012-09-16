#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/stat.h>

#include <err.h>

#include "compile.h"

/* This routines create a dynamic library from a give C code.  
   We want to have a transparent interface for the compiler,
   in order to allow user setting her own compilers.  */


/* This is a set-up for the GCC.  */
static char* gcc_default_flags[] = {"-Wall", "-Wextra", "-pedantic", 
				    "-std=c99", "-xc", NULL};
static char* gcc_dynamic_flags[] = {"-fPIC", "-shared", NULL};
static char* gcc_cflags[] = {NULL};
static struct compiler gcc = {
	.binary = "gcc",
	.default_flags = gcc_default_flags,
	.cflags = gcc_cflags,
	.dynamic_flags = gcc_dynamic_flags,

	.output_option_name = "-o",
	.stdin_option = "-"
};


/* Acces function for GCC compiler.  */
struct compiler *
compiler_get_gcc ()
{
	return &gcc;
}


/* Build an array of strings suitable for execvp, which contains
   a call to the compiler with.  
   	FILES parameter serves adds source files in the compilation.
	OUTPUT parameter is the name passed to "-o" similar option.
	INPUT_OPTION triggers adding stdin as a compilation parameter.  */
static char **
create_compiler_args (struct compiler * compiler, char *files[], 
		      const char * output, bool input_option)
{
	#define list_length(__lst, __var) do {\
		__typeof (__lst) __t = __lst;   \
		for (; *__t; __t++, __var++)  \
		  	;		      \
	} while (0)

	size_t len = 0, ptr = 0;
	char ** lst = NULL;

	len = 1; /* Name of the compiler binary.  */
	list_length (compiler->default_flags, len);
	list_length (compiler->cflags, len);
	list_length (compiler->dynamic_flags, len);
	list_length (files, len);

	len += output != NULL;
	len += input_option;
	len += 1; /* NULL at the end  */

	lst = (char **) malloc (sizeof (char *) * len);
	
	#define copy_list(__src, __dst, __dst_ptr) do { \
		__typeof (__src) __t = __src; 		\
		for (; *__t; __t++)			\
		    __dst[__dst_ptr++] = strdup (*__t); \
	} while (0)

	lst[ptr++] = strdup (compiler->binary);
	copy_list (compiler->default_flags, lst, ptr);
	copy_list (compiler->cflags, lst, ptr);
	if (output) {
		char * out;
		if (-1 == asprintf (&out, "%s%s", 
				    compiler->output_option_name, 
				    output)) {
			warnx ("asprintf failed");
			goto error;
		}

		lst[ptr++] = out;
	}
	copy_list (compiler->dynamic_flags, lst, ptr);
	if (input_option)
		lst[ptr++] = strdup (compiler->stdin_option);

	lst[ptr] = NULL;
	return lst;
error:
	if (lst) {
		char ** lstp = lst;
		while (*lst)
			free (*lst), lst++;
		free (lstp);
	}
	return NULL;
}


/* Compile a program for string.
 	SOURCE is a string containg a program
	LIBRARY_NAME is a string specifying the location 
		     of the resultin binary.  */
enum compile_status
compile_from_string (struct compiler * compiler, char* source, 
		     const char* library_name)
{
	char*  buf = NULL;
	char*  bufp;
	pid_t  pid;
	int rpipe[2];
	int wpipe[2];
	int childret;

	/* We do not have any files to compile, just stdin. */
	char *files[] = {NULL};
	char **opts; 
	char **optsp;

	enum compile_status ret_status = comp_failed;
	
	struct stat st;
	size_t bufsz, cursz, curpos;
	ssize_t ssz;
	
	/* Create a list of args for later feeding to execvp.  */
	opts = create_compiler_args (compiler, files, 
				     library_name, true);
	if (!opts)
		return ret_status;
	
	/* Open a pipe to read from a child process.  */
	if (-1 == pipe (rpipe)) {
		warnx ("pipe");
		goto out;
	}

	/* Open a pipe for writing to a child process.  */
	if (-1 == pipe (wpipe)) {
		warnx ("pipe");
		goto out;
	}

	if (-1 == (pid = fork ())) {
		warnx ("fork");
		goto out;
	}

	/* CLose according ends of both 
	   pipes and dup std input/output.  */
	if (0 == pid) {
		(void) dup2 (wpipe[0], STDIN_FILENO);
		(void) close (wpipe[1]);

		(void) close (rpipe[0]);
		(void) dup2 (rpipe[1], STDOUT_FILENO);
		(void) dup2 (rpipe[1], STDERR_FILENO);

		(void) execvp (opts[0], opts);
		err (EXIT_FAILURE, "execvp");
	} 	
	
	/* Write the content of the program to through the WPIPE pipe.
	   XXX is it safe to do strlen on the text of the program?  */
	(void) close (wpipe[0]);
	if (-1 ==  write (wpipe[1], source, strlen (source))) {
		warnx ("write failed");
		goto out;
	}

	(void) close (wpipe[1]);

	/* Get the output of the compiler throug the RPIPE pipe.  */
	(void) close (rpipe[1]);
	
	if (fstat (rpipe[0], &st) >= 0)
		bufsz = (size_t) st.st_blksize;
	else {
		warnx ("fstat failed");
		goto out;
	}

	buf = (char *) malloc (bufsz);
	curpos = 0;
	cursz = bufsz;
	
	while ((ssz = read (rpipe[0], buf + curpos, bufsz)) > 0) {
		curpos += ssz;
		cursz = curpos + bufsz;
		if (NULL == (bufp = (char *) realloc (buf, cursz)))
			warnx ("no memory");
		buf = bufp;
	}
	buf[curpos] = 0;

	(void) waitpid (pid, &childret, 0);

	/* Convert status to the enum value.
	   XXX we may wan to make it a compiler-specific function.  */
	if (childret == 0) {
		/* TODO We may want to be smarter than that...  */
		if (strstr (buf, "warn") != NULL)
			ret_status = comp_warning;
		else
			ret_status = comp_success;
	} else {
		ret_status = comp_failed;
		(void) fprintf (stderr, "error compiling program "
				"from source, compiler returned:"
				"\n%s", buf);
	}
out:
	if (opts) {
		optsp = opts;
	
		while (*opts)
			free (*opts), opts++;
		free (optsp);
	}

	if (buf)
		free (buf);

	return ret_status;
}


#ifdef TEST_COMPILE
int main ()
{
	char prog[] = "int main () { return 42; }";
	compile_from_string (&gcc, prog, "libfoo.so");
	return EXIT_SUCCESS;
}
#endif

